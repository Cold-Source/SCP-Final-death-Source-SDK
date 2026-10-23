"""Local, dependency-free conversion of ordinary Source surface materials to PBR.

Existing color/normal images are retained. Missing physical information is filled
with conservative shared MRAO presets, not inferred as measured data from RGB.
Special shaders and unsupported features are reported instead of discarded.
"""
from __future__ import annotations
import argparse
import copy
from dataclasses import dataclass, field
import hashlib
import json
from pathlib import Path
import re
import struct
import time


@dataclass
class KV:
    key: str
    value: str | list[KV]
    conditions: list[str] = field(default_factory=list)


TOKEN = re.compile(r'/\*[\s\S]*?\*/|//[^\r\n]*|"(?:\\.|[^"\\])*"|[{}]|[^\s{}"]+')


def parse(text):
    tokens = [m[0] for m in TOKEN.finditer(text) if not m[0].startswith(('//', '/*'))]
    index = 0

    def value(t):
        return t[1:-1] if t.startswith('"') else t

    def entries():
        nonlocal index
        result = []
        while index < len(tokens) and tokens[index] != '}':
            key = value(tokens[index]); index += 1
            if index >= len(tokens): raise ValueError('missing value')
            if tokens[index] == '{':
                index += 1; data = entries()
                if index >= len(tokens) or tokens[index] != '}': raise ValueError('unbalanced block')
                index += 1
            elif tokens[index] == '}':
                raise ValueError('missing value before closing brace')
            else:
                data = value(tokens[index]); index += 1
            conditions = []
            while index < len(tokens) and tokens[index].startswith('['):
                conditions.append(tokens[index]); index += 1
            result.append(KV(key, data, conditions))
        return result

    document = entries()
    if index != len(tokens) or len(document) != 1 or not isinstance(document[0].value, list):
        raise ValueError('unsupported VMT document structure')
    return document[0]


def prop(items, key, default=''):
    matches = [v.value for v in items if v.key.lower() == key.lower() and isinstance(v.value, str) and not v.conditions]
    return matches[-1] if matches else default


def truth(value):
    try: return float(value) != 0
    except ValueError: return bool(value)


def normalized(name):
    name = name.replace('\\', '/').strip().lower()
    if name.startswith('materials/'): name = name[len('materials/'):]
    if name.endswith(('.vmt', '.vtf')): name = name[:-4]
    if '..' in name.split('/') or ':' in name or name.startswith('/'):
        raise ValueError('unsafe material path')
    return name


def patch_merge(target, changes, replace):
    for entry in changes:
        matches = [i for i, old in enumerate(target) if old.key.lower() == entry.key.lower() and old.conditions == entry.conditions]
        if matches:
            old = target[matches[-1]]
            if isinstance(old.value, list) and isinstance(entry.value, list):
                patch_merge(old.value, entry.value, replace)
            elif replace: target[matches[-1]] = copy.deepcopy(entry)
        elif not replace: target.append(copy.deepcopy(entry))


def resolve(name, documents, stack=()):
    if name in stack or len(stack) > 16: raise ValueError('recursive Patch include')
    document = copy.deepcopy(documents[name])
    if document.key.lower() != 'patch': return document
    include = normalized(prop(document.value, 'include'))
    if not include or include not in documents: raise ValueError('Patch include is outside this folder')
    base = resolve(include, documents, (*stack, name))
    for entry in document.value:
        if entry.key.lower() in ('insert', 'replace') and isinstance(entry.value, list):
            patch_merge(base.value, entry.value, entry.key.lower() == 'replace')
        elif entry.key.lower() != 'include': raise ValueError('unsupported Patch directive')
    return base


def quoted(s):
    return '"' + s + '"'


def serialize(node, indent=0):
    prefix = '\t' * indent
    condition = (' ' + ' '.join(node.conditions)) if node.conditions else ''
    if isinstance(node.value, str): return prefix + quoted(node.key) + ' ' + quoted(node.value) + condition + '\n'
    return prefix + quoted(node.key) + '\n' + prefix + '{\n' + ''.join(serialize(v, indent + 1) for v in node.value) + prefix + '}' + condition + '\n'


def set_prop(items, key, value):
    items[:] = [v for v in items if v.key.lower() != key.lower()]
    items.append(KV(key, value))


PRESETS = {
    'default': (0, 192, 255), 'concrete': (0, 230, 255),
    'brick': (0, 225, 255), 'stone': (0, 210, 255), 'wood': (0, 200, 255),
    'paint': (0, 170, 255), 'plastic': (0, 150, 255), 'rubber': (0, 235, 255),
    'organic': (0, 210, 255), 'cloth': (0, 240, 255),
    'ceramic': (0, 110, 255), 'glass': (0, 32, 255), 'bare_metal': (255, 105, 255),
}


def preset(items, name):
    surface = prop(items, '$surfaceprop').lower()
    label = surface + ' ' + name.lower()
    for kind, pattern in [
        ('glass', r'glass|window'),
        ('rubber', r'rubber|tire'), ('cloth', r'cloth|fabric|carpet'),
        ('wood', r'wood|timber|plywood'), ('brick', r'brick'),
        ('concrete', r'concrete|plaster|stucco|cement'),
        ('organic', r'flesh|skin|dirt|grass|sand|gravel|mud'),
        ('ceramic', r'ceramic|porcelain|tile'), ('stone', r'rock|stone|marble'),
        ('plastic', r'plastic'), ('paint', r'paint|door|panel|rust'),
        ('bare_metal', r'chrome|brass|copper|alumin|steel|iron|metal'),
    ]:
        if re.search(pattern, label): return kind
    return 'default'


def constant_vtf(rgb, normal=False):
    # VTF 7.2, RGB888, full chain. Even small shared maps must support picmip.
    h = bytearray(80)
    struct.pack_into('<4sIIIHHIHH', h, 0, b'VTF\0', 7, 2, 80, 16, 16, 128 if normal else 0, 1, 0)
    struct.pack_into('<3f', h, 32, *(v / 255 for v in rgb))
    struct.pack_into('<fI', h, 48, 1.0, 2)
    h[56] = 5
    struct.pack_into('<I', h, 57, 0xffffffff)
    struct.pack_into('<H', h, 63, 1)
    return bytes(h) + b''.join(bytes(rgb) * (max(1, 16 >> mip) ** 2) for mip in reversed(range(5)))


def atomic_write(path, content):
    path.parent.mkdir(parents=True, exist_ok=True)
    temporary = path.with_name(path.name + '.pbr-writing')
    temporary.write_bytes(content)
    temporary.replace(path)


def plan_conversion(root):
    materials = root / 'materials'
    if not materials.is_dir(): raise ValueError('Root must contain a materials directory')
    documents, files, errors = {}, {}, {}
    for path in sorted(materials.rglob('*.vmt')):
        name = normalized(path.relative_to(materials).as_posix())
        raw = path.read_bytes()
        try:
            try: text = raw.decode('utf-8-sig'); encoding = 'utf-8'
            except UnicodeDecodeError: text = raw.decode('cp1251'); encoding = 'cp1251'
            documents[name] = parse(text)
            files[name] = (path, encoding, hashlib.sha256(raw).hexdigest())
        except (ValueError, UnicodeError) as e: errors[name] = str(e)
    actions, skipped = [], dict(errors)
    for name in documents:
        try:
            material = resolve(name, documents)
            shader = material.key.lower(); values = material.value
            if shader == 'pbr': skipped[name] = 'already PBR'; continue
            if shader not in ('lightmappedgeneric', 'vertexlitgeneric'):
                skipped[name] = 'special or unsupported shader: ' + material.key; continue
            special = next((key for key in ('$basetexture2', '$detail', '$blendmodulatetexture', '$lightwarptexture') if prop(values, key)), None)
            if special: skipped[name] = 'requires separate implementation: ' + special; continue
            if truth(prop(values, '$ssbump', '0')):
                skipped[name] = 'SSBump requires conversion to tangent-space normals'; continue
            if truth(prop(values, '$selfillum', '0')) or truth(prop(values, '$additive', '0')):
                skipped[name] = 'emissive/additive material requires separate treatment'; continue
            if any(v.conditions and v.key.lower() in ('$basetexture', '$bumpmap', '$normaltexture') for v in values):
                skipped[name] = 'conditional texture variants'; continue
            if any(isinstance(v.value, list) and v.key.lower() != 'proxies' for v in values):
                skipped[name] = 'shader fallback/override block'; continue
            base = normalized(prop(values, '$basetexture'))
            if not base or base.startswith('_rt') or not (materials / (base + '.vtf')).is_file():
                skipped[name] = 'base texture not supplied in this folder'; continue
            base_header = (materials / (base + '.vtf')).read_bytes()[:80]
            if len(base_header) < 80 or base_header[:4] != b'VTF\0':
                skipped[name] = 'invalid base VTF'; continue
            if struct.unpack_from('<H', base_header, 24)[0] != 1:
                skipped[name] = 'animated texture'; continue
            normal = prop(values, '$bumpmap') or prop(values, '$normaltexture')
            if normal:
                normal_path = materials / (normalized(normal) + '.vtf')
                normal_header = normal_path.read_bytes()[:80] if normal_path.is_file() else b''
                if 'ssbump' in normal.lower() or (len(normal_header) >= 24 and struct.unpack_from('<I', normal_header, 20)[0] & 0x08000000):
                    skipped[name] = 'SSBump requires conversion to tangent-space normals'; continue
            chosen = preset(values, name)
            material.key = 'PBR'
            if shader == 'vertexlitgeneric': set_prop(values, '$model', '1')
            if not prop(values, '$bumpmap') and not prop(values, '$normaltexture'):
                set_prop(values, '$bumpmap', 'scp_pbr/flat_normal')
            if not prop(values, '$mraotexture'):
                set_prop(values, '$mraotexture', 'scp_pbr/mrao_' + chosen)
            # Respect an existing cubemap; otherwise use the map's local probes.
            if not prop(values, '$envmap'): set_prop(values, '$envmap', 'env_cubemap')
            path, encoding, old_hash = files[name]
            output = serialize(material).encode(encoding)
            if parse(output.decode(encoding)).key != 'PBR': raise ValueError('output validation failed')
            actions.append({'name': name, 'path': path, 'content': output, 'old_sha256': old_hash,
                            'preset': chosen, 'source_shader': shader})
        except (ValueError, KeyError) as e: skipped[name] = str(e)
    return actions, skipped


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True, help='Asset folder containing materials/')
    parser.add_argument('--dry-run', action='store_true', help='Report candidates without changing any files')
    args = parser.parse_args()
    root = args.root.resolve()
    actions, skipped = plan_conversion(root)
    if args.dry_run:
        counts = {}
        for reason in skipped.values(): counts[reason] = counts.get(reason, 0) + 1
        print(json.dumps({'convertible': len(actions), 'remaining': counts}, indent=2, ensure_ascii=False))
        return
    log_path = root / 'pbr-conversion.log'
    state_path = root / 'pbr-conversion.json'
    log = log_path.open('a', encoding='utf-8', buffering=1)
    def emit(message):
        print(message, flush=True); log.write(message + '\n')
    state = {'root': str(root), 'started': time.time(), 'total': len(actions), 'done': {}, 'skipped': skipped, 'failed': {}}
    def save(): atomic_write(state_path, json.dumps(state, ensure_ascii=False, indent=2).encode('utf-8'))
    emit(f'PBR START: {len(actions)} convertible materials, {len(skipped)} skipped; no backups')
    try:
        # Re-running repairs the shared maps and skips already-converted VMTs.
        generated = root / 'materials/scp_pbr'
        atomic_write(generated / 'flat_normal.vtf', constant_vtf((128, 128, 255), True))
        for name, rgb in PRESETS.items(): atomic_write(generated / ('mrao_' + name + '.vtf'), constant_vtf(rgb))
        save()
        for index, action in enumerate(actions, 1):
            path = action['path']
            if hashlib.sha256(path.read_bytes()).hexdigest() != action['old_sha256']:
                raise RuntimeError('Material edited during conversion: ' + str(path))
            atomic_write(path, action['content'])
            state['done'][action['name']] = {'preset': action['preset'], 'source_shader': action['source_shader'],
                'old_sha256': action['old_sha256'], 'new_sha256': hashlib.sha256(action['content']).hexdigest()}
            save(); emit(f'PBR DONE {index}/{len(actions)} {action["name"]}')
        state['finished'] = time.time(); save()
        emit(f'PBR FINISHED: {len(actions)} converted; {len(skipped)} skipped (see pbr-conversion.json)')
    except Exception as e:
        state['failed']['conversion'] = str(e); save(); emit('PBR FAILED: ' + str(e)); raise
    finally: log.close()


if __name__ == '__main__': main()

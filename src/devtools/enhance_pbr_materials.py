"""Build individual estimated PBR maps locally; preserve source images and special shaders.

Requires Pillow/numpy and the game's Valve vtex.exe/vtf2tga.exe. Color cannot
uniquely determine physical surface properties: generated normals/roughness/AO
are conservative estimates. Authored normals and existing custom MRAO are kept.
"""
from __future__ import annotations
import argparse
from collections import Counter
from functools import lru_cache
import hashlib
import json
import math
import os
from pathlib import Path
import re
import struct
import subprocess
import time

import numpy as np
from PIL import Image, ImageFilter
import convert_pbr_materials as kv

VERSION = 1
LANCZOS = Image.Resampling.LANCZOS


def exclusive_lock(path):
    handle = path.open('a+b')
    if handle.tell() == 0:
        handle.write(b'0'); handle.flush()
    handle.seek(0)
    try:
        if os.name == 'nt':
            import msvcrt
            msvcrt.locking(handle.fileno(), msvcrt.LK_NBLCK, 1)
        else:
            import fcntl
            fcntl.flock(handle.fileno(), fcntl.LOCK_EX | fcntl.LOCK_NB)
    except OSError:
        handle.close()
        raise RuntimeError('Detailed PBR conversion is already running for this folder') from None
    return handle


def texture_path(materials, name):
    return materials / (kv.normalized(name) + '.vtf')


def scalar(items, key, default):
    try: return float(kv.prop(items, key, str(default)))
    except ValueError: raise ValueError('non-numeric ' + key)


def build_plan(root):
    materials = root / 'materials'
    documents, encodings, skipped = {}, {}, {}
    for path in sorted(materials.rglob('*.vmt')):
        name = kv.normalized(path.relative_to(materials).as_posix())
        try:
            raw = path.read_bytes()
            try: text = raw.decode('utf-8-sig'); encoding = 'utf-8'
            except UnicodeDecodeError: text = raw.decode('cp1251'); encoding = 'cp1251'
            documents[name] = kv.parse(text); encodings[name] = encoding
        except (ValueError, UnicodeError) as e: skipped[name] = str(e)
    jobs = []
    for name in documents:
        try:
            material = kv.resolve(name, documents); p = material.value
            shader = material.key.lower()
            if shader not in ('pbr', 'lightmappedgeneric', 'vertexlitgeneric'):
                raise ValueError('special shader retained: ' + material.key)
            for key in ('$basetexture2', '$blendmodulatetexture', '$lightwarptexture'):
                if kv.prop(p, key): raise ValueError('shader feature retained: ' + key)
            if any(isinstance(v.value, list) and v.key.lower() != 'proxies' for v in p):
                raise ValueError('shader override/fallback block retained')
            if any(v.conditions and v.key.startswith('$') for v in p):
                raise ValueError('conditional material parameters retained')
            for key in ('$additive', '$selfillumfresnel', '$selfillum_envmapmask_alpha'):
                if kv.truth(kv.prop(p, key, '0')): raise ValueError('dynamic/additive feature retained: ' + key)
            if kv.prop(p, '$selfillumfresnelminmaxexp'):
                raise ValueError('view dependent self illumination retained')
            base = kv.prop(p, '$basetexture')
            if not base or base.startswith('_rt'): raise ValueError('no static base texture')
            path = texture_path(materials, base)
            if not path.is_file(): raise ValueError('base VTF is outside the supplied folder')
            header = path.read_bytes()[:80]
            if header[:4] != b'VTF\0' or len(header) < 80: raise ValueError('invalid VTF header')
            if struct.unpack_from('<H', header, 24)[0] != 1: raise ValueError('animated texture retained')
            if struct.unpack_from('<I', header, 20)[0] & 0x4000: raise ValueError('cubemap retained')
            if struct.unpack_from('<I', header, 52)[0] == 24: raise ValueError('HDR texture retained')
            detail = kv.prop(p, '$detail')
            if detail:
                if not texture_path(materials, detail).is_file(): raise ValueError('detail VTF is outside the supplied folder')
                scale = scalar(p, '$detailscale', 4)
                if scale < 1 or scale > 32 or not math.isclose(scale, round(scale)):
                    raise ValueError('fractional detail tiling needs shader support; avoiding baked seams')
                if int(scalar(p, '$detailblendmode', 0)) != 0 or kv.prop(p, '$detailtexturetransform'):
                    raise ValueError('detail mode/transform needs shader support')
                if kv.prop(p, '$basetexturetransform'):
                    raise ValueError('transformed detail needs shader support')
            for key in ('$bumpmap', '$normaltexture', '$selfillummask', '$envmapmask', '$emissiontexture'):
                ref = kv.prop(p, key)
                if ref and ref not in ('dev/flat_normal', 'scp_pbr/flat_normal') and not texture_path(materials, ref).is_file():
                    raise ValueError('required input outside the supplied folder: ' + key)
            if kv.prop(p, '$bumptransform') or kv.prop(p, '$bumpmaptransform'):
                raise ValueError('independent normal UV transform needs shader support')
            # A proxy driving removed parameters would lose its behavior after conversion.
            unsupported_proxy_vars = ('$selfillum', '$selfillummask', '$selfillumtint', '$detail', '$detailblendfactor', '$phong', '$envmaptint')
            proxy_text = ''.join(kv.serialize(v).lower() for v in p if v.key.lower() == 'proxies')
            if any(('"' + key + '"') in proxy_text for key in unsupported_proxy_vars):
                raise ValueError('proxy drives a parameter not supported by PBR')
            stem = re.sub(r'[^a-z0-9_]+', '_', name.split('/')[-1])[:32]
            ident = hashlib.sha256(name.encode()).hexdigest()[:12] + '_' + stem
            jobs.append({'name': name, 'material': material, 'encoding': encodings[name],
                         'source_shader': shader, 'path': materials / (name + '.vmt'),
                         'prefix': 'scp_pbr/generated/' + ident,
                         'flags': struct.unpack_from('<I', header, 20)[0]})
        except (ValueError, KeyError) as e: skipped[name] = str(e)
    return jobs, skipped


def wrapped_blur(gray, radius, flags):
    pad = max(2, math.ceil(radius * 3))
    a = np.asarray(gray)
    a = np.pad(a, ((pad, pad), (0, 0)), mode='edge' if flags & 8 else 'wrap')
    a = np.pad(a, ((0, 0), (pad, pad)), mode='edge' if flags & 4 else 'wrap')
    return np.asarray(Image.fromarray(a).filter(ImageFilter.GaussianBlur(radius)), dtype=np.float32)[pad:-pad, pad:-pad] / 255


def surface_maps(color, flags, chosen, printed=False):
    rgb = np.asarray(color.convert('RGB'), dtype=np.float32) / 255
    gray = Image.fromarray(np.rint(np.dot(rgb, [0.2126, 0.7152, 0.0722]) * 255).astype('uint8'))
    small = wrapped_blur(gray, 1.0, flags)
    large = wrapped_blur(gray, 7.0, flags)
    height = small - large
    if printed: height = np.zeros_like(height)
    strength = 1.5 * max(color.size) / 512
    dx = np.gradient(height, axis=1) if flags & 4 else (np.roll(height, -1, 1) - np.roll(height, 1, 1)) / 2
    dy = np.gradient(height, axis=0) if flags & 8 else (np.roll(height, -1, 0) - np.roll(height, 1, 0)) / 2
    vector = np.stack((-dx * strength, dy * strength, np.ones_like(height)), axis=2)
    vector /= np.maximum(np.linalg.norm(vector, axis=2, keepdims=True), 1e-6)
    normal = Image.fromarray(np.clip(np.rint((vector + 1) * 127.5), 0, 255).astype('uint8'))
    rough = chosen[1] / 255 + np.clip(height * 0.6, -0.12, 0.12)
    ao = np.clip(1 - np.maximum(-height, 0) * 0.15, 0.95, 1)
    mrao = np.stack((np.full_like(rough, chosen[0] / 255), np.clip(rough, .07, 1), ao), axis=2)
    return normal, mrao


class Pipeline:
    def __init__(self, root, tools, work, map_size):
        self.root = root; self.materials = root / 'materials'; self.tools = tools
        self.work = work; self.stage = work / 'stage'; self.stage.mkdir(parents=True, exist_ok=True)
        self.map_size = map_size

    def native(self, command, label):
        with (self.work / 'native-last.log').open('wb') as log:
            process = subprocess.run([str(x) for x in command], cwd=self.tools.parent.parent,
                stdout=log, stderr=subprocess.STDOUT, timeout=120,
                creationflags=getattr(subprocess, 'CREATE_NO_WINDOW', 0))
        if process.returncode: raise RuntimeError(f'{label} failed; see {self.work / "native-last.log"}')

    @lru_cache(maxsize=3)
    def decode(self, ref):
        target = self.stage / 'decoded.tga'
        target.unlink(missing_ok=True)
        self.native([self.tools / 'vtf2tga.exe', '-i', texture_path(self.materials, ref), '-o', target], 'decode')
        with Image.open(target) as image: return image.convert('RGBA').copy()

    def encode(self, image, ref, kind, flags=0):
        image.save(self.stage / 'map.tga')
        alpha = image.mode == 'RGBA'
        config = {'nomip': 0, 'nolod': 0, 'normal': int(kind == 'normal'),
                  'srgb': int(kind in ('color', 'emission')), 'nocompress': 0,
                  'dxt5': int(alpha or kind == 'normal'), 'numchannels': 4 if alpha else 3,
                  'clamps': int(bool(flags & 4)), 'clampt': int(bool(flags & 8))}
        (self.stage / 'map.txt').write_text(''.join(f'"{k}" "{v}"\n' for k, v in config.items()), encoding='ascii')
        out = self.stage / 'map.vtf'; out.unlink(missing_ok=True)
        self.native([self.tools / 'vtex.exe', '-dontusegamedir', '-nopause', '-UseStandardError', '-quiet', '-outdir', self.stage, self.stage / 'map.tga'], 'encode')
        raw = out.read_bytes()
        if raw[:4] != b'VTF\0' or struct.unpack_from('<HH', raw, 16) != image.size or raw[56] != max(image.size).bit_length():
            raise RuntimeError('Generated VTF has invalid dimensions/mips')
        verify = self.stage / 'verify.tga'; verify.unlink(missing_ok=True)
        self.native([self.tools / 'vtf2tga.exe', '-i', out, '-o', verify], 'verify')
        with Image.open(verify) as decoded:
            if decoded.size != image.size: raise RuntimeError('Native decode dimensions mismatch')
            # Catch swapped/corrupted channels and failed compression before publishing.
            difference = np.abs(np.asarray(decoded.convert('RGB'), dtype=np.float32) - np.asarray(image.convert('RGB'), dtype=np.float32)).mean()
            if not np.isfinite(difference) or difference > 15: raise RuntimeError('Unexpected compression error')
        kv.atomic_write(texture_path(self.materials, ref), raw)
        return {'path': 'materials/' + ref + '.vtf', 'bytes': len(raw), 'sha256': hashlib.sha256(raw).hexdigest(), 'size': list(image.size)}

    def material(self, job):
        p = job['material'].value
        original = self.decode(kv.prop(p, '$basetexture'))
        color = original.copy(); flags = job['flags']
        detail = kv.prop(p, '$detail')
        recolored = bool(detail)
        if detail:
            scale = round(scalar(p, '$detailscale', 4))
            tile = self.decode(detail).convert('RGB').resize((max(1, color.width // scale), max(1, color.height // scale)), LANCZOS)
            pattern = np.tile(np.asarray(tile, dtype=np.float32) / 255, (scale + 1, scale + 1, 1))[:color.height, :color.width]
            linear = np.power(np.asarray(color.convert('RGB'), dtype=np.float32) / 255, 2.2)
            factor = scalar(p, '$detailblendfactor', 1)
            result = np.power(np.clip(linear * (1 + (2 * pattern - 1) * factor), 0, 1), 1 / 2.2)
            color = Image.fromarray(np.rint(result * 255).astype('uint8')).convert('RGBA'); color.putalpha(original.getchannel('A'))
        chosen_name = kv.preset(p, job['name']); chosen = kv.PRESETS[chosen_name]
        printed = bool(re.search(r'sign|logo|label|paper|screen|monitor|decal|overlay|credit|maptitle', job['name']))
        estimated_normal, mrao = surface_maps(color, flags, chosen, printed)
        # Retain useful information from existing reflection masks. A mask is
        # not metalness: use it only as a restrained roughness adjustment.
        reflection_mask = kv.prop(p, '$envmapmask')
        gloss = None
        if reflection_mask:
            gloss = np.asarray(self.decode(reflection_mask).convert('L').resize(color.size, LANCZOS), dtype=np.float32) / 255
        elif kv.truth(kv.prop(p, '$basealphaenvmapmask', '0')):
            gloss = np.asarray(original.getchannel('A'), dtype=np.float32) / 255
        if gloss is not None:
            mrao[:, :, 1] = np.clip(mrao[:, :, 1] + 0.08 - 0.16 * gloss, .07, 1)
        assets, normal_mode = [], 'preserved'
        normal_ref = kv.prop(p, '$bumpmap') or kv.prop(p, '$normaltexture')
        if normal_ref in ('dev/flat_normal', 'scp_pbr/flat_normal'): normal_ref = ''
        ssbump = kv.truth(kv.prop(p, '$ssbump', '0'))
        if normal_ref:
            raw_header = texture_path(self.materials, normal_ref).read_bytes()[:24]
            ssbump |= 'ssbump' in normal_ref.lower() or bool(struct.unpack_from('<I', raw_header, 20)[0] & 0x08000000)
        if normal_ref and ssbump:
            encoded = np.asarray(self.decode(normal_ref).convert('RGB'), dtype=np.float32) / 255
            basis = np.array([[math.sqrt(2/3), 0, 1/math.sqrt(3)], [-1/math.sqrt(6), 1/math.sqrt(2), 1/math.sqrt(3)], [-1/math.sqrt(6), -1/math.sqrt(2), 1/math.sqrt(3)]], dtype=np.float32)
            vector = encoded @ basis
            vector /= np.maximum(np.linalg.norm(vector, axis=2, keepdims=True), 1e-6)
            estimated_normal = Image.fromarray(np.clip(np.rint((vector + 1) * 127.5), 0, 255).astype('uint8'))
            normal_mode = 'converted_ssbump'
        elif not normal_ref: normal_mode = 'estimated_from_color' if not printed else 'flat_for_printed_surface'
        if not normal_ref or ssbump:
            ref = job['prefix'] + '_normal'
            assets.append(self.encode(estimated_normal, ref, 'normal', flags)); kv.set_prop(p, '$bumpmap', ref)
            p[:] = [v for v in p if v.key.lower() not in ('$normaltexture', '$ssbump')]
        elif kv.prop(p, '$normaltexture'): kv.set_prop(p, '$bumpmap', normal_ref)
        existing_mrao = kv.prop(p, '$mraotexture')
        if not existing_mrao or existing_mrao.startswith('scp_pbr/mrao_'):
            image = Image.fromarray(np.clip(np.rint(mrao * 255), 0, 255).astype('uint8'))
            if max(image.size) > self.map_size:
                ratio = self.map_size / max(image.size)
                image = image.resize((max(1, round(image.width * ratio)), max(1, round(image.height * ratio))), LANCZOS)
            ref = job['prefix'] + '_mrao'
            assets.append(self.encode(image, ref, 'data', flags)); kv.set_prop(p, '$mraotexture', ref)
        emission = kv.truth(kv.prop(p, '$selfillum', '0'))
        if emission:
            mask_ref = kv.prop(p, '$selfillummask')
            if mask_ref: mask = np.asarray(self.decode(mask_ref).convert('RGB').resize(color.size, LANCZOS), dtype=np.float32) / 255
            else: mask = np.asarray(original.getchannel('A'), dtype=np.float32)[:, :, None] / 255
            tint_text = kv.prop(p, '$selfillumtint', '[1 1 1]')
            numbers = [float(x) for x in re.findall(r'[-+]?(?:\d*\.)?\d+(?:[eE][-+]?\d+)?', tint_text)]
            if len(numbers) != 3: raise ValueError('Unsupported selfillum tint')
            tint = np.array(numbers, dtype=np.float32).reshape(1, 1, 3)
            if '{' in tint_text: tint /= 255
            linear = np.power(np.asarray(color.convert('RGB'), dtype=np.float32) / 255, 2.2)
            glow = Image.fromarray(np.rint(np.power(np.clip(linear * mask * tint, 0, 1), 1/2.2) * 255).astype('uint8'))
            diffuse = Image.fromarray(np.rint(np.power(np.clip(linear * (1 - mask), 0, 1), 1/2.2) * 255).astype('uint8')).convert('RGBA')
            diffuse.putalpha(color.getchannel('A')); color = diffuse; recolored = True
            ref = job['prefix'] + '_emission'
            assets.append(self.encode(glow, ref, 'emission', flags)); kv.set_prop(p, '$emissiontexture', ref)
        if recolored:
            ref = job['prefix'] + '_albedo'
            assets.append(self.encode(color, ref, 'color', flags)); kv.set_prop(p, '$basetexture', ref)
        remove = ('$selfillum', '$selfillummask', '$selfillumtint') if emission else ()
        if detail: remove += ('$detail', '$detailscale', '$detailblendfactor', '$detailblendmode')
        p[:] = [v for v in p if v.key.lower() not in remove]
        job['material'].key = 'PBR'
        if job['source_shader'] == 'vertexlitgeneric': kv.set_prop(p, '$model', '1')
        if not kv.prop(p, '$envmap'): kv.set_prop(p, '$envmap', 'env_cubemap')
        result = kv.serialize(job['material']).encode(job['encoding'])
        kv.parse(result.decode(job['encoding']))
        # Image assets are decoded and verified before the material references them.
        kv.atomic_write(job['path'], result)
        return {'normal': normal_mode, 'estimated_mrao': not existing_mrao or existing_mrao.startswith('scp_pbr/mrao_'),
                'emission_preserved': emission, 'detail_baked': bool(detail), 'assets': assets,
                'material_sha256': hashlib.sha256(result).hexdigest()}


def main():
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument('--root', type=Path, required=True)
    parser.add_argument('--tools', type=Path, required=True, help='Directory with Valve vtex.exe/vtf2tga.exe')
    parser.add_argument('--map-size', type=int, default=1024, choices=(512, 1024, 2048))
    parser.add_argument('--dry-run', action='store_true')
    args = parser.parse_args(); root = args.root.resolve(); tools = args.tools.resolve()
    if not (root / 'materials').is_dir(): parser.error('root must contain materials/')
    for name in ('vtex.exe', 'vtf2tga.exe'):
        if not (tools / name).is_file(): parser.error('Missing tool: ' + name)
    jobs, skipped = build_plan(root)
    if args.dry_run:
        print(json.dumps({'compatible_materials': len(jobs), 'skipped': dict(Counter(skipped.values()))}, ensure_ascii=False, indent=2)); return
    work = root / 'pbr-processing'; work.mkdir(exist_ok=True)
    run_lock = exclusive_lock(work / 'run.lock')
    state_file = root / 'pbr-detailed-progress.json'
    state = json.loads(state_file.read_text(encoding='utf-8')) if state_file.exists() else {'version': VERSION, 'root': str(root), 'map_size': args.map_size, 'done': {}, 'failed': {}, 'started': time.time()}
    if state['version'] != VERSION or state['root'] != str(root) or state['map_size'] != args.map_size:
        raise RuntimeError('Existing progress has different settings')
    state['total'] = len(jobs); state['skipped'] = skipped
    state.pop('finished', None)
    def save(): kv.atomic_write(state_file, json.dumps(state, ensure_ascii=False, indent=2).encode('utf-8'))
    log = (root / 'pbr-detailed.log').open('a', encoding='utf-8', buffering=1)
    def emit(message): print(message, flush=True); log.write(message + '\n')
    pipeline = Pipeline(root, tools, work, args.map_size)
    emit(f'PBR DETAIL START: {len(jobs)} compatible materials; {len(skipped)} special cases retained')
    save()
    try:
        for index, job in enumerate(jobs, 1):
            previous = state['done'].get(job['name'])
            if previous and hashlib.sha256(job['path'].read_bytes()).hexdigest() == previous['material_sha256'] and all(texture_path(root / 'materials', a['path'][10:-4]).is_file() for a in previous['assets']): continue
            begin = time.monotonic(); emit(f'PBR DETAIL START {index}/{len(jobs)} {job["name"]}')
            try:
                result = pipeline.material(job); result['seconds'] = round(time.monotonic() - begin, 2)
                state['done'][job['name']] = result; state['failed'].pop(job['name'], None)
                save(); emit(f'PBR DETAIL DONE {index}/{len(jobs)} {job["name"]} {result["seconds"]}s')
            except Exception as error:
                state['failed'][job['name']] = str(error); save()
                emit('PBR DETAIL FAILED ' + job['name'] + ': ' + str(error)); raise
        state['finished'] = time.time(); save(); emit(f'PBR DETAIL FINISHED: {len(state["done"])} materials; {len(skipped)} special cases retained')
        for path in pipeline.stage.iterdir():
            if path.is_file(): path.unlink()
    finally:
        log.close()
        run_lock.close()


if __name__ == '__main__': main()

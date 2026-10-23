#include "cbase.h"
#include "igamesystem.h"
#include "materialsystem/imaterialproxy.h"
#include "materialsystem/imaterial.h"
#include "materialsystem/imaterialvar.h"
#include "materialsystem/itexture.h"
#include "vtf/vtf.h"
#include "filesystem.h"
#include "tier1/KeyValues.h"
#include <condition_variable>
#include <atomic>
#include <deque>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include "tier0/memdbgon.h"

// Opt-in material proxy. The shipped VTF contains only the original image;
// smaller levels are built on a worker thread and never written to disk.
namespace
{
struct RuntimeImage
{
    std::string name;
    int width = 0, height = 0, flags = 0, topOffset = 0;
    ImageFormat sourceFormat = IMAGE_FORMAT_UNKNOWN;
    ImageFormat format = IMAGE_FORMAT_UNKNOWN;
    std::vector<std::vector<unsigned char>> mips; // Level zero stays in the source file.
};

struct PendingImage
{
    std::string name;
    std::shared_ptr<RuntimeImage> image;
    std::atomic<ITexture *> texture{nullptr};
    bool finished = false;
    std::string error;
};

struct VTFDeleter
{
    void operator()(IVTFTexture *p) const { if (p) DestroyVTFTexture(p); }
};
using VTFPtr = std::unique_ptr<IVTFTexture, VTFDeleter>;

bool BuildImage(RuntimeImage &out)
{
    CUtlBuffer file;
    std::string path = "materials/" + out.name + ".vtf";
    if (!filesystem->ReadFile(path.c_str(), "GAME", file) || file.TellPut() < 80)
        return false;
    VTFPtr source(CreateVTFTexture());
    if (!source || !source->Unserialize(file) || source->FrameCount() != 1 || source->IsCubeMap() || source->Depth() != 1)
        return false;
    out.width = source->Width(); out.height = source->Height(); out.flags = source->Flags();
    out.sourceFormat = source->Format();
    if (out.width < 4 || out.height < 4 || out.width > 4096 || out.height > 4096)
        return false;
    if (out.sourceFormat != IMAGE_FORMAT_DXT1 && out.sourceFormat != IMAGE_FORMAT_DXT5 &&
        out.sourceFormat != IMAGE_FORMAT_RGB888 && out.sourceFormat != IMAGE_FORMAT_BGR888 &&
        out.sourceFormat != IMAGE_FORMAT_RGBA8888 && out.sourceFormat != IMAGE_FORMAT_BGRA8888)
        return false;
    out.format = (out.sourceFormat == IMAGE_FORMAT_DXT1 || out.sourceFormat == IMAGE_FORMAT_DXT5)
        ? out.sourceFormat : IMAGE_FORMAT_BGRA8888;

    // ImageFileInfo also supports older VTFs and files with an existing mip chain.
    int topBytes = 0;
    source->ImageFileInfo(0, 0, 0, &out.topOffset, &topBytes);
    // The SDK VTF reader expands its in-memory mip count. Locate mip zero from
    // the serialized header instead, where the actual file mip count is stored.
    const unsigned char *raw = static_cast<const unsigned char *>(file.Base());
    uint32 headerBytes = 0, minor = 0;
    Q_memcpy(&headerBytes, raw + 12, 4); Q_memcpy(&minor, raw + 8, 4);
    int imageOffset = -1;
    if (minor >= 3)
    {
        uint32 resources = 0; Q_memcpy(&resources, raw + 68, 4);
        if (resources > 32 || 80 + resources * 8 > headerBytes || headerBytes > (uint32)file.TellPut()) return false;
        for (uint32 i = 0; i < resources; ++i)
        {
            uint32 tag = 0, offset = 0;
            Q_memcpy(&tag, raw + 80 + i * 8, 4); Q_memcpy(&offset, raw + 84 + i * 8, 4);
            if (tag == 0x30) imageOffset = offset;
        }
    }
    else
    {
        ImageFormat thumbnail; Q_memcpy(&thumbnail, raw + 57, 4);
        imageOffset = headerBytes;
        if (raw[61] && raw[62]) imageOffset += ImageLoader::GetMemRequired(raw[61], raw[62], 1, thumbnail, false);
    }
    if (imageOffset < 0) return false;
    out.topOffset = imageOffset;
    for (int mip = 1; mip < raw[56]; ++mip)
        out.topOffset += ImageLoader::GetMemRequired(Max(1, out.width >> mip), Max(1, out.height >> mip), 1, out.sourceFormat, false);
    topBytes = ImageLoader::GetMemRequired(out.width, out.height, 1, out.sourceFormat, false);
    if (out.topOffset < 0 || topBytes > file.TellPut() - out.topOffset) return false;

    source->ConvertImageFormat(IMAGE_FORMAT_RGBA8888, false);
    VTFPtr smaller(CreateVTFTexture());
    int flags = out.flags & ~(TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_IGNORE_PICMIP);
    if (!smaller || !smaller->Init(Max(1, out.width / 2), Max(1, out.height / 2), 1, IMAGE_FORMAT_RGBA8888, flags, 1))
        return false;
    ImageLoader::ResampleInfo_t info;
    info.m_pSrc = source->ImageData(0, 0, 0); info.m_pDest = smaller->ImageData(0, 0, 0);
    info.m_nSrcWidth = out.width; info.m_nSrcHeight = out.height;
    info.m_nDestWidth = smaller->Width(); info.m_nDestHeight = smaller->Height();
    bool normal = (out.flags & TEXTUREFLAGS_NORMAL) && !(out.flags & TEXTUREFLAGS_SSBUMP);
    info.m_flSrcGamma = info.m_flDestGamma = normal ? 1.0f : 2.2f;
    if (normal) info.m_nFlags |= ImageLoader::RESAMPLE_NORMALMAP;
    if (flags & TEXTUREFLAGS_CLAMPS) info.m_nFlags |= ImageLoader::RESAMPLE_CLAMPS;
    if (flags & TEXTUREFLAGS_CLAMPT) info.m_nFlags |= ImageLoader::RESAMPLE_CLAMPT;
    if (flags & TEXTUREFLAGS_ONEBITALPHA) info.m_nFlags |= ImageLoader::RESAMPLE_ALPHATEST;
    if (!ImageLoader::ResampleRGBA8888(info)) return false;
    smaller->GenerateMipmaps(); smaller->ConvertImageFormat(out.format, false);
    out.mips.resize(smaller->MipCount() + 1);
    for (int mip = 0; mip < smaller->MipCount(); ++mip)
    {
        const unsigned char *data = smaller->ImageData(0, 0, mip);
        out.mips[mip + 1].assign(data, data + smaller->ComputeMipSize(mip));
    }
    return true;
}

class RuntimeRegenerator : public ITextureRegenerator
{
public:
    explicit RuntimeRegenerator(std::shared_ptr<const RuntimeImage> image) : m_Image(std::move(image)) {}
    void RegenerateTextureBits(ITexture *, IVTFTexture *dst, Rect_t *) override
    {
        const RuntimeImage &image = *m_Image;
        int skip = 0;
        while (skip + 1 < (int)image.mips.size() &&
               (Max(1, image.width >> skip) != dst->Width() || Max(1, image.height >> skip) != dst->Height())) ++skip;
        if (Max(1, image.width >> skip) != dst->Width() || Max(1, image.height >> skip) != dst->Height() || dst->Format() != image.format)
            return;
        for (int mip = 0; mip < dst->MipCount(); ++mip)
        {
            int sourceMip = mip + skip;
            if (sourceMip == 0)
            {
                CUtlBuffer file;
                std::string path = "materials/" + image.name + ".vtf";
                if (!filesystem->ReadFile(path.c_str(), "GAME", file)) return;
                int bytes = ImageLoader::GetMemRequired(image.width, image.height, 1, image.sourceFormat, false);
                if (image.topOffset < 0 || bytes > file.TellPut() - image.topOffset) return;
                const unsigned char *src = static_cast<const unsigned char *>(file.Base()) + image.topOffset;
                if (image.sourceFormat == image.format) Q_memcpy(dst->ImageData(0, 0, mip), src, bytes);
                else if (!ImageLoader::ConvertImageFormat(src, image.sourceFormat, dst->ImageData(0, 0, mip), image.format, image.width, image.height)) return;
            }
            else
            {
                if (sourceMip >= (int)image.mips.size() || (int)image.mips[sourceMip].size() != dst->ComputeMipSize(mip)) return;
                Q_memcpy(dst->ImageData(0, 0, mip), image.mips[sourceMip].data(), image.mips[sourceMip].size());
            }
        }
    }
    void Release() override { delete this; }
private:
    std::shared_ptr<const RuntimeImage> m_Image;
};

class RuntimeTextureQuality : public CAutoGameSystemPerFrame
{
public:
    RuntimeTextureQuality() : CAutoGameSystemPerFrame("RuntimeTextureQuality") {}
    bool Init() override
    {
        m_Stop = false;
        m_Worker = std::thread([this]() { Work(); });
        return true;
    }
    std::shared_ptr<PendingImage> Request(const char *name)
    {
        std::string key(name);
        for (char &c : key) { if (c == '\\') c = '/'; c = (char)tolower((unsigned char)c); }
        if (key.empty() || key.find("..") != std::string::npos || key.find(':') != std::string::npos) return nullptr;
        std::lock_guard<std::mutex> lock(m_Mutex);
        auto &entry = m_Entries[key];
        if (!entry)
        {
            entry = std::make_shared<PendingImage>(); entry->name = key;
            m_Queue.push_back(entry); m_Wake.notify_one();
        }
        return entry;
    }
    void Update(float) override
    {
        std::shared_ptr<PendingImage> entry;
        {
            std::lock_guard<std::mutex> lock(m_Mutex);
            if (m_Ready.empty()) return;
            entry = m_Ready.front(); m_Ready.pop_front();
        }
        if (!entry->image) { Warning("Runtime texture quality: %s: %s\n", entry->name.c_str(), entry->error.c_str()); return; }
        const RuntimeImage &image = *entry->image;
        std::string name = "__fd_runtime_quality/" + entry->name;
        int flags = image.flags & ~(TEXTUREFLAGS_NOMIP | TEXTUREFLAGS_NOLOD | TEXTUREFLAGS_IGNORE_PICMIP);
        ITexture *texture = materials->CreateProceduralTexture(name.c_str(), TEXTURE_GROUP_WORLD, image.width, image.height, image.format, flags);
        if (IsErrorTexture(texture)) return;
        texture->IncrementReferenceCount();
        texture->SetTextureRegenerator(new RuntimeRegenerator(entry->image));
        texture->Download();
        entry->texture = texture;
    }
    void Shutdown() override
    {
        { std::lock_guard<std::mutex> lock(m_Mutex); m_Stop = true; m_Queue.clear(); }
        m_Wake.notify_all();
        if (m_Worker.joinable()) m_Worker.join();
        for (auto &it : m_Entries)
        {
            ITexture *texture = it.second->texture.load();
            if (texture) { texture->SetTextureRegenerator(nullptr); texture->DecrementReferenceCount(); }
            it.second->texture = nullptr;
        }
        m_Ready.clear(); m_Entries.clear();
    }
    void Status()
    {
        std::lock_guard<std::mutex> lock(m_Mutex);
        for (const auto &it : m_Entries)
        {
            const PendingImage &e = *it.second;
            ITexture *texture = e.texture.load();
            if (texture) Msg("Runtime texture %s: %dx%d, %d GPU bytes (source %dx%d)\n", e.name.c_str(), texture->GetActualWidth(), texture->GetActualHeight(), texture->GetApproximateVidMemBytes(), e.image->width, e.image->height);
            else Msg("Runtime texture %s: %s\n", e.name.c_str(), e.finished ? "failed" : "building");
        }
    }
private:
    void Work()
    {
        for (;;)
        {
            std::shared_ptr<PendingImage> entry;
            { std::unique_lock<std::mutex> lock(m_Mutex); m_Wake.wait(lock, [this]() { return m_Stop || !m_Queue.empty(); });
              if (m_Stop) return; entry = m_Queue.front(); m_Queue.pop_front(); }
            auto image = std::make_shared<RuntimeImage>(); image->name = entry->name;
            bool ok = BuildImage(*image);
            { std::lock_guard<std::mutex> lock(m_Mutex);
              if (ok) entry->image = image; else entry->error = "unsupported or unreadable VTF; using the original texture";
              entry->finished = true; m_Ready.push_back(entry); }
        }
    }
    std::map<std::string, std::shared_ptr<PendingImage>> m_Entries;
    std::deque<std::shared_ptr<PendingImage>> m_Queue, m_Ready;
    std::mutex m_Mutex;
    std::condition_variable m_Wake;
    std::thread m_Worker;
    bool m_Stop = false;
};
RuntimeTextureQuality g_RuntimeTextureQuality;

class RuntimeTextureQualityProxy : public IMaterialProxy
{
    struct Binding { IMaterialVar *var; std::shared_ptr<PendingImage> entry; };
    std::vector<Binding> m_Bindings;
    IMaterial *m_Material = nullptr;
public:
    bool Init(IMaterial *material, KeyValues *keys) override
    {
        m_Material = material;
        for (KeyValues *key = keys->GetFirstValue(); key; key = key->GetNextValue())
        {
            if (key->GetName()[0] != '$') continue;
            bool found = false;
            IMaterialVar *var = material->FindVar(key->GetName(), &found, false);
            // Proxy initialization can precede the shader loading its textures.
            if (!found) continue;
            auto entry = g_RuntimeTextureQuality.Request(key->GetString());
            if (entry) m_Bindings.push_back({var, entry});
        }
        return !m_Bindings.empty();
    }
    void OnBind(void *) override
    {
        for (const Binding &binding : m_Bindings)
        {
            if (!binding.var->IsTexture()) continue;
            ITexture *replacement = binding.entry->texture.load();
            if (!replacement || binding.var->GetTextureValue() == replacement) continue;
            ITexture *original = binding.var->GetTextureValue();
            if (original) original->IncrementReferenceCount();
            binding.var->SetTextureValue(replacement);
            if (original) { original->DecrementReferenceCount(); original->DeleteIfUnreferenced(); }
        }
    }
    void Release() override { delete this; }
    IMaterial *GetMaterial() override { return m_Material; }
};
}

EXPOSE_INTERFACE(RuntimeTextureQualityProxy, IMaterialProxy, "FDTextureQuality" IMATERIAL_PROXY_INTERFACE_VERSION);

CON_COMMAND(fd_texture_quality_status, "Show the dimensions and GPU memory of runtime generated textures.")
{
    g_RuntimeTextureQuality.Status();
}

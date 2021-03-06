#pragma once
#include <d3d11.h>
#include "../ThrowIfFailed.h"

#include <cstdint>

#define MAKE_FLAG(Enum)\
inline Enum operator|(Enum a, Enum b)\
{\
    return static_cast<Enum>(static_cast<uint32_t>(a) | static_cast<uint32_t>(b));\
}\
inline Enum operator&(Enum a, Enum b)\
{\
    return static_cast<Enum>(static_cast<uint32_t>(a) & static_cast<uint32_t>(b));\
}

enum class BufferUsage : uint32_t
{
    Default = D3D11_USAGE_DEFAULT,
    Immutable = D3D11_USAGE_IMMUTABLE,
    Dynamic = D3D11_USAGE_DYNAMIC,
    Staging = D3D11_USAGE_STAGING
}; MAKE_FLAG(BufferUsage);

enum class BufferBind : uint32_t
{
    VertexBuffer = D3D11_BIND_VERTEX_BUFFER,
    IndexBuffer = D3D11_BIND_INDEX_BUFFER,
    ConstantBuffer = D3D11_BIND_CONSTANT_BUFFER,
    ShaderResource = D3D11_BIND_SHADER_RESOURCE,
    StreamOutput = D3D11_BIND_STREAM_OUTPUT,
    RenderTarget = D3D11_BIND_RENDER_TARGET,
    DepthStencil = D3D11_BIND_DEPTH_STENCIL,
    UnorderedAccess = D3D11_BIND_UNORDERED_ACCESS,
    BindDecoder = D3D11_BIND_DECODER,
    VideoEncoder = D3D11_BIND_VIDEO_ENCODER
}; MAKE_FLAG(BufferBind);

enum class BufferCpuAccess : uint32_t
{
    None = 0,
    Write = D3D11_CPU_ACCESS_WRITE,
    Read = D3D11_CPU_ACCESS_READ
}; MAKE_FLAG(BufferCpuAccess);

enum class BufferMisc : uint32_t
{
    None = 0,
    GenerateMips = D3D11_RESOURCE_MISC_GENERATE_MIPS,
    Shared = D3D11_RESOURCE_MISC_SHARED,
    TextureCube = D3D11_RESOURCE_MISC_TEXTURECUBE,
    DrawIndirectArgs = D3D11_RESOURCE_MISC_DRAWINDIRECT_ARGS,
    BufferAllowRawView = D3D11_RESOURCE_MISC_BUFFER_ALLOW_RAW_VIEWS,
    BufferStructured = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED,
    ResourceClamp = D3D11_RESOURCE_MISC_RESOURCE_CLAMP,
    SharedKeyedMutex = D3D11_RESOURCE_MISC_SHARED_KEYEDMUTEX,
    GDICompatible = D3D11_RESOURCE_MISC_GDI_COMPATIBLE,
    SharedNTHandle = D3D11_RESOURCE_MISC_SHARED_NTHANDLE,
    RestrictedContent = D3D11_RESOURCE_MISC_RESTRICTED_CONTENT,
    RestrictSharedResource = D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE,
    RestrictSharedResourceDriver = D3D11_RESOURCE_MISC_RESTRICT_SHARED_RESOURCE_DRIVER,
    Guarded = D3D11_RESOURCE_MISC_GUARDED,
    TilePool = D3D11_RESOURCE_MISC_TILE_POOL,
    Tiled = D3D11_RESOURCE_MISC_TILED,
    HWProtected = D3D11_RESOURCE_MISC_HW_PROTECTED
}; MAKE_FLAG(BufferMisc);

template<typename T>
class Buffer
{
public:
    Buffer(BufferUsage usage, BufferBind bindFlags, BufferCpuAccess cpuFlags, BufferMisc miscFlags, size_t size, T * data = nullptr);
    Buffer(ID3D11Device * device, BufferUsage usage, BufferBind bindFlags, BufferCpuAccess cpuFlags, BufferMisc miscFlags, size_t size, T * data = nullptr);
    virtual ~Buffer();

    operator ID3D11Buffer*  () const { return buffer; }
    operator ID3D11Buffer*const* () const { return &buffer; }

    Buffer & operator=(Buffer & other) = delete;
    T* map(ID3D11DeviceContext *cxt = Global::context);
    void unmap(ID3D11DeviceContext *cxt = Global::context);
private:
    ID3D11Buffer * buffer;
};

template<typename T>
Buffer<T>::Buffer(BufferUsage usage, BufferBind bindFlags, BufferCpuAccess cpuFlags, BufferMisc miscFlags, size_t size, T * data)
    : Buffer(Global::device, usage, bindFlags, cpuFlags, miscFlags, size, data)
{
}

template<typename T>
Buffer<T>::Buffer(ID3D11Device * device, BufferUsage usage, BufferBind bindFlags, BufferCpuAccess cpuFlags, BufferMisc miscFlags, size_t size, T * data)
{
    D3D11_BUFFER_DESC desc = {};
    desc.Usage = static_cast<D3D11_USAGE>(usage);
    desc.ByteWidth = static_cast<UINT>(sizeof(T) * size);
    desc.BindFlags = static_cast<UINT>(bindFlags);
    desc.MiscFlags = static_cast<UINT>(miscFlags);
    desc.CPUAccessFlags = static_cast<UINT>(cpuFlags);
    desc.StructureByteStride = sizeof(T);

    D3D11_SUBRESOURCE_DATA subData = {};
    subData.pSysMem = data;

    ThrowIfFailed(device->CreateBuffer(&desc, data ? &subData : nullptr, &buffer));
}

template<typename T>
Buffer<T>::~Buffer()
{
    buffer->Release();
}

template<typename T>
T * Buffer<T>::map(ID3D11DeviceContext * cxt)
{
    D3D11_MAPPED_SUBRESOURCE data = {};
    cxt->Map(buffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &data);
    return (T*)data.pData;
}

template<typename T>
void Buffer<T>::unmap(ID3D11DeviceContext * cxt)
{
    cxt->Unmap(buffer, 0);
}

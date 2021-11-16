#pragma once
#include "imvk_allocator.h"
#include "imvk_option.h"
#include "imvk_platform.h"
#include "immat.h"
#include <vulkan/vulkan.h>
#include <string.h>


namespace ImGui 
{
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkImageMat Class define
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class VKSHADER_API VkImageMat final : public ImMat
{
public:
    // empty
    VkImageMat();
    // vec
    VkImageMat(int w, size_t elemsize, VkAllocator* allocator);
    // image
    VkImageMat(int w, int h, size_t elemsize, VkAllocator* allocator);
    // dim
    VkImageMat(int w, int h, int c, size_t elemsize, VkAllocator* allocator);
    // packed vec
    VkImageMat(int w, size_t elemsize, int elempack, VkAllocator* allocator);
    // packed image
    VkImageMat(int w, int h, size_t elemsize, int elempack, VkAllocator* allocator);
    // packed dim
    VkImageMat(int w, int h, int c, size_t elemsize, int elempack, VkAllocator* allocator);
    // copy
    VkImageMat(const VkImageMat& m);
    // external vec
    VkImageMat(int w, VkImageMemory* data, size_t elemsize, VkAllocator* allocator);
    // external image
    VkImageMat(int w, int h, VkImageMemory* data, size_t elemsize, VkAllocator* allocator);
    // external dim
    VkImageMat(int w, int h, int c, VkImageMemory* data, size_t elemsize, VkAllocator* allocator);
    // external packed vec
    VkImageMat(int w, VkImageMemory* data, size_t elemsize, int elempack, VkAllocator* allocator);
    // external packed image
    VkImageMat(int w, int h, VkImageMemory* data, size_t elemsize, int elempack, VkAllocator* allocator);
    // external packed dim
    VkImageMat(int w, int h, int c, VkImageMemory* data, size_t elemsize, int elempack, VkAllocator* allocator);
    // release
    ~VkImageMat() {};
    // assign
    VkImageMat& operator=(const VkImageMat& m);
    // allocate vec
    void create(int w, size_t elemsize, VkAllocator* allocator);
    // allocate image
    void create(int w, int h, size_t elemsize, VkAllocator* allocator);
    // allocate dim
    void create(int w, int h, int c, size_t elemsize, VkAllocator* allocator);
    // allocate packed vec
    void create(int w, size_t elemsize, int elempack, VkAllocator* allocator);
    // allocate packed image
    void create(int w, int h, size_t elemsize, int elempack, VkAllocator* allocator);
    // allocate packed dim
    void create(int w, int h, int c, size_t elemsize, int elempack, VkAllocator* allocator);
    // allocate like
    void create_like(const ImMat& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkImageMat& im, VkAllocator* allocator);

    // mapped
    ImMat mapped() const;
    void* mapped_ptr() const;

    // low-level reference
    VkImage image() const;
    VkImageView imageview() const;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkImageMat Class
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
inline VkImageMat::VkImageMat()
    : ImMat()
{
    device = IM_DD_VULKAN_IMAGE;
}

inline VkImageMat::VkImageMat(int _w, size_t _elemsize, VkAllocator* _allocator)
    : ImMat()
{
    create(_w, _elemsize, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
    : ImMat()
{
    create(_w, _h, _elemsize, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
    : ImMat()
{
    create(_w, _h, _c, _elemsize, _allocator);
}

inline VkImageMat::VkImageMat(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat()
{
    create(_w, _elemsize, _elempack, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat()
{
    create(_w, _h, _elemsize, _elempack, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat()
{
    create(_w, _h, _c, _elemsize, _elempack, _allocator);
}

inline VkImageMat::VkImageMat(const VkImageMat& m)
    : ImMat(m)
{
}

inline VkImageMat::VkImageMat(int _w, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(_w, _data, _elemsize, (Allocator*)_allocator)
{
    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline VkImageMat::VkImageMat(int _w, int _h, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(_w, _h, _data, _elemsize, (Allocator*)_allocator)
{
    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(_w, _h, _c, _data, _elemsize, (Allocator*)_allocator)
{
    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline VkImageMat::VkImageMat(int _w, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(_w, _data, _elemsize, (Allocator*)_allocator)
{
    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline VkImageMat::VkImageMat(int _w, int _h, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(_w, _h, _data, _elemsize, (Allocator*)_allocator)
{
    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(_w, _h, _c, _data, _elemsize, (Allocator*)_allocator)
{
    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline VkImageMat& VkImageMat::operator=(const VkImageMat& m)
{
    if (this == &m)
        return *this;

    if (m.refcount) IM_XADD(m.refcount, 1);

    release();

    data = m.data;
    refcount = m.refcount;
    elemsize = m.elemsize;
    elempack = m.elempack;
    allocator = m.allocator;

    dims = m.dims;
    w = m.w;
    h = m.h;
    c = m.c;

    cstep = m.cstep;

    type = m.type;
    color_space = m.color_space;
    color_format = m.color_format;
    color_range = m.color_range;

    device = m.device;
    device_number = m.device_number;
    time_stamp = m.time_stamp;
    duration = m.duration;
    depth = m.depth;

    return *this;
}

inline void VkImageMat::create(int _w, size_t _elemsize, VkAllocator* _allocator)
{
    if (dims == 1 && w == _w && elemsize == _elemsize && elempack == 1 && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
    depth = _elemsize == 1 ? 8 : _elemsize == 2 ? 16 : 32;

    cstep = w;

    if (total() > 0)
    {
        data = (VkImageMemory *)_allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }

    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline void VkImageMat::create(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
{
    if (dims == 2 && w == _w && h == _h && elemsize == _elemsize && elempack == 1 && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
    depth = _elemsize == 1 ? 8 : _elemsize == 2 ? 16 : 32;

    cstep = (size_t)w * h;

    if (total() > 0)
    {
        data = (VkImageMemory *)_allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }

    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline void VkImageMat::create(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
{
    if (dims == 3 && w == _w && h == _h && c == _c && elemsize == _elemsize && elempack == 1 && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = c == 1 ? IM_CF_GRAY : c == 3 ? IM_CF_BGR : IM_CF_ABGR;
    color_range = IM_CR_FULL_RANGE;
    depth = _elemsize == 1 ? 8 : _elemsize == 2 ? 16 : 32;

    cstep = Im_AlignSize((size_t)w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        data = (VkImageMemory *)_allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }

    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline void VkImageMat::create(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
{
    if (dims == 1 && w == _w && elemsize == _elemsize && elempack == _elempack && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;
    allocator = _allocator;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
    depth = _elemsize == 1 ? 8 : _elemsize == 2 ? 16 : 32;

    cstep = w;

    if (total() > 0)
    {
        data = (VkImageMemory *)_allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }

    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline void VkImageMat::create(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
{
    if (dims == 2 && w == _w && h == _h && elemsize == _elemsize && elempack == _elempack && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;
    allocator = _allocator;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
    depth = _elemsize == 1 ? 8 : _elemsize == 2 ? 16 : 32;

    cstep = (size_t)w * h;

    if (total() > 0)
    {
        data = (VkImageMemory *)_allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }

    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline void VkImageMat::create(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
{
    if (dims == 3 && w == _w && h == _h && c == _c && elemsize == _elemsize && elempack == _elempack && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;
    allocator = _allocator;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = c == 1 ? IM_CF_GRAY : c == 3 ? IM_CF_BGR : IM_CF_ABGR;
    color_range = IM_CR_FULL_RANGE;
    depth = _elemsize == 1 ? 8 : _elemsize == 2 ? 16 : 32;

    cstep = Im_AlignSize((size_t)w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        data = (VkImageMemory *)_allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }

    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline void VkImageMat::create_like(const ImMat& m, VkAllocator* _allocator)
{
    int _dims = m.dims;
    if (_dims == 1)
        create(m.w, m.elemsize, m.elempack, _allocator);
    if (_dims == 2)
        create(m.w, m.h, m.elemsize, m.elempack, _allocator);
    if (_dims == 3)
        create(m.w, m.h, m.c, m.elemsize, m.elempack, _allocator);
    type = m.type;
    color_space = m.color_space;
    color_format = m.color_format;
    color_range = m.color_range;
    time_stamp = m.time_stamp;
    duration = m.duration;
    depth = m.depth;
    allocator = _allocator;
    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline void VkImageMat::create_like(const VkImageMat& m, VkAllocator* _allocator)
{
    int _dims = m.dims;
    if (_dims == 1)
        create(m.w, m.elemsize, m.elempack, _allocator);
    if (_dims == 2)
        create(m.w, m.h, m.elemsize, m.elempack, _allocator);
    if (_dims == 3)
        create(m.w, m.h, m.c, m.elemsize, m.elempack, _allocator);
    type = m.type;
    color_space = m.color_space;
    color_format = m.color_format;
    color_range = m.color_range;
    time_stamp = m.time_stamp;
    duration = m.duration;
    depth = m.depth;
    allocator = _allocator;
    device = IM_DD_VULKAN_IMAGE;
    device_number = _allocator ? _allocator->getDeviceIndex() : -1;
}

inline ImMat VkImageMat::mapped() const
{
    VkImageMemory* _data = (VkImageMemory*)data;
    VkAllocator* _allocator = (VkAllocator*)allocator;
    if (!_allocator->mappable || !_data->mapped_ptr)
        return ImMat();

    if (dims == 1)
        return ImMat(w, mapped_ptr(), elemsize, elempack, 0);

    if (dims == 2)
        return ImMat(w, h, mapped_ptr(), elemsize, elempack, 0);

    if (dims == 3)
        return ImMat(w, h, c, mapped_ptr(), elemsize, elempack, 0);

    return ImMat();
}

inline void* VkImageMat::mapped_ptr() const
{
    VkImageMemory* _data = (VkImageMemory*)data;
    VkAllocator* _allocator = (VkAllocator*)allocator;
    if (!_allocator->mappable || !_data->mapped_ptr)
        return 0;

    return (unsigned char*)_data->mapped_ptr + _data->bind_offset;
}

inline VkImage VkImageMat::image() const
{
    VkImageMemory* _data = (VkImageMemory*)data;
    return _data->image;
}

inline VkImageView VkImageMat::imageview() const
{
    VkImageMemory* _data = (VkImageMemory*)data;
    return _data->imageview;
}

} // namespace ImVulkan 

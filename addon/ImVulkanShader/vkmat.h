#pragma once
#include <imgui_mat.h>
#include "allocator.h"
#include "option.h"
#include "platform.h"
#include <vulkan/vulkan.h>
#include <string.h>

namespace ImGui
{
class VkImageMat;
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkMat Class define
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class VkMat final : public ImMat
{
public:
    // empty
    VkMat();
    // vec
    VkMat(int w, size_t elemsize, VkAllocator* allocator);
    // image
    VkMat(int w, int h, size_t elemsize, VkAllocator* allocator);
    // dim
    VkMat(int w, int h, int c, size_t elemsize, VkAllocator* allocator);
    // packed vec
    VkMat(int w, size_t elemsize, int elempack, VkAllocator* allocator);
    // packed image
    VkMat(int w, int h, size_t elemsize, int elempack, VkAllocator* allocator);
    // packed dim
    VkMat(int w, int h, int c, size_t elemsize, int elempack, VkAllocator* allocator);
    // copy
    VkMat(const VkMat& m);
    // external vec
    VkMat(int w, VkBufferMemory* data, size_t elemsize, VkAllocator* allocator);
    // external image
    VkMat(int w, int h, VkBufferMemory* data, size_t elemsize, VkAllocator* allocator);
    // external dim
    VkMat(int w, int h, int c, VkBufferMemory* data, size_t elemsize, VkAllocator* allocator);
    // external packed vec
    VkMat(int w, VkBufferMemory* data, size_t elemsize, int elempack, VkAllocator* allocator);
    // external packed image
    VkMat(int w, int h, VkBufferMemory* data, size_t elemsize, int elempack, VkAllocator* allocator);
    // external packed dim
    VkMat(int w, int h, int c, VkBufferMemory* data, size_t elemsize, int elempack, VkAllocator* allocator);
    // release
    ~VkMat();
    // assign
    VkMat& operator=(const VkMat& m);
    VkMat& operator=(const ImMat& m);
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
    // allocate vec with type
    void create_type(int w, ImMatDataType t, VkAllocator* allocator);
    // allocate image with type
    void create_type(int w, int h, ImMatDataType t, VkAllocator* allocator);
    // allocate dim with type
    void create_type(int w, int h, int c, ImMatDataType t, VkAllocator* allocator);
    // allocate like
    void create_like(const ImMat& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkMat& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkImageMat& im, VkAllocator* allocator);

    // mapped
    ImMat mapped() const;
    void* mapped_ptr() const;

    // refcount--
    void release();

    // empty check
    bool empty() const;

    // low-level reference
    VkBuffer buffer() const;
    size_t buffer_offset() const;
    size_t buffer_capacity() const;

    // device buffer
    VkBufferMemory* buffer_data;

    // the allocator
    VkAllocator* allocator;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkImageMat Class define
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class VkImageMat final : public ImMat
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
    ~VkImageMat();
    // assign
    VkImageMat& operator=(const VkImageMat& m);
    VkImageMat& operator=(const ImMat& m);
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
    void create_like(const VkMat& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkImageMat& im, VkAllocator* allocator);

    // mapped
    ImMat mapped() const;
    void* mapped_ptr() const;

    // refcount--
    void release();

    // empty check
    bool empty() const;

    // low-level reference
    VkImage image() const;
    VkImageView imageview() const;

    // device image
    VkImageMemory* image_data;

    // the allocator
    VkAllocator* allocator;
};

// type for vulkan specialization constant and push constant
union vk_specialization_type
{
    int i;
    float f;
    uint32_t u32;
};
union vk_constant_type
{
    int i;
    float f;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkMat Class
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
inline VkMat::VkMat()
    : ImMat(), buffer_data(0), allocator(0)
{
    device = IMMAT_DD_VULKAN;
}

inline VkMat::VkMat(int _w, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(), buffer_data(0), allocator(0)
{
    create(_w, _elemsize, _allocator);
}

inline VkMat::VkMat(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(), buffer_data(0), allocator(0)
{
    create(_w, _h, _elemsize, _allocator);
}

inline VkMat::VkMat(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(), buffer_data(0), allocator(0)
{
    create(_w, _h, _c, _elemsize, _allocator);
}

inline VkMat::VkMat(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(), buffer_data(0), allocator(0)
{
    create(_w, _elemsize, _elempack, _allocator);
}

inline VkMat::VkMat(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(), buffer_data(0), allocator(0)
{
    create(_w, _h, _elemsize, _elempack, _allocator);
}

inline VkMat::VkMat(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(), buffer_data(0), allocator(0)
{
    create(_w, _h, _c, _elemsize, _elempack, _allocator);
}

inline VkMat::VkMat(const VkMat& m)
    : ImMat(m), buffer_data(m.buffer_data), allocator(m.allocator)
{
    device = m.device;
    device_number = m.device_number;
    data = m.buffer_data;
    user_data = m.allocator;
}

inline VkMat::VkMat(int _w, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(_w, nullptr, _elemsize), buffer_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkMat::VkMat(int _w, int _h, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(_w, _h, nullptr, _elemsize), buffer_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkMat::VkMat(int _w, int _h, int _c, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(_w, _h, _c, nullptr, _elemsize), buffer_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkMat::VkMat(int _w, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(_w, nullptr, _elemsize, _elempack), buffer_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkMat::VkMat(int _w, int _h, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(_w, _h, nullptr, _elemsize, _elempack), buffer_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkMat::VkMat(int _w, int _h, int _c, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(_w, _h, _c, nullptr, _elemsize, _elempack), buffer_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkMat::~VkMat()
{
    release();
}

inline VkMat& VkMat::operator=(const VkMat& m)
{
    if (this == &m)
        return *this;

    if (m.refcount)
        IM_XADD(m.refcount, 1);

    release();

    buffer_data = m.buffer_data;
    data = buffer_data;
    user_data = allocator;
    device = m.device;
    device_number = m.device_number;
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

    return *this;
}

inline VkMat& VkMat::operator=(const ImMat& m)
{
    if (m.device != IMMAT_DD_VULKAN)
        return *this;

    if (m.refcount)
        IM_XADD(m.refcount, 1);

    release();

    buffer_data = (VkBufferMemory*)m.data;
    data = buffer_data;
    device = m.device;
    device_number = m.device_number;
    refcount = m.refcount;
    elemsize = m.elemsize;
    elempack = m.elempack;
    allocator = (VkAllocator*)m.user_data;
    user_data = allocator;

    dims = m.dims;
    w = m.w;
    h = m.h;
    c = m.c;

    cstep = m.cstep;

    type = m.type;
    color_space = m.color_space;
    color_format = m.color_format;
    color_range = m.color_range;

    return *this;
}

inline void VkMat::create(int _w, size_t _elemsize, VkAllocator* _allocator)
{
    if (dims == 1 && w == _w && elemsize == _elemsize && elempack == 1 && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = w;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        buffer_data = allocator->fastMalloc(totalsize);
        data = buffer_data;
        refcount = (int*)((unsigned char*)buffer_data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
{
    if (dims == 2 && w == _w && h == _h && elemsize == _elemsize && elempack == 1 && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = w * h;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        buffer_data = allocator->fastMalloc(totalsize);
        data = buffer_data;

        refcount = (int*)((unsigned char*)buffer_data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
{
    if (dims == 3 && w == _w && h == _h && c == _c && elemsize == _elemsize && elempack == 1 && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;

    cstep = Im_AlignSize(w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        buffer_data = allocator->fastMalloc(totalsize);
        data = buffer_data;

        refcount = (int*)((unsigned char*)buffer_data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
{
    if (dims == 1 && w == _w && elemsize == _elemsize && elempack == _elempack && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = w;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        buffer_data = allocator->fastMalloc(totalsize);
        data = buffer_data;

        refcount = (int*)((unsigned char*)buffer_data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
{
    if (dims == 2 && w == _w && h == _h && elemsize == _elemsize && elempack == _elempack && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = w * h;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        buffer_data = allocator->fastMalloc(totalsize);
        data = buffer_data;

        refcount = (int*)((unsigned char*)buffer_data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
{
    if (dims == 3 && w == _w && h == _h && c == _c && elemsize == _elemsize && elempack == _elempack && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;

    cstep = Im_AlignSize(w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        buffer_data = allocator->fastMalloc(totalsize);
        data = buffer_data;

        refcount = (int*)((unsigned char*)buffer_data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create_type(int _w, ImMatDataType _t, VkAllocator* _allocator)
{
    size_t _elemsize = IM_ESIZE(_t);
    if (dims == 1 && w == _w && elemsize == _elemsize && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 1;
    w = _w;
    h = 1;
    c = 1;

    cstep = w;
    type = _t;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        buffer_data = allocator->fastMalloc(totalsize);
        data = buffer_data;

        refcount = (int*)((unsigned char*)buffer_data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create_type(int _w, int _h, ImMatDataType _t, VkAllocator* _allocator)
{
    size_t _elemsize = IM_ESIZE(_t);
    if (dims == 2 && w == _w && h == _h && elemsize == _elemsize && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 2;
    w = _w;
    h = _h;
    c = 1;

    cstep = w * h;
    type = _t;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        buffer_data = allocator->fastMalloc(totalsize);
        data = buffer_data;

        refcount = (int*)((unsigned char*)buffer_data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create_type(int _w, int _h, int _c, ImMatDataType _t, VkAllocator* _allocator)
{
    size_t _elemsize = IM_ESIZE(_t);
    if (dims == 3 && w == _w && h == _h && c == _c && elemsize == _elemsize && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 3;
    w = _w;
    h = _h;
    c = _c;

    cstep = Im_AlignSize(w * h * elemsize, 16) / elemsize;
    type = _t;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        buffer_data = allocator->fastMalloc(totalsize);
        data = buffer_data;

        refcount = (int*)((unsigned char*)buffer_data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create_like(const ImMat& m, VkAllocator* _allocator)
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
}

inline void VkMat::create_like(const VkMat& m, VkAllocator* _allocator)
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
}

inline void VkMat::create_like(const VkImageMat& im, VkAllocator* _allocator)
{
    int _dims = im.dims;
    if (_dims == 1)
        create(im.w, im.elemsize, im.elempack, _allocator);
    if (_dims == 2)
        create(im.w, im.h, im.elemsize, im.elempack, _allocator);
    if (_dims == 3)
        create(im.w, im.h, im.c, im.elemsize, im.elempack, _allocator);
    type = im.type;
    color_space = im.color_space;
    color_format = im.color_format;
    color_range = im.color_range;
}

inline ImMat VkMat::mapped() const
{
    if (!allocator->mappable)
        return ImMat();

    if (dims == 1)
        return ImMat(w, mapped_ptr(), elemsize, elempack);

    if (dims == 2)
        return ImMat(w, h, mapped_ptr(), elemsize, elempack);

    if (dims == 3)
        return ImMat(w, h, c, mapped_ptr(), elemsize, elempack);

    return ImMat();
}

inline void* VkMat::mapped_ptr() const
{
    if (!allocator)
        return NULL;
    
    if (!allocator->mappable)
        return NULL;

    return (unsigned char*)buffer_data->mapped_ptr + buffer_data->offset;
}

inline void VkMat::release()
{
    if (refcount && IM_XADD(refcount, -1) == 1)
    {
        if (allocator && buffer_data)
        {
            allocator->fastFree(buffer_data);
        }
    }

    buffer_data = 0;
    data = 0;
    user_data = 0;

    elemsize = 0;
    elempack = 0;

    dims = 0;
    w = 0;
    h = 0;
    c = 0;
    type = IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = 0;

    refcount = 0;
}

inline bool VkMat::empty() const
{
    return buffer_data == 0 || total() == 0;
}

inline VkBuffer VkMat::buffer() const
{
    return buffer_data->buffer;
}

inline size_t VkMat::buffer_offset() const
{
    return buffer_data->offset;
}

inline size_t VkMat::buffer_capacity() const
{
    return buffer_data->capacity;
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkImageMat Class
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
inline VkImageMat::VkImageMat()
    : ImMat(), image_data(0), allocator(0)
{
    device = IMMAT_DD_VULKAN;
}

inline VkImageMat::VkImageMat(int _w, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(), image_data(0), allocator(0)
{
    create(_w, _elemsize, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(), image_data(0), allocator(0)
{
    create(_w, _h, _elemsize, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(), image_data(0), allocator(0)
{
    create(_w, _h, _c, _elemsize, _allocator);
}

inline VkImageMat::VkImageMat(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(), image_data(0), allocator(0)
{
    create(_w, _elemsize, _elempack, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(), image_data(0), allocator(0)
{
    create(_w, _h, _elemsize, _elempack, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(), image_data(0), allocator(0)
{
    create(_w, _h, _c, _elemsize, _elempack, _allocator);
}

inline VkImageMat::VkImageMat(const VkImageMat& m)
    : ImMat(m), image_data(m.image_data), allocator(m.allocator)
{
    device = m.device;
    device_number = m.device_number;
    data = m.image_data;
    user_data = m.allocator;
}

inline VkImageMat::VkImageMat(int _w, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(_w, nullptr, _elemsize), image_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkImageMat::VkImageMat(int _w, int _h, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(_w, _h, nullptr, _elemsize), image_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : ImMat(_w, _h, _c, nullptr, _elemsize), image_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkImageMat::VkImageMat(int _w, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(_w, nullptr, _elemsize, _elempack), image_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkImageMat::VkImageMat(int _w, int _h, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(_w, _h, nullptr, _elemsize, _elempack), image_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : ImMat(_w, _h, _c, nullptr, _elemsize, _elempack), image_data(_data), allocator(_allocator)
{
    device = IMMAT_DD_VULKAN;
    data = _data;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();
}

inline VkImageMat::~VkImageMat()
{
    release();
}

inline VkImageMat& VkImageMat::operator=(const VkImageMat& m)
{
    if (this == &m)
        return *this;

    if (m.refcount) IM_XADD(m.refcount, 1);

    release();

    image_data = m.image_data;
    data = image_data;
    user_data = allocator;
    device = m.device;
    device_number = m.device_number;
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
    color_format = m.color_format;
    color_space = m.color_space;
    color_range = m.color_range;

    return *this;
}

inline VkImageMat& VkImageMat::operator=(const ImMat& m)
{
    if (m.device != IMMAT_DD_VULKAN)
        return *this;

    if (m.refcount) IM_XADD(m.refcount, 1);

    release();

    image_data = (VkImageMemory*)m.data;
    data = image_data;
    device = m.device;
    device_number = m.device_number;
    refcount = m.refcount;
    elemsize = m.elemsize;
    elempack = m.elempack;
    allocator = (VkAllocator*)m.user_data;
    user_data = allocator;

    dims = m.dims;
    w = m.w;
    h = m.h;
    c = m.c;

    cstep = m.cstep;

    type = m.type;
    color_format = m.color_format;
    color_space = m.color_space;
    color_range = m.color_range;

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
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 1;
    w = _w;
    h = 1;
    c = 1;

    cstep = w;

    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        image_data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!image_data)
            return;
        data = image_data;
        refcount = (int*)((unsigned char*)image_data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageMat::create(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
{
    if (dims == 2 && w == _w && h == _h && elemsize == _elemsize && elempack == 1 && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = w * h;

    if (total() > 0)
    {
        image_data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!image_data)
            return;
        data = image_data;

        refcount = (int*)((unsigned char*)image_data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageMat::create(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
{
    if (dims == 3 && w == _w && h == _h && c == _c && elemsize == _elemsize && elempack == 1 && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;

    cstep = Im_AlignSize(w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        image_data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!image_data)
            return;
        data = image_data;

        refcount = (int*)((unsigned char*)image_data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageMat::create(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
{
    if (dims == 1 && w == _w && elemsize == _elemsize && elempack == _elempack && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = w;

    if (total() > 0)
    {
        image_data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!image_data)
            return;
        data = image_data;

        refcount = (int*)((unsigned char*)image_data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageMat::create(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
{
    if (dims == 2 && w == _w && h == _h && elemsize == _elemsize && elempack == _elempack && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = w * h;

    if (total() > 0)
    {
        image_data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!image_data)
            return;
        data = image_data;

        refcount = (int*)((unsigned char*)image_data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageMat::create(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
{
    if (dims == 3 && w == _w && h == _h && c == _c && elemsize == _elemsize && elempack == _elempack && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;
    allocator = _allocator;
    device = IMMAT_DD_VULKAN;
    user_data = allocator;
    if (allocator) device_number = allocator->getDeviceIndex();

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;

    cstep = Im_AlignSize(w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        image_data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!image_data)
            return;
        data = image_data;

        refcount = (int*)((unsigned char*)image_data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }
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
}

inline void VkImageMat::create_like(const VkMat& m, VkAllocator* _allocator)
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
}

inline ImMat VkImageMat::mapped() const
{
    if (!allocator->mappable || !image_data->mapped_ptr)
        return ImMat();

    if (dims == 1)
        return ImMat(w, mapped_ptr(), elemsize, elempack);

    if (dims == 2)
        return ImMat(w, h, mapped_ptr(), elemsize, elempack);

    if (dims == 3)
        return ImMat(w, h, c, mapped_ptr(), elemsize, elempack);

    return ImMat();
}

inline void* VkImageMat::mapped_ptr() const
{
    if (!allocator->mappable || !image_data->mapped_ptr)
        return 0;

    return (unsigned char*)image_data->mapped_ptr + image_data->bind_offset;
}

inline void VkImageMat::release()
{
    if (refcount && IM_XADD(refcount, -1) == 1)
    {
        if (allocator && image_data)
        {
            allocator->fastFree(image_data);
        }
    }

    image_data = 0;
    data = 0;
    user_data = 0;

    elemsize = 0;
    elempack = 0;

    dims = 0;
    w = 0;
    h = 0;
    c = 0;
    type = IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = 0;

    refcount = 0;
}

inline bool VkImageMat::empty() const
{
    return image_data == 0 || total() == 0;
}

inline VkImage VkImageMat::image() const
{
    return image_data->image;
}

inline VkImageView VkImageMat::imageview() const
{
    return image_data->imageview;
}

extern const ImMat * color_table[2][2][4];

} // namespace ImGui

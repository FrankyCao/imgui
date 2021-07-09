#pragma once
#include "allocator.h"
#include "option.h"
#include "platform.h"
#include <vulkan/vulkan.h>
#include <string.h>
#include <imgui_mat.h>

namespace ImGui 
{
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
    void create_type(int w, ImDataType t, VkAllocator* allocator);
    // allocate image with type
    void create_type(int w, int h, ImDataType t, VkAllocator* allocator);
    // allocate dim with type
    void create_type(int w, int h, int c, ImDataType t, VkAllocator* allocator);
    // allocate like
    void create_like(const ImMat& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkMat& m, VkAllocator* allocator);

    // mapped
    ImMat mapped() const;
    void* mapped_ptr() const;

    // refcount--
    void release();

    // low-level reference
    VkBuffer buffer() const;
    size_t buffer_offset() const;
    size_t buffer_capacity() const;

    // device buffer
    VkBufferMemory* data;

    // the allocator
    VkAllocator* allocator;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkMat Class
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
inline VkMat::VkMat()
    : data(0), ImMat()
{
    device = IM_DD_VULKAN;
}

inline VkMat::VkMat(int _w, size_t _elemsize, VkAllocator* _allocator)
    : data(0), ImMat()
{
    create(_w, _elemsize, _allocator);
}

inline VkMat::VkMat(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
    : data(0), ImMat()
{
    create(_w, _h, _elemsize, _allocator);
}

inline VkMat::VkMat(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
    : data(0), ImMat()
{
    create(_w, _h, _c, _elemsize, _allocator);
}

inline VkMat::VkMat(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), ImMat()
{
    create(_w, _elemsize, _elempack, _allocator);
}

inline VkMat::VkMat(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), ImMat()
{
    create(_w, _h, _elemsize, _elempack, _allocator);
}

inline VkMat::VkMat(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), ImMat()
{
    create(_w, _h, _c, _elemsize, _elempack, _allocator);
}

inline VkMat::VkMat(const VkMat& m)
    : data(m.data), ImMat(m)
{
}

inline VkMat::VkMat(int _w, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), ImMat(_w, _data, _elemsize, (Allocator *)_allocator)
{
    device = IM_DD_VULKAN;
    device_number = _allocator ? _allocator->getDeviceIndex() : 0;
}

inline VkMat::VkMat(int _w, int _h, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), ImMat(_w, _h, _data, _elemsize, (Allocator *)_allocator)
{
    device = IM_DD_VULKAN;
    device_number = _allocator ? _allocator->getDeviceIndex() : 0;
}

inline VkMat::VkMat(int _w, int _h, int _c, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), ImMat(_w, _h, _c, _data, _elemsize, (Allocator *)_allocator)
{
    device = IM_DD_VULKAN;
    device_number = _allocator ? _allocator->getDeviceIndex() : 0;
}

inline VkMat::VkMat(int _w, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), ImMat(_w, _data, _elemsize, _elempack, (Allocator *)_allocator)
{
    device = IM_DD_VULKAN;
    device_number = _allocator ? _allocator->getDeviceIndex() : 0;
}

inline VkMat::VkMat(int _w, int _h, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), ImMat(_w, _h, _data, _elemsize, _elempack, (Allocator *)_allocator)
{
    device = IM_DD_VULKAN;
    device_number = _allocator ? _allocator->getDeviceIndex() : 0;
}

inline VkMat::VkMat(int _w, int _h, int _c, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), ImMat(_w, _h, _c, _data, _elemsize, _elempack, (Allocator *)_allocator)
{
    device = IM_DD_VULKAN;
    device_number = _allocator ? _allocator->getDeviceIndex() : 0;
}

inline VkMat::~VkMat()
{
    release();
}

inline VkMat& VkMat::operator=(const VkMat& m)
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

    ImMat::data = m.data;
    ImMat::allocator = (Allocator *)m.allocator;

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

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;

    cstep = w;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }

    ImMat::data = data;
    ImMat::allocator = (Allocator *)allocator;
}

inline void VkMat::create(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
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

    cstep = (size_t)w * h;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }

    ImMat::data = data;
    ImMat::allocator = (Allocator *)allocator;
}

inline void VkMat::create(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
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
    color_format = c == 1 ? IM_CF_GRAY : c == 3 ? IM_CF_RGB : IM_CF_ARGB;
    color_range = IM_CR_FULL_RANGE;

    cstep = Im_AlignSize((size_t)w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }

    ImMat::data = data;
    ImMat::allocator = (Allocator *)allocator;
}

inline void VkMat::create(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
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

    cstep = w;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }

    ImMat::data = data;
    ImMat::allocator = (Allocator *)allocator;
}

inline void VkMat::create(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
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

    cstep = (size_t)w * h;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }

    ImMat::data = data;
    ImMat::allocator = (Allocator *)allocator;
}

inline void VkMat::create(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
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
    color_format = c == 1 ? IM_CF_GRAY : c == 3 ? IM_CF_RGB : IM_CF_ARGB;
    color_range = IM_CR_FULL_RANGE;

    cstep = Im_AlignSize((size_t)w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }

    ImMat::data = data;
    ImMat::allocator = (Allocator *)allocator;
}

inline void VkMat::create_type(int _w, ImDataType _t, VkAllocator* _allocator)
{
    if (dims == 1 && w == _w && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = IM_ESIZE(_t);
    elempack = 1;
    allocator = _allocator;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;

    cstep = w;
    type = _t;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }

    ImMat::data = data;
    ImMat::allocator = (Allocator *)allocator;
}

inline void VkMat::create_type(int _w, int _h, ImDataType _t, VkAllocator* _allocator)
{
    if (dims == 2 && w == _w && h == _h && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = IM_ESIZE(_t);
    elempack = 1;
    allocator = _allocator;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;

    cstep = (size_t)w * h;
    type = _t;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }

    ImMat::data = data;
    ImMat::allocator = (Allocator *)allocator;
}

inline void VkMat::create_type(int _w, int _h, int _c, ImDataType _t, VkAllocator* _allocator)
{
    if (dims == 3 && w == _w && h == _h && c == _c && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = IM_ESIZE(_t);
    elempack = 1;
    allocator = _allocator;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;

    cstep = Im_AlignSize((size_t)w * h * elemsize, 16) / elemsize;
    type = _t;
    color_space = IM_CS_SRGB;
    color_format = c == 1 ? IM_CF_GRAY : c == 3 ? IM_CF_RGB : IM_CF_ARGB;
    color_range = IM_CR_FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = Im_AlignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }

    ImMat::data = data;
    ImMat::allocator = (Allocator *)allocator;
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
    time_stamp = m.time_stamp;
    device = IM_DD_VULKAN;
    device_number = allocator ? allocator->getDeviceIndex() : 0;
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
    time_stamp = m.time_stamp;
    device = IM_DD_VULKAN;
    device_number = allocator ? allocator->getDeviceIndex() : 0;
}

inline ImMat VkMat::mapped() const
{
    if (!allocator->mappable)
        return ImMat();

    if (dims == 1)
        return ImMat(w, mapped_ptr(), elemsize, elempack, 0);

    if (dims == 2)
        return ImMat(w, h, mapped_ptr(), elemsize, elempack, 0);

    if (dims == 3)
        return ImMat(w, h, c, mapped_ptr(), elemsize, elempack, 0);

    return ImMat();
}

inline void* VkMat::mapped_ptr() const
{
    if (!allocator)
        return NULL;
    
    if (!allocator->mappable)
        return NULL;

    return (unsigned char*)data->mapped_ptr + data->offset;
}

inline void VkMat::release()
{
    if (refcount && IM_XADD(refcount, -1) == 1)
    {
        if (allocator && data)
        {
            allocator->fastFree(data);
        }
    }

    data = 0;

    elemsize = 0;
    elempack = 0;

    dims = 0;
    w = 0;
    h = 0;
    c = 0;

    cstep = 0;

    refcount = 0;

    type = IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;

    device = IM_DD_VULKAN;
    device_number = 0;
    time_stamp = NAN;
}

inline VkBuffer VkMat::buffer() const
{
    return data->buffer;
}

inline size_t VkMat::buffer_offset() const
{
    return data->offset;
}

inline size_t VkMat::buffer_capacity() const
{
    return data->capacity;
}

} // namespace ImGui

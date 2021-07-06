#pragma once
#include <imgui_mat.h>
#include "allocator.h"
#include "option.h"
#include "platform.h"
#include <vulkan/vulkan.h>
#include <string.h>

namespace ImVulkan 
{
class VkImageMat;
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkMat Class define
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class VkMat
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
    void create_type(int w, ImGui::ImMatDataType t, VkAllocator* allocator);
    // allocate image with type
    void create_type(int w, int h, ImGui::ImMatDataType t, VkAllocator* allocator);
    // allocate dim with type
    void create_type(int w, int h, int c, ImGui::ImMatDataType t, VkAllocator* allocator);
    // allocate like
    void create_like(const ImGui::ImMat& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkMat& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkImageMat& im, VkAllocator* allocator);

    // mapped
    ImGui::ImMat mapped() const;
    void* mapped_ptr() const;

    // refcount++
    void addref();
    // refcount--
    void release();

    bool empty() const;
    size_t total() const;

    // bits per element
    int elembits() const;

    // shape only
    ImGui::ImMat shape() const;

    // low-level reference
    VkBuffer buffer() const;
    size_t buffer_offset() const;
    size_t buffer_capacity() const;

    // device buffer
    VkBufferMemory* data;

    // pointer to the reference counter
    // when points to user-allocated data, the pointer is NULL
    int* refcount;

    // element size in bytes
    // 4 = float32/int32
    // 2 = float16
    // 1 = int8/uint8
    // 0 = empty
    size_t elemsize;

    // packed count inside element
    // c/1-h-w-1  h/1-w-1  w/1-1  scalar
    // c/4-h-w-4  h/4-w-4  w/4-4  sse/neon
    // c/8-h-w-8  h/8-w-8  w/8-8  avx/fp16
    int elempack;

    // the allocator
    VkAllocator* allocator;

    // the dimension rank
    int dims;

    int w;
    int h;
    int c;

    size_t cstep;

    // type
    // 0 = INT8/UINT8
    // 1 = INT16/UINT16
    // 2 = INT32/UINT32
    // 3 = INT64/UINT64
    // 4 = FLOAT16
    // 5 = FLOAT32
    // 6 = FLOAT64
    ImGui::ImMatDataType type;

    // color
    // 0 = SRGB
    // 1 = BT601
    // 2 = BT709
    // 3 = BT2020

    ImGui::ImMatColorSpace color_space;
    // format
    // 0 = GRAY
    // 1 = BGR
    // 2 = ABGR
    // 3 = RGB
    // 4 = ARGB
    // 5 = YUV420
    // 6 = YUV422
    // 7 = YUV444
    // 8 = YUVA
    // 9 = NV12

    ImGui::ImMatColorFormat color_format;

    // range
    // 0 = FULL_RANGE
    // 1 = NARROW_RANGE
    ImGui::ImMatColorRange color_range;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkImageMat Class define
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class VkImageMat
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
    void create_like(const ImGui::ImMat& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkMat& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkImageMat& im, VkAllocator* allocator);

    // mapped
    ImGui::ImMat mapped() const;
    void* mapped_ptr() const;

    // refcount++
    void addref();
    // refcount--
    void release();

    bool empty() const;
    size_t total() const;

    // bits per element
    int elembits() const;

    // shape only
    ImGui::ImMat shape() const;

    // low-level reference
    VkImage image() const;
    VkImageView imageview() const;

    // device image
    VkImageMemory* data;

    // pointer to the reference counter
    // when points to user-allocated data, the pointer is NULL
    int* refcount;

    // element size in bytes
    // 4 = float32/int32
    // 2 = float16
    // 1 = int8/uint8
    // 0 = empty
    size_t elemsize;

    // packed count inside element
    // c/1-h-w-1  h/1-w-1  w/1-1  scalar
    // c/4-h-w-4  h/4-w-4  w/4-4  sse/neon
    // c/8-h-w-8  h/8-w-8  w/8-8  avx/fp16
    int elempack;

    // the allocator
    VkAllocator* allocator;

    // the dimension rank
    int dims;

    int w;
    int h;
    int c;

    // type
    // 0 = INT8/UINT8
    // 1 = INT16/UINT16
    // 2 = INT32/UINT32
    // 3 = INT64/UINT64
    // 4 = FLOAT16
    // 5 = FLOAT32
    // 6 = FLOAT64

    ImGui::ImMatDataType type;
    // color
    // 0 = SRGB
    // 1 = BT601
    // 2 = BT709
    // 3 = BT2020

    ImGui::ImMatColorSpace color_space;
    // format
    // 0 = GRAY
    // 1 = BGR
    // 2 = ABGR
    // 3 = RGB
    // 4 = ARGB
    // 5 = YUV420
    // 6 = YUV422
    // 7 = YUV444
    // 8 = YUVA
    // 9 = NV12
    ImGui::ImMatColorFormat color_format;

    // range
    // 0 = FULL_RANGE
    // 1 = NARROW_RANGE
    ImGui::ImMatColorRange color_range;
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
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    type = ImGui::IMMAT_FLOAT32;
    color_format = ImGui::IMMAT_GRAY;
    color_space = ImGui::IMMAT_SRGB;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkMat::VkMat(int _w, size_t _elemsize, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _elemsize, _allocator);
}

inline VkMat::VkMat(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _elemsize, _allocator);
}

inline VkMat::VkMat(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _c, _elemsize, _allocator);
}

inline VkMat::VkMat(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _elemsize, _elempack, _allocator);
}

inline VkMat::VkMat(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _elemsize, _elempack, _allocator);
}

inline VkMat::VkMat(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _c, _elemsize, _elempack, _allocator);
}

inline VkMat::VkMat(const VkMat& m)
    : data(m.data), refcount(m.refcount), elemsize(m.elemsize), elempack(m.elempack), allocator(m.allocator), dims(m.dims), w(m.w), h(m.h), c(m.c)
{
    if (refcount) IM_XADD(refcount, 1);

    cstep = m.cstep;
    type = m.type;
    color_format = m.color_format;
    color_space = m.color_space;
    color_range = m.color_range;
}

inline VkMat::VkMat(int _w, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(1), w(_w), h(1), c(1)
{
    cstep = w;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkMat::VkMat(int _w, int _h, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(2), w(_w), h(_h), c(1)
{
    cstep = w * h;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkMat::VkMat(int _w, int _h, int _c, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(3), w(_w), h(_h), c(_c)
{
    cstep = alignSize(w * h * elemsize, 16) / elemsize;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = c == 1 ? ImGui::IMMAT_GRAY : ImGui::IMMAT_RGB;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkMat::VkMat(int _w, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(1), w(_w), h(1), c(1)
{
    cstep = w;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkMat::VkMat(int _w, int _h, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(2), w(_w), h(_h), c(1)
{
    cstep = w * h;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkMat::VkMat(int _w, int _h, int _c, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(3), w(_w), h(_h), c(_c)
{
    cstep = alignSize(w * h * elemsize, 16) / elemsize;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = c == 1 ? ImGui::IMMAT_GRAY : ImGui::IMMAT_RGB;
    color_range = ImGui::IMMAT_FULL_RANGE;
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
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    cstep = w;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
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

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    cstep = w * h;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
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

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = c == 1 ? ImGui::IMMAT_GRAY : ImGui::IMMAT_RGB;
    color_range = ImGui::IMMAT_FULL_RANGE;

    cstep = alignSize(w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
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

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    cstep = w;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
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

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    cstep = w * h;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
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

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = c == 1 ? ImGui::IMMAT_GRAY : ImGui::IMMAT_RGB;
    color_range = ImGui::IMMAT_FULL_RANGE;

    cstep = alignSize(w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create_type(int _w, ImGui::ImMatDataType _t, VkAllocator* _allocator)
{
    size_t _elemsize = IM_ESIZE(_t);
    if (dims == 1 && w == _w && elemsize == _elemsize && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;

    cstep = w;
    type = _t;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create_type(int _w, int _h, ImGui::ImMatDataType _t, VkAllocator* _allocator)
{
    size_t _elemsize = IM_ESIZE(_t);
    if (dims == 2 && w == _w && h == _h && elemsize == _elemsize && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;

    cstep = w * h;
    type = _t;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create_type(int _w, int _h, int _c, ImGui::ImMatDataType _t, VkAllocator* _allocator)
{
    size_t _elemsize = IM_ESIZE(_t);
    if (dims == 3 && w == _w && h == _h && c == _c && elemsize == _elemsize && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;
    allocator = _allocator;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;

    cstep = alignSize(w * h * elemsize, 16) / elemsize;
    type = _t;
    color_space = ImGui::IMMAT_SRGB;
    color_format = c == 1 ? ImGui::IMMAT_GRAY : ImGui::IMMAT_RGB;
    color_range = ImGui::IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkMat::create_like(const ImGui::ImMat& m, VkAllocator* _allocator)
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

inline ImGui::ImMat VkMat::mapped() const
{
    if (!allocator->mappable)
        return ImGui::ImMat();

    if (dims == 1)
        return ImGui::ImMat(w, mapped_ptr(), elemsize, elempack);

    if (dims == 2)
        return ImGui::ImMat(w, h, mapped_ptr(), elemsize, elempack);

    if (dims == 3)
        return ImGui::ImMat(w, h, c, mapped_ptr(), elemsize, elempack);

    return ImGui::ImMat();
}

inline void* VkMat::mapped_ptr() const
{
    if (!allocator)
        return NULL;
    
    if (!allocator->mappable)
        return NULL;

    return (unsigned char*)data->mapped_ptr + data->offset;
}

inline void VkMat::addref()
{
    if (refcount) IM_XADD(refcount, 1);
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
    type = ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    cstep = 0;

    refcount = 0;
}

inline bool VkMat::empty() const
{
    return data == 0 || total() == 0;
}

inline size_t VkMat::total() const
{
    return cstep * c;
}

inline int VkMat::elembits() const
{
    return elempack ? elemsize * 8 / elempack : 0;
}

inline ImGui::ImMat VkMat::shape() const
{
    if (dims == 1)
        return ImGui::ImMat(w * elempack);
    if (dims == 2)
        return ImGui::ImMat(w, h * elempack);
    if (dims == 3)
        return ImGui::ImMat(w, h, c * elempack);

    return ImGui::ImMat();
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

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkImageMat Class
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
inline VkImageMat::VkImageMat()
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0)
{
    type = ImGui::IMMAT_FLOAT32;
    color_format = ImGui::IMMAT_GRAY;
    color_space = ImGui::IMMAT_SRGB;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkImageMat::VkImageMat(int _w, size_t _elemsize, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0)
{
    create(_w, _elemsize, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0)
{
    create(_w, _h, _elemsize, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0)
{
    create(_w, _h, _c, _elemsize, _allocator);
}

inline VkImageMat::VkImageMat(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0)
{
    create(_w, _elemsize, _elempack, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0)
{
    create(_w, _h, _elemsize, _elempack, _allocator);
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0)
{
    create(_w, _h, _c, _elemsize, _elempack, _allocator);
}

inline VkImageMat::VkImageMat(const VkImageMat& m)
    : data(m.data), refcount(m.refcount), elemsize(m.elemsize), elempack(m.elempack), allocator(m.allocator), dims(m.dims), w(m.w), h(m.h), c(m.c)
{
    if (refcount) IM_XADD(refcount, 1);
}

inline VkImageMat::VkImageMat(int _w, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(1), w(_w), h(1), c(1)
{
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkImageMat::VkImageMat(int _w, int _h, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(2), w(_w), h(_h), c(1)
{
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(3), w(_w), h(_h), c(_c)
{
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = c == 1 ? ImGui::IMMAT_GRAY : ImGui::IMMAT_RGB;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkImageMat::VkImageMat(int _w, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(1), w(_w), h(1), c(1)
{
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkImageMat::VkImageMat(int _w, int _h, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(2), w(_w), h(_h), c(1)
{
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(3), w(_w), h(_h), c(_c)
{
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = c == 1 ? ImGui::IMMAT_GRAY : ImGui::IMMAT_RGB;
    color_range = ImGui::IMMAT_FULL_RANGE;
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

    data = m.data;
    refcount = m.refcount;
    elemsize = m.elemsize;
    elempack = m.elempack;
    allocator = m.allocator;

    dims = m.dims;
    w = m.w;
    h = m.h;
    c = m.c;
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

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
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

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
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

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = c == 1 ? ImGui::IMMAT_GRAY : ImGui::IMMAT_RGB;
    color_range = ImGui::IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
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

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
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

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
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

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? ImGui::IMMAT_INT8 : _elemsize == 2 ? ImGui::IMMAT_INT16 : ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = c == 1 ? ImGui::IMMAT_GRAY : ImGui::IMMAT_RGB;
    color_range = ImGui::IMMAT_FULL_RANGE;

    if (total() > 0)
    {
        data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageMat::create_like(const ImGui::ImMat& m, VkAllocator* _allocator)
{
    int _dims = m.dims;
    if (_dims == 1)
        create(m.w, m.elemsize, m.elempack, _allocator);
    if (_dims == 2)
        create(m.w, m.h, m.elemsize, m.elempack, _allocator);
    if (_dims == 3)
        create(m.w, m.h, m.c, m.elemsize, m.elempack, _allocator);
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
}

inline void VkImageMat::create_like(const VkImageMat& im, VkAllocator* _allocator)
{
    int _dims = im.dims;
    if (_dims == 1)
        create(im.w, im.elemsize, im.elempack, _allocator);
    if (_dims == 2)
        create(im.w, im.h, im.elemsize, im.elempack, _allocator);
    if (_dims == 3)
        create(im.w, im.h, im.c, im.elemsize, im.elempack, _allocator);
}

inline ImGui::ImMat VkImageMat::mapped() const
{
    if (!allocator->mappable || !data->mapped_ptr)
        return ImGui::ImMat();

    if (dims == 1)
        return ImGui::ImMat(w, mapped_ptr(), elemsize, elempack);

    if (dims == 2)
        return ImGui::ImMat(w, h, mapped_ptr(), elemsize, elempack);

    if (dims == 3)
        return ImGui::ImMat(w, h, c, mapped_ptr(), elemsize, elempack);

    return ImGui::ImMat();
}

inline void* VkImageMat::mapped_ptr() const
{
    if (!allocator->mappable || !data->mapped_ptr)
        return 0;

    return (unsigned char*)data->mapped_ptr + data->bind_offset;
}

inline void VkImageMat::addref()
{
    if (refcount) IM_XADD(refcount, 1);
}

inline void VkImageMat::release()
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
    type = ImGui::IMMAT_FLOAT32;
    color_space = ImGui::IMMAT_SRGB;
    color_format = ImGui::IMMAT_GRAY;
    color_range = ImGui::IMMAT_FULL_RANGE;

    refcount = 0;
}

inline bool VkImageMat::empty() const
{
    return data == 0 || total() == 0;
}

inline size_t VkImageMat::total() const
{
    return w * h * c;
}

inline int VkImageMat::elembits() const
{
    return elempack ? elemsize * 8 / elempack : 0;
}

inline ImGui::ImMat VkImageMat::shape() const
{
    if (dims == 1)
        return ImGui::ImMat(w * elempack);
    if (dims == 2)
        return ImGui::ImMat(w, h * elempack);
    if (dims == 3)
        return ImGui::ImMat(w, h, c * elempack);

    return ImGui::ImMat();
}

inline VkImage VkImageMat::image() const
{
    return data->image;
}

inline VkImageView VkImageMat::imageview() const
{
    return data->imageview;
}

extern const ImGui::ImMat * color_table[2][2][4];

} // namespace ImVulkan 

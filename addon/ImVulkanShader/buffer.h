#pragma once
#include "allocator.h"
#include "option.h"
#include "platform.h"
#include <vulkan/vulkan.h>

namespace ImVulkan 
{
class VkImageBuffer;
class VkImageMat;
enum DataType {
    INT8 = 0,
    INT16,
    INT32,
    INT64,
    FLOAT16,
    FLOAT32,
    FLOAT64,
    NB_DATA_TYPE
};

enum ColorRange {
    FULL_RANGE = 0,
    NARROW_RANGE
};

enum ColorSpace {
    SRGB = 0,
    BT601,
    BT709,
    BT2020,
    HSV,
    HLS,
    CMY,
    LAB
};

enum ColorFormat {
    GRAY = 0,
    BGR,
    RGB,
    YUV420,
    YUV422,
    YUV444,
    NV12,
};

#define ESIZE(a)  (a == ImVulkan::INT8 ? (size_t)1u : (a == ImVulkan::INT16 || a == ImVulkan::FLOAT16) ? (size_t)2u : (a == ImVulkan::INT32 || a == ImVulkan::FLOAT32) ? (size_t)4u : (a == ImVulkan::INT64 || a == ImVulkan::FLOAT64) ? (size_t)8u : (size_t)0u)
#define ISMONO(a) (a == ImVulkan::GRAY)
#define ISRGB(a) (a == ImVulkan::BGR || a == ImVulkan::RGB)
#define ISYUV(a) (a == ImVulkan::YUV420 || a == ImVulkan::YUV422 || a == ImVulkan::YUV444 || a == ImVulkan::NV12)

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// ImageBuffer Class define
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

class ImageBuffer
{
public:
    // empty
    ImageBuffer();
    // vec
    ImageBuffer(int w, size_t elemsize = 4u, Allocator* allocator = 0);
    // image
    ImageBuffer(int w, int h, size_t elemsize = 4u, Allocator* allocator = 0);
    // dim
    ImageBuffer(int w, int h, int c, size_t elemsize = 4u, Allocator* allocator = 0);
    // packed vec
    ImageBuffer(int w, size_t elemsize, int elempack, Allocator* allocator = 0);
    // packed image
    ImageBuffer(int w, int h, size_t elemsize, int elempack, Allocator* allocator = 0);
    // packed dim
    ImageBuffer(int w, int h, int c, size_t elemsize, int elempack, Allocator* allocator = 0);
    // copy
    ImageBuffer(const ImageBuffer& m);
    // external vec
    ImageBuffer(int w, void* data, size_t elemsize = 4u, Allocator* allocator = 0);
    // external image
    ImageBuffer(int w, int h, void* data, size_t elemsize = 4u, Allocator* allocator = 0);
    // external dim
    ImageBuffer(int w, int h, int c, void* data, size_t elemsize = 4u, Allocator* allocator = 0);
    // external packed vec
    ImageBuffer(int w, void* data, size_t elemsize, int elempack, Allocator* allocator = 0);
    // external packed image
    ImageBuffer(int w, int h, void* data, size_t elemsize, int elempack, Allocator* allocator = 0);
    // external packed dim
    ImageBuffer(int w, int h, int c, void* data, size_t elemsize, int elempack, Allocator* allocator = 0);
    // release
    ~ImageBuffer();
    // assign
    ImageBuffer& operator=(const ImageBuffer& m);
    // set all
    template<typename T> void fill(T v);
    // deep copy
    ImageBuffer clone(Allocator* allocator = 0) const;
    // deep copy from other buffer, inplace
    void clone_from(const ImageBuffer& mat, Allocator* allocator = 0);
    // reshape vec
    ImageBuffer reshape(int w, Allocator* allocator = 0) const;
    // reshape image
    ImageBuffer reshape(int w, int h, Allocator* allocator = 0) const;
    // reshape dim
    ImageBuffer reshape(int w, int h, int c, Allocator* allocator = 0) const;
    // allocate vec
    void create(int w, size_t elemsize = 4u, Allocator* allocator = 0);
    // allocate image
    void create(int w, int h, size_t elemsize = 4u, Allocator* allocator = 0);
    // allocate dim
    void create(int w, int h, int c, size_t elemsize = 4u, Allocator* allocator = 0);
    // allocate packed vec
    void create(int w, size_t elemsize, int elempack, Allocator* allocator = 0);
    // allocate packed image
    void create(int w, int h, size_t elemsize, int elempack, Allocator* allocator = 0);
    // allocate packed dim
    void create(int w, int h, int c, size_t elemsize, int elempack, Allocator* allocator = 0);
    // allocate like
    void create_like(const ImageBuffer& m, Allocator* allocator = 0);
    // allocate vec with type
    void create_type(int w, DataType t = INT8, Allocator* allocator = 0);
    // allocate image with type
    void create_type(int w, int h, DataType t = INT8, Allocator* allocator = 0);
    // allocate dim with type
    void create_type(int w, int h, int c, DataType t = INT8, Allocator* allocator = 0);
    // extern vec with type
    void create_type(int w, void* data, DataType t = INT8, Allocator* allocator = 0);
    // extern image with type
    void create_type(int w, int h, void* data, DataType t = INT8, Allocator* allocator = 0);
    // extern dim with type
    void create_type(int w, int h, int c, void* data, DataType t = INT8, Allocator* allocator = 0);
    // allocate like
    void create_like(const VkImageBuffer& m, Allocator* allocator = 0);
    // allocate like
    void create_like(const VkImageMat& im, Allocator* allocator = 0);
    // refcount++
    void addref();
    // refcount--
    void release();

    bool empty() const;
    size_t total() const;

    // bits per element
    int elembits() const;

    // shape only
    ImageBuffer shape() const;

    // data reference
    ImageBuffer channel(int c);
    const ImageBuffer channel(int c) const;
    float* row(int y);
    const float* row(int y) const;
    template<typename T>
    T* row(int y);
    template<typename T>
    const T* row(int y) const;

    // range reference
    ImageBuffer channel_range(int c, int channels);
    const ImageBuffer channel_range(int c, int channels) const;
    ImageBuffer row_range(int y, int rows);
    const ImageBuffer row_range(int y, int rows) const;
    ImageBuffer range(int x, int n);
    const ImageBuffer range(int x, int n) const;

    // access raw data
    template<typename T>
    operator T*();
    template<typename T>
    operator const T*() const;

    // convenient access float vec element
    float& operator[](size_t i);
    const float& operator[](size_t i) const;

    // pointer to the data
    void* data;

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
    Allocator* allocator;

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
    DataType type;
    // color
    // 0 = SRGB
    // 1 = BT601
    // 2 = BT709
    // 3 = BT2020
    ColorSpace color_space;
    // 0 = GRAY
    // 1 = BGR
    // 2 = RGB
    // 3 = YUV420
    // 4 = YUV422
    // 5 = YUV444
    // 6 = NV12
    ColorFormat color_format;
    // 0 = FULL_RANGE
    // 1 = NARROW_RANGE
    ColorRange color_range;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkImageBuffer Class define
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class VkImageBuffer
{
public:
    // empty
    VkImageBuffer();
    // vec
    VkImageBuffer(int w, size_t elemsize, VkAllocator* allocator);
    // image
    VkImageBuffer(int w, int h, size_t elemsize, VkAllocator* allocator);
    // dim
    VkImageBuffer(int w, int h, int c, size_t elemsize, VkAllocator* allocator);
    // packed vec
    VkImageBuffer(int w, size_t elemsize, int elempack, VkAllocator* allocator);
    // packed image
    VkImageBuffer(int w, int h, size_t elemsize, int elempack, VkAllocator* allocator);
    // packed dim
    VkImageBuffer(int w, int h, int c, size_t elemsize, int elempack, VkAllocator* allocator);
    // copy
    VkImageBuffer(const VkImageBuffer& m);
    // external vec
    VkImageBuffer(int w, VkBufferMemory* data, size_t elemsize, VkAllocator* allocator);
    // external image
    VkImageBuffer(int w, int h, VkBufferMemory* data, size_t elemsize, VkAllocator* allocator);
    // external dim
    VkImageBuffer(int w, int h, int c, VkBufferMemory* data, size_t elemsize, VkAllocator* allocator);
    // external packed vec
    VkImageBuffer(int w, VkBufferMemory* data, size_t elemsize, int elempack, VkAllocator* allocator);
    // external packed image
    VkImageBuffer(int w, int h, VkBufferMemory* data, size_t elemsize, int elempack, VkAllocator* allocator);
    // external packed dim
    VkImageBuffer(int w, int h, int c, VkBufferMemory* data, size_t elemsize, int elempack, VkAllocator* allocator);
    // release
    ~VkImageBuffer();
    // assign
    VkImageBuffer& operator=(const VkImageBuffer& m);
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
    void create_type(int w, DataType t, VkAllocator* allocator);
    // allocate image with type
    void create_type(int w, int h, DataType t, VkAllocator* allocator);
    // allocate dim with type
    void create_type(int w, int h, int c, DataType t, VkAllocator* allocator);
    // allocate like
    void create_like(const ImageBuffer& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkImageBuffer& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkImageMat& im, VkAllocator* allocator);

    // mapped
    ImageBuffer mapped() const;
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
    ImageBuffer shape() const;

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
    DataType type;
    // color
    // 0 = SRGB
    // 1 = BT601
    // 2 = BT709
    // 3 = BT2020
    ColorSpace color_space;
    // 0 = GRAY
    // 1 = BGR
    // 2 = RGB
    // 3 = YUV420
    // 4 = YUV422
    // 5 = YUV444
    // 6 = NV12
    ColorFormat color_format;
    // 0 = FULL_RANGE
    // 1 = NARROW_RANGE
    ColorRange color_range;
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
    void create_like(const ImageBuffer& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkImageBuffer& m, VkAllocator* allocator);
    // allocate like
    void create_like(const VkImageMat& im, VkAllocator* allocator);

    // mapped
    ImageBuffer mapped() const;
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
    ImageBuffer shape() const;

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
    DataType type;
    // color
    // 0 = SRGB
    // 1 = BT601
    // 2 = BT709
    // 3 = BT2020
    ColorSpace color_space;
    // 0 = GRAY
    // 1 = BGR
    // 2 = RGB
    // 3 = YUV420
    // 4 = YUV422
    // 5 = YUV444
    // 6 = NV12
    ColorFormat color_format;
    // 0 = FULL_RANGE
    // 1 = NARROW_RANGE
    ColorRange color_range;
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
// ImageBuffer class
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
inline ImageBuffer::ImageBuffer()
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
}

inline ImageBuffer::ImageBuffer(int _w, size_t _elemsize, Allocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _elemsize, _allocator);
}

inline ImageBuffer::ImageBuffer(int _w, int _h, size_t _elemsize, Allocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _elemsize, _allocator);
}

inline ImageBuffer::ImageBuffer(int _w, int _h, int _c, size_t _elemsize, Allocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _c, _elemsize, _allocator);
}

inline ImageBuffer::ImageBuffer(int _w, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _elemsize, _elempack, _allocator);
}

inline ImageBuffer::ImageBuffer(int _w, int _h, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _elemsize, _elempack, _allocator);
}

inline ImageBuffer::ImageBuffer(int _w, int _h, int _c, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _c, _elemsize, _elempack, _allocator);
}

inline ImageBuffer::ImageBuffer(const ImageBuffer& m)
    : data(m.data), refcount(m.refcount), elemsize(m.elemsize), elempack(m.elempack), allocator(m.allocator), dims(m.dims), w(m.w), h(m.h), c(m.c), cstep(m.cstep)
{
    if (refcount) _XADD(refcount, 1);
    type = m.type;
    color_format = m.color_format;
    color_space = m.color_space;
    color_range = m.color_range;
}

inline ImageBuffer::ImageBuffer(int _w, void* _data, size_t _elemsize, Allocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(1), w(_w), h(1), c(1)
{
    cstep = w;
}

inline ImageBuffer::ImageBuffer(int _w, int _h, void* _data, size_t _elemsize, Allocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(2), w(_w), h(_h), c(1)
{
    cstep = (size_t)w * h;
}

inline ImageBuffer::ImageBuffer(int _w, int _h, int _c, void* _data, size_t _elemsize, Allocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(3), w(_w), h(_h), c(_c)
{
    cstep = alignSize((size_t)w * h * elemsize, 16) / elemsize;
}

inline ImageBuffer::ImageBuffer(int _w, void* _data, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(1), w(_w), h(1), c(1)
{
    cstep = w;
}

inline ImageBuffer::ImageBuffer(int _w, int _h, void* _data, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(2), w(_w), h(_h), c(1)
{
    cstep = (size_t)w * h;
}

inline ImageBuffer::ImageBuffer(int _w, int _h, int _c, void* _data, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(3), w(_w), h(_h), c(_c)
{
    cstep = alignSize((size_t)w * h * elemsize, 16) / elemsize;
}

inline ImageBuffer::~ImageBuffer()
{
    release();
}

inline ImageBuffer& ImageBuffer::operator=(const ImageBuffer& m)
{
    if (this == &m)
        return *this;

    if (m.refcount)
        _XADD(m.refcount, 1);

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

template<typename T>
inline void ImageBuffer::fill(T _v)
{
    int size = (int)total();
    T* ptr = (T*)data;
    for (int i = 0; i < size; i++)
    {
        ptr[i] = _v;
    }
}

inline ImageBuffer ImageBuffer::clone(Allocator* _allocator) const
{
    if (empty())
        return ImageBuffer();

    ImageBuffer m;
    if (dims == 1)
        m.create(w, elemsize, elempack, _allocator);
    else if (dims == 2)
        m.create(w, h, elemsize, elempack, _allocator);
    else if (dims == 3)
        m.create(w, h, c, elemsize, elempack, _allocator);

    if (total() > 0)
    {
        memcpy(m.data, data, total() * elemsize);
    }
    m.color_format = color_format;
    m.color_range = color_range;
    m.color_space = color_space;
    m.type = type;
    return m;
}

inline void ImageBuffer::clone_from(const ImageBuffer& mat, Allocator* allocator)
{
    *this = mat.clone(allocator);
}

inline ImageBuffer ImageBuffer::reshape(int _w, Allocator* _allocator) const
{
    if (w * h * c != _w)
        return ImageBuffer();

    if (dims == 3 && cstep != (size_t)w * h)
    {
        ImageBuffer m;
        m.create(_w, elemsize, elempack, _allocator);

        // flatten
        for (int i = 0; i < c; i++)
        {
            const void* ptr = (unsigned char*)data + i * cstep * elemsize;
            void* mptr = (unsigned char*)m.data + (size_t)i * w * h * elemsize;
            memcpy(mptr, ptr, (size_t)w * h * elemsize);
        }

        return m;
    }

    ImageBuffer m = *this;

    m.dims = 1;
    m.w = _w;
    m.h = 1;
    m.c = 1;

    m.cstep = _w;

    return m;
}

inline ImageBuffer ImageBuffer::reshape(int _w, int _h, Allocator* _allocator) const
{
    if (w * h * c != _w * _h)
        return ImageBuffer();

    if (dims == 3 && cstep != (size_t)w * h)
    {
        ImageBuffer m;
        m.create(_w, _h, elemsize, elempack, _allocator);

        // flatten
        for (int i = 0; i < c; i++)
        {
            const void* ptr = (unsigned char*)data + i * cstep * elemsize;
            void* mptr = (unsigned char*)m.data + (size_t)i * w * h * elemsize;
            memcpy(mptr, ptr, (size_t)w * h * elemsize);
        }

        return m;
    }

    ImageBuffer m = *this;

    m.dims = 2;
    m.w = _w;
    m.h = _h;
    m.c = 1;

    m.cstep = (size_t)_w * _h;

    return m;
}

inline ImageBuffer ImageBuffer::reshape(int _w, int _h, int _c, Allocator* _allocator) const
{
    if (w * h * c != _w * _h * _c)
        return ImageBuffer();

    if (dims < 3)
    {
        if ((size_t)_w * _h != alignSize((size_t)_w * _h * elemsize, 16) / elemsize)
        {
            ImageBuffer m;
            m.create(_w, _h, _c, elemsize, elempack, _allocator);

            // align channel
            for (int i = 0; i < _c; i++)
            {
                const void* ptr = (unsigned char*)data + (size_t)i * _w * _h * elemsize;
                void* mptr = (unsigned char*)m.data + i * m.cstep * m.elemsize;
                memcpy(mptr, ptr, (size_t)_w * _h * elemsize);
            }

            return m;
        }
    }
    else if (c != _c)
    {
        // flatten and then align
        ImageBuffer tmp = reshape(_w * _h * _c, _allocator);
        return tmp.reshape(_w, _h, _c, _allocator);
    }

    ImageBuffer m = *this;

    m.dims = 3;
    m.w = _w;
    m.h = _h;
    m.c = _c;

    m.cstep = alignSize((size_t)_w * _h * elemsize, 16) / elemsize;

    return m;
}

inline void ImageBuffer::create(int _w, size_t _elemsize, Allocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = w;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImageBuffer::create(int _w, int _h, size_t _elemsize, Allocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = (size_t)w * h;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImageBuffer::create(int _w, int _h, int _c, size_t _elemsize, Allocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = alignSize((size_t)w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImageBuffer::create(int _w, size_t _elemsize, int _elempack, Allocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = w;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImageBuffer::create(int _w, int _h, size_t _elemsize, int _elempack, Allocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = (size_t)w * h;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImageBuffer::create(int _w, int _h, int _c, size_t _elemsize, int _elempack, Allocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;


    cstep = alignSize((size_t)w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImageBuffer::create_like(const ImageBuffer& m, Allocator* _allocator)
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

// create with DataType
inline void ImageBuffer::create_type(int _w, DataType _t, Allocator* _allocator)
{
    if (dims == 1 && w == _w && type == _t && allocator == _allocator)
        return;

    release();
    elemsize = ESIZE(_t);
    elempack = 1;
    allocator = _allocator;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;

    cstep = w;
    type = _t;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImageBuffer::create_type(int _w, int _h, DataType _t, Allocator* _allocator)
{
    if (dims == 2 && w == _w && h == _h && type == _t && allocator == _allocator)
        return;

    release();
    elemsize = ESIZE(_t);
    elempack = 1;
    allocator = _allocator;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;

    cstep = (size_t)w * h;
    type = _t;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImageBuffer::create_type(int _w, int _h, int _c, DataType _t, Allocator* _allocator)
{
    if (dims == 3 && w == _w && h == _h && c == _c && type == _t && allocator == _allocator)
        return;

    release();
    elemsize = ESIZE(_t);
    elempack = 1;
    allocator = _allocator;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;

    cstep = alignSize((size_t)w * h * elemsize, 4) / elemsize;
    type = _t;
    color_space = SRGB;
    color_format = BGR;
    color_range = FULL_RANGE;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImageBuffer::create_type(int _w, void* _data, DataType _t, Allocator* _allocator)
{
    if (dims == 1 && w == _w && type == _t && allocator == _allocator)
        return;

    release();
    elemsize = ESIZE(_t);
    elempack = 1;
    allocator = _allocator;
    refcount = 0;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;

    cstep = w;
    type = _t;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;
    data = _data;
}

inline void ImageBuffer::create_type(int _w, int _h, void* _data, DataType _t, Allocator* _allocator)
{
    if (dims == 2 && w == _w && h == _h && type == _t && allocator == _allocator)
        return;

    release();
    elemsize = ESIZE(_t);
    elempack = 1;
    allocator = _allocator;
    refcount = 0;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;

    cstep = (size_t)w * h;
    type = _t;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;
    data = _data;
}

inline void ImageBuffer::create_type(int _w, int _h, int _c, void* _data, DataType _t, Allocator* _allocator)
{
    if (dims == 3 && w == _w && h == _h && c == _c && type == _t && allocator == _allocator)
        return;

    release();
    elemsize = ESIZE(_t);
    elempack = 1;
    allocator = _allocator;
    refcount = 0;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;

    cstep = alignSize((size_t)w * h * elemsize, 4) / elemsize;
    type = _t;
    color_space = SRGB;
    color_format = BGR;
    color_range = FULL_RANGE;
    data = _data;
}

inline void ImageBuffer::create_like(const VkImageBuffer& m, Allocator* _allocator)
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

inline void ImageBuffer::create_like(const VkImageMat& im, Allocator* _allocator)
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

inline void ImageBuffer::addref()
{
    if (refcount) _XADD(refcount, 1);
}

inline void ImageBuffer::release()
{
    if (refcount && _XADD(refcount, -1) == 1)
    {
        if (allocator)
            allocator->fastFree(data);
        else
            fastFree(data);
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

    type = FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;
}

inline bool ImageBuffer::empty() const
{
    return data == 0 || total() == 0;
}

inline size_t ImageBuffer::total() const
{
    return cstep * c;
}

inline int ImageBuffer::elembits() const
{
    return elempack ? static_cast<int>(elemsize * 8) / elempack : 0;
}

inline ImageBuffer ImageBuffer::shape() const
{
    if (dims == 1)
        return ImageBuffer(w * elempack, (void*)0);
    if (dims == 2)
        return ImageBuffer(w, h * elempack, (void*)0);
    if (dims == 3)
        return ImageBuffer(w, h, c * elempack, (void*)0);

    return ImageBuffer();
}

inline ImageBuffer ImageBuffer::channel(int _c)
{
    return ImageBuffer(w, h, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack, allocator);
}

inline const ImageBuffer ImageBuffer::channel(int _c) const
{
    return ImageBuffer(w, h, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack, allocator);
}

inline float* ImageBuffer::row(int y)
{
    return (float*)((unsigned char*)data + (size_t)w * y * elemsize);
}

inline const float* ImageBuffer::row(int y) const
{
    return (const float*)((unsigned char*)data + (size_t)w * y * elemsize);
}

template<typename T>
inline T* ImageBuffer::row(int y)
{
    return (T*)((unsigned char*)data + (size_t)w * y * elemsize);
}

template<typename T>
inline const T* ImageBuffer::row(int y) const
{
    return (const T*)((unsigned char*)data + (size_t)w * y * elemsize);
}

inline ImageBuffer ImageBuffer::channel_range(int _c, int channels)
{
    return ImageBuffer(w, h, channels, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack, allocator);
}

inline const ImageBuffer ImageBuffer::channel_range(int _c, int channels) const
{
    return ImageBuffer(w, h, channels, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack, allocator);
}

inline ImageBuffer ImageBuffer::row_range(int y, int rows)
{
    return ImageBuffer(w, rows, (unsigned char*)data + (size_t)w * y * elemsize, elemsize, elempack, allocator);
}

inline const ImageBuffer ImageBuffer::row_range(int y, int rows) const
{
    return ImageBuffer(w, rows, (unsigned char*)data + (size_t)w * y * elemsize, elemsize, elempack, allocator);
}

inline ImageBuffer ImageBuffer::range(int x, int n)
{
    return ImageBuffer(n, (unsigned char*)data + x * elemsize, elemsize, elempack, allocator);
}

inline const ImageBuffer ImageBuffer::range(int x, int n) const
{
    return ImageBuffer(n, (unsigned char*)data + x * elemsize, elemsize, elempack, allocator);
}

template<typename T>
inline ImageBuffer::operator T*()
{
    return (T*)data;
}

template<typename T>
inline ImageBuffer::operator const T*() const
{
    return (const T*)data;
}

inline float& ImageBuffer::operator[](size_t i)
{
    return ((float*)data)[i];
}

inline const float& ImageBuffer::operator[](size_t i) const
{
    return ((const float*)data)[i];
}

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// VkImageBuffer Class
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
inline VkImageBuffer::VkImageBuffer()
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
}

inline VkImageBuffer::VkImageBuffer(int _w, size_t _elemsize, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _elemsize, _allocator);
}

inline VkImageBuffer::VkImageBuffer(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _elemsize, _allocator);
}

inline VkImageBuffer::VkImageBuffer(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _c, _elemsize, _allocator);
}

inline VkImageBuffer::VkImageBuffer(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _elemsize, _elempack, _allocator);
}

inline VkImageBuffer::VkImageBuffer(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _elemsize, _elempack, _allocator);
}

inline VkImageBuffer::VkImageBuffer(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _c, _elemsize, _elempack, _allocator);
}

inline VkImageBuffer::VkImageBuffer(const VkImageBuffer& m)
    : data(m.data), refcount(m.refcount), elemsize(m.elemsize), elempack(m.elempack), allocator(m.allocator), dims(m.dims), w(m.w), h(m.h), c(m.c)
{
    if (refcount) _XADD(refcount, 1);

    cstep = m.cstep;
    type = m.type;
    color_format = m.color_format;
    color_space = m.color_space;
    color_range = m.color_range;
}

inline VkImageBuffer::VkImageBuffer(int _w, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(1), w(_w), h(1), c(1)
{
    cstep = w;
}

inline VkImageBuffer::VkImageBuffer(int _w, int _h, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(2), w(_w), h(_h), c(1)
{
    cstep = w * h;
}

inline VkImageBuffer::VkImageBuffer(int _w, int _h, int _c, VkBufferMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(3), w(_w), h(_h), c(_c)
{
    cstep = alignSize(w * h * elemsize, 16) / elemsize;
}

inline VkImageBuffer::VkImageBuffer(int _w, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(1), w(_w), h(1), c(1)
{
    cstep = w;
}

inline VkImageBuffer::VkImageBuffer(int _w, int _h, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(2), w(_w), h(_h), c(1)
{
    cstep = w * h;
}

inline VkImageBuffer::VkImageBuffer(int _w, int _h, int _c, VkBufferMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(3), w(_w), h(_h), c(_c)
{
    cstep = alignSize(w * h * elemsize, 16) / elemsize;
}

inline VkImageBuffer::~VkImageBuffer()
{
    release();
}

inline VkImageBuffer& VkImageBuffer::operator=(const VkImageBuffer& m)
{
    if (this == &m)
        return *this;

    if (m.refcount)
        _XADD(m.refcount, 1);

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

inline void VkImageBuffer::create(int _w, size_t _elemsize, VkAllocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = w;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageBuffer::create(int _w, int _h, size_t _elemsize, VkAllocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = w * h;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageBuffer::create(int _w, int _h, int _c, size_t _elemsize, VkAllocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = alignSize(w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageBuffer::create(int _w, size_t _elemsize, int _elempack, VkAllocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = w;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageBuffer::create(int _w, int _h, size_t _elemsize, int _elempack, VkAllocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = w * h;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageBuffer::create(int _w, int _h, int _c, size_t _elemsize, int _elempack, VkAllocator* _allocator)
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
    type = _elemsize == 1 ? INT8 : _elemsize == 2 ? INT16 : FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = alignSize(w * h * elemsize, 16) / elemsize;

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageBuffer::create_type(int _w, DataType _t, VkAllocator* _allocator)
{
    size_t _elemsize = ESIZE(_t);
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

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
    type = _t;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;
}

inline void VkImageBuffer::create_type(int _w, int _h, DataType _t, VkAllocator* _allocator)
{
    size_t _elemsize = ESIZE(_t);
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

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
    type = _t;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;
}

inline void VkImageBuffer::create_type(int _w, int _h, int _c, DataType _t, VkAllocator* _allocator)
{
    size_t _elemsize = ESIZE(_t);
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

    if (total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);

        data = allocator->fastMalloc(totalsize);

        refcount = (int*)((unsigned char*)data + offsetof(VkBufferMemory, refcount));
        *refcount = 1;
    }
    type = _t;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;
}

inline void VkImageBuffer::create_like(const ImageBuffer& m, VkAllocator* _allocator)
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

inline void VkImageBuffer::create_like(const VkImageBuffer& m, VkAllocator* _allocator)
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

inline void VkImageBuffer::create_like(const VkImageMat& im, VkAllocator* _allocator)
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

inline ImageBuffer VkImageBuffer::mapped() const
{
    if (!allocator->mappable)
        return ImageBuffer();

    if (dims == 1)
        return ImageBuffer(w, mapped_ptr(), elemsize, elempack, 0);

    if (dims == 2)
        return ImageBuffer(w, h, mapped_ptr(), elemsize, elempack, 0);

    if (dims == 3)
        return ImageBuffer(w, h, c, mapped_ptr(), elemsize, elempack, 0);

    return ImageBuffer();
}

inline void* VkImageBuffer::mapped_ptr() const
{
    if (!allocator->mappable)
        return 0;

    return (unsigned char*)data->mapped_ptr + data->offset;
}

inline void VkImageBuffer::addref()
{
    if (refcount) _XADD(refcount, 1);
}

inline void VkImageBuffer::release()
{
    if (refcount && _XADD(refcount, -1) == 1)
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
    type = FLOAT32;
    color_space = SRGB;
    color_format = GRAY;
    color_range = FULL_RANGE;

    cstep = 0;

    refcount = 0;
}

inline bool VkImageBuffer::empty() const
{
    return data == 0 || total() == 0;
}

inline size_t VkImageBuffer::total() const
{
    return cstep * c;
}

inline int VkImageBuffer::elembits() const
{
    return elempack ? elemsize * 8 / elempack : 0;
}

inline ImageBuffer VkImageBuffer::shape() const
{
    if (dims == 1)
        return ImageBuffer(w * elempack, (void*)0);
    if (dims == 2)
        return ImageBuffer(w, h * elempack, (void*)0);
    if (dims == 3)
        return ImageBuffer(w, h, c * elempack, (void*)0);

    return ImageBuffer();
}

inline VkBuffer VkImageBuffer::buffer() const
{
    return data->buffer;
}

inline size_t VkImageBuffer::buffer_offset() const
{
    return data->offset;
}

inline size_t VkImageBuffer::buffer_capacity() const
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
    if (refcount) _XADD(refcount, 1);
}

inline VkImageMat::VkImageMat(int _w, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(1), w(_w), h(1), c(1)
{
}

inline VkImageMat::VkImageMat(int _w, int _h, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(2), w(_w), h(_h), c(1)
{
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, VkImageMemory* _data, size_t _elemsize, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(3), w(_w), h(_h), c(_c)
{
}

inline VkImageMat::VkImageMat(int _w, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(1), w(_w), h(1), c(1)
{
}

inline VkImageMat::VkImageMat(int _w, int _h, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(2), w(_w), h(_h), c(1)
{
}

inline VkImageMat::VkImageMat(int _w, int _h, int _c, VkImageMemory* _data, size_t _elemsize, int _elempack, VkAllocator* _allocator)
    : data(_data), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(3), w(_w), h(_h), c(_c)
{
}

inline VkImageMat::~VkImageMat()
{
    release();
}

inline VkImageMat& VkImageMat::operator=(const VkImageMat& m)
{
    if (this == &m)
        return *this;

    if (m.refcount) _XADD(m.refcount, 1);

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

    if (total() > 0)
    {
        data = allocator->fastMalloc(w, h, c, elemsize, elempack);
        if (!data)
            return;

        refcount = (int*)((unsigned char*)data + offsetof(VkImageMemory, refcount));
        *refcount = 1;
    }
}

inline void VkImageMat::create_like(const ImageBuffer& m, VkAllocator* _allocator)
{
    int _dims = m.dims;
    if (_dims == 1)
        create(m.w, m.elemsize, m.elempack, _allocator);
    if (_dims == 2)
        create(m.w, m.h, m.elemsize, m.elempack, _allocator);
    if (_dims == 3)
        create(m.w, m.h, m.c, m.elemsize, m.elempack, _allocator);
}

inline void VkImageMat::create_like(const VkImageBuffer& m, VkAllocator* _allocator)
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

inline ImageBuffer VkImageMat::mapped() const
{
    if (!allocator->mappable || !data->mapped_ptr)
        return ImageBuffer();

    if (dims == 1)
        return ImageBuffer(w, mapped_ptr(), elemsize, elempack, 0);

    if (dims == 2)
        return ImageBuffer(w, h, mapped_ptr(), elemsize, elempack, 0);

    if (dims == 3)
        return ImageBuffer(w, h, c, mapped_ptr(), elemsize, elempack, 0);

    return ImageBuffer();
}

inline void* VkImageMat::mapped_ptr() const
{
    if (!allocator->mappable || !data->mapped_ptr)
        return 0;

    return (unsigned char*)data->mapped_ptr + data->bind_offset;
}

inline void VkImageMat::addref()
{
    if (refcount) _XADD(refcount, 1);
}

inline void VkImageMat::release()
{
    if (refcount && _XADD(refcount, -1) == 1)
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

inline ImageBuffer VkImageMat::shape() const
{
    if (dims == 1)
        return ImageBuffer(w * elempack, (void*)0);
    if (dims == 2)
        return ImageBuffer(w, h * elempack, (void*)0);
    if (dims == 3)
        return ImageBuffer(w, h, c * elempack, (void*)0);

    return ImageBuffer();
}

inline VkImage VkImageMat::image() const
{
    return data->image;
}

inline VkImageView VkImageMat::imageview() const
{
    return data->imageview;
}

extern const ImageBuffer * color_table[2][2][4];

} // namespace ImVulkan 
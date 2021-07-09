#ifndef __IMGUI_MAT_H__
#define __IMGUI_MAT_H__
#include <imgui.h>
#if __AVX__
// the alignment of all the allocated buffers
#define IM_MALLOC_ALIGN 32
#else
// the alignment of all the allocated buffers
#define IM_MALLOC_ALIGN 16
#endif

// exchange-add operation for atomic operations on reference counters
#if defined __riscv && !defined __riscv_atomic
// riscv target without A extension
static inline int IM_XADD(int* addr, int delta)
{
    int tmp = *addr;
    *addr += delta;
    return tmp;
}
#elif defined __INTEL_COMPILER && !(defined WIN32 || defined _WIN32)
// atomic increment on the linux version of the Intel(tm) compiler
#define IM_XADD(addr, delta) (int)_InterlockedExchangeAdd(const_cast<void*>(reinterpret_cast<volatile void*>(addr)), delta)
#elif defined __GNUC__
#if defined __clang__ && __clang_major__ >= 3 && !defined __ANDROID__ && !defined __EMSCRIPTEN__ && !defined(__CUDACC__)
#ifdef __ATOMIC_ACQ_REL
#define IM_XADD(addr, delta) __c11_atomic_fetch_add((_Atomic(int)*)(addr), delta, __ATOMIC_ACQ_REL)
#else
#define IM_XADD(addr, delta) __atomic_fetch_add((_Atomic(int)*)(addr), delta, 4)
#endif
#else
#if defined __ATOMIC_ACQ_REL && !defined __clang__
// version for gcc >= 4.7
#define IM_XADD(addr, delta) (int)__atomic_fetch_add((unsigned*)(addr), (unsigned)(delta), __ATOMIC_ACQ_REL)
#else
#define IM_XADD(addr, delta) (int)__sync_fetch_and_add((unsigned*)(addr), (unsigned)(delta))
#endif
#endif
#elif defined _MSC_VER && !defined RC_INVOKED
#define IM_XADD(addr, delta) (int)_InterlockedExchangeAdd((long volatile*)addr, delta)
#else
// thread-unsafe branch
static inline int IM_XADD(int* addr, int delta)
{
    int tmp = *addr;
    *addr += delta;
    return tmp;
}
#endif

//////////////////////////////////////////////////
//  memory functions
/////////////////////////////////////////////////
template<typename _Tp>
inline _Tp* Im_AlignPtr(_Tp* ptr, int n = (int)sizeof(_Tp))
{
    return (_Tp*)(((size_t)ptr + n - 1) & -n);
}
inline size_t Im_AlignSize(size_t sz, int n)
{
    return (sz + n - 1) & -n;
}
inline void* Im_FastMalloc(size_t size)
{
#if _MSC_VER
    return _aligned_malloc(size, IM_MALLOC_ALIGN);
#elif (defined(__unix__) || defined(__APPLE__)) && _POSIX_C_SOURCE >= 200112L || (__ANDROID__ && __ANDROID_API__ >= 17)
    void* ptr = 0;
    if (posix_memalign(&ptr, IM_MALLOC_ALIGN, size))
        ptr = 0;
    return ptr;
#elif __ANDROID__ && __ANDROID_API__ < 17
    return memalign(IM_MALLOC_ALIGN, size);
#else
    unsigned char* udata = (unsigned char*)malloc(size + sizeof(void*) + IM_MALLOC_ALIGN);
    if (!udata)
        return 0;
    unsigned char** adata = Im_AlignPtr((unsigned char**)udata + 1, IM_MALLOC_ALIGN);
    adata[-1] = udata;
    return adata;
#endif
}
inline void Im_FastFree(void* ptr)
{
    if (ptr)
    {
#if _MSC_VER
        _aligned_free(ptr);
#elif (defined(__unix__) || defined(__APPLE__)) && _POSIX_C_SOURCE >= 200112L || (__ANDROID__ && __ANDROID_API__ >= 17)
        free(ptr);
#elif __ANDROID__ && __ANDROID_API__ < 17
        free(ptr);
#else
        unsigned char* udata = ((unsigned char**)ptr)[-1];
        free(udata);
#endif
    }
}

////////////////////////////////////////////////////////////////////
// Type define
enum ImDataType {
    IM_DT_INT8 = 0,
    IM_DT_INT16,
    IM_DT_INT32,
    IM_DT_INT64,
    IM_DT_FLOAT16,
    IM_DT_FLOAT32,
    IM_DT_FLOAT64,
    IM_DT_NB_DATA_TYPE
};

enum ImDataDevice {
    IM_DD_CPU = 0,
    IM_DD_VULKAN,
    IM_DD_CUDA,
};

enum ImColorRange {
    IM_CR_FULL_RANGE = 0,
    IM_CR_NARROW_RANGE
};

enum ImColorSpace {
    IM_CS_SRGB = 0,
    IM_CS_BT601,
    IM_CS_BT709,
    IM_CS_BT2020,
    IM_CS_HSV,
    IM_CS_HLS,
    IM_CS_CMY,
    IM_CS_LAB
};

enum ImColorFormat {
    IM_CF_GRAY = 0,
    IM_CF_BGR,
    IM_CF_ABGR,
    IM_CF_RGB,
    IM_CF_ARGB,
    IM_CF_YUV420,
    IM_CF_YUV422,
    IM_CF_YUV444,
    IM_CF_YUVA,
    IM_CF_NV12,
};

enum ImInterpolateMode {
    IM_INTERPOLATE_NEAREST = 0,
    IM_INTERPOLATE_BILINEAR,
    IM_INTERPOLATE_BICUBIC,
    IM_INTERPOLATE_AREA,
    IM_INTERPOLATE_TRILINEAR,
    IM_INTERPOLATE_TETRAHEDRAL,
    IM_NB_INTERP_MODE
};

#define IM_ESIZE(a)    (a == IM_DT_INT8 ? (size_t)1u : (a == IM_DT_INT16 || a == IM_DT_FLOAT16) ? (size_t)2u : (a == IM_DT_INT32 || a == IM_DT_FLOAT32) ? (size_t)4u : (a == IM_DT_INT64 || a == IM_DT_FLOAT64) ? (size_t)8u : (size_t)0u)
#define IM_ISMONO(a)   (a == IM_CF_GRAY)
#define IM_ISRGB(a)    (a == IM_CF_BGR || a == IM_CF_RGB || a == IM_CF_ABGR || a == IM_CF_ARGB)
#define IM_ISYUV(a)    (a == IM_CF_YUV420 || a == IM_CF_YUV422 || a == IM_CF_YUV444 || a == IM_CF_YUVA || a == IM_CF_NV12)
#define IM_ISALPHA(a)  (a == IM_CF_ABGR || a == IM_CF_ARGB || a == IM_CF_YUVA)

////////////////////////////////////////////////////////////////////

namespace ImGui 
{
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// Allocator Class define
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////

class Allocator
{
public:
    virtual ~Allocator() {};
    virtual void* fastMalloc(size_t size) = 0;
    virtual void* fastMalloc(int w, int h, int c, size_t elemsize, int elempack) = 0;
    virtual void fastFree(void* ptr) = 0;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// ImMat Class define
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
class ImMat
{
public:
    // empty
    ImMat();
    // vec
    ImMat(int w, size_t elemsize = 4u, Allocator* allocator = 0);
    // image
    ImMat(int w, int h, size_t elemsize = 4u, Allocator* allocator = 0);
    // dim
    ImMat(int w, int h, int c, size_t elemsize = 4u, Allocator* allocator = 0);
    // packed vec
    ImMat(int w, size_t elemsize, int elempack, Allocator* allocator = 0);
    // packed image
    ImMat(int w, int h, size_t elemsize, int elempack, Allocator* allocator = 0);
    // packed dim
    ImMat(int w, int h, int c, size_t elemsize, int elempack, Allocator* allocator = 0);
    // copy
    ImMat(const ImMat& m);
    // external vec
    ImMat(int w, void* data, size_t elemsize = 4u, Allocator* allocator = 0);
    // external image
    ImMat(int w, int h, void* data, size_t elemsize = 4u, Allocator* allocator = 0);
    // external dim
    ImMat(int w, int h, int c, void* data, size_t elemsize = 4u, Allocator* allocator = 0);
    // external packed vec
    ImMat(int w, void* data, size_t elemsize, int elempack, Allocator* allocator = 0);
    // external packed image
    ImMat(int w, int h, void* data, size_t elemsize, int elempack, Allocator* allocator = 0);
    // external packed dim
    ImMat(int w, int h, int c, void* data, size_t elemsize, int elempack, Allocator* allocator = 0);
    // release
    ~ImMat();
    // assign
    ImMat& operator=(const ImMat& m);
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
    // allocate vec with type
    void create_type(int w, ImDataType t = IM_DT_INT8, Allocator* allocator = 0);
    // allocate image with type
    void create_type(int w, int h, ImDataType t = IM_DT_INT8, Allocator* allocator = 0);
    // allocate dim with type
    void create_type(int w, int h, int c, ImDataType t = IM_DT_INT8, Allocator* allocator = 0);
    // extern vec with type
    void create_type(int w, void* data, ImDataType t = IM_DT_INT8, Allocator* allocator = 0);
    // extern image with type
    void create_type(int w, int h, void* data, ImDataType t = IM_DT_INT8, Allocator* allocator = 0);
    // extern dim with type
    void create_type(int w, int h, int c, void* data, ImDataType t = IM_DT_INT8, Allocator* allocator = 0);
    // allocate like
    void create_like(const ImMat& m, Allocator* allocator = 0);

    // set all
    template<typename T> void fill(T v);
    // deep copy
    ImMat clone(Allocator* allocator = 0) const;
    // deep copy from other buffer, inplace
    void clone_from(const ImMat& mat, Allocator* allocator = 0);
    // reshape vec
    ImMat reshape(int w, Allocator* allocator = 0) const;
    // reshape image
    ImMat reshape(int w, int h, Allocator* allocator = 0) const;
    // reshape dim
    ImMat reshape(int w, int h, int c, Allocator* allocator = 0) const;

    // refcount++
    void addref();
    // refcount--
    void release();

    bool empty() const;
    size_t total() const;

    // bits per element
    int elembits() const;

    // shape only
    ImMat shape() const;

    // data reference
    ImMat channel(int c);
    const ImMat channel(int c) const;
    float* row(int y);
    const float* row(int y) const;
    template<typename T>
    T* row(int y);
    template<typename T>
    const T* row(int y) const;

    // range reference
    ImMat channel_range(int c, int channels);
    const ImMat channel_range(int c, int channels) const;
    ImMat row_range(int y, int rows);
    const ImMat row_range(int y, int rows) const;
    ImMat range(int x, int n);
    const ImMat range(int x, int n) const;

    // access raw data
    template<typename T> operator T*();
    template<typename T> operator const T*() const;

    // access element data
    template<typename _Tp> _Tp& at(int i=0) 
    {
        IM_ASSERT(device == IM_DD_CPU && dims == 1);
        return *(_Tp*)((unsigned char*)data + i * elemsize); 
    };
    template<typename _Tp> const _Tp& at(int i=0) const 
    {
        IM_ASSERT(device == IM_DD_CPU && dims == 1);
        return *(const _Tp*)((unsigned char*)data + i * elemsize); 
    };
    template<typename _Tp> _Tp& at(int x, int y) 
    {
        IM_ASSERT(device == IM_DD_CPU && dims == 2);
        return *(_Tp*)((unsigned char*)data + (y * w + x) * elemsize); 
    };
    template<typename _Tp> const _Tp& at(int x, int y) const 
    {
        IM_ASSERT(device == IM_DD_CPU && dims == 2);
        return *(const _Tp*)((unsigned char*)data + (y * w + x) * elemsize); 
    };
    template<typename _Tp> _Tp& at(int x, int y, int _c) 
    {
        IM_ASSERT(device == IM_DD_CPU && dims == 3);
        if (elempack == 1)
            return *(_Tp*)((unsigned char*)data + _c * cstep * elemsize + (y * w + x) * elemsize); 
        else
            return *(_Tp*)((unsigned char*)data + (y * w + x) * elemsize * c + _c); 
    };
    template<typename _Tp> const _Tp& at(int x, int y, int _c) const 
    {
        IM_ASSERT(device == IM_DD_CPU && dims == 3);
        if (elempack == 1)
            return *(const _Tp*)((unsigned char*)data + _c * cstep * elemsize + (y * w + x) * elemsize);
        else
            return *(const _Tp*)((unsigned char*)data + (y * w + x) * elemsize * c + _c); 
    };

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

    // data device
    // 0 = cpu
    // 1 = vulkan
    // 2 = cuda
    ImDataDevice device;

    // device number
    // 0 = cpu
    // 0 - n = gpu index
    int device_number;

    // time stamp
    double time_stamp;

    // type
    // 0 = INT8/UINT8
    // 1 = INT16/UINT16
    // 2 = INT32/UINT32
    // 3 = INT64/UINT64
    // 4 = FLOAT16
    // 5 = FLOAT32
    // 6 = FLOAT64
    ImDataType type;

    // color
    // 0 = SRGB
    // 1 = BT601
    // 2 = BT709
    // 3 = BT2020
    ImColorSpace color_space;

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
    ImColorFormat color_format;

    // range
    // 0 = FULL_RANGE
    // 1 = NARROW_RANGE
    ImColorRange color_range;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// ImMat class
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
inline ImMat::ImMat()
    : data(0), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0), time_stamp(NAN)
{
    type = IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
}

inline ImMat::ImMat(int _w, size_t _elemsize, Allocator* _allocator)
    : data(0), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0), time_stamp(NAN)
{
    create(_w, _elemsize, _allocator);
}

inline ImMat::ImMat(int _w, int _h, size_t _elemsize, Allocator* _allocator)
    : data(0), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0), time_stamp(NAN)
{
    create(_w, _h, _elemsize, _allocator);
}

inline ImMat::ImMat(int _w, int _h, int _c, size_t _elemsize, Allocator* _allocator)
    : data(0), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0), time_stamp(NAN)
{
    create(_w, _h, _c, _elemsize, _allocator);
}

inline ImMat::ImMat(int _w, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(0), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0), time_stamp(NAN)
{
    create(_w, _elemsize, _elempack, _allocator);
}

inline ImMat::ImMat(int _w, int _h, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(0), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0), time_stamp(NAN)
{
    create(_w, _h, _elemsize, _elempack, _allocator);
}

inline ImMat::ImMat(int _w, int _h, int _c, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(0), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(0), elempack(0), allocator(0), dims(0), w(0), h(0), c(0), cstep(0), time_stamp(NAN)
{
    create(_w, _h, _c, _elemsize, _elempack, _allocator);
}

inline ImMat::ImMat(const ImMat& m)
    : data(m.data), device(m.device), device_number(m.device_number), refcount(m.refcount), elemsize(m.elemsize), elempack(m.elempack), allocator(m.allocator), dims(m.dims), w(m.w), h(m.h), c(m.c), cstep(m.cstep), time_stamp(m.time_stamp)
{
    if (refcount) IM_XADD(refcount, 1);

    cstep = m.cstep;
    type = m.type;
    color_format = m.color_format;
    color_space = m.color_space;
    color_range = m.color_range;
}

inline ImMat::ImMat(int _w, void* _data, size_t _elemsize, Allocator* _allocator)
    : data(_data), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(1), w(_w), h(1), c(1), time_stamp(NAN)
{
    cstep = w;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
}

inline ImMat::ImMat(int _w, int _h, void* _data, size_t _elemsize, Allocator* _allocator)
    : data(_data), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(2), w(_w), h(_h), c(1), time_stamp(NAN)
{
    cstep = (size_t)w * h;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
}

inline ImMat::ImMat(int _w, int _h, int _c, void* _data, size_t _elemsize, Allocator* _allocator)
    : data(_data), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(_elemsize), elempack(1), allocator(_allocator), dims(3), w(_w), h(_h), c(_c), time_stamp(NAN)
{
    cstep = Im_AlignSize((size_t)w * h * elemsize, 16) / elemsize;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = c == 1 ? IM_CF_GRAY : c == 3 ? IM_CF_RGB : IM_CF_ARGB;
    color_range = IM_CR_FULL_RANGE;
}

inline ImMat::ImMat(int _w, void* _data, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(_data), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(1), w(_w), h(1), c(1), time_stamp(NAN)
{
    cstep = w;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
}

inline ImMat::ImMat(int _w, int _h, void* _data, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(_data), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(2), w(_w), h(_h), c(1), time_stamp(NAN)
{
    cstep = (size_t)w * h;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
}

inline ImMat::ImMat(int _w, int _h, int _c, void* _data, size_t _elemsize, int _elempack, Allocator* _allocator)
    : data(_data), device(IM_DD_CPU), device_number(0), refcount(0), elemsize(_elemsize), elempack(_elempack), allocator(_allocator), dims(3), w(_w), h(_h), c(_c), time_stamp(NAN)
{
    cstep = Im_AlignSize((size_t)w * h * elemsize, 16) / elemsize;
    type = _elemsize == 1 ? IM_DT_INT8 : _elemsize == 2 ? IM_DT_INT16 : IM_DT_FLOAT32;
    color_space = IM_CS_SRGB;
    color_format = c == 1 ? IM_CF_GRAY : c == 3 ? IM_CF_RGB : IM_CF_ARGB;
    color_range = IM_CR_FULL_RANGE;
}

inline ImMat::~ImMat()
{
    release();
}

inline ImMat& ImMat::operator=(const ImMat& m)
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

    return *this;
}

inline void ImMat::create(int _w, size_t _elemsize, Allocator* _allocator)
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

        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = Im_FastMalloc(totalsize + (int)sizeof(*refcount));
        if (!data)
            return;

        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImMat::create(int _w, int _h, size_t _elemsize, Allocator* _allocator)
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

        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = Im_FastMalloc(totalsize + (int)sizeof(*refcount));
        if (!data)
            return;

        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImMat::create(int _w, int _h, int _c, size_t _elemsize, Allocator* _allocator)
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

        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = Im_FastMalloc(totalsize + (int)sizeof(*refcount));
        if (!data)
            return;

        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImMat::create(int _w, size_t _elemsize, int _elempack, Allocator* _allocator)
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

        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = Im_FastMalloc(totalsize + (int)sizeof(*refcount));
        if (!data)
            return;

        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImMat::create(int _w, int _h, size_t _elemsize, int _elempack, Allocator* _allocator)
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

        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = Im_FastMalloc(totalsize + (int)sizeof(*refcount));
        if (!data)
            return;

        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImMat::create(int _w, int _h, int _c, size_t _elemsize, int _elempack, Allocator* _allocator)
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

        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = Im_FastMalloc(totalsize + (int)sizeof(*refcount));
        if (!data)
            return;

        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImMat::create_type(int _w, ImDataType _t, Allocator* _allocator)
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

        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = Im_FastMalloc(totalsize + (int)sizeof(*refcount));
        if (!data)
            return;

        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImMat::create_type(int _w, int _h, ImDataType _t, Allocator* _allocator)
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

        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = Im_FastMalloc(totalsize + (int)sizeof(*refcount));
        if (!data)
            return;

        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImMat::create_type(int _w, int _h, int _c, ImDataType _t, Allocator* _allocator)
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

        if (allocator)
            data = allocator->fastMalloc(totalsize + (int)sizeof(*refcount));
        else
            data = Im_FastMalloc(totalsize + (int)sizeof(*refcount));
        if (!data)
            return;

        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
}

inline void ImMat::create_type(int _w, void* _data, ImDataType _t, Allocator* _allocator)
{
    if (dims == 1 && w == _w && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = IM_ESIZE(_t);
    elempack = 1;
    allocator = _allocator;
    refcount = 0;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;

    cstep = w;
    type = _t;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
    data = _data;
}

inline void ImMat::create_type(int _w, int _h, void* _data, ImDataType _t, Allocator* _allocator)
{
    if (dims == 2 && w == _w && h == _h && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();

    elemsize = IM_ESIZE(_t);
    elempack = 1;
    allocator = _allocator;
    refcount = 0;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;

    cstep = (size_t)w * h;
    type = _t;
    color_space = IM_CS_SRGB;
    color_format = IM_CF_GRAY;
    color_range = IM_CR_FULL_RANGE;
    data = _data;
}

inline void ImMat::create_type(int _w, int _h, int _c, void* _data, ImDataType _t, Allocator* _allocator)
{
    if (dims == 3 && w == _w && h == _h && c == _c && elempack == 1 && type == _t && allocator == _allocator)
        return;

    release();
 
    elemsize = IM_ESIZE(_t);
    elempack = 1;
    allocator = _allocator;
    refcount = 0;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;

    cstep = Im_AlignSize((size_t)w * h * elemsize, 4) / elemsize;
    type = _t;
    color_space = IM_CS_SRGB;
    color_format = c == 1 ? IM_CF_GRAY : c == 3 ? IM_CF_RGB : IM_CF_ARGB;
    color_range = IM_CR_FULL_RANGE;
    data = _data;
}

inline void ImMat::create_like(const ImMat& m, Allocator* _allocator)
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

inline void ImMat::addref()
{
    if (refcount) IM_XADD(refcount, 1);
}

inline void ImMat::release()
{
    if (refcount && IM_XADD(refcount, -1) == 1)
    {
        if (device == IM_DD_CPU)
        {
            if (allocator && data)
                allocator->fastFree(data);
            else if (data)
                Im_FastFree(data);
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
    time_stamp = NAN;
}

inline bool ImMat::empty() const
{
    return data == 0 || total() == 0;
}

inline size_t ImMat::total() const
{
    return cstep * c;
}

inline int ImMat::elembits() const
{
    return elempack ? static_cast<int>(elemsize * 8) / elempack : 0;
}

inline ImMat ImMat::shape() const
{
    if (dims == 1)
        return ImMat(w * elempack, (void*)0);
    if (dims == 2)
        return ImMat(w, h * elempack, (void*)0);
    if (dims == 3)
        return ImMat(w, h, c * elempack, (void*)0);

    return ImMat();
}

inline ImMat ImMat::channel(int _c)
{
    return ImMat(w, h, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack, allocator);
}

inline const ImMat ImMat::channel(int _c) const
{
    return ImMat(w, h, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack, allocator);
}

inline float* ImMat::row(int y)
{
    return (float*)((unsigned char*)data + (size_t)w * y * elemsize);
}

inline const float* ImMat::row(int y) const
{
    return (const float*)((unsigned char*)data + (size_t)w * y * elemsize);
}

template<typename T>
inline T* ImMat::row(int y)
{
    return (T*)((unsigned char*)data + (size_t)w * y * elemsize);
}

template<typename T>
inline const T* ImMat::row(int y) const
{
    return (const T*)((unsigned char*)data + (size_t)w * y * elemsize);
}

inline ImMat ImMat::channel_range(int _c, int channels)
{
    return ImMat(w, h, channels, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack, allocator);
}

inline const ImMat ImMat::channel_range(int _c, int channels) const
{
    return ImMat(w, h, channels, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack, allocator);
}

inline ImMat ImMat::row_range(int y, int rows)
{
    return ImMat(w, rows, (unsigned char*)data + (size_t)w * y * elemsize, elemsize, elempack, allocator);
}

inline const ImMat ImMat::row_range(int y, int rows) const
{
    return ImMat(w, rows, (unsigned char*)data + (size_t)w * y * elemsize, elemsize, elempack, allocator);
}

inline ImMat ImMat::range(int x, int n)
{
    return ImMat(n, (unsigned char*)data + x * elemsize, elemsize, elempack, allocator);
}

inline const ImMat ImMat::range(int x, int n) const
{
    return ImMat(n, (unsigned char*)data + x * elemsize, elemsize, elempack, allocator);
}

template<typename T>
inline ImMat::operator T*()
{
    return (T*)data;
}

template<typename T>
inline ImMat::operator const T*() const
{
    return (const T*)data;
}

inline float& ImMat::operator[](size_t i)
{
    return ((float*)data)[i];
}

inline const float& ImMat::operator[](size_t i) const
{
    return ((const float*)data)[i];
}

template<typename T>
inline void ImMat::fill(T _v)
{
    int size = (int)total();
    T* ptr = (T*)data;
    for (int i = 0; i < size; i++)
    {
        ptr[i] = _v;
    }
}

inline ImMat ImMat::clone(Allocator* _allocator) const
{
    if (empty())
        return ImMat();

    ImMat m;
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

inline void ImMat::clone_from(const ImMat& mat, Allocator* allocator)
{
    *this = mat.clone(allocator);
}

inline ImMat ImMat::reshape(int _w, Allocator* _allocator) const
{
    if (w * h * c != _w)
        return ImMat();

    if (dims == 3 && cstep != (size_t)w * h)
    {
        ImMat m;
        m.create(_w, elemsize, elempack, _allocator);
        if (!m.data)
            return m;
        // flatten
        for (int i = 0; i < c; i++)
        {
            const void* ptr = (unsigned char*)data + i * cstep * elemsize;
            void* mptr = (unsigned char*)m.data + (size_t)i * w * h * elemsize;
            memcpy(mptr, ptr, (size_t)w * h * elemsize);
        }

        return m;
    }

    ImMat m = *this;

    m.dims = 1;
    m.w = _w;
    m.h = 1;
    m.c = 1;

    m.cstep = _w;
    m.color_format = IM_CF_GRAY;
    return m;
}

inline ImMat ImMat::reshape(int _w, int _h, Allocator* _allocator) const
{
    if (w * h * c != _w * _h)
        return ImMat();

    if (dims == 3 && cstep != (size_t)w * h)
    {
        ImMat m;
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

    ImMat m = *this;

    m.dims = 2;
    m.w = _w;
    m.h = _h;
    m.c = 1;
    m.color_format = IM_CF_GRAY;

    m.cstep = (size_t)_w * _h;

    return m;
}

inline ImMat ImMat::reshape(int _w, int _h, int _c, Allocator* _allocator) const
{
    if (w * h * c != _w * _h * _c)
        return ImMat();

    if (dims < 3)
    {
        if ((size_t)_w * _h != Im_AlignSize((size_t)_w * _h * elemsize, 16) / elemsize)
        {
            ImMat m;
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
        ImMat tmp = reshape(_w * _h * _c, _allocator);
        return tmp.reshape(_w, _h, _c, _allocator);
    }

    ImMat m = *this;

    m.dims = 3;
    m.w = _w;
    m.h = _h;
    m.c = _c;
    m.color_format = _c == 1 ? IM_CF_GRAY : _c == 3 ? IM_CF_RGB : IM_CF_ARGB;
    m.cstep = Im_AlignSize((size_t)_w * _h * elemsize, 16) / elemsize;

    return m;
}

} // namespace ImGui 

#endif /* __IMGUI_MAT_H__ */
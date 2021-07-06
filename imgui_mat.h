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
inline _Tp* alignPtr(_Tp* ptr, int n = (int)sizeof(_Tp))
{
    return (_Tp*)(((size_t)ptr + n - 1) & -n);
}
inline size_t alignSize(size_t sz, int n)
{
    return (sz + n - 1) & -n;
}
inline void* fastMalloc(size_t size)
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
    unsigned char** adata = alignPtr((unsigned char**)udata + 1, IM_MALLOC_ALIGN);
    adata[-1] = udata;
    return adata;
#endif
}
inline void fastFree(void* ptr)
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

namespace ImGui
{
enum ImMatDataType {
    IMMAT_INT8 = 0,
    IMMAT_INT16,
    IMMAT_INT32,
    IMMAT_INT64,
    IMMAT_FLOAT16,
    IMMAT_FLOAT32,
    IMMAT_FLOAT64,
    IMMAT_NB_DATA_TYPE
};

enum ImMatDataDevice {
    IMMAT_DD_CPU = 0,
    IMMAT_DD_VULKAN,
    IMMAT_DD_CUDA,
};

enum ImMatColorRange {
    IMMAT_FULL_RANGE = 0,
    IMMAT_NARROW_RANGE
};

enum ImMatColorSpace {
    IMMAT_SRGB = 0,
    IMMAT_BT601,
    IMMAT_BT709,
    IMMAT_BT2020,
    IMMAT_HSV,
    IMMAT_HLS,
    IMMAT_CMY,
    IMMAT_LAB
};

enum ImMatColorFormat {
    IMMAT_GRAY = 0,
    IMMAT_BGR,
    IMMAT_ABGR,
    IMMAT_RGB,
    IMMAT_ARGB,
    IMMAT_YUV420,
    IMMAT_YUV422,
    IMMAT_YUV444,
    IMMAT_YUVA,
    IMMAT_NV12,
};

enum ImMatInterpolateMode {
    IMMAT_INTERPOLATE_NEAREST = 0,
    IMMAT_INTERPOLATE_BILINEAR,
    IMMAT_INTERPOLATE_BICUBIC,
    IMMAT_INTERPOLATE_AREA,
    IMMAT_INTERPOLATE_TRILINEAR,
    IMMAT_INTERPOLATE_TETRAHEDRAL,
    IMMAT_NB_INTERP_MODE
};

#define IM_ESIZE(a)    (a == ImGui::IMMAT_INT8 ? (size_t)1u : (a == ImGui::IMMAT_INT16 || a == ImGui::IMMAT_FLOAT16) ? (size_t)2u : (a == ImGui::IMMAT_INT32 || a == ImGui::IMMAT_FLOAT32) ? (size_t)4u : (a == ImGui::IMMAT_INT64 || a == ImGui::IMMAT_FLOAT64) ? (size_t)8u : (size_t)0u)
#define IM_ISMONO(a)   (a == ImGui::IMMAT_GRAY)
#define IM_ISRGB(a)    (a == ImGui::IMMAT_BGR || a == ImGui::IMMAT_RGB || a == ImGui::IMMAT_ABGR || a == ImGui::IMMAT_ARGB)
#define IM_ISYUV(a)    (a == ImGui::IMMAT_YUV420 || a == ImGui::IMMAT_YUV422 || a == ImGui::IMMAT_YUV444 || a == ImGui::IMMAT_YUVA || a == ImGui::IMMAT_NV12)
#define IM_ISALPHA(a)  (a == ImGui::IMMAT_ABGR || a == ImGui::IMMAT_ARGB || a == ImGui::IMMAT_YUVA)

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
    ImMat(int w, size_t elemsize = 4u);
    // image
    ImMat(int w, int h, size_t elemsize = 4u);
    // dim
    ImMat(int w, int h, int c, size_t elemsize = 4u);
    // packed vec
    ImMat(int w, size_t elemsize, int elempack);
    // packed image
    ImMat(int w, int h, size_t elemsize, int elempack);
    // packed dim
    ImMat(int w, int h, int c, size_t elemsize, int elempack);
    // copy
    ImMat(const ImMat& m);
    // external vec
    ImMat(int w, void* data, size_t elemsize = 4u, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // external image
    ImMat(int w, int h, void* data, size_t elemsize = 4u, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // external dim
    ImMat(int w, int h, int c, void* data, size_t elemsize = 4u, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // external packed vec
    ImMat(int w, void* data, size_t elemsize, int elempack, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // external packed image
    ImMat(int w, int h, void* data, size_t elemsize, int elempack, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // external packed dim
    ImMat(int w, int h, int c, void* data, size_t elemsize, int elempack, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // release
    ~ImMat();
    // assign
    ImMat& operator=(const ImMat& m);
    // set all
    template<typename T> void fill(T v);
    // deep copy
    ImMat clone() const;
    // deep copy from other buffer, inplace
    void clone_from(const ImMat& mat);
    // reshape vec
    ImMat reshape(int w) const;
    // reshape image
    ImMat reshape(int w, int h) const;
    // reshape dim
    ImMat reshape(int w, int h, int c) const;
    // allocate vec
    void create(int w, size_t elemsize = 4u, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // allocate image
    void create(int w, int h, size_t elemsize = 4u, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // allocate dim
    void create(int w, int h, int c, size_t elemsize = 4u, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // allocate packed vec
    void create(int w, size_t elemsize, int elempack, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // allocate packed image
    void create(int w, int h, size_t elemsize, int elempack, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // allocate packed dim
    void create(int w, int h, int c, size_t elemsize, int elempack, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // allocate like
    void create_like(const ImMat& m);
    // allocate vec with type
    void create_type(int w, ImMatDataType t = IMMAT_INT8, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // allocate image with type
    void create_type(int w, int h, ImMatDataType t = IMMAT_INT8, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // allocate dim with type
    void create_type(int w, int h, int c, ImMatDataType t = IMMAT_INT8, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // extern vec with type
    void create_type(int w, void* data, ImMatDataType t = IMMAT_INT8, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // extern image with type
    void create_type(int w, int h, void* data, ImMatDataType t = IMMAT_INT8, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);
    // extern dim with type
    void create_type(int w, int h, int c, void* data, ImMatDataType t = IMMAT_INT8, ImMatDataDevice _device = IMMAT_DD_CPU, int _dev_num = -1);

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
    template<typename T> T* row(int y);
    template<typename T> const T* row(int y) const;

    template<typename _Tp> _Tp& at(int i=0) 
    {
        IM_ASSERT(device == IMMAT_DD_CPU && dims == 1);
        return *(_Tp*)((unsigned char*)data + i * elemsize * elempack); 
    };
    template<typename _Tp> const _Tp& at(int i=0) const 
    {
        IM_ASSERT(device == IMMAT_DD_CPU && dims == 1);
        return *(const _Tp*)((unsigned char*)data + i * elemsize * elempack); 
    };
    template<typename _Tp> _Tp& at(int x, int y) 
    {
        IM_ASSERT(device == IMMAT_DD_CPU && dims == 2);
        return *(_Tp*)((unsigned char*)data + (y * w + x) * elemsize * elempack); 
    };
    template<typename _Tp> const _Tp& at(int x, int y) const 
    {
        IM_ASSERT(device == IMMAT_DD_CPU && dims == 2);
        return *(const _Tp*)((unsigned char*)data + (y * w + x) * elemsize * elempack); 
    };
    template<typename _Tp> _Tp& at(int x, int y, int _c) 
    {
        IM_ASSERT(device == IMMAT_DD_CPU && dims == 3);
        return *(_Tp*)((unsigned char*)data + _c * cstep * elemsize * elempack + (y * w + x) * elemsize * elempack); 
    };
    template<typename _Tp> const _Tp& at(int x, int y, int _c) const 
    {
        IM_ASSERT(device == IMMAT_DD_CPU && dims == 3);
        return *(const _Tp*)((unsigned char*)data + _c * cstep * elemsize * elempack + (y * w + x) * elemsize * elempack); 
    };

    // range reference
    ImMat channel_range(int c, int channels);
    const ImMat channel_range(int c, int channels) const;
    ImMat row_range(int y, int rows);
    const ImMat row_range(int y, int rows) const;
    ImMat range(int x, int n);
    const ImMat range(int x, int n) const;

    // access raw data
    template<typename T>
    operator T*();
    template<typename T>
    operator const T*() const;

    // convenient access float vec element
    template<typename T>
    T& operator[](size_t i);
    template<typename T>
    const T& operator[](size_t i) const;

    // pointer to the data
    void* data;

    // data device
    // 0 = cpu
    // 1 = vulkan
    // 2 = cuda
    ImMatDataDevice device;

    // device number
    // -1 = cpu
    // 0 - n = gpu index
    int device_number;

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
    ImMatDataType type;

    // color
    // 0 = SRGB
    // 1 = BT601
    // 2 = BT709
    // 3 = BT2020
    ImMatColorSpace color_space;

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
    ImMatColorFormat color_format;

    // range
    // 0 = FULL_RANGE
    // 1 = NARROW_RANGE
    ImMatColorRange color_range;
};

//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
// ImMat class implementation
//////////////////////////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////////////////////////
inline ImMat::ImMat()
    : data(0), device(IMMAT_DD_CPU), device_number(-1), refcount(0), elemsize(0), elempack(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    type = IMMAT_FLOAT32;
    color_format = IMMAT_GRAY;
    color_space = IMMAT_SRGB;
    color_range = IMMAT_FULL_RANGE;
}

inline ImMat::ImMat(int _w, size_t _elemsize)
    : data(0), device(IMMAT_DD_CPU), device_number(-1), refcount(0), elemsize(0), elempack(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _elemsize);
}

inline ImMat::ImMat(int _w, int _h, size_t _elemsize)
    : data(0), device(IMMAT_DD_CPU), device_number(-1), refcount(0), elemsize(0), elempack(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _elemsize);
}

inline ImMat::ImMat(int _w, int _h, int _c, size_t _elemsize)
    : data(0), device(IMMAT_DD_CPU), device_number(-1), refcount(0), elemsize(0), elempack(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _c, _elemsize);
}

inline ImMat::ImMat(int _w, size_t _elemsize, int _elempack)
    : data(0), device(IMMAT_DD_CPU), device_number(-1), refcount(0), elemsize(0), elempack(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _elemsize, _elempack);
}

inline ImMat::ImMat(int _w, int _h, size_t _elemsize, int _elempack)
    : data(0), device(IMMAT_DD_CPU), device_number(-1), refcount(0), elemsize(0), elempack(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _elemsize, _elempack);
}

inline ImMat::ImMat(int _w, int _h, int _c, size_t _elemsize, int _elempack)
    : data(0), device(IMMAT_DD_CPU), device_number(-1), refcount(0), elemsize(0), elempack(0), dims(0), w(0), h(0), c(0), cstep(0)
{
    create(_w, _h, _c, _elemsize, _elempack);
}

inline ImMat::ImMat(const ImMat& m)
    : data(m.data), device(m.device), device_number(m.device_number), refcount(m.refcount), elemsize(m.elemsize), elempack(m.elempack), dims(m.dims), w(m.w), h(m.h), c(m.c), cstep(m.cstep)
{
    if (refcount) IM_XADD(refcount, 1);
    type = m.type;
    color_format = m.color_format;
    color_space = m.color_space;
    color_range = m.color_range;
}

inline ImMat::ImMat(int _w, void* _data, size_t _elemsize, ImMatDataDevice _device, int _dev_num)
    : data(_data), device(_device), device_number(_dev_num), refcount(0), elemsize(_elemsize), elempack(1), dims(1), w(_w), h(1), c(1)
{
    cstep = w;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;
}

inline ImMat::ImMat(int _w, int _h, void* _data, size_t _elemsize, ImMatDataDevice _device, int _dev_num)
    : data(_data), device(_device), device_number(_dev_num), refcount(0), elemsize(_elemsize), elempack(1), dims(2), w(_w), h(_h), c(1)
{
    cstep = (size_t)w * h;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;
}

inline ImMat::ImMat(int _w, int _h, int _c, void* _data, size_t _elemsize, ImMatDataDevice _device, int _dev_num)
    : data(_data), device(_device), device_number(_dev_num), refcount(0), elemsize(_elemsize), elempack(1), dims(3), w(_w), h(_h), c(_c)
{
    cstep = alignSize((size_t)w * h * elemsize, 16) / elemsize;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;
}

inline ImMat::ImMat(int _w, void* _data, size_t _elemsize, int _elempack, ImMatDataDevice _device, int _dev_num)
    : data(_data), device(_device), device_number(_dev_num), refcount(0), elemsize(_elemsize), elempack(_elempack), dims(1), w(_w), h(1), c(1)
{
    cstep = w;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;
}

inline ImMat::ImMat(int _w, int _h, void* _data, size_t _elemsize, int _elempack, ImMatDataDevice _device, int _dev_num)
    : data(_data), device(_device), device_number(_dev_num), refcount(0), elemsize(_elemsize), elempack(_elempack), dims(2), w(_w), h(_h), c(1)
{
    cstep = (size_t)w * h;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;
}

inline ImMat::ImMat(int _w, int _h, int _c, void* _data, size_t _elemsize, int _elempack, ImMatDataDevice _device, int _dev_num)
    : data(_data), device(_device), device_number(_dev_num), refcount(0), elemsize(_elemsize), elempack(_elempack), dims(3), w(_w), h(_h), c(_c)
{
    cstep = alignSize((size_t)w * h * elemsize, 16) / elemsize;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;
}

inline ImMat::~ImMat()
{
    release();
}

inline ImMat& ImMat::operator=(const ImMat& m)
{
    if (this == &m)
        return *this;

    if (m.refcount)
        IM_XADD(m.refcount, 1);

    release();

    data            = m.data;
    device          = m.device;
    device_number   = m.device_number;
    refcount        = m.refcount;
    elemsize        = m.elemsize;
    elempack        = m.elempack;

    dims            = m.dims;
    w               = m.w;
    h               = m.h;
    c               = m.c;

    cstep           = m.cstep;

    type            = m.type;
    color_space     = m.color_space;
    color_format    = m.color_format;
    color_range     = m.color_range;

    return *this;
}

template<typename T>
inline void ImMat::fill(T _v)
{
    int size = (int)total();
    if (device != IMMAT_DD_CPU)
        return;                 // TODO::Dicky fill with diffent backend?
    T* ptr = (T*)data;
    for (int i = 0; i < size; i++)
    {
        ptr[i] = _v;
    }
}

inline ImMat ImMat::clone() const
{
    if (empty())
        return ImMat();

    ImMat m;
    if (dims == 1)
        m.create(w, elemsize, elempack);
    else if (dims == 2)
        m.create(w, h, elemsize, elempack);
    else if (dims == 3)
        m.create(w, h, c, elemsize, elempack);

    if (device == IMMAT_DD_CPU && total() > 0)
    {
        memcpy(m.data, data, total() * elemsize);
    }
    else
    {
        m.data = data;  // TODO::Dicky data in gpu need counting reference?
    }
    m.color_format = color_format;
    m.color_range = color_range;
    m.color_space = color_space;
    m.type = type;
    m.device = device;
    m.device_number = device_number;
    return m;
}

inline void ImMat::clone_from(const ImMat& mat)
{
    *this = mat.clone();
}

inline ImMat ImMat::reshape(int _w) const
{
    if (w * h * c != _w)
        return ImMat();

    if (dims == 3 && cstep != (size_t)w * h)
    {
        ImMat m;
        m.create(_w, elemsize, elempack);

        if (device == IMMAT_DD_CPU && !m.data)
            return m;
        else if (m.device != IMMAT_DD_CPU)
        {
            m.device = device;
            m.device_number = device_number;
            return m;
        }
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
    m.color_format = IMMAT_GRAY;
    return m;
}

inline ImMat ImMat::reshape(int _w, int _h) const
{
    if (w * h * c != _w * _h)
        return ImMat();

    if (dims == 3 && cstep != (size_t)w * h)
    {
        ImMat m;
        m.create(_w, _h, elemsize, elempack);

        if (device == IMMAT_DD_CPU && !m.data)
            return m;
        else if (m.device != IMMAT_DD_CPU)
        {
            m.device = device;
            m.device_number = device_number;
            return m;
        }
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
    m.color_format = IMMAT_GRAY;

    m.cstep = (size_t)_w * _h;

    return m;
}

inline ImMat ImMat::reshape(int _w, int _h, int _c) const
{
    if (w * h * c != _w * _h * _c)
        return ImMat();

    if (dims < 3)
    {
        if ((size_t)_w * _h != alignSize((size_t)_w * _h * elemsize, 16) / elemsize)
        {
            ImMat m;
            m.create(_w, _h, _c, elemsize, elempack);
            
            if (device == IMMAT_DD_CPU && !m.data)
                return m;
            else if (m.device != IMMAT_DD_CPU)
            {
                m.device = device;
                m.device_number = device_number;
                return m;
            }

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
        ImMat tmp = reshape(_w * _h * _c);
        return tmp.reshape(_w, _h, _c);
    }

    ImMat m = *this;

    m.dims = 3;
    m.w = _w;
    m.h = _h;
    m.c = _c;
    m.color_format = _c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    m.cstep = alignSize((size_t)_w * _h * elemsize, 16) / elemsize;

    return m;
}

inline void ImMat::create(int _w, size_t _elemsize, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 1 && w == _w && elemsize == _elemsize && elempack == 1 && _device == device && _dev_num == device_number)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = w;

    if (_device == IMMAT_DD_CPU && total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
    else if (_device != IMMAT_DD_CPU)
    {
        device = _device;
        device_number = _dev_num;
        // TODO::Dicky
    }
}

inline void ImMat::create(int _w, int _h, size_t _elemsize, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 2 && w == _w && h == _h && elemsize == _elemsize && elempack == 1 && _device == device && _dev_num == device_number)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = (size_t)w * h;

    if (_device == IMMAT_DD_CPU && total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
    else if (_device != IMMAT_DD_CPU)
    {
        device = _device;
        device_number = _dev_num;
        // TODO::Dicky
    }
}

inline void ImMat::create(int _w, int _h, int _c, size_t _elemsize, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 3 && w == _w && h == _h && c == _c && elemsize == _elemsize && elempack == 1 && _device == device && _dev_num == device_number)
        return;

    release();

    elemsize = _elemsize;
    elempack = 1;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;

    cstep = alignSize((size_t)w * h * elemsize, 16) / elemsize;

    if (_device == IMMAT_DD_CPU && total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
    else if (_device != IMMAT_DD_CPU)
    {
        device = _device;
        device_number = _dev_num;
        // TODO::Dicky
    }
}

inline void ImMat::create(int _w, size_t _elemsize, int _elempack, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 1 && w == _w && elemsize == _elemsize && elempack == _elempack && _device == device && _dev_num == device_number)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = w;

    if (_device == IMMAT_DD_CPU && total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
    else if (_device != IMMAT_DD_CPU)
    {
        device = _device;
        device_number = _dev_num;
        // TODO::Dicky
    }
}

inline void ImMat::create(int _w, int _h, size_t _elemsize, int _elempack, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 2 && w == _w && h == _h && elemsize == _elemsize && elempack == _elempack && _device == device && _dev_num == device_number)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    cstep = (size_t)w * h;

    if (_device == IMMAT_DD_CPU && total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
    else if (_device != IMMAT_DD_CPU)
    {
        device = _device;
        device_number = _dev_num;
        // TODO::Dicky
    }
}

inline void ImMat::create(int _w, int _h, int _c, size_t _elemsize, int _elempack, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 3 && w == _w && h == _h && c == _c && elemsize == _elemsize && elempack == _elempack && _device == device && _dev_num == device_number)
        return;

    release();

    elemsize = _elemsize;
    elempack = _elempack;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;
    type = _elemsize == 1 ? IMMAT_INT8 : _elemsize == 2 ? IMMAT_INT16 : IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;


    cstep = alignSize((size_t)w * h * elemsize, 16) / elemsize;

    if (_device == IMMAT_DD_CPU && total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
    else if (_device != IMMAT_DD_CPU)
    {
        device = _device;
        device_number = _dev_num;
        // TODO::Dicky
    }
}

inline void ImMat::create_like(const ImMat& m)
{
    int _dims = m.dims;
    if (_dims == 1)
        create(m.w, m.elemsize, m.elempack, m.device, m.device_number);
    if (_dims == 2)
        create(m.w, m.h, m.elemsize, m.elempack, m.device, m.device_number);
    if (_dims == 3)
        create(m.w, m.h, m.c, m.elemsize, m.elempack, m.device, m.device_number);
    type = m.type;
    color_space = m.color_space;
    color_format = m.color_format;
    color_range = m.color_range;
}

// create with DataType
inline void ImMat::create_type(int _w, ImMatDataType _t, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 1 && w == _w && type == _t && device == _device && device_number == _dev_num)
        return;

    release();
    elemsize = IM_ESIZE(_t);
    elempack = 1;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;

    cstep = w;
    type = _t;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    if (_device == IMMAT_DD_CPU && total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
    else if (_device != IMMAT_DD_CPU)
    {
        device = _device;
        device_number = _dev_num;
        // TODO::Dicky
    }
}

inline void ImMat::create_type(int _w, int _h, ImMatDataType _t, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 2 && w == _w && h == _h && type == _t && device == _device && device_number == _dev_num)
        return;

    release();
    elemsize = IM_ESIZE(_t);
    elempack = 1;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;

    cstep = (size_t)w * h;
    type = _t;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;

    if (_device == IMMAT_DD_CPU && total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
    else if (_device != IMMAT_DD_CPU)
    {
        device = _device;
        device_number = _dev_num;
        // TODO::Dicky
    }
}

inline void ImMat::create_type(int _w, int _h, int _c, ImMatDataType _t, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 3 && w == _w && h == _h && c == _c && type == _t && device == _device && device_number == _dev_num)
        return;

    release();
    elemsize = IM_ESIZE(_t);
    elempack = 1;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;

    cstep = alignSize((size_t)w * h * elemsize, 4) / elemsize;
    type = _t;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;

    if (_device == IMMAT_DD_CPU && total() > 0)
    {
        size_t totalsize = alignSize(total() * elemsize, 4);
        data = fastMalloc(totalsize + (int)sizeof(*refcount));
        refcount = (int*)(((unsigned char*)data) + totalsize);
        *refcount = 1;
    }
    else if (_device != IMMAT_DD_CPU)
    {
        device = _device;
        device_number = _dev_num;
        // TODO::Dicky
    }
}

inline void ImMat::create_type(int _w, void* _data, ImMatDataType _t, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 1 && w == _w && type == _t && device == _device && device_number == _dev_num)
        return;

    release();
    elemsize = IM_ESIZE(_t);
    elempack = 1;
    refcount = 0;

    dims = 1;
    w = _w;
    h = 1;
    c = 1;

    cstep = w;
    type = _t;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;
    data = _data;
    device = _device;
    device_number = _dev_num;
}

inline void ImMat::create_type(int _w, int _h, void* _data, ImMatDataType _t, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 2 && w == _w && h == _h && type == _t && device == _device && device_number == _dev_num)
        return;

    release();
    elemsize = IM_ESIZE(_t);
    elempack = 1;
    refcount = 0;

    dims = 2;
    w = _w;
    h = _h;
    c = 1;

    cstep = (size_t)w * h;
    type = _t;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;
    data = _data;
    device = _device;
    device_number = _dev_num;
}

inline void ImMat::create_type(int _w, int _h, int _c, void* _data, ImMatDataType _t, ImMatDataDevice _device, int _dev_num)
{
    if (dims == 3 && w == _w && h == _h && c == _c && type == _t && device == _device && device_number == _dev_num)
        return;

    release();
    elemsize = IM_ESIZE(_t);
    elempack = 1;
    refcount = 0;

    dims = 3;
    w = _w;
    h = _h;
    c = _c;

    cstep = alignSize((size_t)w * h * elemsize, 4) / elemsize;
    type = _t;
    color_space = IMMAT_SRGB;
    color_format = c == 1 ? IMMAT_GRAY : IMMAT_RGB;
    color_range = IMMAT_FULL_RANGE;
    data = _data;
    device = _device;
    device_number = _dev_num;
}

inline void ImMat::addref()
{
    if (refcount) IM_XADD(refcount, 1);
}

inline void ImMat::release()
{
    if (refcount && IM_XADD(refcount, -1) == 1)
    {
        if (device == IMMAT_DD_CPU) 
        {
            fastFree(data);
            data = 0;
        }
        else
        {
            device = IMMAT_DD_CPU;
            device_number = -1;
            // TODO::Dicky
        }
    }

    elemsize = 0;
    elempack = 0;

    dims = 0;
    w = 0;
    h = 0;
    c = 0;

    cstep = 0;

    refcount = 0;

    type = IMMAT_FLOAT32;
    color_space = IMMAT_SRGB;
    color_format = IMMAT_GRAY;
    color_range = IMMAT_FULL_RANGE;
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
    if (device != IMMAT_DD_CPU)
        return ImMat();
    return ImMat(w, h, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack);
}

inline const ImMat ImMat::channel(int _c) const
{
    if (device != IMMAT_DD_CPU)
        return ImMat();
    return ImMat(w, h, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack);
}

inline float* ImMat::row(int y)
{
    if (device != IMMAT_DD_CPU)
        return 0;
    return (float*)((unsigned char*)data + (size_t)w * y * elemsize);
}

inline const float* ImMat::row(int y) const
{
    if (device != IMMAT_DD_CPU)
        return 0;
    return (const float*)((unsigned char*)data + (size_t)w * y * elemsize);
}

template<typename T>
inline T* ImMat::row(int y)
{
    if (device != IMMAT_DD_CPU)
        return 0;
    return (T*)((unsigned char*)data + (size_t)w * y * elemsize);
}

template<typename T>
inline const T* ImMat::row(int y) const
{
    if (device != IMMAT_DD_CPU)
        return 0;
    return (const T*)((unsigned char*)data + (size_t)w * y * elemsize);
}

inline ImMat ImMat::channel_range(int _c, int channels)
{
    if (device != IMMAT_DD_CPU)
        return ImMat();
    return ImMat(w, h, channels, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack);
}

inline const ImMat ImMat::channel_range(int _c, int channels) const
{
    if (device != IMMAT_DD_CPU)
        return ImMat();
    return ImMat(w, h, channels, (unsigned char*)data + cstep * _c * elemsize, elemsize, elempack);
}

inline ImMat ImMat::row_range(int y, int rows)
{
    if (device != IMMAT_DD_CPU)
        return ImMat();
    return ImMat(w, rows, (unsigned char*)data + (size_t)w * y * elemsize, elemsize, elempack);
}

inline const ImMat ImMat::row_range(int y, int rows) const
{
    if (device != IMMAT_DD_CPU)
        return ImMat();
    return ImMat(w, rows, (unsigned char*)data + (size_t)w * y * elemsize, elemsize, elempack);
}

inline ImMat ImMat::range(int x, int n)
{
    if (device != IMMAT_DD_CPU)
        return ImMat();
    return ImMat(n, (unsigned char*)data + x * elemsize, elemsize, elempack);
}

inline const ImMat ImMat::range(int x, int n) const
{
    if (device != IMMAT_DD_CPU)
        return ImMat();
    return ImMat(n, (unsigned char*)data + x * elemsize, elemsize, elempack);
}

template<typename T>
inline ImMat::operator T*()
{
    if (device != IMMAT_DD_CPU)
        return 0;
    return (T*)data;
}

template<typename T>
inline ImMat::operator const T*() const
{
    if (device != IMMAT_DD_CPU)
        return 0;
    return (const T*)data;
}

template<typename T>
inline T& ImMat::operator[](size_t i)
{
    if (device != IMMAT_DD_CPU)
        return (T)0;
    return ((T*)data)[i];
}

template<typename T>
inline const T& ImMat::operator[](size_t i) const
{
    if (device != IMMAT_DD_CPU)
        return (T)0;
    return ((T*)data)[i];
}

} // namespace ImGui

#endif /* __IMGUI_MAT_H__ */
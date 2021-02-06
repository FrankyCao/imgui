# include <imgui.h>
# define IMGUI_DEFINE_MATH_OPERATORS
# include <imgui_internal.h>
# include <application.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavutil/pixdesc.h>
#include <libswscale/swscale.h>
#include <libavcodec/avcodec.h>
#ifdef __cplusplus
}
#endif
#include "Config.h"

#if defined _MSC_VER && _MSC_VER >= 1200
#pragma warning( disable: 4244 4510 4610 )
#endif

#ifdef __GNUC__
#  pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

#if defined _WIN32
    #include <windows.h>
    #if defined _MSC_VER && _MSC_VER < 1900
    struct timespec
    {
        time_t tv_sec;
        long   tv_nsec;
    };
  #endif
#elif defined __linux__ || defined __APPLE__ || defined __HAIKU__
    #include <unistd.h>
    #include <stdio.h>
    #include <sys/types.h>
    #include <sys/time.h>
#if defined __APPLE__
    #include <sys/sysctl.h>
    #include <mach/clock.h>
    #include <mach/mach.h>
#endif
#endif

static std::string ini_file = std::string(DEFAULT_CONFIG_PATH) + "Application_Example_Video.ini";

#define LIBAVFORMAT_INTERRUPT_OPEN_TIMEOUT_MS 30000
#define LIBAVFORMAT_INTERRUPT_READ_TIMEOUT_MS 30000

struct AVInterruptCallbackMetadata
{
    timespec value;
    unsigned int timeout_after_ms;
    int timeout;
};

#ifdef _WIN32
// http://stackoverflow.com/questions/5404277/porting-clock-gettime-to-windows

static
inline LARGE_INTEGER get_filetime_offset()
{
    SYSTEMTIME s;
    FILETIME f;
    LARGE_INTEGER t;

    s.wYear = 1970;
    s.wMonth = 1;
    s.wDay = 1;
    s.wHour = 0;
    s.wMinute = 0;
    s.wSecond = 0;
    s.wMilliseconds = 0;
    SystemTimeToFileTime(&s, &f);
    t.QuadPart = f.dwHighDateTime;
    t.QuadPart <<= 32;
    t.QuadPart |= f.dwLowDateTime;
    return t;
}

static
inline void get_monotonic_time(timespec *tv)
{
    LARGE_INTEGER           t;
    FILETIME				f;
    double                  microseconds;
    static LARGE_INTEGER    offset;
    static double           frequencyToMicroseconds;
    static int              initialized = 0;
    static BOOL             usePerformanceCounter = 0;

    if (!initialized)
    {
        LARGE_INTEGER performanceFrequency;
        initialized = 1;
        usePerformanceCounter = QueryPerformanceFrequency(&performanceFrequency);
        if (usePerformanceCounter)
        {
            QueryPerformanceCounter(&offset);
            frequencyToMicroseconds = (double)performanceFrequency.QuadPart / 1000000.;
        }
        else
        {
            offset = get_filetime_offset();
            frequencyToMicroseconds = 10.;
        }
    }

    if (usePerformanceCounter)
    {
        QueryPerformanceCounter(&t);
    } else {
        GetSystemTimeAsFileTime(&f);
        t.QuadPart = f.dwHighDateTime;
        t.QuadPart <<= 32;
        t.QuadPart |= f.dwLowDateTime;
    }

    t.QuadPart -= offset.QuadPart;
    microseconds = (double)t.QuadPart / frequencyToMicroseconds;
    t.QuadPart = microseconds;
    tv->tv_sec = t.QuadPart / 1000000;
    tv->tv_nsec = (t.QuadPart % 1000000) * 1000;
}
#else
static inline void get_monotonic_time(timespec *time)
{
#if defined(__APPLE__) && defined(__MACH__)
    clock_serv_t cclock;
    mach_timespec_t mts;
    host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
    clock_get_time(cclock, &mts);
    mach_port_deallocate(mach_task_self(), cclock);
    time->tv_sec = mts.tv_sec;
    time->tv_nsec = mts.tv_nsec;
#else
    clock_gettime(CLOCK_MONOTONIC, time);
#endif
}
#endif

static inline timespec get_monotonic_time_diff(timespec start, timespec end)
{
    timespec temp;
    if (end.tv_nsec - start.tv_nsec < 0)
    {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else
    {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}

static inline double get_monotonic_time_diff_ms(timespec time1, timespec time2)
{
    timespec delta = get_monotonic_time_diff(time1, time2);
    double milliseconds = delta.tv_sec * 1000 + (double)delta.tv_nsec / 1000000.0;

    return milliseconds;
}

static inline int ffmpeg_interrupt_callback(void *ptr)
{
    AVInterruptCallbackMetadata* metadata = (AVInterruptCallbackMetadata*)ptr;
    assert(metadata);

    if (metadata->timeout_after_ms == 0)
    {
        return 0; // timeout is disabled
    }

    timespec now;
    get_monotonic_time(&now);

    metadata->timeout = get_monotonic_time_diff_ms(metadata->value, now) > metadata->timeout_after_ms;

    return metadata->timeout ? -1 : 0;
}

static enum AVPixelFormat hw_pix_fmt = AV_PIX_FMT_NONE;
static enum AVHWDeviceType hw_type = AV_HWDEVICE_TYPE_NONE;
static inline enum AVPixelFormat get_hw_format(AVCodecContext *ctx, const enum AVPixelFormat *pix_fmts)
{
    const enum AVPixelFormat *p;
    for (p = pix_fmts; *p != -1; p++) {
        if (*p == hw_pix_fmt)
            return *p;
    }
    fprintf(stderr, "Failed to get HW surface format.\n");
    return AV_PIX_FMT_NONE;
}

#define ISYUV420P(format)   \
        (format == AV_PIX_FMT_YUV420P || \
         format == AV_PIX_FMT_YUVJ420P || \
         format == AV_PIX_FMT_YUV420P9 || \
         format == AV_PIX_FMT_YUV420P10 || \
         format == AV_PIX_FMT_YUV420P12 || \
         format == AV_PIX_FMT_YUV420P14 || \
         format == AV_PIX_FMT_YUV420P16)

#define ISYUV422P(format) \
        (format == AV_PIX_FMT_YUV422P || \
         format == AV_PIX_FMT_YUVJ422P || \
         format == AV_PIX_FMT_YUV422P9 || \
         format == AV_PIX_FMT_YUV422P10 || \
         format == AV_PIX_FMT_YUV422P12 || \
         format == AV_PIX_FMT_YUV422P14 || \
         format == AV_PIX_FMT_YUV422P16)

#define ISYUV444P(format) \
        (format == AV_PIX_FMT_YUV444P || \
         format == AV_PIX_FMT_YUVJ420P || \
         format == AV_PIX_FMT_YUV444P9 || \
         format == AV_PIX_FMT_YUV444P10 || \
         format == AV_PIX_FMT_YUV444P12 || \
         format == AV_PIX_FMT_YUV444P14 || \
         format == AV_PIX_FMT_YUV444P16)

#define ISNV12(format) \
        (format == AV_PIX_FMT_NV12 || \
         format == AV_PIX_FMT_NV21 || \
         format == AV_PIX_FMT_NV16 || \
         format == AV_PIX_FMT_NV20LE || \
         format == AV_PIX_FMT_NV20BE || \
         format == AV_PIX_FMT_P010LE || \
         format == AV_PIX_FMT_P010BE || \
         format == AV_PIX_FMT_P016LE || \
         format == AV_PIX_FMT_P016BE || \
         format == AV_PIX_FMT_NV24 || \
         format == AV_PIX_FMT_NV42 || \
         format == AV_PIX_FMT_NV20)

#ifdef IMGUI_VULKAN_SHADER
static const char YUV2RGB8_data[] = R"(
#version 450
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require
#define GRAY    0
#define BGR     1
#define RGB     2
#define YUV420  3
#define YUV422  4
#define YUV444  5
#define NV12    6
layout (binding = 0) readonly buffer Y { uint8_t Y_data[]; };
layout (binding = 1) readonly buffer U { uint8_t U_data[]; };
layout (binding = 2) readonly buffer V { uint8_t V_data[]; };
layout (binding = 3) writeonly buffer Out { float Out_data[]; };
layout (binding = 4) writeonly buffer RGBA { uint8_t RGBA_data[]; };
layout (binding = 5) readonly buffer matix_y2r { float convert_matix_y2r[]; };
sfpmat3 matix_mat_y2r = {
    {sfp(convert_matix_y2r[0]), sfp(convert_matix_y2r[3]), sfp(convert_matix_y2r[6])},
    {sfp(convert_matix_y2r[1]), sfp(convert_matix_y2r[4]), sfp(convert_matix_y2r[7])},
    {sfp(convert_matix_y2r[2]), sfp(convert_matix_y2r[5]), sfp(convert_matix_y2r[8])},
};
layout (push_constant) uniform parameter
{
    int w;
    int h;
    int cstep;
    int in_format;
    int in_space;
    int in_range;
    float in_scale;
    int out_rgba;
} p;
sfpvec3 yuv_to_rgb(sfpvec3 yuv)
{
    sfpvec3 rgb;
    sfpvec3 yuv_offset = {sfp(0.f), sfp(0.5f), sfp(0.5f)};
    if (p.in_range == 1)
        yuv_offset.x = sfp(16.0f / 255.0f);
    rgb = matix_mat_y2r * (yuv - yuv_offset);
    return clamp(rgb, sfp(0.f), sfp(1.f));
}
sfpvec3 load_src_yuv(int x, int y, int z)
{
    sfpvec3 yuv_in = {sfp(0.f), sfp(0.5f), sfp(0.5f)};
    int uv_scale_w = p.in_format == YUV420 || p.in_format == YUV422 ? 2 : 1;
    int uv_scale_h = p.in_format == YUV420 || p.in_format == NV12 ? 2 : 1;
    int y_offset = y * p.w + x;
    int u_offset = (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w;
    int v_offset = (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w;
    ivec2 uv_offset = ((y / 2) * p.w / 2 + x / 2) * 2 + ivec2(0, 1);
    yuv_in.x = sfp(uint(Y_data[y_offset])) / sfp(p.in_scale);
    if (p.in_format == NV12)
    {
        yuv_in.y = sfp(uint(U_data[uv_offset.x])) / sfp(p.in_scale);
        yuv_in.z = sfp(uint(U_data[uv_offset.y])) / sfp(p.in_scale);
    }
    else
    {
        yuv_in.y = sfp(uint(U_data[u_offset])) / sfp(p.in_scale);
        yuv_in.z = sfp(uint(V_data[v_offset])) / sfp(p.in_scale);
    }
    return yuv_in;
}
void store_dst_rgb(int x, int y, int z, sfpvec3 rgb)
{
    if (p.out_rgba == 1)
    {
        ivec4 o_offset = (y * p.w + x) * p.cstep + ivec4(0, 1, 2, 3);
        RGBA_data[o_offset.r] = uint8_t(clamp(uint(floor(rgb.r * sfp(255.0))), 0, 255));
        RGBA_data[o_offset.g] = uint8_t(clamp(uint(floor(rgb.g * sfp(255.0))), 0, 255));
        RGBA_data[o_offset.b] = uint8_t(clamp(uint(floor(rgb.b * sfp(255.0))), 0, 255));
        RGBA_data[o_offset.a] = uint8_t(255);
    }
    else
    {
        ivec4 v_offset = (y * p.w + x) + ivec4(0, 1, 2, 3) * (p.w * p.h);
        buffer_st1(Out_data, v_offset.r, float(clamp(rgb.r, sfp(0.f), sfp(1.f))));
        buffer_st1(Out_data, v_offset.g, float(clamp(rgb.g, sfp(0.f), sfp(1.f))));
        buffer_st1(Out_data, v_offset.b, float(clamp(rgb.b, sfp(0.f), sfp(1.f))));
        buffer_st1(Out_data, v_offset.a, 1.f);
    }
}
void main()
{
    int gx = int(gl_GlobalInvocationID.x);
    int gy = int(gl_GlobalInvocationID.y);
    int gz = int(gl_GlobalInvocationID.z);
    sfpvec3 rgb = yuv_to_rgb(load_src_yuv(gx, gy, gz));
    store_dst_rgb(gx, gy, gz, rgb);
}
)";
static const char YUV2RGB16_data[] = R"(
#version 450
#extension GL_EXT_shader_8bit_storage: require
#extension GL_EXT_shader_16bit_storage: require
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require
#define GRAY    0
#define BGR     1
#define RGB     2
#define YUV420  3
#define YUV422  4
#define YUV444  5
#define NV12    6
layout (binding = 0) readonly buffer Y { uint16_t Y_data[]; };
layout (binding = 1) readonly buffer U { uint16_t U_data[]; };
layout (binding = 2) readonly buffer V { uint16_t V_data[]; };
layout (binding = 3) writeonly buffer Out { float Out_data[]; };
layout (binding = 4) writeonly buffer RGBA { uint8_t RGBA_data[]; };
layout (binding = 5) readonly buffer matix_y2r { float convert_matix_y2r[]; };
sfpmat3 matix_mat_y2r = {
    {sfp(convert_matix_y2r[0]), sfp(convert_matix_y2r[3]), sfp(convert_matix_y2r[6])},
    {sfp(convert_matix_y2r[1]), sfp(convert_matix_y2r[4]), sfp(convert_matix_y2r[7])},
    {sfp(convert_matix_y2r[2]), sfp(convert_matix_y2r[5]), sfp(convert_matix_y2r[8])},
};
layout (push_constant) uniform parameter
{
    int w;
    int h;
    int cstep;
    int in_format;
    int in_space;
    int in_range;
    float in_scale;
    int out_rgba;
} p;
sfpvec3 yuv_to_rgb(sfpvec3 yuv)
{
    sfpvec3 rgb;
    sfpvec3 yuv_offset = {sfp(0.f), sfp(0.5f), sfp(0.5f)};
    if (p.in_range == 1)
        yuv_offset.x = sfp(16.0f / 255.0f);
    rgb = matix_mat_y2r * (yuv - yuv_offset);
    return clamp(rgb, sfp(0.f), sfp(1.f));
}
sfpvec3 load_src_yuv(int x, int y, int z)
{
    sfpvec3 yuv_in = {sfp(0.f), sfp(0.5f), sfp(0.5f)};
    int uv_scale_w = p.in_format == YUV420 || p.in_format == YUV422 ? 2 : 1;
    int uv_scale_h = p.in_format == YUV420 || p.in_format == NV12 ? 2 : 1;
    int y_offset = y * p.w + x;
    int u_offset = (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w;
    int v_offset = (y / uv_scale_h) * p.w / uv_scale_w + x / uv_scale_w;
    ivec2 uv_offset = ((y / 2) * p.w / 2 + x / 2) * 2 + ivec2(0, 1);
    yuv_in.x = sfp(uint(Y_data[y_offset])) / sfp(p.in_scale);
    if (p.in_format == NV12)
    {
        yuv_in.y = sfp(uint(U_data[uv_offset.x])) / sfp(p.in_scale);
        yuv_in.z = sfp(uint(U_data[uv_offset.y])) / sfp(p.in_scale);
    }
    else
    {
        yuv_in.y = sfp(uint(U_data[u_offset])) / sfp(p.in_scale);
        yuv_in.z = sfp(uint(V_data[v_offset])) / sfp(p.in_scale);
    }
    return yuv_in;
}
void store_dst_rgb(int x, int y, int z, sfpvec3 rgb)
{
    if (p.out_rgba == 1)
    {
        ivec4 o_offset = (y * p.w + x) * p.cstep + ivec4(0, 1, 2, 3);
        RGBA_data[o_offset.r] = uint8_t(clamp(uint(floor(rgb.r * sfp(255.0))), 0, 255));
        RGBA_data[o_offset.g] = uint8_t(clamp(uint(floor(rgb.g * sfp(255.0))), 0, 255));
        RGBA_data[o_offset.b] = uint8_t(clamp(uint(floor(rgb.b * sfp(255.0))), 0, 255));
        RGBA_data[o_offset.a] = uint8_t(255);
    }
    else
    {
        ivec4 v_offset = (y * p.w + x) + ivec4(0, 1, 2, 3) * (p.w * p.h);
        buffer_st1(Out_data, v_offset.r, float(clamp(rgb.r, sfp(0.f), sfp(1.f))));
        buffer_st1(Out_data, v_offset.g, float(clamp(rgb.g, sfp(0.f), sfp(1.f))));
        buffer_st1(Out_data, v_offset.b, float(clamp(rgb.b, sfp(0.f), sfp(1.f))));
        buffer_st1(Out_data, v_offset.a, 1.f);
    }
}
void main()
{
    int gx = int(gl_GlobalInvocationID.x);
    int gy = int(gl_GlobalInvocationID.y);
    int gz = int(gl_GlobalInvocationID.z);
    sfpvec3 rgb = yuv_to_rgb(load_src_yuv(gx, gy, gz));
    store_dst_rgb(gx, gy, gz, rgb);
}
)";
#endif

class Example
{
public:
    Example() 
    {
        // load file dialog resource
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
        prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

        // init code
        memset(&packet, 0, sizeof(packet));

#ifdef IMGUI_VULKAN_SHADER
        vkdev = ImVulkan::get_gpu_device(0);
        blob_allocator = vkdev->acquire_blob_allocator();
        staging_allocator = vkdev->acquire_staging_allocator();
        opt.blob_vkallocator = blob_allocator;
        opt.staging_vkallocator = staging_allocator;
        opt.use_image_storage = true;
        opt.use_fp16_arithmetic = true;
        //opt.use_fp16_packed = true;
        //opt.use_fp16_storage = true;
        cmd = new ImVulkan::VkCompute(vkdev);
        std::vector<ImVulkan::vk_specialization_type> specializations(0);
        static std::vector<uint32_t> spirv_8;
        ImVulkan::compile_spirv_module(YUV2RGB8_data, opt, spirv_8);
        pipeline_8 = new ImVulkan::Pipeline(vkdev);
        pipeline_8->set_optimal_local_size_xyz(8, 8, 4);
        pipeline_8->create(spirv_8.data(), spirv_8.size() * 4, specializations);
        static std::vector<uint32_t> spirv_16;
        ImVulkan::compile_spirv_module(YUV2RGB16_data, opt, spirv_16);
        pipeline_16 = new ImVulkan::Pipeline(vkdev);
        pipeline_16->set_optimal_local_size_xyz(8, 8, 4);
        pipeline_16->create(spirv_16.data(), spirv_16.size() * 4, specializations);
        cmd->reset();
#endif
    }
    ~Example() 
    { 
        // Store file dialog bookmark
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
        end_file_dialog_demo_window(&filedialog, bookmark_path.c_str());
        CloseMedia();
#ifdef IMGUI_VULKAN_SHADER
        if (vkdev)
        {
            if (pipeline_8) { delete pipeline_8; pipeline_8 = nullptr; }
            if (pipeline_16) { delete pipeline_16; pipeline_16 = nullptr; }
            if (cmd) { delete cmd; cmd = nullptr; }
            if (blob_allocator) { vkdev->reclaim_blob_allocator(blob_allocator); blob_allocator = nullptr; }
            if (staging_allocator) { vkdev->reclaim_staging_allocator(staging_allocator); staging_allocator = nullptr; }
        }
#endif
    }
    void CloseMedia()
    {
        if (video_dec_ctx) { avcodec_free_context(&video_dec_ctx); video_dec_ctx = nullptr; }
        if (audio_dec_ctx) { avcodec_free_context(&audio_dec_ctx); audio_dec_ctx = nullptr; }
        if (fmt_ctx) { avformat_close_input(&fmt_ctx); fmt_ctx = nullptr; }
        if (video_texture) { ImGui::ImDestroyTexture(video_texture); video_texture = nullptr; }
        if (picture) { av_frame_free(&picture); picture = nullptr; }
        if (packet.data) { av_packet_unref(&packet); packet.data = nullptr; }
        if (img_convert_ctx) { sws_freeContext(img_convert_ctx); img_convert_ctx = nullptr; }
        if (hw_device_ctx) { av_buffer_unref(&hw_device_ctx); hw_device_ctx = nullptr; }
        video_codec_name = "";
        video_pfmt = AV_PIX_FMT_NONE;
        hw_pix_fmt = AV_PIX_FMT_NONE;
        hw_type = AV_HWDEVICE_TYPE_NONE;
        video_fps = 0;
        video_frames = 0;
        play_time = 0;
        total_time = 0;
        video_width = 0;
        video_height = 0;
        video_depth = 0;
        video_color_space = AVCOL_SPC_UNSPECIFIED;
        video_color_range = AVCOL_RANGE_UNSPECIFIED;

        audio_codec_name = "";
        audio_sfmt = AV_SAMPLE_FMT_NONE;
        audio_channels = 0;
        audio_sample_rate = 0;
        audio_depth = 0;

        is_playing = false;
        is_opened = false;
        first_frame_number = -1;
        frame_number = 0;
        picture_pts = AV_NOPTS_VALUE;
        audio_pts = AV_NOPTS_VALUE;
    }
    int OpenMediaFile(std::string filepath)
    {
        char buf[256];
        if (is_opened)
        {
            CloseMedia();
        }
        fmt_ctx = avformat_alloc_context();
        interrupt_metadata.timeout_after_ms = LIBAVFORMAT_INTERRUPT_OPEN_TIMEOUT_MS;
        get_monotonic_time(&interrupt_metadata.value);
        fmt_ctx->interrupt_callback.callback = ffmpeg_interrupt_callback;
        fmt_ctx->interrupt_callback.opaque = &interrupt_metadata;
        if (avformat_open_input(&fmt_ctx, filepath.c_str(), NULL, NULL) < 0) 
        {
            CloseMedia();
            return -1;
        }
        if (avformat_find_stream_info(fmt_ctx, NULL) < 0) 
        {
            CloseMedia();
            return -1;
        }
        if (open_codec_context(&video_stream_idx, &video_dec_ctx, AVMEDIA_TYPE_VIDEO) >= 0)
        {
            const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get(video_dec_ctx->pix_fmt);
            video_stream = fmt_ctx->streams[video_stream_idx];
            video_fps = r2d(video_stream->avg_frame_rate);
            video_frames = video_stream->nb_frames;
            video_width = video_dec_ctx->width;
            video_height = video_dec_ctx->height;
            video_pfmt = video_dec_ctx->pix_fmt;
            video_depth = video_dec_ctx->bits_per_raw_sample;
            if (video_depth == 0)
            {
                video_depth = desc->comp[0].depth;
            }
            video_color_space = video_dec_ctx->colorspace;
            video_color_range = video_dec_ctx->color_range;
            video_aspect_ratio = (float)video_width / ((float)video_height + 1e-10);
            avcodec_string(buf, sizeof(buf), video_dec_ctx, 0);
            video_codec_name = std::string(buf);
            picture = av_frame_alloc();
        }
        if (open_codec_context(&audio_stream_idx, &audio_dec_ctx, AVMEDIA_TYPE_AUDIO) >= 0)
        {
            audio_stream = fmt_ctx->streams[audio_stream_idx];
            audio_sfmt = audio_dec_ctx->sample_fmt;
            audio_channels = audio_dec_ctx->channels;
            audio_sample_rate = audio_dec_ctx->sample_rate;
            audio_depth = audio_dec_ctx->bits_per_coded_sample;
            avcodec_string(buf, sizeof(buf), audio_dec_ctx, 0);
            audio_codec_name = std::string(buf);
        }
        if (!audio_stream && !video_stream)
        {
            CloseMedia();
            return -1;
        }
        is_opened = true;
        total_time = (double)fmt_ctx->duration / (double)AV_TIME_BASE;
        if (total_time < 1e-6)
        {
            if (video_stream)
                total_time = (double)video_stream->duration * r2d(video_stream->time_base);
            else
                total_time = (double)audio_stream->duration * r2d(audio_stream->time_base);
        }
        /* interrupt callback */
        interrupt_metadata.timeout_after_ms = 0;
        img_convert_ctx = sws_getCachedContext(
                                            img_convert_ctx,
                                            video_dec_ctx->coded_width,
                                            video_dec_ctx->coded_height,
                                            video_pfmt,
                                            video_width,
                                            video_height,
                                            AV_PIX_FMT_RGBA,
                                            SWS_BICUBIC,
                                            NULL, NULL, NULL);

        return 0;
    }
    void SeekMedia(float sec)
    {
        if (!video_stream || !video_dec_ctx || video_fps == 0 || video_frames <= 0)
            return;
        int delta = 16;
        int64_t _frame_number = (int64_t)(sec * video_fps + 0.5);
        _frame_number = std::min(_frame_number, (int64_t)video_frames);
        if (first_frame_number < 0 && video_frames > 1)
            grabFrame(false);
        for(;;)
        {
            int64_t _frame_number_temp = std::max(_frame_number - delta, (int64_t)0);
            double sec = (double)_frame_number_temp / video_fps;
            int64_t time_stamp = video_stream->start_time;
            double  time_base  = r2d(video_stream->time_base);
            time_stamp += (int64_t)(sec / time_base + 0.5);
            if (video_frames > 1) 
                av_seek_frame(fmt_ctx, video_stream_idx, time_stamp, AVSEEK_FLAG_BACKWARD);
            avcodec_flush_buffers(video_dec_ctx);
            if (_frame_number > 0)
            {
                grabFrame(false);
                if (_frame_number > 1)
                {
                    frame_number = dts_to_frame_number(picture_pts) - first_frame_number;
                    if (frame_number < 0 || frame_number > _frame_number - 1)
                    {
                        if (_frame_number_temp == 0 || delta >= INT_MAX / 4)
                            break;
                        delta = delta < 16 ? delta * 2 : delta * 3 / 2;
                        continue;
                    }
                    while (frame_number < _frame_number - 1)
                    {
                        bool decode = false;
                        if (frame_number == _frame_number - 2)
                            decode = true;
                        if (!grabFrame(decode))
                            break;
                    }
                    frame_number++;
                    break;
                }
                else
                {
                    frame_number = 1;
                    break;
                }
            }
            else
            {
                frame_number = 0;
                break;
            }
        }
    }
    void PlayMedia()
    {
        if (grabFrame())
        {
            int ret = 0;
            AVFrame *tmp_frame = nullptr;
            AVFrame *sw_frame = av_frame_alloc();
            if (!sw_frame)
            {
                fprintf(stderr, "Can not alloc frame\n");
                return;
            }
            if (picture->format == hw_pix_fmt) 
            {
                /* retrieve data from GPU to CPU */
                if ((ret = av_hwframe_transfer_data(sw_frame, picture, 0)) < 0) 
                {
                    fprintf(stderr, "Error transferring the data to system memory\n");
                    av_frame_unref(sw_frame);
                    return;
                }
                else
                {
                    tmp_frame = sw_frame;
                }
            }
            else
                tmp_frame = picture;

#ifdef IMGUI_VULKAN_SHADER
            const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get((AVPixelFormat)tmp_frame->format);
            int video_shift = desc->comp[0].depth + desc->comp[0].shift;
            bool using_vkimage = true;
            if (video_width > (int)vkdev->info.max_image_dimension_3d() || video_height > (int)vkdev->info.max_image_dimension_3d() || video_depth > (int)vkdev->info.max_image_dimension_3d())
            {
                using_vkimage = false;
            }
            ImVulkan::ColorSpace color_space;
            switch (video_color_space)
            {
                case AVCOL_SPC_BT470BG:
                case AVCOL_SPC_SMPTE170M:
                case AVCOL_SPC_SMPTE240M:
                    color_space = ImVulkan::BT601;
                break;
                case AVCOL_SPC_BT709:
                    color_space = ImVulkan::BT709;
                break;
                case AVCOL_SPC_BT2020_NCL:
                case AVCOL_SPC_BT2020_CL:
                    color_space = ImVulkan::BT2020;
                break;
                default:
                    color_space = ImVulkan::BT709;
                    break;
            }
            ImVulkan::ColorRange color_range;
            switch (video_color_range)
            {
                case AVCOL_RANGE_MPEG:
                    color_range = ImVulkan::NARROW_RANGE;
                break;
                case AVCOL_RANGE_JPEG:
                    color_range = ImVulkan::FULL_RANGE;
                break;
                default:
                    color_range = ImVulkan::NARROW_RANGE;
                break;
            }
            if (matix_y2r_gpu.empty())
            {
                const ImVulkan::ImageBuffer conv_mat_y2r = *ImVulkan::color_table[0][color_range][color_space];
                cmd->record_clone(conv_mat_y2r, matix_y2r_gpu, opt);
                cmd->submit_and_wait();
                cmd->flash();
            }
            ImVulkan::ImageBuffer im_Y, im_U, im_V, im_RGB;
            ImVulkan::VkImageBuffer vk_Y, vk_U, vk_V, vk_RGB;
            if (using_vkimage)
            {
                vk_RGB.create_type(video_width, video_height, 4, ImVulkan::FLOAT32, blob_allocator);
            }
            else
            {
                im_RGB.create_type(video_width, video_height, 4, ImVulkan::INT8);
                vk_RGB.create_like(im_RGB, blob_allocator);
            }
            int UV_shift_w = ISYUV420P(tmp_frame->format) || ISYUV422P(tmp_frame->format) ? 1 : 0;
            int UV_shift_h = ISYUV420P(tmp_frame->format) || ISNV12(tmp_frame->format) ? 1 : 0; 
            im_Y.create_type(tmp_frame->width, tmp_frame->height, 1, tmp_frame->data[0], video_depth == 8 ? ImVulkan::INT8 : ImVulkan::INT16);
            cmd->record_clone(im_Y, vk_Y, opt);
            im_U.create_type(tmp_frame->width >> UV_shift_w, tmp_frame->height >> UV_shift_h, 1, tmp_frame->data[1], video_depth == 8 ? ImVulkan::INT8 : ImVulkan::INT16);
            cmd->record_clone(im_U, vk_U, opt);
            if (!ISNV12(tmp_frame->format))
            {
                im_V.create_type(tmp_frame->width >> UV_shift_w, tmp_frame->height >> UV_shift_h, 1, tmp_frame->data[2], video_depth == 8 ? ImVulkan::INT8 : ImVulkan::INT16);
                cmd->record_clone(im_V, vk_V, opt);
            }

            std::vector<ImVulkan::VkImageBuffer> bindings(6);
            bindings[0] = vk_Y;
            bindings[1] = vk_U;
            if (!ISNV12(tmp_frame->format))
                bindings[2] = vk_V;
            if (using_vkimage)
                bindings[3] = vk_RGB;
            else
                bindings[4] = vk_RGB;
            bindings[5] = matix_y2r_gpu;
            std::vector<ImVulkan::vk_constant_type> constants(8);
            constants[0].i = vk_RGB.w;
            constants[1].i = vk_RGB.h;
            constants[2].i = vk_RGB.c;
            constants[3].i =    ISYUV420P(tmp_frame->format) ? ImVulkan::YUV420 :
                                ISYUV422P(tmp_frame->format) ? ImVulkan::YUV422 :
                                ISYUV444P(tmp_frame->format) ? ImVulkan::YUV444 :
                                ISNV12(tmp_frame->format) ? ImVulkan::NV12 : ImVulkan::YUV420;
            constants[4].i = color_space;
            constants[5].i = color_range;
            constants[6].f = (float)(1 << video_shift);
            constants[7].i = using_vkimage ? 0 : 1;
            if (video_depth == 8)
            {
                cmd->record_pipeline(pipeline_8, bindings, constants, vk_RGB);
            }
            else
            {
                cmd->record_pipeline(pipeline_16, bindings, constants, vk_RGB);
            }
            if (using_vkimage)
                cmd->record_buffer_to_image(vk_RGB, vkimage, opt);
            else
                cmd->record_clone(vk_RGB, im_RGB, opt);

            cmd->submit_and_wait();
            cmd->flash();
            if (using_vkimage)
            {
                if (!video_texture) video_texture = ImGui::ImCreateTexture(vkimage);
            }
            else
            {
                ImGui::ImGenerateOrUpdateTexture(video_texture, video_width, video_height, 4, (const unsigned char *)im_RGB.data);
            }
#else
            if (video_pfmt != tmp_frame->format)
            {
                img_convert_ctx = sws_getCachedContext(
                                            img_convert_ctx,
                                            tmp_frame->width,
                                            tmp_frame->height,
                                            (AVPixelFormat)tmp_frame->format,
                                            video_width,
                                            video_height,
                                            AV_PIX_FMT_RGBA,
                                            SWS_BICUBIC,
                                            NULL, NULL, NULL);
                if (!img_convert_ctx)
                {
                    av_frame_unref(sw_frame);
                    return;
                }
                video_pfmt = (AVPixelFormat)tmp_frame->format;
            }
            AVFrame rgb_picture;
            memset(&rgb_picture, 0, sizeof(rgb_picture));
            rgb_picture.format = AV_PIX_FMT_RGBA;
            rgb_picture.width = video_width;
            rgb_picture.height = video_height;
            ret = av_frame_get_buffer(&rgb_picture, 64);
            if (ret == 0)
            {
                sws_scale(
                    img_convert_ctx,
                    tmp_frame->data,
                    tmp_frame->linesize,
                    0, video_dec_ctx->height,
                    rgb_picture.data,
                    rgb_picture.linesize
                );
                ImGui::ImGenerateOrUpdateTexture(video_texture, video_width, video_height, 4, rgb_picture.data[0]);
                av_frame_unref(&rgb_picture);
            }
#endif
            av_frame_unref(sw_frame);
        }
    }
    int hw_decoder_init(AVCodecContext *ctx, const enum AVHWDeviceType type)
    {
        int err = 0;
        if ((err = av_hwdevice_ctx_create(&hw_device_ctx, type, NULL, NULL, 0)) < 0) 
        {
            fprintf(stderr, "Failed to create specified HW device.\n");
            return err;
        }
        ctx->hw_device_ctx = av_buffer_ref(hw_device_ctx);
        return err;
    }
    int open_codec_context(int *stream_idx, AVCodecContext **dec_ctx, enum AVMediaType type)
    {
        int ret, stream_index;
        AVStream *st = nullptr;
        AVCodec *dec = nullptr;
        AVDictionary *opts = nullptr;
        ret = av_find_best_stream(fmt_ctx, type, -1, -1, &dec, 0);
        if (ret < 0) 
        {
            return ret;
        }
        else 
        {
            stream_index = ret;
            st = fmt_ctx->streams[stream_index];
            if (type == AVMEDIA_TYPE_VIDEO)
            {
                for (int i = 0;; i++) 
                {
                    const AVCodecHWConfig *config = avcodec_get_hw_config(dec, i);
                    if (!config)
                    {
                        fprintf(stderr, "Decoder %s does not support HW.\n", dec->name);
                        break;
                    }
                    if (config->methods & AV_CODEC_HW_CONFIG_METHOD_HW_DEVICE_CTX) 
                    {
                        hw_pix_fmt = config->pix_fmt;
                        hw_type = config->device_type;
                        break;
                    }
                }
            }
            /* Allocate a codec context for the decoder */
            *dec_ctx = avcodec_alloc_context3(dec);
            if (!*dec_ctx) 
            {
                return -1;
            }
            /* Copy codec parameters from input stream to output codec context */
            if ((ret = avcodec_parameters_to_context(*dec_ctx, st->codecpar)) < 0) 
            {
                return ret;
            }

            /* Init hw decoders */
            if (hw_pix_fmt != AV_PIX_FMT_NONE)
            {
                (*dec_ctx)->get_format = get_hw_format;
                if ((ret = hw_decoder_init(*dec_ctx, hw_type)) < 0)
                    return ret;
            }

            /* Init the decoders */
            if ((ret = avcodec_open2(*dec_ctx, dec, &opts)) < 0) 
            {
                return ret;
            }
            *stream_idx = stream_index;
        }
        return 0;
    }
public:
    // init icon
    ImTextureID icon_play_texture = nullptr;
    ImTextureID icon_pause_texture = nullptr;
    // init file dialog
    ImGuiFileDialog filedialog;

    // init input
    AVFormatContext *fmt_ctx = nullptr;
    AVCodecContext *video_dec_ctx = nullptr;
    AVBufferRef *hw_device_ctx = nullptr;
    AVCodecContext *audio_dec_ctx = nullptr;
    AVStream *video_stream = nullptr;
    AVStream *audio_stream = nullptr;
    int video_stream_idx = -1;
    int audio_stream_idx = -1;

    AVInterruptCallbackMetadata interrupt_metadata;
    struct SwsContext *img_convert_ctx = nullptr;
    // init video texture
    ImTextureID video_texture = nullptr;
#ifdef IMGUI_VULKAN_SHADER
    ImVulkan::VulkanDevice* vkdev = nullptr;
    ImVulkan::VkAllocator* blob_allocator = nullptr;
    ImVulkan::VkAllocator* staging_allocator = nullptr;
    ImVulkan::Option opt;
    ImVulkan::Pipeline * pipeline_8 = nullptr;
    ImVulkan::Pipeline * pipeline_16 = nullptr;
    ImVulkan::VkCompute * cmd = nullptr;
    ImVulkan::VkImageMat vkimage;
    ImVulkan::VkImageBuffer matix_y2r_gpu;
#endif
public:
    // video info
    std::string video_codec_name;
    enum AVPixelFormat video_pfmt = AV_PIX_FMT_NONE;
    enum AVColorSpace video_color_space = AVCOL_SPC_UNSPECIFIED;
    enum AVColorRange video_color_range = AVCOL_RANGE_UNSPECIFIED;
    float video_fps = 0;
    int video_frames = 0;
    float play_time = 0;
    float total_time = 0;
    int video_width = 0;
    int video_height = 0;
    int video_depth = 0;
    float video_aspect_ratio = 1.f;
    // audio info
    std::string audio_codec_name;
    enum AVSampleFormat audio_sfmt = AV_SAMPLE_FMT_NONE;
    int audio_channels = 0;
    int audio_sample_rate = 0;
    int audio_depth = 0;

    // play status
    AVPacket packet;
    AVFrame* picture = nullptr;
    bool is_playing = false;
    bool is_opened = false;
    int64_t frame_number = 0;
    int64_t first_frame_number = -1;
    int64_t picture_pts = AV_NOPTS_VALUE;
    int64_t audio_pts = AV_NOPTS_VALUE;

private:
    double r2d(AVRational r) const
    {
        return r.num == 0 || r.den == 0 ? 0. : (double)r.num / (double)r.den;
    }
    int64_t dts_to_frame_number(int64_t dts)
    {
        double sec = dts_to_sec(video_stream, dts);
        return (int64_t)(video_fps * sec + 0.5);
    }
    double dts_to_sec(AVStream *stream, int64_t dts) const
    {
        return (double)(dts - stream->start_time) * r2d(stream->time_base);
    }
    int video_decode(AVCodecContext *avctx, int *got_picture_ptr, AVPacket *packet)
    {
        int ret = 0;
        *got_picture_ptr = 0;
        if (!picture)
            return -1;
        ret = avcodec_send_packet(avctx, packet);
        if (ret < 0) 
        {
            fprintf(stderr, "Error during decoding %d\n", ret);
            return ret;
        }
        while (ret >= 0) 
        {
            ret = avcodec_receive_frame(avctx, picture);
            if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF) 
            {
                return ret;
            }
            else if (ret < 0)
            {
                fprintf(stderr, "Error while decoding %d\n", ret);
                return ret;
            }
            else
            {
                *got_picture_ptr = 1;
                return 0;
            }
        }
        return 0;
    }
    bool grabFrame(bool decode = true)
    {
        bool valid = false;
        int got_picture;

        int count_errs = 0;
        const int max_number_of_attempts = 1 << 9;

        if( !fmt_ctx || !video_stream )  return false;
        if( video_stream->nb_frames > 0 && frame_number > video_stream->nb_frames )
            return false;

        picture_pts = AV_NOPTS_VALUE;
            
        // activate interrupt callback
        get_monotonic_time(&interrupt_metadata.value);
        interrupt_metadata.timeout_after_ms = LIBAVFORMAT_INTERRUPT_READ_TIMEOUT_MS;

        while (!valid)
        {
            av_packet_unref(&packet);
            if (interrupt_metadata.timeout)
            {
                valid = false;
                break;
            }
            int ret = av_read_frame(fmt_ctx, &packet);
            if (ret == AVERROR(EAGAIN))
                continue;
            if (ret == AVERROR_EOF)
            {
                // flush cached frames from video decoder
                packet.data = NULL;
                packet.size = 0;
                packet.stream_index = video_stream_idx;
            }
            if( packet.stream_index != video_stream_idx )
            {
                av_packet_unref(&packet);
                count_errs++;
                if (count_errs > max_number_of_attempts)
                    break;
                continue;
            }
            if (!decode)
            {
                picture_pts = packet.pts != AV_NOPTS_VALUE ? packet.pts : packet.dts;
                break;
            }
            // Decode video frame
            ret = video_decode(video_dec_ctx, &got_picture, &packet);
            // Did we get a video frame?
            if (got_picture)
            {
                if (picture_pts == AV_NOPTS_VALUE)
                    picture_pts = picture->pts != AV_NOPTS_VALUE && picture->pts != 0 ? picture->pts : picture->pkt_dts;

                valid = true;
            }
            else
            {
                count_errs++;
                if (count_errs > max_number_of_attempts)
                    break;
            }
            if (packet.size == 0 && ret < 0)
            {
                is_playing = false;
                SeekMedia(0);
            }
        }

        if (valid)
        {
            frame_number++;
            play_time = dts_to_sec(video_stream, picture_pts) * 1000.0;
        }

        if (valid && first_frame_number < 0)
            first_frame_number = dts_to_frame_number(picture_pts);

        interrupt_metadata.timeout_after_ms = 0;
        return valid;
    }
};

const char* Application_GetName(void* handle)
{
    return "Application Example Video";
}

void Application_Initialize(void** handle)
{
    *handle = new Example();
    Example * example = (Example *)*handle;
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = ini_file.c_str();
    io.DeltaTime = 1.0f / 30.f;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
}

void Application_Finalize(void** handle)
{
    if (handle && *handle)
    {
        Example * example = (Example *)*handle;
        delete example;
        *handle = nullptr;
    }
}

bool Application_Frame(void* handle)
{
    bool done = false;
    auto& io = ImGui::GetIO();
    Example * example = (Example *)handle;
    if (!example)
        return true;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("文件")) 
        {
            if (ImGui::Button("关于..."))
                ImGui::OpenPopup("关于");
            ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
            ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
            if (ImGui::BeginPopupModal("关于", NULL, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("ImGUI Video\n\n");
                ImGui::Separator();
                int i = ImGui::GetCurrentWindow()->ContentSize.x;
                ImGui::Indent((i - 40.0f) * 0.5f);
                if (ImGui::Button("OK", ImVec2(40, 0))) { ImGui::CloseCurrentPopup(); }
                ImGui::SetItemDefaultFocus();
                ImGui::EndPopup();
            }
            ImGui::Separator();
            if (ImGui::Button("打开文件"))
            {
                const char *filters = "视频文件(*.mp4 *.mov *.mkv *.avi){.mp4,.mov,.mkv,.avi,.MP4,.MOV,.MKV,.AVI},.*";
				example->filedialog.OpenModal("ChooseFileDlgKey",
                                    ICON_IGFD_FOLDER_OPEN " 打开视频文件", filters, ".");
            }
            ImGui::Separator();
            if (ImGui::Button("退出")) 
            {
                done = true;
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    ImVec2 maxSize = ImVec2((float)io.DisplaySize.x, (float)io.DisplaySize.y);
	ImVec2 minSize = maxSize * 0.5f;
    if (example->filedialog.Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
	{
        if (example->filedialog.IsOk())
		{
            std::string filePathName = example->filedialog.GetFilePathName();
            if (example->OpenMediaFile(filePathName.c_str()) != 0)
            {
                ImGui::OpenPopup("Open Error?");
            }
            else
            {
                if (example->video_texture)
                {
                    ImGui::ImDestroyTexture(example->video_texture);
                    example->video_texture = nullptr;
                }
                example->PlayMedia();
            }
        }
        example->filedialog.Close();
    }

    // load video texture

    if (example->is_playing && example->is_opened)
    {
        example->PlayMedia();
    }

    // load video end

    // Show PlayControl panel
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.9f);
    ImVec2 panel_size(io.DisplaySize.x - 20.0, 96);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(panel_size, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.5);
    if (ImGui::Begin("Control", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize))
    {
        // add button
        int i = ImGui::FindWindowByName("Control")->Size.x;
        ImGui::Indent((i - 32.0f) * 0.5f);
        ImVec2 size = ImVec2(32.0f, 32.0f); // Size of the image we want to make visible
        if (ImGui::Button(example->is_playing ? ICON_FAD_PAUSE: ICON_FAD_PLAY, size))
        {
            if (example->is_opened)
                example->is_playing = !example->is_playing;
        }
        ImGui::Unindent((i - 32.0f) * 0.5f);
        // add slider bar
        ImGui::Separator();
        if (example->total_time > 0)
        {
            float time = example->play_time;
            float current_time = time / 1000;
            static ImGuiSliderFlags flags = ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoLabel;
            if (ImGui::SliderFloat("time", &current_time, 0, example->total_time, "%.2f", flags))
            {
                example->SeekMedia(current_time);
                example->PlayMedia();
            }
            ImGui::SameLine();
            int hours = time / 1000 / 60 / 60; time -= hours * 60 * 60 * 1000;
            int mins = time / 1000 / 60; time -= mins * 60 * 1000;
            int secs = time / 1000; time -= secs * 1000;
            int ms = time;
            ImGui::Text("%02d:%02d:%02d.%03d", hours, mins, secs, ms);

            ImGui::SameLine();
            float ftime = example->total_time * 1000.0f;
            hours = ftime / 1000 / 60 / 60; ftime -= hours * 60 * 60 * 1000;
            mins = ftime / 1000 / 60; ftime -= mins * 60 * 1000;
            secs = ftime / 1000; ftime -= secs * 1000;
            ms = ftime;
            ImGui::Text("/ %02d:%02d:%02d.%03d", hours, mins, secs, ms);

            ImGui::SameLine();
            ImGui::Text("[%.3fms %.1ffps]", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        ImGui::End();
    }

    // handle key event
    if (!io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space), false))
    {
        if (example->is_opened)
            example->is_playing = !example->is_playing;
    }

    if (!io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape), false))
    {
        done = true;
    }

    if (!io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow), true))
    {
        float time = example->play_time;
        time -= 1000.f;
        if (time < 0) time = 0;
        example->SeekMedia(time / 1000.f);
        example->PlayMedia();
    }

    if (!io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow), true))
    {
        float time = example->play_time;
        time += 1000.f;
        if (time > example->total_time * 1000.f) time = example->total_time * 1000.f;
        example->SeekMedia(time / 1000.f);
        example->PlayMedia();
    }

    if (!io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow), true))
    {
        float time = example->play_time;
        time -= 5000.f;
        if (time < 0) time = 0;
        example->SeekMedia(time / 1000.f);
        example->PlayMedia();
    }

    if (!io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow), true))
    {
        float time = example->play_time;
        time += 5000.f;
        if (time > example->total_time * 1000.f) time = example->total_time * 1000.f;
        example->SeekMedia(time / 1000.f);
        example->PlayMedia();
    }

    if (example->video_texture)
    {
        ImGuiWindowFlags flags = ImGuiWindowFlags_None;
        flags = ImGuiWindowFlags_NoTitleBar |
                ImGuiWindowFlags_NoFocusOnAppearing |
                ImGuiWindowFlags_NoBackground |
                ImGuiWindowFlags_NoMove | 
                ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoResize |
                ImGuiWindowFlags_NoSavedSettings |
                ImGuiWindowFlags_NoInputs;
        ImVec2 window_size = io.DisplaySize;
        float adj_x = example->video_width > example->video_height ? window_size.x : window_size.y * example->video_aspect_ratio;
        float adj_y = example->video_width > example->video_height ? window_size.x / example->video_aspect_ratio : window_size.y;
        float offset_x = (window_size.x - adj_x) / 2.0;
        float offset_y = (window_size.y - adj_y) / 2.0;
        ImGui::SetNextWindowSize(ImVec2(adj_x, adj_y), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(offset_x, offset_y), ImGuiCond_Always);
        if (ImGui::Begin("screen", nullptr, flags)) 
        {
            ImVec2 content_region = ImGui::GetContentRegionAvail();
            ImGui::Image((void *)(intptr_t)example->video_texture, content_region,
                        ImVec2(0.0f, 0.0f), ImVec2(1.0f, 1.0f), ImVec4(1.0, 1.0, 1.0, 1.0));
            ImGui::End();
        }
    }

    // Message Boxes
    // Always center this window when appearing
    ImVec2 modal_center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.5f);
    ImGui::SetNextWindowPos(modal_center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));
    if (ImGui::BeginPopupModal("Open Error?", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::Text("无法打开媒体文件");
        ImGui::Separator();

        if (ImGui::Button("确定", ImVec2(120, 0)))
        {
            ImGui::CloseCurrentPopup();
        }
        ImGui::SetItemDefaultFocus();
        ImGui::EndPopup();
    }

    // Show Media Info
    if (example->is_opened)
    {
        const float DISTANCE = 40.0f;
        static int corner = 0;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (corner != -1)
        {
            window_flags |= ImGuiWindowFlags_NoMove;
            ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
            ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (ImGui::Begin("Media Info", &example->is_opened, window_flags))
        {
            ImGui::SetNextItemOpen(true, ImGuiCond_Once);
            if (ImGui::TreeNode("Media Info"))
            {
                ImGui::Separator();
                ImGui::Text("  Media name: %s", example->fmt_ctx->url);
                ImGui::Text("Media format: %s", example->fmt_ctx->iformat->long_name);
                float ftime = example->total_time * 1000.0f;
                int hours = ftime / 1000 / 60 / 60; ftime -= hours * 60 * 60 * 1000;
                int mins = ftime / 1000 / 60; ftime -= mins * 60 * 1000;
                int secs = ftime / 1000; ftime -= secs * 1000;
                int ms = ftime;
                ImGui::Text("  Media time: %02d:%02d:%02d.%03d", hours, mins, secs, ms);
                ImGui::Separator();

                if (example->video_stream_idx != -1)
                {
                    ImGui::Text("Video Stream");
                    ImGui::Text("     Codec: %s", example->video_codec_name.c_str());
                    ImGui::Text("    Format: %s", av_get_pix_fmt_name(example->video_pfmt));
                    ImGui::Text("     Depth: %d", example->video_depth);
                    ImGui::Text("     Width: %d", example->video_width);
                    ImGui::Text("    Height: %d", example->video_height);
                    ImGui::Text("       FPS: %.2f", example->video_fps);
                    ImGui::Text("    Frames: %d", example->video_frames);
                    if (example->video_color_space != AVCOL_SPC_UNSPECIFIED)
                    {
                        ImGui::Text("ColorSpace: %s", av_color_space_name(example->video_color_space));
                        ImGui::Text("ColorRange: %s", av_color_range_name(example->video_color_range));
                    }
                    else
                        ImGui::Text("ColorSpace: %s", av_get_colorspace_name(example->video_color_space));

                    ImGui::Separator();
                }
                if (example->audio_stream_idx != -1)
                {
                    ImGui::Text("Audio Stream");
                    ImGui::Text("     Codec: %s", example->audio_codec_name.c_str());
                    ImGui::Text("    Format: %s", av_get_sample_fmt_name(example->audio_sfmt));
                    ImGui::Text("     Depth: %d", example->audio_depth);
                    ImGui::Text("      Rate: %d", example->audio_sample_rate);
                    ImGui::Text("  Channels: %d", example->audio_channels);
                }
                if (ImGui::BeginPopupContextWindow())
                {
                    if (ImGui::MenuItem("Custom",       NULL, corner == -1)) corner = -1;
                    if (ImGui::MenuItem("Top-left",     NULL, corner == 0)) corner = 0;
                    if (ImGui::MenuItem("Top-right",    NULL, corner == 1)) corner = 1;
                    if (ImGui::MenuItem("Bottom-left",  NULL, corner == 2)) corner = 2;
                    if (ImGui::MenuItem("Bottom-right", NULL, corner == 3)) corner = 3;
                    ImGui::EndPopup();
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }

    return done;
}

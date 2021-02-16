#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <application.h>
#include "ImGuiHelper.h"
#include "ImGuiFileDialog.h"
#include "imgui_knob.h"
#include "implot.h"
#ifdef IMGUI_VULKAN_SHADER
#include "ImVulkanShader.h"
#endif
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#ifdef __cplusplus
extern "C" {
#endif
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
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

#define AUDIO_INBUF_SIZE 20480
#define AUDIO_REFILL_THRESH 4096

class Example
{
public:
    Example() 
    {
        // set ffmpeg log level
        av_log_set_level(AV_LOG_QUIET);
        //av_log_set_level(AV_LOG_DEBUG);
        // load file dialog resource
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
        prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

        // init code
        memset(&packet, 0, sizeof(packet));
        audio_left_channel_level = 0.f;
        audio_right_channel_level = 0.f;
        memset(audio_left_data, 0, sizeof(float) * audio_data_size);
        memset(audio_right_data, 0, sizeof(float) * audio_data_size);

#ifdef IMGUI_VULKAN_SHADER
        yuv2rgb = new ImVulkan::ColorConvert_vulkan(0);
        resize = new ImVulkan::Resize_vulkan(0);
#endif
    }
    ~Example() 
    { 
        // Store file dialog bookmark
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
        end_file_dialog_demo_window(&filedialog, bookmark_path.c_str());
        CloseMedia();
#ifdef IMGUI_VULKAN_SHADER
        if (yuv2rgb) { delete yuv2rgb; yuv2rgb = nullptr; }
        if (resize) { delete resize; resize = nullptr; }
#endif
    }
    void CloseMedia()
    {
        if (video_dec_ctx) { avcodec_free_context(&video_dec_ctx); video_dec_ctx = nullptr; }
        if (audio_dec_ctx) { avcodec_free_context(&audio_dec_ctx); audio_dec_ctx = nullptr; }
        if (fmt_ctx) { avformat_close_input(&fmt_ctx); fmt_ctx = nullptr; }
        if (video_texture) { ImGui::ImDestroyTexture(video_texture); video_texture = nullptr; }
        if (picture) { av_frame_free(&picture); picture = nullptr; }
        if (audio_frame) { av_frame_free(&audio_frame); audio_frame = nullptr; }
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
        audio_left_channel_level = 0.f;
        audio_right_channel_level = 0.f;
        memset(audio_left_data, 0, sizeof(float) * audio_data_size);
        memset(audio_right_data, 0, sizeof(float) * audio_data_size);
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
            audio_frame = av_frame_alloc();
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
        int stream_index = -1;
        if (!video_stream || !video_dec_ctx || video_fps == 0 || video_frames <= 0)
            return;
        int delta = 16;
        int64_t _frame_number = (int64_t)(sec * video_fps + 0.5);
        _frame_number = std::min(_frame_number, (int64_t)video_frames);
        if (first_frame_number < 0 && video_frames > 1)
            grabFrame(stream_index, false);
        for(;;)
        {
            int64_t _frame_number_temp = std::max(_frame_number - delta, (int64_t)0);
            double sec = (double)_frame_number_temp / video_fps;
            int64_t time_stamp = video_stream->start_time;
            double  time_base  = r2d(video_stream->time_base);
            time_stamp += (int64_t)(sec / time_base + 0.5);
            if (video_frames > 1) 
                av_seek_frame(fmt_ctx, video_stream_idx, time_stamp, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_ANY);
            avcodec_flush_buffers(video_dec_ctx);
            if (_frame_number > 0)
            {
                grabFrame(stream_index, false);
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
                        if (!grabFrame(stream_index, decode))
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
        int stream_index = -1;
        if (grabFrame(stream_index, true))
        {
        }
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
    // init audio level
    static const int audio_data_size = 1024;
    int audio_left_channel_level = 0;
    int audio_right_channel_level = 0;
    float audio_left_data[audio_data_size];
    float audio_right_data[audio_data_size];

#ifdef IMGUI_VULKAN_SHADER
    ImVulkan::ColorConvert_vulkan * yuv2rgb = nullptr;
    ImVulkan::Resize_vulkan * resize = nullptr;
    ImVulkan::VkImageMat vkimage;
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
    AVFrame* audio_frame = nullptr;
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
    int decode(AVCodecContext *avctx, int *got_data_ptr, AVPacket *packet, AVFrame * frame)
    {
        int ret = 0;
        *got_data_ptr = 0;
        if (!frame)
            return -1;
        ret = avcodec_send_packet(avctx, packet);
        if (ret < 0) 
        {
            fprintf(stderr, "Error during decoding %d\n", ret);
            return ret;
        }
        while (ret >= 0) 
        {
            ret = avcodec_receive_frame(avctx, frame);
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
                *got_data_ptr = 1;
                return 0;
            }
        }
        return 0;
    }
    bool grabFrame(int& stream_index, bool is_decode = true)
    {
        bool valid = false;
        int got_picture;
        AVCodecContext * dec = nullptr;
        AVFrame * frame = nullptr;

        int count_errs = 0;
        const int max_number_of_attempts = 1 << 9;

        if( !fmt_ctx)  return false;
        if( video_stream_idx == -1 && audio_stream_idx == -1 )
            return false;
        if( video_stream->nb_frames > 0 && frame_number > video_stream->nb_frames )
            return false;

        picture_pts = AV_NOPTS_VALUE;
        audio_pts = AV_NOPTS_VALUE;
            
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
            if (packet.stream_index == video_stream_idx)
            {
                dec = video_dec_ctx;
                frame = picture;
                stream_index = video_stream_idx;
            }
            else if (packet.stream_index == audio_stream_idx)
            {
                dec = audio_dec_ctx;
                frame = audio_frame;
                stream_index = audio_stream_idx;
            }
            else
            {
                fprintf(stderr, "Unknown stream\n");
                continue;
            }
            if (ret == AVERROR(EAGAIN))
                continue;
            if (ret == AVERROR_EOF)
            {
                // flush cached frames from video decoder
                dec = audio_dec_ctx;
                frame = audio_frame;
                packet.data = NULL;
                packet.size = 0;
                packet.stream_index = video_stream_idx;
            }
            //if (packet.stream_index != video_stream_idx)
            if ((packet.stream_index != video_stream_idx && packet.stream_index != audio_stream_idx) || !frame || !dec)
            {
                av_packet_unref(&packet);
                count_errs++;
                if (count_errs > max_number_of_attempts)
                    break;
                continue;
            }
            if (!is_decode)
            {
                if (packet.stream_index == video_stream_idx)
                {
                    picture_pts = packet.pts != AV_NOPTS_VALUE ? packet.pts : packet.dts;
                }
                if (packet.stream_index == audio_stream_idx)
                {
                    audio_pts = packet.pts != AV_NOPTS_VALUE ? packet.pts : packet.dts;
                }
                av_packet_unref(&packet);
                valid = true;
                break;
            }
            ret = decode(dec, &got_picture, &packet, frame);
            // Did we get a frame?
            if (got_picture)
            {
                if (packet.stream_index == video_stream_idx)
                {
                    if (picture_pts == AV_NOPTS_VALUE)
                        picture_pts = picture->pts != AV_NOPTS_VALUE && picture->pts != 0 ? picture->pts : picture->pkt_dts;
                    render_video_frame();
                    valid = true;
                }
                else if (packet.stream_index == audio_stream_idx)
                {
                    if (audio_pts == AV_NOPTS_VALUE)
                        audio_pts = audio_frame->pts != AV_NOPTS_VALUE && audio_frame->pts != 0 ? audio_frame->pts : audio_frame->pkt_dts;
                    render_audio_frame();
                    valid = false;
                }
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
            av_frame_unref(frame);
        }

        if (valid && stream_index == video_stream_idx)
        {
            frame_number++;
            play_time = dts_to_sec(video_stream, picture_pts) * 1000.0;
        }

        if (valid && first_frame_number < 0 && stream_index == video_stream_idx)
            first_frame_number = dts_to_frame_number(picture_pts);

        interrupt_metadata.timeout_after_ms = 0;
        return valid;
    }
    void render_video_frame()
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
                av_frame_free(&sw_frame);
                return;
            }
            else
            {
                tmp_frame = sw_frame;
            }
            av_frame_unref(picture);
        }
        else
        {
            tmp_frame = picture;
        }

#ifdef IMGUI_VULKAN_SHADER
        const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get((AVPixelFormat)tmp_frame->format);
        int video_shift = desc->comp[0].depth + desc->comp[0].shift;
        ImVulkan::ColorSpace color_space =  video_color_space == AVCOL_SPC_BT470BG ||
                                            video_color_space == AVCOL_SPC_SMPTE170M ||
                                            video_color_space == AVCOL_SPC_BT470BG ? ImVulkan::BT601 :
                                            video_color_space == AVCOL_SPC_BT709 ? ImVulkan::BT709 :
                                            video_color_space == AVCOL_SPC_BT2020_NCL ||
                                            video_color_space == AVCOL_SPC_BT2020_CL ? ImVulkan::BT2020 : ImVulkan::BT709;
        ImVulkan::ColorRange color_range =  video_color_range == AVCOL_RANGE_MPEG ? ImVulkan::NARROW_RANGE :
                                            video_color_range == AVCOL_RANGE_JPEG ? ImVulkan::FULL_RANGE : ImVulkan::NARROW_RANGE;
        ImVulkan::ColorFormat color_format = ISYUV420P(tmp_frame->format) ? ImVulkan::YUV420 :
                                            ISYUV422P(tmp_frame->format) ? ImVulkan::YUV422 :
                                            ISYUV444P(tmp_frame->format) ? ImVulkan::YUV444 :
                                            ISNV12(tmp_frame->format) ? ImVulkan::NV12 : ImVulkan::YUV420;
        
        ImVulkan::ImageBuffer im_Y, im_U, im_V;
        int data_shift = video_depth > 8 ? 1 : 0;
        int UV_shift_w = ISYUV420P(tmp_frame->format) || ISYUV422P(tmp_frame->format) ? 1 : 0;
        int UV_shift_h = ISYUV420P(tmp_frame->format) || ISNV12(tmp_frame->format) ? 1 : 0;
        im_Y.create_type(tmp_frame->linesize[0] >> data_shift, tmp_frame->height, 1, tmp_frame->data[0], video_depth > 8 ? ImVulkan::INT16 : ImVulkan::INT8);
        im_U.create_type(tmp_frame->linesize[1] >> (UV_shift_w + data_shift), tmp_frame->height >> UV_shift_h, 1, tmp_frame->data[1], video_depth > 8 ? ImVulkan::INT16 : ImVulkan::INT8);
        if (!ISNV12(tmp_frame->format))
        {
            im_V.create_type(tmp_frame->linesize[2] >> (UV_shift_w + data_shift), tmp_frame->height >> UV_shift_h, 1, tmp_frame->data[2], video_depth > 8 ? ImVulkan::INT16 : ImVulkan::INT8);
        }

        if (video_width > 1920 || video_height > 1920 || video_depth > 1920)
        {
            float frame_scale = 1920.f / (float)video_width;
            ImVulkan::VkImageBuffer im_RGB;
            yuv2rgb->YUV2RGBA(im_Y, im_U, im_V, im_RGB, color_format, color_space, color_range, video_depth, video_shift);
            resize->Resize(im_RGB, vkimage, frame_scale, 0.f, ImVulkan::INTERPOLATE_AREA);
        }
        else
        {
            yuv2rgb->YUV2RGBA(im_Y, im_U, im_V, vkimage, color_format, color_space, color_range, video_depth, video_shift);
        }
        if (!video_texture) video_texture = ImGui::ImCreateTexture(vkimage);
#else
        if (video_pfmt != tmp_frame->format)
        {
            // TODO::Dicky if linesize != width will cause video oblique stroke
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
        av_frame_free(&sw_frame);
    }
    void render_audio_frame()
    {
        double sum_left = 0;
        double sum_right = 0;
        float left_data;
        float right_data;
        int data_len = ImMin(audio_frame->nb_samples, audio_data_size);
        for (int i = 0; i < data_len; i++)
        {
            if (audio_frame->format == AV_SAMPLE_FMT_FLTP)
            {
                left_data = *((float *)audio_frame->data[0] + i);
                sum_left += fabs(left_data);
                audio_left_data[i] = left_data;
                if (audio_frame->channels > 1)
                {
                    right_data = *((float *)audio_frame->data[1] + i);
                    sum_right += fabs(right_data);
                    audio_right_data[i] = right_data;
                }
            }
            else if (audio_frame->format == AV_SAMPLE_FMT_S16P)
            {
                left_data = *((short *)audio_frame->data[0] + i) / (float)(1 << 15);
                sum_left += fabs(sum_left);
                audio_left_data[i] = left_data;
                if (audio_frame->channels > 1)
                {
                    right_data = *((float *)audio_frame->data[1] + i) / (float)(1 << 15);
                    sum_right += fabs(right_data);
                    audio_right_data[i] = right_data;
                }
            }
        }
        audio_left_channel_level = 90.3 + 20.0 * log10(sum_left / data_len);
        audio_right_channel_level = 90.3 + 20.0 * log10(sum_right / data_len);
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
            if (type == AVMEDIA_TYPE_VIDEO && hw_pix_fmt != AV_PIX_FMT_NONE)
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
};

const char* Application_GetName(void* handle)
{
    return "Application Example Video";
}

void Application_Initialize(void** handle)
{
    *handle = new Example();
    Example * example = (Example *)*handle;
    ImPlot::CreateContext();
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
    ImPlot::DestroyContext();
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
    ImVec2 panel_size(io.DisplaySize.x - 20.0, 120);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(panel_size, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.5);
    if (ImGui::Begin("Control", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize))
    {
        int i = ImGui::FindWindowByName("Control")->Size.x;
        // add wave plots
        ImPlot::PushStyleVar(ImPlotStyleVar_PlotBorderSize, 0.f);
        ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0.f, 0.f));
        ImPlot::SetNextPlotLimits(0,example->audio_data_size,-1,1);
        if (ImPlot::BeginPlot("##Audio wave left", NULL, NULL, ImVec2(256,36), 
                            ImPlotFlags_CanvasOnly | ImPlotFlags_NoChild,
                            ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax,
                            ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax)) 
        {
            ImPlot::PlotLine("##lwave", example->audio_left_data, example->audio_data_size);
            ImPlot::EndPlot();
        }
        ImGui::SameLine();
        ImPlot::SetNextPlotLimits(0,example->audio_data_size,-1,1);
        if (ImPlot::BeginPlot("##Audio wave right", NULL, NULL, ImVec2(256,36), 
                            ImPlotFlags_CanvasOnly | ImPlotFlags_NoChild,
                            ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax,
                            ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax)) 
        {
            ImPlot::PlotLine("##rwave", example->audio_right_data, example->audio_data_size);
            ImPlot::EndPlot();
        }
        ImGui::SameLine();
        ImPlot::PopStyleVar(2);
        // add button
        ImGui::Indent((i - 32.0f) * 0.5f);
        ImVec2 size = ImVec2(32.0f, 32.0f); // Size of the image we want to make visible
        if (ImGui::Button(example->is_playing ? ICON_FAD_PAUSE: ICON_FAD_PLAY, size))
        {
            if (example->is_opened)
                example->is_playing = !example->is_playing;
        }
        ImGui::Unindent((i - 32.0f) * 0.5f);
        ImGui::Separator();
        // add audio meter bar
        ImGui::UvMeter("##lhuvr", ImVec2(panel_size.x, 10), &example->audio_left_channel_level, 0, 90, 200); ImGui::ShowTooltipOnHover("Left Uv meters.");
        ImGui::UvMeter("##rhuvr", ImVec2(panel_size.x, 10), &example->audio_right_channel_level, 0, 90, 200); ImGui::ShowTooltipOnHover("Right Uv meters.");
        ImGui::Separator();
        // add slider bar
        if (example->total_time > 0)
        {
            float time = example->play_time;
            float current_time = time / 1000;
            static ImGuiSliderFlags flags = ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoLabel;
            if (ImGui::SliderFloat("time", &current_time, 0, example->total_time, "%.2f", flags))
            {
                example->SeekMedia(current_time);
                if (!example->is_playing) example->PlayMedia();
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

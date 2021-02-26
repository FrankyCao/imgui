#ifndef __MEDIA_PLAYER_H__
#define __MEDIA_PLAYER_H__
#ifdef __cplusplus
extern "C" {
#endif
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
#include <libavutil/frame.h>
#include <libavutil/time.h>
#include <libavutil/imgutils.h>
#include <libavutil/samplefmt.h>
#include <libavutil/avstring.h>
#include <libavutil/bprint.h>
#include <libavutil/fifo.h>
#include <libavutil/opt.h>
#include <libavfilter/avfilter.h>
#include <libavfilter/buffersink.h>
#include <libavfilter/buffersrc.h>
#ifdef __cplusplus
}
#endif
#if defined(_WIN32)
#include <SDL.h>
#include <SDL_thread.h>
#else
#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>
#endif
#include "ImGuiHelper.h"
#ifdef IMGUI_VULKAN_SHADER
#include "ImVulkanShader.h"
#endif
#ifdef IMGUI_GLFW
#include <GLFW/glfw3.h>
#endif
#include <string>

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

#define MAX_QUEUE_SIZE (15 * 1024 * 1024)
#define MIN_FRAMES 25
#define EXTERNAL_CLOCK_MIN_FRAMES 2
#define EXTERNAL_CLOCK_MAX_FRAMES 10

/* Minimum SDL audio buffer size, in samples. */
#define SDL_AUDIO_MIN_BUFFER_SIZE 512
/* Calculate actual buffer size keeping in mind not cause too frequent audio callbacks */
#define SDL_AUDIO_MAX_CALLBACKS_PER_SEC 30

/* Step size for volume control in dB */
#define SDL_VOLUME_STEP (0.75)

/* no AV sync correction is done if below the minimum AV sync threshold */
#define AV_SYNC_THRESHOLD_MIN 0.04
/* AV sync correction is done if above the maximum AV sync threshold */
#define AV_SYNC_THRESHOLD_MAX 0.1
/* If a frame duration is longer than this, it will not be duplicated to compensate AV sync */
#define AV_SYNC_FRAMEDUP_THRESHOLD 0.1
/* no AV correction is done if too big error */
#define AV_NOSYNC_THRESHOLD 10.0

/* maximum audio speed change to get correct sync */
#define SAMPLE_CORRECTION_PERCENT_MAX 10

/* external clock speed adjustment constants for realtime sources based on buffer fullness */
#define EXTERNAL_CLOCK_SPEED_MIN  0.900
#define EXTERNAL_CLOCK_SPEED_MAX  1.010
#define EXTERNAL_CLOCK_SPEED_STEP 0.001

/* we use about AUDIO_DIFF_AVG_NB A-V differences to make the average */
#define AUDIO_DIFF_AVG_NB   20

/* polls for possible required screen refresh at least this often, should be less than 1/fps */
#define REFRESH_RATE 0.01

/* NOTE: the size must be big enough to compensate the hardware audio buffersize size */
/* TODO: We assume that a decoded and resampled frame fits into this buffer */
#define SAMPLE_ARRAY_SIZE (8 * 1024)

#define VIDEO_PICTURE_QUEUE_SIZE 3
#define SUBPICTURE_QUEUE_SIZE 16
#define SAMPLE_QUEUE_SIZE 9
#define FRAME_QUEUE_SIZE FFMAX(SAMPLE_QUEUE_SIZE, FFMAX(VIDEO_PICTURE_QUEUE_SIZE, SUBPICTURE_QUEUE_SIZE))

typedef struct MyAVPacketList {
    AVPacket *pkt;
    int serial;
} MyAVPacketList;

typedef struct PacketQueue {
    AVFifoBuffer *pkt_list;
    int nb_packets;
    int size;
    int64_t duration;
    int abort_request;
    int serial;
    SDL_mutex *mutex;
    SDL_cond *cond;
} PacketQueue;

typedef struct AudioParams {
    int freq;
    int channels;
    int64_t channel_layout;
    enum AVSampleFormat fmt;
    int frame_size;
    int bytes_per_sec;
} AudioParams;

typedef struct Clock {
    double pts;           /* clock base */
    double pts_drift;     /* clock base minus time at which we updated the clock */
    double last_updated;
    double speed;
    int serial;           /* clock is based on a packet with this serial */
    int paused;
    int *queue_serial;    /* pointer to the current packet queue serial, used for obsolete clock detection */
} Clock;

/* Common struct for handling all types of decoded data and allocated render buffers. */
typedef struct Frame {
    AVFrame *frame;
    AVSubtitle sub;
    int serial;
    double pts;           /* presentation timestamp for the frame */
    double duration;      /* estimated duration of the frame */
    int64_t pos;          /* byte position of the frame in the input file */
    int width;
    int height;
    int format;
    AVRational sar;
    int uploaded;
    int flip_v;
} Frame;

typedef struct FrameQueue {
    Frame queue[FRAME_QUEUE_SIZE];
    int rindex;
    int windex;
    int size;
    int max_size;
    int keep_last;
    int rindex_shown;
    SDL_mutex *mutex;
    SDL_cond *cond;
    PacketQueue *pktq;
} FrameQueue;

enum {
    AV_SYNC_AUDIO_MASTER, /* default choice */
    AV_SYNC_VIDEO_MASTER,
    AV_SYNC_EXTERNAL_CLOCK, /* synchronize to an external clock */
};

typedef struct Decoder {
    AVPacket *pkt;
    PacketQueue *queue;
    AVCodecContext *avctx;
    int pkt_serial;
    int finished;
    int packet_pending;
    SDL_cond *empty_queue_cond;
    int64_t start_pts;
    AVRational start_pts_tb;
    int64_t next_pts;
    AVRational next_pts_tb;
    SDL_Thread *decoder_tid;
} Decoder;

typedef struct VideoState {
    SDL_Thread *read_tid;
    SDL_Thread *render_tid;
    AVInputFormat *iformat;
    int abort_request;
    int force_refresh;
    int paused;
    int last_paused;
    int queue_attachments_req;
    int seek_req;
    int seek_flags;
    int64_t seek_pos;
    int64_t seek_rel;
    int read_pause_return;
    AVFormatContext *ic;
    int realtime;

    Clock audclk;
    Clock vidclk;
    Clock extclk;

    FrameQueue pictq;
    FrameQueue subpq;
    FrameQueue sampq;

    Decoder auddec;
    Decoder viddec;
    Decoder subdec;

    int audio_stream;

    int av_sync_type;

    double audio_clock;
    int audio_clock_serial;
    double audio_diff_cum; /* used for AV difference average computation */
    double audio_diff_avg_coef;
    double audio_diff_threshold;
    int audio_diff_avg_count;
    AVStream *audio_st;
    PacketQueue audioq;
    int audio_hw_buf_size;
    uint8_t *audio_buf;
    uint8_t *audio_buf1;
    unsigned int audio_buf_size; /* in bytes */
    unsigned int audio_buf1_size;
    int audio_buf_index; /* in bytes */
    int audio_write_buf_size;
    int audio_volume;
    int muted;
    struct AudioParams audio_src;
    struct AudioParams audio_filter_src;
    struct AudioParams audio_tgt;
    struct SwrContext *swr_ctx;
    int frame_drops_early;
    int frame_drops_late;

    int16_t sample_array[SAMPLE_ARRAY_SIZE];
    int sample_array_index;
    int last_i_start;

    ImTextureID video_texture = nullptr;

    int subtitle_stream;
    AVStream *subtitle_st;
    PacketQueue subtitleq;

    double frame_timer;
    double frame_last_returned_time;
    double frame_last_filter_delay;
    int video_stream;
    AVStream *video_st;
    PacketQueue videoq;
    double max_frame_duration;      // maximum duration of a frame - above this, we consider the jump a timestamp discontinuity
    struct SwsContext *img_convert_ctx;
    struct SwsContext *sub_convert_ctx;
    int eof;

    char *filename;
    int step;

    AVFilterContext *in_audio_filter;   // the first filter in the audio chain
    AVFilterContext *out_audio_filter;  // the last filter in the audio chain
    AVFilterGraph *agraph;              // audio filter graph

    enum AVPixelFormat video_pfmt = AV_PIX_FMT_NONE;
    enum AVColorSpace video_color_space = AVCOL_SPC_UNSPECIFIED;
    enum AVColorRange video_color_range = AVCOL_RANGE_UNSPECIFIED;
    float video_fps = 0.f;
    int video_frames = 0;
    int video_width = 0;
    int video_height = 0;
    int video_depth = 0;
    float video_aspect_ratio = 1.f;
    float video_clip = 1.f;
    enum AVPixelFormat hw_pix_fmt = AV_PIX_FMT_NONE;
    enum AVHWDeviceType hw_type = AV_HWDEVICE_TYPE_NONE;

    enum AVSampleFormat audio_sfmt = AV_SAMPLE_FMT_NONE;
    int audio_channels = 0;
    int audio_sample_rate = 0;
    int audio_depth = 0;
    int audio_left_channel_level;
    int audio_right_channel_level;

    double total_time; 

    int last_video_stream, last_audio_stream, last_subtitle_stream;

    std::string video_codec_name;
    std::string audio_codec_name;
    std::string subtitle_codec_name;

    std::string stats_string;

    SDL_cond *continue_read_thread;
#ifdef IMGUI_VULKAN_SHADER
    ImVulkan::ColorConvert_vulkan * yuv2rgb = nullptr;
    ImVulkan::Resize_vulkan * resize = nullptr;
    ImVulkan::VkImageMat vkimage;
#endif
#ifdef IMGUI_APPLICATION_GLFW
    GLFWwindow* current_window = nullptr;
#endif
#ifdef IMGUI_APPLICATION_SDL
    SDL_Window* current_window = nullptr;
    SDL_GLContext current_glcontext = nullptr;
#endif
} VideoState;

extern int audio_disable;
extern int video_disable;
extern int subtitle_disable;
extern int display_disable;
extern int loop;
extern int framedrop;
extern int decoder_reorder_pts;
extern int64_t audio_callback_time;
extern int lowres;
extern int fast;
extern int genpts;
extern int seek_by_bytes;
extern int autoexit;
extern int startup_volume;
extern int show_status;
extern int av_sync_type;
extern int64_t start_time;
extern int64_t duration;
extern const char* wanted_stream_spec[AVMEDIA_TYPE_NB];
extern int infinite_buffer;
extern int find_stream_info;
extern char *afilters;
extern int filter_nbthreads;
extern const char *audio_codec_name;
extern const char *subtitle_codec_name;
extern const char *video_codec_name;
extern AVDictionary *sws_dict;
extern AVDictionary *swr_opts;
extern AVDictionary *format_opts, *codec_opts, *resample_opts;
extern SDL_AudioDeviceID audio_dev;

void init_opts(void);
void uninit_opts(void);
AVDictionary *filter_codec_opts(AVDictionary *opts, enum AVCodecID codec_id,
                                AVFormatContext *s, AVStream *st, AVCodec *codec);
AVDictionary **setup_find_stream_info_opts(AVFormatContext *s, AVDictionary *codec_opts);
int opt_add_vfilter(void *optctx, const char *opt, const char *arg);
int configure_audio_filters(VideoState *is, const char *afilters, int force_output_format);
VideoState *stream_open(const char *filename, AVInputFormat *iformat);
void stream_close(VideoState * is);
void stream_seek(VideoState *is, int64_t pos, int64_t rel, int seek_by_bytes);
void stream_toggle_pause(VideoState *is);
void step_to_next_frame(VideoState *is);
void toggle_pause(VideoState *is);
void toggle_mute(VideoState *is);
void update_volume(VideoState *is, int sign, double step);

#endif /* __MEDIA_PLAYER_H__ */

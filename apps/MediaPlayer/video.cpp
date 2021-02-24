#include "clock.h"
#include "video.h"
#include "packet.h"
#include "frame.h"
#include "decoder.h"
#include "ImGuiHelper.h"

static double compute_target_delay(double delay, VideoState *is)
{
    double sync_threshold, diff = 0;

    /* update delay to follow master synchronisation source */
    if (get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER) {
        /* if video is slave, we try to correct big delays by
           duplicating or deleting a frame */
        diff = get_clock(&is->vidclk) - get_master_clock(is);

        /* skip or repeat frame. We take into account the
            delay to compute the threshold. I still don't know
            if it is the best guess */
        sync_threshold = FFMAX(AV_SYNC_THRESHOLD_MIN, FFMIN(AV_SYNC_THRESHOLD_MAX, delay));
        if (!isnan(diff) && fabs(diff) < is->max_frame_duration) {
            if (diff <= -sync_threshold)
                delay = FFMAX(0, delay + diff);
            else if (diff >= sync_threshold && delay > AV_SYNC_FRAMEDUP_THRESHOLD)
                delay = delay + diff;
            else if (diff >= sync_threshold)
                delay = 2 * delay;
        }
    }

    av_log(NULL, AV_LOG_TRACE, "video: delay=%0.3f A-V=%f\n",
            delay, -diff);

    return delay;
}

static double vp_duration(VideoState *is, Frame *vp, Frame *nextvp) {
    if (vp->serial == nextvp->serial) {
        double duration = nextvp->pts - vp->pts;
        if (isnan(duration) || duration <= 0 || duration > is->max_frame_duration)
            return vp->duration;
        else
            return duration;
    } else {
        return 0.0;
    }
}

static void update_video_pts(VideoState *is, double pts, int64_t pos, int serial) {
    /* update current video pts */
    set_clock(&is->vidclk, pts, serial);
    sync_clock_to_slave(&is->extclk, &is->vidclk);
}

static void video_image_display(VideoState *is)
{
    Frame *vp;
    Frame *sp = NULL;
    vp = frame_queue_peek_last(&is->pictq);
    if (!vp || !vp->frame)
    {
        return;
    }

    if (is->subtitle_st) 
    {
        if (frame_queue_nb_remaining(&is->subpq) > 0) 
        {
            sp = frame_queue_peek(&is->subpq);
            if (vp->pts >= sp->pts + ((float) sp->sub.start_display_time / 1000)) 
            {
                if (!sp->uploaded) 
                {
                    // TODO::Dicky Draw Subtitle
                    sp->uploaded = 1;
                }
            }
            else
                sp = NULL;
        }
    }

    int ret = 0;
    AVFrame *tmp_frame = nullptr;
    AVFrame *sw_frame = av_frame_alloc();
    if (!sw_frame)
    {
        fprintf(stderr, "Can not alloc frame\n");
        return;
    }
    if (vp->frame->format == is->hw_pix_fmt) 
    {
        /* retrieve data from GPU to CPU */
        if ((ret = av_hwframe_transfer_data(sw_frame, vp->frame, 0)) < 0) 
        {
            fprintf(stderr, "Error transferring the data to system memory\n");
            av_frame_free(&sw_frame);
            return;
        }
        else
        {
            tmp_frame = sw_frame;
        }
    }
    else
    {
        tmp_frame = vp->frame;
    }
#ifdef IMGUI_VULKAN_SHADER
    const AVPixFmtDescriptor *desc = av_pix_fmt_desc_get((AVPixelFormat)tmp_frame->format);
    if (is->video_depth == 0)
    {
        is->video_depth = desc->comp[0].depth;
    }
    int video_shift = desc->comp[0].depth + desc->comp[0].shift;
    ImVulkan::ColorSpace color_space =  is->video_color_space == AVCOL_SPC_BT470BG ||
                                        is->video_color_space == AVCOL_SPC_SMPTE170M ||
                                        is->video_color_space == AVCOL_SPC_BT470BG ? ImVulkan::BT601 :
                                        is->video_color_space == AVCOL_SPC_BT709 ? ImVulkan::BT709 :
                                        is->video_color_space == AVCOL_SPC_BT2020_NCL ||
                                        is->video_color_space == AVCOL_SPC_BT2020_CL ? ImVulkan::BT2020 : ImVulkan::BT709;
    ImVulkan::ColorRange color_range =  is->video_color_range == AVCOL_RANGE_MPEG ? ImVulkan::NARROW_RANGE :
                                        is->video_color_range == AVCOL_RANGE_JPEG ? ImVulkan::FULL_RANGE : ImVulkan::NARROW_RANGE;
    ImVulkan::ColorFormat color_format = ISYUV420P(tmp_frame->format) ? ImVulkan::YUV420 :
                                        ISYUV422P(tmp_frame->format) ? ImVulkan::YUV422 :
                                        ISYUV444P(tmp_frame->format) ? ImVulkan::YUV444 :
                                        ISNV12(tmp_frame->format) ? ImVulkan::NV12 : ImVulkan::YUV420;
    
    ImVulkan::ImageBuffer im_Y, im_U, im_V;
    int data_shift = is->video_depth > 8 ? 1 : 0;
    is->video_clip = (float)is->video_width / (float)(tmp_frame->linesize[0] >> data_shift);
    int UV_shift_w = ISYUV420P(tmp_frame->format) || ISYUV422P(tmp_frame->format) ? 1 : 0;
    int UV_shift_h = ISYUV420P(tmp_frame->format) || ISNV12(tmp_frame->format) ? 1 : 0;
    im_Y.create_type(tmp_frame->linesize[0] >> data_shift, tmp_frame->height, 1, tmp_frame->data[0], is->video_depth > 8 ? ImVulkan::INT16 : ImVulkan::INT8);
    im_U.create_type(tmp_frame->linesize[1] >> data_shift, tmp_frame->height >> UV_shift_h, 1, tmp_frame->data[1], is->video_depth > 8 ? ImVulkan::INT16 : ImVulkan::INT8);
    if (!ISNV12(tmp_frame->format))
    {
        im_V.create_type(tmp_frame->linesize[2] >> data_shift, tmp_frame->height >> UV_shift_h, 1, tmp_frame->data[2], is->video_depth > 8 ? ImVulkan::INT16 : ImVulkan::INT8);
    }

    if (is->video_width > 1920 || is->video_height > 1920 || is->video_depth > 1920)
    {
        float frame_scale = 1920.f / (float)is->video_width;
        ImVulkan::VkImageBuffer im_RGB;
        is->yuv2rgb->YUV2RGBA(im_Y, im_U, im_V, im_RGB, color_format, color_space, color_range, is->video_depth, video_shift);
        is->resize->Resize(im_RGB, is->vkimage, frame_scale, 0.f, ImVulkan::INTERPOLATE_AREA);
    }
    else
    {
        is->yuv2rgb->YUV2RGBA(im_Y, im_U, im_V, is->vkimage, color_format, color_space, color_range, is->video_depth, video_shift);
    }
    if (!is->video_texture) is->video_texture = ImGui::ImCreateTexture(is->vkimage);
#else
    int data_shift = is->video_depth > 8 ? 1 : 0;
    int out_w = tmp_frame->linesize[0] >> data_shift;
    int out_h = tmp_frame->height;
    is->video_clip = (float)is->video_width / (float)(tmp_frame->linesize[0] >> data_shift);
    if (is->video_pfmt != tmp_frame->format)
    {
        is->img_convert_ctx = sws_getCachedContext(
                                    is->img_convert_ctx,
                                    out_w,
                                    out_h,
                                    (AVPixelFormat)tmp_frame->format,
                                    out_w,
                                    out_h,
                                    AV_PIX_FMT_RGBA,
                                    SWS_BICUBIC,
                                    NULL, NULL, NULL);
        if (!is->img_convert_ctx)
        {
            av_frame_unref(sw_frame);
            return;
        }
        is->video_pfmt = (AVPixelFormat)tmp_frame->format;
    }
    AVFrame rgb_picture;
    memset(&rgb_picture, 0, sizeof(rgb_picture));
    rgb_picture.format = AV_PIX_FMT_RGBA;
    rgb_picture.width = out_w;
    rgb_picture.height = out_h;
    ret = av_frame_get_buffer(&rgb_picture, 64);
    if (ret == 0)
    {
        sws_scale(
            is->img_convert_ctx,
            tmp_frame->data,
            tmp_frame->linesize,
            0, out_h,
            rgb_picture.data,
            rgb_picture.linesize
        );
        ImGui::ImGenerateOrUpdateTexture(is->video_texture, out_w, out_h, 4, rgb_picture.data[0]);
        av_frame_unref(&rgb_picture);
    }
#endif
    av_frame_free(&sw_frame);
}

/* called to display each frame */
static void video_refresh(void *opaque, double *remaining_time)
{
    VideoState *is = (VideoState *)opaque;
    double time;

    Frame *sp, *sp2;

    if (!is->paused && get_master_sync_type(is) == AV_SYNC_EXTERNAL_CLOCK && is->realtime)
        check_external_clock_speed(is);

    if (is->video_st) {
retry:
        if (frame_queue_nb_remaining(&is->pictq) == 0) {
            // nothing to do, no picture to display in the queue
        } else {
            double last_duration, duration, delay;
            Frame *vp, *lastvp;

            /* dequeue the picture */
            lastvp = frame_queue_peek_last(&is->pictq);
            vp = frame_queue_peek(&is->pictq);

            if (vp->serial != is->videoq.serial) {
                frame_queue_next(&is->pictq);
                goto retry;
            }

            if (lastvp->serial != vp->serial)
                is->frame_timer = av_gettime_relative() / 1000000.0;

            if (is->paused)
                goto display;

            /* compute nominal last_duration */
            last_duration = vp_duration(is, lastvp, vp);
            delay = compute_target_delay(last_duration, is);

            time= av_gettime_relative()/1000000.0;
            if (time < is->frame_timer + delay) {
                *remaining_time = FFMIN(is->frame_timer + delay - time, *remaining_time);
                goto display;
            }

            is->frame_timer += delay;
            if (delay > 0 && time - is->frame_timer > AV_SYNC_THRESHOLD_MAX)
                is->frame_timer = time;

            SDL_LockMutex(is->pictq.mutex);
            if (!isnan(vp->pts))
                update_video_pts(is, vp->pts, vp->pos, vp->serial);
            SDL_UnlockMutex(is->pictq.mutex);

            if (frame_queue_nb_remaining(&is->pictq) > 1) {
                Frame *nextvp = frame_queue_peek_next(&is->pictq);
                duration = vp_duration(is, vp, nextvp);
                if(!is->step && (framedrop>0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) && time > is->frame_timer + duration){
                    is->frame_drops_late++;
                    frame_queue_next(&is->pictq);
                    goto retry;
                }
            }

            if (is->subtitle_st) {
                while (frame_queue_nb_remaining(&is->subpq) > 0) {
                    sp = frame_queue_peek(&is->subpq);

                    if (frame_queue_nb_remaining(&is->subpq) > 1)
                        sp2 = frame_queue_peek_next(&is->subpq);
                    else
                        sp2 = NULL;

                    if (sp->serial != is->subtitleq.serial
                            || (is->vidclk.pts > (sp->pts + ((float) sp->sub.end_display_time / 1000)))
                            || (sp2 && is->vidclk.pts > (sp2->pts + ((float) sp2->sub.start_display_time / 1000))))
                    {
                        if (sp->uploaded) {
                            int i;
                            for (i = 0; i < sp->sub.num_rects; i++) {
                                AVSubtitleRect *sub_rect = sp->sub.rects[i];
                                uint8_t *pixels;
                                int pitch, j;
                                /* TODO::Dicky Render Subtitle
                                if (!SDL_LockTexture(is->sub_texture, (SDL_Rect *)sub_rect, (void **)&pixels, &pitch)) {
                                    for (j = 0; j < sub_rect->h; j++, pixels += pitch)
                                        memset(pixels, 0, sub_rect->w << 2);
                                    SDL_UnlockTexture(is->sub_texture);
                                }
                                */
                            }
                        }
                        frame_queue_next(&is->subpq);
                    } else {
                        break;
                    }
                }
            }

            frame_queue_next(&is->pictq);
            is->force_refresh = 1;

            if (is->step && !is->paused)
                stream_toggle_pause(is);
        }
display:
        /* display picture */
        if (!display_disable && is->force_refresh && is->pictq.rindex_shown)
            video_image_display(is);
    }
    is->force_refresh = 0;
    if (show_status) {
        AVBPrint buf;
        static int64_t last_time;
        int64_t cur_time;
        int aqsize, vqsize, sqsize;
        double av_diff;

        cur_time = av_gettime_relative();
        if (!last_time || (cur_time - last_time) >= 30000) {
            aqsize = 0;
            vqsize = 0;
            sqsize = 0;
            if (is->audio_st)
                aqsize = is->audioq.size;
            if (is->video_st)
                vqsize = is->videoq.size;
            if (is->subtitle_st)
                sqsize = is->subtitleq.size;
            av_diff = 0;
            if (is->audio_st && is->video_st)
                av_diff = get_clock(&is->audclk) - get_clock(&is->vidclk);
            else if (is->video_st)
                av_diff = get_master_clock(is) - get_clock(&is->vidclk);
            else if (is->audio_st)
                av_diff = get_master_clock(is) - get_clock(&is->audclk);

            av_bprint_init(&buf, 0, AV_BPRINT_SIZE_AUTOMATIC);
            av_bprintf(&buf,
                        "%7.2f %s:%7.3f fd=%4d aq=%5dKB vq=%5dKB sq=%5dB f=%" PRId64 "/%" PRId64 "   \r",
                        get_master_clock(is),
                        (is->audio_st && is->video_st) ? "A-V" : (is->video_st ? "M-V" : (is->audio_st ? "M-A" : "   ")),
                        av_diff,
                        is->frame_drops_early + is->frame_drops_late,
                        aqsize / 1024,
                        vqsize / 1024,
                        sqsize,
                        (is->video_st && is->viddec.avctx) ? is->viddec.avctx->pts_correction_num_faulty_dts : 0,
                        (is->video_st && is->viddec.avctx) ? is->viddec.avctx->pts_correction_num_faulty_pts : 0);
            is->stats_string = std::string(buf.str);
            av_bprint_finalize(&buf, NULL);

            last_time = cur_time;
        }
    }
}

static int queue_picture(VideoState *is, AVFrame *src_frame, double pts, double duration, int64_t pos, int serial)
{
    Frame *vp;

    if (!(vp = frame_queue_peek_writable(&is->pictq)))
        return -1;

    vp->sar = src_frame->sample_aspect_ratio;
    vp->uploaded = 0;

    vp->width = src_frame->width;
    vp->height = src_frame->height;
    vp->format = src_frame->format;

    vp->pts = pts;
    vp->duration = duration;
    vp->pos = pos;
    vp->serial = serial;

    av_frame_move_ref(vp->frame, src_frame);
    frame_queue_push(&is->pictq);
    return 0;
}

static int get_video_frame(VideoState *is, AVFrame *frame)
{
    int got_picture;

    if ((got_picture = decoder_decode_frame(&is->viddec, frame, NULL)) < 0)
        return -1;

    if (got_picture) {
        double dpts = NAN;

        if (frame->pts != AV_NOPTS_VALUE)
            dpts = av_q2d(is->video_st->time_base) * frame->pts;

        frame->sample_aspect_ratio = av_guess_sample_aspect_ratio(is->ic, is->video_st, frame);

        if (framedrop>0 || (framedrop && get_master_sync_type(is) != AV_SYNC_VIDEO_MASTER)) {
            if (frame->pts != AV_NOPTS_VALUE) {
                double diff = dpts - get_master_clock(is);
                if (!isnan(diff) && fabs(diff) < AV_NOSYNC_THRESHOLD &&
                    diff - is->frame_last_filter_delay < 0 &&
                    is->viddec.pkt_serial == is->vidclk.serial &&
                    is->videoq.nb_packets) {
                    is->frame_drops_early++;
                    av_frame_unref(frame);
                    got_picture = 0;
                }
            }
        }
    }

    return got_picture;
}

int video_thread(void *arg)
{
    VideoState *is = (VideoState *)arg;
    AVFrame *frame = av_frame_alloc();
    double pts;
    double duration;
    int ret;
    AVRational tb = is->video_st->time_base;
    AVRational frame_rate = av_guess_frame_rate(is->ic, is->video_st, NULL);

    if (!frame)
        return AVERROR(ENOMEM);

    for (;;) 
    {
        ret = get_video_frame(is, frame);
        if (ret < 0)
            goto the_end;
        if (!ret)
            continue;

        duration = (frame_rate.num && frame_rate.den ? av_q2d((AVRational){frame_rate.den, frame_rate.num}) : 0);
        pts = (frame->pts == AV_NOPTS_VALUE) ? NAN : frame->pts * av_q2d(tb);
        ret = queue_picture(is, frame, pts, duration, frame->pkt_pos, is->viddec.pkt_serial);
        av_frame_unref(frame);

        if (ret < 0)
            goto the_end;
    }
the_end:
    av_frame_free(&frame);
    return 0;
}

static int video_playing_thread(void *arg)
{
    VideoState *is = (VideoState *)arg;
    double remaining_time = 0.0;
    while (1)
    {
        if (is->abort_request)
        {
            break;
        }
        if (remaining_time > 0.0)
        {
            av_usleep((unsigned)(remaining_time * 1000000.0));
        }
        remaining_time = REFRESH_RATE;

        if (!is->paused || is->force_refresh)
            video_refresh(is, &remaining_time);
    }

    return 0;
}

int open_video_render(VideoState *is)
{
    // 创建视频显示线程
    is->render_tid = SDL_CreateThread(video_playing_thread, "video playing thread", is);
    return 0;
}

#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <application.h>
#include <ImGuiHelper.h>
#include <ImGuiFileDialog.h>
#include <imgui_knob.h>
#include <implot.h>
#include "media_player.h"
#include "packet.h"
#include "frame.h"
#include "demux.h"
#include "audio.h"
#include "video.h"
#include "clock.h"
#include "Config.h"

static std::string ini_file = std::string(DEFAULT_CONFIG_PATH) + "Media_Player.ini";
static std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
static ImGuiFileDialog filedialog;

int audio_disable = 0;
int video_disable = 0;
int subtitle_disable = 0;
int display_disable = 0;
int loop = 0;
int framedrop = 1;
int decoder_reorder_pts = -1;
int64_t audio_callback_time = 0;
int lowres = 0;
int fast = 0;
int genpts = 0;
int seek_by_bytes = -1;
int autoexit = 0;
int startup_volume = 100;
int show_status = 0;
int av_sync_type = AV_SYNC_AUDIO_MASTER;
int64_t start_time = AV_NOPTS_VALUE;
int64_t duration = AV_NOPTS_VALUE;
const char* wanted_stream_spec[AVMEDIA_TYPE_NB] = {0};
int infinite_buffer = -1;
int find_stream_info = 1;
char *afilters = NULL;
int filter_nbthreads = 0;
const char *audio_codec_name = nullptr;
const char *subtitle_codec_name = nullptr;
const char *video_codec_name = nullptr;
AVDictionary *sws_dict;
AVDictionary *swr_opts;
AVDictionary *format_opts, *codec_opts, *resample_opts;
SDL_AudioDeviceID audio_dev;

static VideoState * is = nullptr;
static bool show_info = false;
static bool show_wavefrom = false;

static inline int compute_mod(int a, int b)
{
    return a < 0 ? a%b + b : a%b;
}

// Application Framework Functions
const char* Application_GetName(void* handle)
{
    return "Media Player";
}

void Application_Initialize(void** handle)
{
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = ini_file.c_str();
    io.DeltaTime = 1.0f / 30.f;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());
    // init ffmpeg
    av_log_set_level(AV_LOG_QUIET);
    //av_log_set_level(AV_LOG_WARNING);
    avformat_network_init();
    init_opts();
}

void Application_Finalize(void** handle)
{
    if (is) stream_close(is);
    uninit_opts();
    avformat_network_deinit();
    ImPlot::DestroyContext();
    end_file_dialog_demo_window(&filedialog, bookmark_path.c_str());
}

bool Application_Frame(void* handle)
{
    bool done = false;
    auto& io = ImGui::GetIO();
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
                ImGui::Text("ImGUI Media Player\n\n");
                ImGui::Separator();
                ImGui::Text("Dicky 2021\n\n");
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
				filedialog.OpenModal("ChooseFileDlgKey",
                                    ICON_IGFD_FOLDER_OPEN " 打开视频文件", filters, ".");
            }
            ImGui::Separator();
            if (ImGui::Button("退出")) 
            {
                if (is) stream_close(is);
                is = nullptr;
                done = true;
            }

            ImGui::EndMenu();
        }
        ImGui::EndMainMenuBar();
    }
    ImVec2 maxSize = ImVec2((float)io.DisplaySize.x, (float)io.DisplaySize.y);
	ImVec2 minSize = maxSize * 0.5f;
    if (filedialog.Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
	{
        if (filedialog.IsOk())
		{
            std::string filePathName = filedialog.GetFilePathName();
            if (is != nullptr)
            {
                if (is) stream_close(is);
                is = nullptr;
            }
            is = stream_open(filePathName.c_str(), nullptr);
            if (is == nullptr)
            {
                ImGui::OpenPopup("Open Error?");
            }
        }
        filedialog.Close();
    }

    // Show PlayControl panel
    ImVec2 center(io.DisplaySize.x * 0.5f, io.DisplaySize.y * 0.9f);
    ImVec2 panel_size(io.DisplaySize.x - 20.0, show_status ? 140 : 120);
    ImGui::SetNextWindowPos(center, ImGuiCond_Always, ImVec2(0.5f, 0.5f));
    ImGui::SetNextWindowSize(panel_size, ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.5);
    if (ImGui::Begin("Control", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize))
    {
        int i = ImGui::FindWindowByName("Control")->Size.x;
        // add button
        ImGui::Indent((i - 32.0f) * 0.5f);
        ImVec2 size = ImVec2(32.0f, 32.0f); // Size of the image we want to make visible
        if (ImGui::Button(!is ? ICON_FAD_PLAY : is->paused ? ICON_FAD_PLAY : ICON_FAD_PAUSE, size))
        {
            if (is) toggle_pause(is);
        }
        ImGui::SameLine();
        if (ImGui::Button(!is ? ICON_FA5_VOLUME_UP : is->muted ? ICON_FA5_VOLUME_MUTE : ICON_FA5_VOLUME_UP, size))
        {
            if (is) toggle_mute(is);
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FK_TAG , size))
        {
            if (is) show_info = !show_info;
        }
        ImGui::SameLine();
        if (ImGui::Button(ICON_FAD_WAVEFORM , size))
        {
            if (is) show_wavefrom = !show_wavefrom;
        }
        // add button end

        ImGui::Unindent((i - 32.0f) * 0.5f);
        ImGui::Separator();
        // add audio meter bar
        static int left_stack = 0;
        static int left_count = 0;
        static int right_stack = 0;
        static int right_count = 0;
        if (is)
        {
            ImGui::UvMeter("##lhuvr", ImVec2(panel_size.x, 10), &is->audio_left_channel_level, 0, 96, 200, &left_stack, &left_count);
            ImGui::UvMeter("##rhuvr", ImVec2(panel_size.x, 10), &is->audio_right_channel_level, 0, 96, 200, &right_stack, &right_count);
        }
        else
        {
            int zero_channel_level = 0;
            ImGui::UvMeter("##lhuvr", ImVec2(panel_size.x, 10), &zero_channel_level, 0, 96, 200);
            ImGui::UvMeter("##rhuvr", ImVec2(panel_size.x, 10), &zero_channel_level, 0, 96, 200);
        }
        ImGui::Separator();
        // add slider bar
        if (is && is->total_time > 0)
        {
            float time = get_master_clock(is);
            if (time == NAN) time = 0;
            float oldtime = time;
            static ImGuiSliderFlags flags = ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoLabel;
            if (ImGui::SliderFloat("time", &time, 0, is->total_time, "%.2f", flags))
            {
                float incr = time - oldtime;
                if (fabs(incr) > 1.0)
                {
                    if (is->ic->start_time != AV_NOPTS_VALUE && time < is->ic->start_time / (double)AV_TIME_BASE)
                        time = is->ic->start_time / (double)AV_TIME_BASE;
                    stream_seek(is, (int64_t)(time * AV_TIME_BASE), (int64_t)(incr * AV_TIME_BASE), 0);
                }
            }
            ImGui::SameLine();
            int hours = time / 60 / 60; time -= hours * 60 * 60;
            int mins = time / 60; time -= mins * 60;
            int secs = time; time -= secs;
            int ms = time * 1000;
            ImGui::Text("%02d:%02d:%02d.%03d", hours, mins, secs, ms);

            ImGui::SameLine();
            float ftime = is->total_time * 1000.0f;
            hours = ftime / 1000 / 60 / 60; ftime -= hours * 60 * 60 * 1000;
            mins = ftime / 1000 / 60; ftime -= mins * 60 * 1000;
            secs = ftime / 1000; ftime -= secs * 1000;
            ms = ftime;
            ImGui::Text("/ %02d:%02d:%02d.%03d", hours, mins, secs, ms);

            ImGui::SameLine();
            ImGui::Text("[%.3fms %.1ffps]", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        }
        else
        {
            // draw empty bar
            float time = 0;
            ImGui::SliderFloat("time", &time, 0, 0, "%.2f", ImGuiSliderFlags_NoInput | ImGuiSliderFlags_NoLabel);
        }
        if (show_status)
        {
            ImGui::Separator();
            // add stats bar
            if (is && !is->stats_string.empty())
                ImGui::Text("%s", is->stats_string.c_str());
            else
                ImGui::Text("no media");
        }
        ImGui::End();
    }

    // handle key event
    if (is && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Space), false))
    {
        toggle_pause(is);
    }
    if (is && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape), false))
    {
        if (is) stream_close(is);
        is = nullptr;
        done = true;
    }
    if (is && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_LeftArrow), true))
    {
        float time = get_master_clock(is);
        if (time == NAN) time = 0;
        time -= 1.f;
        if (time < 0) time = 0;
        stream_seek(is, (int64_t)(time * AV_TIME_BASE), (int64_t)(-1 * AV_TIME_BASE), 0);
    }
    if (is && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_RightArrow), true))
    {
        float time = get_master_clock(is);
        if (time == NAN) time = 0;
        time += 1.f;
        if (time > is->total_time * 1000.f) time = is->total_time * 1000.f;
        stream_seek(is, (int64_t)(time * AV_TIME_BASE), (int64_t)(1 * AV_TIME_BASE), 0);
    }

    if (is && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_DownArrow), true))
    {
        float time = get_master_clock(is);
        if (time == NAN) time = 0;
        time -= 5.f;
        if (time < 0) time = 0;
        stream_seek(is, (int64_t)(time * AV_TIME_BASE), (int64_t)(-5 * AV_TIME_BASE), 0);
    }

    if (is && !io.KeyCtrl && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_UpArrow), true))
    {
        float time = get_master_clock(is);
        if (time == NAN) time = 0;
        time += 5.f;
        if (time > is->total_time * 1000.f) time = is->total_time * 1000.f;
        stream_seek(is, (int64_t)(time * AV_TIME_BASE), (int64_t)(5 * AV_TIME_BASE), 0);
    }

    if (is && !io.KeyShift && !io.KeyAlt && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Z), true))
    {
        if (is->paused)
        {
            step_to_next_frame(is);
        }
    }

    // Video texture display
    if (is && is->video_texture)
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
        bool bViewisLandscape = window_size.x >= window_size.y ? true : false;
        bool bRenderisLandscape = is->video_width >= is->video_height ? true : false;
        bool bNeedChangeScreenInfo = bViewisLandscape ^ bRenderisLandscape;
        float adj_w = bNeedChangeScreenInfo ? window_size.y : window_size.x;
        float adj_h = bNeedChangeScreenInfo ? window_size.x : window_size.y;
        float adj_x = adj_h * is->video_aspect_ratio;
        float adj_y = adj_h;
        if (adj_x > adj_w) { adj_y *= adj_w / adj_x; adj_x = adj_w; }
        float offset_x = (window_size.x - adj_x) / 2.0;
        float offset_y = (window_size.y - adj_y) / 2.0;
        ImGui::SetNextWindowSize(ImVec2(adj_x, adj_y), ImGuiCond_Always);
        ImGui::SetNextWindowPos(ImVec2(offset_x, offset_y), ImGuiCond_Always);
        if (ImGui::Begin("screen", nullptr, flags)) 
        {
            ImVec2 content_region = ImGui::GetContentRegionAvail();
            ImGui::Image((void *)(intptr_t)is->video_texture, content_region,
                        ImVec2(0.0f, 0.0f), ImVec2(is->video_clip, 1.0f), ImVec4(1.0, 1.0, 1.0, 1.0));
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
    if (is && is->ic && show_info)
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
        if (ImGui::Begin("Media Info", nullptr, window_flags))
        {
            ImGui::Text("  Media name: %s", is->ic->url);
            ImGui::Text("Media format: %s", is->ic->iformat->long_name);
            float ftime = is->total_time * 1000.0f;
            int hours = ftime / 1000 / 60 / 60; ftime -= hours * 60 * 60 * 1000;
            int mins = ftime / 1000 / 60; ftime -= mins * 60 * 1000;
            int secs = ftime / 1000; ftime -= secs * 1000;
            int ms = ftime;
            ImGui::Text("  Media time: %02d:%02d:%02d.%03d", hours, mins, secs, ms);
            ImGui::Separator();

            if (is->video_st)
            {
                ImGui::Text("Video Stream");
                ImGui::Text("     Codec: %s", is->video_codec_name.c_str());
                ImGui::Text("    Format: %s", av_get_pix_fmt_name(is->video_pfmt));
                ImGui::Text("     Depth: %d", is->video_depth);
                ImGui::Text("     Width: %d", is->video_width);
                ImGui::Text("    Height: %d", is->video_height);
                ImGui::Text("       FPS: %.2f", is->video_fps);
                ImGui::Text("    Frames: %d", is->video_frames);
                if (is->video_color_space != AVCOL_SPC_UNSPECIFIED)
                {
                    ImGui::Text("ColorSpace: %s", av_color_space_name(is->video_color_space));
                    ImGui::Text("ColorRange: %s", av_color_range_name(is->video_color_range));
                }
                else
                    ImGui::Text("ColorSpace: %s", av_get_colorspace_name(is->video_color_space));

                ImGui::Separator();
            }
            if (is->audio_st)
            {
                ImGui::Text("Audio Stream");
                ImGui::Text("     Codec: %s", is->audio_codec_name.c_str());
                ImGui::Text("    Format: %s", av_get_sample_fmt_name(is->audio_sfmt));
                ImGui::Text("     Depth: %d", is->audio_depth);
                ImGui::Text("      Rate: %d", is->audio_sample_rate);
                ImGui::Text("  Channels: %d", is->audio_channels);
            }
            if (is->subtitle_st)
            {
                ImGui::Text("Subtitle Stream");
                ImGui::Text("     Codec: %s", is->subtitle_codec_name.c_str());
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
        }
        ImGui::End();
    }

    // Show wavefrom
    if (is && is->audio_st && show_wavefrom)
    {
        const float DISTANCE = 40.0f;
        static int corner = 3;
        ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoFocusOnAppearing | ImGuiWindowFlags_NoNav;
        if (corner != -1)
        {
            window_flags |= ImGuiWindowFlags_NoMove;
            ImVec2 window_pos = ImVec2((corner & 1) ? io.DisplaySize.x - DISTANCE : DISTANCE, (corner & 2) ? io.DisplaySize.y - DISTANCE : DISTANCE);
            ImVec2 window_pos_pivot = ImVec2((corner & 1) ? 1.0f : 0.0f, (corner & 2) ? 1.0f : 0.0f);
            ImGui::SetNextWindowPos(window_pos, ImGuiCond_Always, window_pos_pivot);
        }
        ImGui::SetNextWindowBgAlpha(0.35f); // Transparent background
        if (ImGui::Begin("Wave Plot", nullptr, window_flags))
        {
            int data_used = 256;
            int view_height = data_used / 4;
            float left_wave[data_used], right_wave[data_used];
            int64_t time_diff;
            int i, x, h, i_start;
            int channels = is->audio_tgt.channels;
            if (!is->paused)
            {
                int n = 2 * channels;
                int delay = is->audio_write_buf_size;
                delay /= n;
                if (audio_callback_time)
                {
                    time_diff = av_gettime_relative() - audio_callback_time;
                    delay -= (time_diff * is->audio_tgt.freq) / 1000000;
                }
                delay += 2 * data_used;
                if (delay < data_used)
                    delay = data_used;
                i_start = x = compute_mod(is->sample_array_index - delay * channels, SAMPLE_ARRAY_SIZE);
                {
                    h = INT_MIN;
                    for (i = 0; i < 1000; i += channels) 
                    {
                        int idx = (SAMPLE_ARRAY_SIZE + x - i) % SAMPLE_ARRAY_SIZE;
                        int a = is->sample_array[idx];
                        int b = is->sample_array[(idx + 4 * channels) % SAMPLE_ARRAY_SIZE];
                        int c = is->sample_array[(idx + 5 * channels) % SAMPLE_ARRAY_SIZE];
                        int d = is->sample_array[(idx + 9 * channels) % SAMPLE_ARRAY_SIZE];
                        int score = a - d;
                        if (h < score && (b ^ c) < 0) 
                        {
                            h = score;
                            i_start = idx;
                        }
                    }
                    is->last_i_start = i_start;
                }
            }
            else
                i_start = is->last_i_start;
            i = i_start;
            for (x = 0; x < data_used; x++) 
            {
                float left_data = is->sample_array[i + 0] / 32768.f;
                left_wave[x] = left_data;
                if (channels > 1)
                {
                    float right_data = is->sample_array[i + 1] / 32768.f;
                    right_wave[x] = right_data;
                }
                i += channels;
                if (i >= SAMPLE_ARRAY_SIZE)
                    i -= SAMPLE_ARRAY_SIZE;
            }
            ImPlotStyle& style = ImPlot::GetStyle();
            style.Colors[ImPlotCol_PlotBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
            style.Colors[ImPlotCol_FrameBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.20f);
            style.Colors[ImPlotCol_Line] = ImVec4(0.00f, 1.00f, 0.00f, 1.00f);
            ImPlotFlags flags = ImPlotFlags_CanvasOnly | ImPlotFlags_NoChild;
            ImPlotAxisFlags axis_flags = ImPlotAxisFlags_NoDecorations | ImPlotAxisFlags_LockMin | ImPlotAxisFlags_LockMax;
            ImPlot::PushStyleVar(ImPlotStyleVar_PlotBorderSize, 0.f);
            ImPlot::PushStyleVar(ImPlotStyleVar_PlotPadding, ImVec2(0.f, 0.f));
            ImPlot::SetNextPlotLimits(0, data_used, -1, 1);
            if (ImPlot::BeginPlot("##Audio wave left", NULL, NULL, ImVec2(data_used, view_height), flags, axis_flags, axis_flags)) 
            {
                ImPlot::PlotLine("##lwave", left_wave, data_used);
                ImPlot::EndPlot();
            }
            ImGui::Separator();
            ImPlot::SetNextPlotLimits(0, data_used, -1, 1);
            if (ImPlot::BeginPlot("##Audio wave right", NULL, NULL, ImVec2(data_used, view_height), flags, axis_flags, axis_flags)) 
            {
                ImPlot::PlotLine("##rwave", right_wave, data_used);
                ImPlot::EndPlot();
            }
            ImPlot::PopStyleVar(2);
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
        ImGui::End();
    }
    return done;
}

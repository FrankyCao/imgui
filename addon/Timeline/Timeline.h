#pragma once
namespace ImGui
{
    // TimeLine
    IMGUI_API bool BeginTimeline(const char* str_id, float max_value=0.f, int num_visible_rows=0, int opt_exact_num_rows=0,ImVec2* popt_offset_and_scale=NULL);
    IMGUI_API bool TimelineEvent(const char* str_id, float* values, bool keep_range_constant=false);
    IMGUI_API void EndTimeline(int num_vertical_grid_lines=5.f,float current_time=0.f,ImU32 timeline_running_color=IM_COL32(0,128,0,200));
} // namespace ImGui

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"
#include <string>
#include <cmath>
#include <ctype.h>      // toupper
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif

namespace ImGui 
{
// Timeline Stuff Here (from: https://github.com/nem0/LumixEngine/blob/timeline_gui/external/imgui/imgui_user.inl)=
// Improved with code by @meshula (panzoomer here: https://github.com/ocornut/imgui/issues/76)
static float s_max_timeline_value=0.f;
static int s_timeline_num_rows = 0;
static int s_timeline_display_start = 0;
static int s_timeline_display_end = 0;
static int s_timeline_display_index = 0;
static ImVec2* s_ptimeline_offset_and_scale = NULL;

bool BeginTimeline(const char* str_id, float max_value, int num_visible_rows, int opt_exact_num_rows, ImVec2 *popt_offset_and_scale)
{
    // reset global variables
    s_max_timeline_value=0.f;
    s_timeline_num_rows = s_timeline_display_start = s_timeline_display_end = 0.f;
    s_timeline_display_index = -1.f;
    s_ptimeline_offset_and_scale = popt_offset_and_scale;

    if (s_ptimeline_offset_and_scale) 
    {
        if (s_ptimeline_offset_and_scale->y == 0.f) 
        {
            s_ptimeline_offset_and_scale->y = 1.f;
        }
    }
    const float row_height = ImGui::GetTextLineHeightWithSpacing();
    const bool rv = BeginChild(str_id,ImVec2(0,num_visible_rows>=0 ? (row_height*num_visible_rows) : (ImGui::GetContentRegionAvail().y-row_height)),false);
    ImGui::PushStyleColor(ImGuiCol_Separator,GImGui->Style.Colors[ImGuiCol_Border]);
    ImGui::Columns(2,str_id);
    const float contentRegionWidth = ImGui::GetWindowContentRegionWidth();
    if (ImGui::GetColumnOffset(1) >= contentRegionWidth * 0.48f) 
        ImGui::SetColumnOffset(1, contentRegionWidth * 0.15f);
    s_max_timeline_value = max_value>=0 ? max_value : (contentRegionWidth*0.85f);
    if (opt_exact_num_rows>0) 
    {
        // Item culling
        s_timeline_num_rows = opt_exact_num_rows;
        ImGui::CalcListClipping(s_timeline_num_rows, row_height, &s_timeline_display_start, &s_timeline_display_end);
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + (s_timeline_display_start * row_height));
    }
    return rv;
}
bool TimelineEvent(const char* str_id, float* values,bool keep_range_constant)
{
    ++s_timeline_display_index;
    if (s_timeline_num_rows > 0 &&
        (s_timeline_display_index < s_timeline_display_start || s_timeline_display_index >= s_timeline_display_end)) 
    {
        if (s_timeline_display_index == s_timeline_display_start - 1) 
        {
            ImGui::NextColumn();
            ImGui::NextColumn();
        }    // This fixes a clipping issue at the top visible row
        return false;   // item culling
    }

    const float row_height = ImGui::GetTextLineHeightWithSpacing();
    const float TIMELINE_RADIUS = row_height * 0.45f;
    const float row_height_offset = (row_height - TIMELINE_RADIUS * 2.f) * 0.5f;

    ImGuiWindow* win = GetCurrentWindow();
    const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
    const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]);
    const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SeparatorActive]);
    bool changed = false;
    bool hovered = false;
    bool active = false;

    ImGui::Text("%s",str_id);
    ImGui::NextColumn();

    const float s_timeline_time_offset = s_ptimeline_offset_and_scale ? s_ptimeline_offset_and_scale->x : 0.f;
    const float s_timeline_time_scale = s_ptimeline_offset_and_scale ? s_ptimeline_offset_and_scale->y : 1.f;

    const float columnOffset = ImGui::GetColumnOffset(1);
    const float columnWidth = ImGui::GetColumnWidth(1)-GImGui->Style.ScrollbarSize;
    const float columnWidthScaled = columnWidth * s_timeline_time_scale;
    const float columnWidthOffsetScaled = columnWidthScaled * s_timeline_time_offset;
    const ImVec2 cursor_pos(GetWindowContentRegionMin().x + win->Pos.x+columnOffset-TIMELINE_RADIUS,win->DC.CursorPos.y);
    bool mustMoveBothEnds=false;
    const bool isMouseDraggingZero = IsMouseDragging(0);
    float posx[2] = {0, 0};

    for (int i = 0; i < 2; ++i)
    {
        ImVec2 pos = cursor_pos;
        pos.x += columnWidthScaled * values[i] / s_max_timeline_value - columnWidthOffsetScaled + TIMELINE_RADIUS;
        pos.y += row_height_offset+TIMELINE_RADIUS;
        posx[i] = pos.x;
        if (pos.x+TIMELINE_RADIUS < cursor_pos.x ||
            pos.x-2.f*TIMELINE_RADIUS > cursor_pos.x+columnWidth) 
            continue;   // culling

        SetCursorScreenPos(pos - ImVec2(TIMELINE_RADIUS, TIMELINE_RADIUS));
        PushID(i);
        InvisibleButton(str_id, ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
        active = IsItemActive();
        if (active || IsItemHovered())
        {
            ImGui::SetTooltip("%f", values[i]);
            if (!keep_range_constant)
            {
                // @meshula:The item hovered line needs to be compensated for vertical scrolling. Thx!
                ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y + win->Scroll.y);
                ImVec2 b(pos.x, GetWindowContentRegionMax().y + win->Pos.y + win->Scroll.y);
                // possible aternative:
                //ImVec2 a(pos.x, win->Pos.y);
                //ImVec2 b(pos.x, win->Pos.y+win->Size.y);
                win->DrawList->AddLine(a, b, line_color);
            }
            hovered = true;
        }
        if (active && isMouseDraggingZero)
        {
            if (!keep_range_constant)
            {
                values[i] += GetIO().MouseDelta.x / columnWidthScaled * s_max_timeline_value;
                if (values[i]<0.f) 
                    values[i]=0.f;
                else if (values[i]>s_max_timeline_value) 
                    values[i]=s_max_timeline_value;
            }
            else mustMoveBothEnds = true;
            changed = hovered = true;
        }
        PopID();
        win->DrawList->AddCircleFilled(
                    pos, TIMELINE_RADIUS, IsItemActive() || IsItemHovered() ? active_color : inactive_color, 32);
    }

    ImVec2 start(posx[0] + TIMELINE_RADIUS,cursor_pos.y + row_height * 0.3f);
    ImVec2 end(posx[1] - TIMELINE_RADIUS,start.y + row_height * 0.4f);
    if (start.x < cursor_pos.x) 
        start.x = cursor_pos.x;
    if (end.x > cursor_pos.x + columnWidth + TIMELINE_RADIUS) 
        end.x = cursor_pos.x + columnWidth + TIMELINE_RADIUS;
    const bool isInvisibleButtonCulled = start.x >= cursor_pos.x + columnWidth || end.x <= cursor_pos.x;

    bool isInvisibleButtonItemActive = false;
    bool isInvisibleButtonItemHovered = false;
    if (!isInvisibleButtonCulled)
    {
        PushID(-1);
        SetCursorScreenPos(start);
        InvisibleButton(str_id, end - start);
        isInvisibleButtonItemActive = IsItemActive();
        isInvisibleButtonItemHovered = isInvisibleButtonItemActive || IsItemHovered();
        PopID();
        win->DrawList->AddRectFilled(start, end, isInvisibleButtonItemActive || isInvisibleButtonItemHovered ? active_color : inactive_color);
    }
    if ((isInvisibleButtonItemActive && isMouseDraggingZero) || mustMoveBothEnds)
    {
        const float deltaX = GetIO().MouseDelta.x / columnWidthScaled * s_max_timeline_value;
        values[0] += deltaX;
        values[1] += deltaX;
        changed = hovered = true;
    }
    else if (isInvisibleButtonItemHovered) 
        hovered = true;

    SetCursorScreenPos(cursor_pos + ImVec2(0, row_height));

    if (changed)
    {
        if (values[0] > values[1]) 
        {
            float tmp = values[0];
            values[0] = values[1];
            values[1] = tmp;
        }
        if (values[1] > s_max_timeline_value)
        {
            values[0] -= values[1] - s_max_timeline_value;
            values[1] = s_max_timeline_value;
        }
        if (values[0] < 0)
        {
            values[1] -= values[0];
            values[0] = 0;
        }
    }

    if (hovered) 
        ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);

    ImGui::NextColumn();
    return changed;
}

void EndTimeline(int num_vertical_grid_lines, float current_time, ImU32 timeline_running_color)
{
    const float row_height = ImGui::GetTextLineHeightWithSpacing();
    if (s_timeline_num_rows>0) 
        ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ((s_timeline_num_rows - s_timeline_display_end) * row_height));
    ImGui::NextColumn();

    ImGuiWindow* win = GetCurrentWindow();

    const float columnOffset = ImGui::GetColumnOffset(1);
    const float columnWidth = ImGui::GetColumnWidth(1)-GImGui->Style.ScrollbarSize;
    const float s_timeline_time_offset = s_ptimeline_offset_and_scale ? s_ptimeline_offset_and_scale->x : 0.f;
    const float s_timeline_time_scale = s_ptimeline_offset_and_scale ? s_ptimeline_offset_and_scale->y : 1.f;
    const float columnWidthScaled = columnWidth*s_timeline_time_scale;
    const float columnWidthOffsetScaled = columnWidthScaled * s_timeline_time_offset;
    const float horizontal_interval = columnWidth / num_vertical_grid_lines;

    ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Button]);
    ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Border]);
    ImU32 text_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);
    ImU32 moving_line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SeparatorActive]);
    const float rounding = GImGui->Style.ScrollbarRounding;
    const float startY = ImGui::GetWindowHeight() + win->Pos.y;

    // Draw black vertical lines (inside scrolling area)
    for (int i = 1; i <= num_vertical_grid_lines; ++i)
    {
        ImVec2 a = GetWindowContentRegionMin() + win->Pos;
        a.x += s_timeline_time_scale * i * horizontal_interval + columnOffset - columnWidthOffsetScaled;
        win->DrawList->AddLine(a, ImVec2(a.x,startY), line_color);
    }

    // Draw moving vertical line
    if (current_time>0.f && current_time<s_max_timeline_value)
    {
        ImVec2 a = GetWindowContentRegionMin() + win->Pos;
        a.x += columnWidthScaled * (current_time / s_max_timeline_value) + columnOffset - columnWidthOffsetScaled;
        win->DrawList->AddLine(a, ImVec2(a.x, startY), moving_line_color, 3);
    }

    ImGui::Columns(1);
    ImGui::PopStyleColor();

    EndChild();
    const bool isChildWindowHovered = s_ptimeline_offset_and_scale ? ImGui::IsItemHovered() : false;

    // Draw bottom axis ribbon (outside scrolling region)
    win = GetCurrentWindow();
    float startx = ImGui::GetCursorScreenPos().x + columnOffset;
    float endy = ImGui::GetCursorScreenPos().y + row_height;//GetWindowContentRegionMax().y + win->Pos.y;
    ImVec2 start(startx, ImGui::GetCursorScreenPos().y);
    ImVec2 end(startx+columnWidth,endy);//start.y+row_height);
    float maxx = start.x + columnWidthScaled - columnWidthOffsetScaled;
    if (maxx < end.x) 
        end.x = maxx;
    if (current_time <= 0)
        win->DrawList->AddRectFilled(start, end, color, rounding);
    else if (current_time > s_max_timeline_value) 
        win->DrawList->AddRectFilled(start, end, timeline_running_color, rounding);
    else 
    {
        ImVec2 median(start.x + columnWidthScaled * (current_time / s_max_timeline_value) - columnWidthOffsetScaled,end.y);
        if (median.x < startx) 
            median.x = startx;
        else 
        {
            if (median.x > startx + columnWidth)
                median.x = startx + columnWidth;
            win->DrawList->AddRectFilled(start, median, timeline_running_color, rounding, ImDrawFlags_NoRoundCornerTR | ImDrawFlags_NoRoundCornerBL);
        }
        median.y=start.y;
        if (median.x < startx + columnWidth)
        {
            win->DrawList->AddRectFilled(median, end, color, rounding, ImDrawFlags_NoRoundCornerTL | ImDrawFlags_NoRoundCornerBR);
            if (median.x > startx)
                win->DrawList->AddLine(median, ImVec2(median.x,end.y), moving_line_color, 3);
        }
    }

    char tmp[256]="";
    for (int i = 0; i < num_vertical_grid_lines; ++i)
    {
        ImVec2 a = start;
        a.x = start.x + s_timeline_time_scale * i * horizontal_interval - columnWidthOffsetScaled;
        if (a.x < startx || a.x >= startx + columnWidth) 
            continue;

        ImFormatString(tmp, sizeof(tmp), "%.2f", i * s_max_timeline_value / num_vertical_grid_lines);
        win->DrawList->AddText(a, text_color, tmp);

    }
    ImGui::SetCursorPosY(ImGui::GetCursorPosY() + row_height);

    // zoom and pan
    if (s_ptimeline_offset_and_scale) 
    {
        const ImGuiIO& io = ImGui::GetIO();
        if (isChildWindowHovered && io.KeyCtrl) 
        {
            if (ImGui::IsMouseDragging(1)) 
            {
                // pan
                s_ptimeline_offset_and_scale->x -= io.MouseDelta.x / columnWidthScaled;
                if (s_ptimeline_offset_and_scale->x > 1.f)
                    s_ptimeline_offset_and_scale->x = 1.f;
                else if (s_ptimeline_offset_and_scale->x < 0.f) 
                    s_ptimeline_offset_and_scale->x = 0.f;
            }
            else if (io.MouseReleased[2])
            {
                // reset
                s_ptimeline_offset_and_scale->x = 0.f;
                s_ptimeline_offset_and_scale->y = 1.f;
            }
            if (io.MouseWheel != 0)
            {
                // zoom
                s_ptimeline_offset_and_scale->y *= (io.MouseWheel > 0) ? 1.05f : 0.95f;
                if (s_ptimeline_offset_and_scale->y < 0.25f) 
                    s_ptimeline_offset_and_scale->y = 0.25f;
                else if (s_ptimeline_offset_and_scale->y > 4.f) 
                    s_ptimeline_offset_and_scale->y = 4.f;
            }
        }
    }
}
} // namespace ImGui
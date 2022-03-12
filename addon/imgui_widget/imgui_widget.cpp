#include "imgui_widget.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"
#include <iostream>
#include <cmath>
#include <sstream>
#include <iomanip>
#include <imgui.h>
#include <stdio.h>

static const ImS32          IM_S32_MIN = INT_MIN;    // (-2147483647 - 1), (0x80000000);
static const ImS32          IM_S32_MAX = INT_MAX;    // (2147483647), (0x7FFFFFFF)
static const ImU32          IM_U32_MIN = 0;
static const ImU32          IM_U32_MAX = UINT_MAX;   // (0xFFFFFFFF)
#ifdef LLONG_MIN
static const ImS64          IM_S64_MIN = LLONG_MIN;  // (-9223372036854775807ll - 1ll);
static const ImS64          IM_S64_MAX = LLONG_MAX;  // (9223372036854775807ll);
#else
static const ImS64          IM_S64_MIN = -9223372036854775807LL - 1;
static const ImS64          IM_S64_MAX = 9223372036854775807LL;
#endif
static const ImU64          IM_U64_MIN = 0;
#ifdef ULLONG_MAX
static const ImU64          IM_U64_MAX = ULLONG_MAX; // (0xFFFFFFFFFFFFFFFFull);
#else
static const ImU64          IM_U64_MAX = (2ULL * 9223372036854775807LL + 1);
#endif

template<typename TYPE>
static const char* ImAtoi(const char* src, TYPE* output)
{
    int negative = 0;
    if (*src == '-') { negative = 1; src++; }
    if (*src == '+') { src++; }
    TYPE v = 0;
    while (*src >= '0' && *src <= '9')
        v = (v * 10) + (*src++ - '0');
    *output = negative ? -v : v;
    return src;
}

template<typename TYPE, typename SIGNEDTYPE>
TYPE ImGui::RoundScalarWithFormatKnobT(const char* format, ImGuiDataType data_type, TYPE v)
{
    const char* fmt_start = ImParseFormatFindStart(format);
    if (fmt_start[0] != '%' || fmt_start[1] == '%') // Don't apply if the value is not visible in the format string
        return v;
    char v_str[64];
    ImFormatString(v_str, IM_ARRAYSIZE(v_str), fmt_start, v);
    const char* p = v_str;
    while (*p == ' ')
        p++;
    if (data_type == ImGuiDataType_Float || data_type == ImGuiDataType_Double)
        v = (TYPE)ImAtof(p);
    else
        ImAtoi(p, (SIGNEDTYPE*)&v);
    return v;
}

template<typename TYPE, typename FLOATTYPE>
float ImGui::SliderCalcRatioFromValueT(ImGuiDataType data_type, TYPE v, TYPE v_min, TYPE v_max, float power, float linear_zero_pos)
{
    if (v_min == v_max)
        return 0.0f;

    const bool is_power = (power != 1.0f) && (data_type == ImGuiDataType_Float || data_type == ImGuiDataType_Double);
    const TYPE v_clamped = (v_min < v_max) ? ImClamp(v, v_min, v_max) : ImClamp(v, v_max, v_min);
    if (is_power)
    {
        if (v_clamped < 0.0f)
        {
            const float f = 1.0f - (float)((v_clamped - v_min) / (ImMin((TYPE)0, v_max) - v_min));
            return (1.0f - ImPow(f, 1.0f/power)) * linear_zero_pos;
        }
        else
        {
            const float f = (float)((v_clamped - ImMax((TYPE)0, v_min)) / (v_max - ImMax((TYPE)0, v_min)));
            return linear_zero_pos + ImPow(f, 1.0f/power) * (1.0f - linear_zero_pos);
        }
    }

    // Linear slider
    return (float)((FLOATTYPE)(v_clamped - v_min) / (FLOATTYPE)(v_max - v_min));
}

// FIXME: Move some of the code into SliderBehavior(). Current responsability is larger than what the equivalent DragBehaviorT<> does, we also do some rendering, etc.
template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
bool ImGui::SliderBehaviorKnobT(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, TYPE* v, const TYPE v_min, const TYPE v_max, const char* format, float power, ImGuiSliderFlags flags, ImRect* out_grab_bb)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImGuiAxis axis = (flags & ImGuiSliderFlags_Vertical) ? ImGuiAxis_Y : ImGuiAxis_X;
    const bool is_decimal = (data_type == ImGuiDataType_Float) || (data_type == ImGuiDataType_Double);
    const bool is_power = (power != 1.0f) && is_decimal;

    const float grab_padding = 2.0f;
    const float slider_sz = (bb.Max[axis] - bb.Min[axis]) - grab_padding * 2.0f;
    float grab_sz = style.GrabMinSize;
    SIGNEDTYPE v_range = (v_min < v_max ? v_max - v_min : v_min - v_max);
    if (!is_decimal && v_range >= 0)                                             // v_range < 0 may happen on integer overflows
        grab_sz = ImMax((float)(slider_sz / (v_range + 1)), style.GrabMinSize);  // For integer sliders: if possible have the grab size represent 1 unit
    grab_sz = ImMin(grab_sz, slider_sz);
    const float slider_usable_sz = slider_sz - grab_sz;
    const float slider_usable_pos_min = bb.Min[axis] + grab_padding + grab_sz * 0.5f;
    const float slider_usable_pos_max = bb.Max[axis] - grab_padding - grab_sz * 0.5f;

    // For power curve sliders that cross over sign boundary we want the curve to be symmetric around 0.0f
    float linear_zero_pos;   // 0.0->1.0f
    if (is_power && v_min * v_max < 0.0f)
    {
        // Different sign
        const FLOATTYPE linear_dist_min_to_0 = ImPow(v_min >= 0 ? (FLOATTYPE)v_min : -(FLOATTYPE)v_min, (FLOATTYPE)1.0f / power);
        const FLOATTYPE linear_dist_max_to_0 = ImPow(v_max >= 0 ? (FLOATTYPE)v_max : -(FLOATTYPE)v_max, (FLOATTYPE)1.0f / power);
        linear_zero_pos = (float)(linear_dist_min_to_0 / (linear_dist_min_to_0 + linear_dist_max_to_0));
    }
    else
    {
        // Same sign
        linear_zero_pos = v_min < 0.0f ? 1.0f : 0.0f;
    }

    // Process interacting with the slider
    bool value_changed = false;
    if (g.ActiveId == id)
    {
        bool set_new_value = false;
        float clicked_t = 0.0f;
        if (g.ActiveIdSource == ImGuiInputSource_Mouse)
        {
            if (!g.IO.MouseDown[0])
            {
                ClearActiveID();
            }
            else
            {
                const float mouse_abs_pos = g.IO.MousePos[axis];
                clicked_t = (slider_usable_sz > 0.0f) ? ImClamp((mouse_abs_pos - slider_usable_pos_min) / slider_usable_sz, 0.0f, 1.0f) : 0.0f;
                if (axis == ImGuiAxis_Y)
                    clicked_t = 1.0f - clicked_t;
                set_new_value = true;
            }
        }
        else if (g.ActiveIdSource == ImGuiInputSource_Nav)
        {
            const ImVec2 delta2 = GetNavInputAmount2d(ImGuiNavDirSourceFlags_Keyboard | ImGuiNavDirSourceFlags_PadDPad, ImGuiInputReadMode_RepeatFast, 0.0f, 0.0f);
            float delta = (axis == ImGuiAxis_X) ? delta2.x : -delta2.y;
            if (g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated)
            {
                ClearActiveID();
            }
            else if (delta != 0.0f)
            {
                clicked_t = SliderCalcRatioFromValueT<TYPE,FLOATTYPE>(data_type, *v, v_min, v_max, power, linear_zero_pos);
                const int decimal_precision = is_decimal ? ImParseFormatPrecision(format, 3) : 0;
                if ((decimal_precision > 0) || is_power)
                {
                    delta /= 100.0f;    // Gamepad/keyboard tweak speeds in % of slider bounds
                    if (IsNavInputDown(ImGuiNavInput_TweakSlow))
                        delta /= 10.0f;
                }
                else
                {
                    if ((v_range >= -100.0f && v_range <= 100.0f) || IsNavInputDown(ImGuiNavInput_TweakSlow))
                        delta = ((delta < 0.0f) ? -1.0f : +1.0f) / (float)v_range; // Gamepad/keyboard tweak speeds in integer steps
                    else
                        delta /= 100.0f;
                }
                if (IsNavInputDown(ImGuiNavInput_TweakFast))
                    delta *= 10.0f;
                set_new_value = true;
                if ((clicked_t >= 1.0f && delta > 0.0f) || (clicked_t <= 0.0f && delta < 0.0f)) // This is to avoid applying the saturation when already past the limits
                    set_new_value = false;
                else
                    clicked_t = ImSaturate(clicked_t + delta);
            }
        }

        if (set_new_value)
        {
            TYPE v_new;
            if (is_power)
            {
                // Account for power curve scale on both sides of the zero
                if (clicked_t < linear_zero_pos)
                {
                    // Negative: rescale to the negative range before powering
                    float a = 1.0f - (clicked_t / linear_zero_pos);
                    a = ImPow(a, power);
                    v_new = ImLerp(ImMin(v_max, (TYPE)0), v_min, a);
                }
                else
                {
                    // Positive: rescale to the positive range before powering
                    float a;
                    if (ImFabs(linear_zero_pos - 1.0f) > 1.e-6f)
                        a = (clicked_t - linear_zero_pos) / (1.0f - linear_zero_pos);
                    else
                        a = clicked_t;
                    a = ImPow(a, power);
                    v_new = ImLerp(ImMax(v_min, (TYPE)0), v_max, a);
                }
            }
            else
            {
                // Linear slider
                if (is_decimal)
                {
                    v_new = ImLerp(v_min, v_max, clicked_t);
                }
                else
                {
                    // For integer values we want the clicking position to match the grab box so we round above
                    // This code is carefully tuned to work with large values (e.g. high ranges of U64) while preserving this property..
                    FLOATTYPE v_new_off_f = (v_max - v_min) * clicked_t;
                    TYPE v_new_off_floor = (TYPE)(v_new_off_f);
                    TYPE v_new_off_round = (TYPE)(v_new_off_f + (FLOATTYPE)0.5);
                    if (v_new_off_floor < v_new_off_round)
                        v_new = v_min + v_new_off_round;
                    else
                        v_new = v_min + v_new_off_floor;
                }
            }

            // Round to user desired precision based on format string
            v_new = RoundScalarWithFormatKnobT<TYPE,SIGNEDTYPE>(format, data_type, v_new);

            // Apply result
            if (*v != v_new)
            {
                *v = v_new;
                value_changed = true;
            }
        }
    }

    if (slider_sz < 1.0f)
    {
        *out_grab_bb = ImRect(bb.Min, bb.Min);
    }
    else
    {
        // Output grab position so it can be displayed by the caller
        float grab_t = SliderCalcRatioFromValueT<TYPE, FLOATTYPE>(data_type, *v, v_min, v_max, power, linear_zero_pos);
        if (axis == ImGuiAxis_Y)
            grab_t = 1.0f - grab_t;
        const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
        if (axis == ImGuiAxis_X)
            *out_grab_bb = ImRect(grab_pos - grab_sz * 0.5f, bb.Min.y + grab_padding, grab_pos + grab_sz * 0.5f, bb.Max.y - grab_padding);
        else
            *out_grab_bb = ImRect(bb.Min.x + grab_padding, grab_pos - grab_sz * 0.5f, bb.Max.x - grab_padding, grab_pos + grab_sz * 0.5f);
    }

    return value_changed;
}

// For 32-bits and larger types, slider bounds are limited to half the natural type range.
// So e.g. an integer Slider between INT_MAX-10 and INT_MAX will fail, but an integer Slider between INT_MAX/2-10 and INT_MAX/2 will be ok.
// It would be possible to lift that limitation with some work but it doesn't seem to be worth it for sliders.
bool ImGui::SliderBehavior(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, void* v, const void* v_min, const void* v_max, const char* format, float power, ImGuiSliderFlags flags, ImRect* out_grab_bb)
{
    switch (data_type)
    {
    case ImGuiDataType_S8:  { ImS32 v32 = (ImS32)*(ImS8*)v;  bool r = SliderBehaviorKnobT<ImS32, ImS32, float >(bb, id, ImGuiDataType_S32, &v32, *(const ImS8*)v_min,  *(const ImS8*)v_max,  format, power, flags, out_grab_bb); if (r) *(ImS8*)v  = (ImS8)v32;  return r; }
    case ImGuiDataType_U8:  { ImU32 v32 = (ImU32)*(ImU8*)v;  bool r = SliderBehaviorKnobT<ImU32, ImS32, float >(bb, id, ImGuiDataType_U32, &v32, *(const ImU8*)v_min,  *(const ImU8*)v_max,  format, power, flags, out_grab_bb); if (r) *(ImU8*)v  = (ImU8)v32;  return r; }
    case ImGuiDataType_S16: { ImS32 v32 = (ImS32)*(ImS16*)v; bool r = SliderBehaviorKnobT<ImS32, ImS32, float >(bb, id, ImGuiDataType_S32, &v32, *(const ImS16*)v_min, *(const ImS16*)v_max, format, power, flags, out_grab_bb); if (r) *(ImS16*)v = (ImS16)v32; return r; }
    case ImGuiDataType_U16: { ImU32 v32 = (ImU32)*(ImU16*)v; bool r = SliderBehaviorKnobT<ImU32, ImS32, float >(bb, id, ImGuiDataType_U32, &v32, *(const ImU16*)v_min, *(const ImU16*)v_max, format, power, flags, out_grab_bb); if (r) *(ImU16*)v = (ImU16)v32; return r; }
    case ImGuiDataType_S32:
        IM_ASSERT(*(const ImS32*)v_min >= IM_S32_MIN/2 && *(const ImS32*)v_max <= IM_S32_MAX/2);
        return SliderBehaviorKnobT<ImS32, ImS32, float >(bb, id, data_type, (ImS32*)v,  *(const ImS32*)v_min,  *(const ImS32*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_U32:
        IM_ASSERT(*(const ImU32*)v_max <= IM_U32_MAX/2);
        return SliderBehaviorKnobT<ImU32, ImS32, float >(bb, id, data_type, (ImU32*)v,  *(const ImU32*)v_min,  *(const ImU32*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_S64:
        IM_ASSERT(*(const ImS64*)v_min >= IM_S64_MIN/2 && *(const ImS64*)v_max <= IM_S64_MAX/2);
        return SliderBehaviorKnobT<ImS64, ImS64, double>(bb, id, data_type, (ImS64*)v,  *(const ImS64*)v_min,  *(const ImS64*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_U64:
        IM_ASSERT(*(const ImU64*)v_max <= IM_U64_MAX/2);
        return SliderBehaviorKnobT<ImU64, ImS64, double>(bb, id, data_type, (ImU64*)v,  *(const ImU64*)v_min,  *(const ImU64*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_Float:
        IM_ASSERT(*(const float*)v_min >= -FLT_MAX/2.0f && *(const float*)v_max <= FLT_MAX/2.0f);
        return SliderBehaviorKnobT<float, float, float >(bb, id, data_type, (float*)v,  *(const float*)v_min,  *(const float*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_Double:
        IM_ASSERT(*(const double*)v_min >= -DBL_MAX/2.0f && *(const double*)v_max <= DBL_MAX/2.0f);
        return SliderBehaviorKnobT<double,double,double>(bb, id, data_type, (double*)v, *(const double*)v_min, *(const double*)v_max, format, power, flags, out_grab_bb);
    case ImGuiDataType_COUNT: break;
    }
    IM_ASSERT(0);
    return false;
}

void ImGui::UvMeter(char const *label, ImVec2 const &size, int *value, int v_min, int v_max, int steps, int* stack, int* count)
{
    float fvalue = (float)*value;
    float *fstack = nullptr;
    float _f = 0.f;
    if (stack) { fstack = &_f; *fstack = (float)*stack; }
    UvMeter(label, size, &fvalue, (float)v_min, (float)v_max, steps, fstack, count);
    *value = (int)fvalue;
    if (stack) *stack = (int)*fstack;
}

void ImGui::UvMeter(char const *label, ImVec2 const &size, float *value, float v_min, float v_max, int steps, float* stack, int* count)
{
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton(label, size);
    float steps_size = (v_max - v_min) / (float)steps;
    if (stack && count)
    {
        if (*value > *stack) 
        {
            *stack = *value;
            *count = 0;
        }
        else
        {
            *(count) += 1;
            if (*count > 10)
            {
                *stack -= steps_size / 2;
                if (*stack < v_min) *stack = v_min;
            }
        }
    }

    if (size.y > size.x)
    {
        float stepHeight = size.y / (v_max - v_min + 1);
        auto y = pos.y + size.y;
        auto hue = 0.4f;
        auto sat = 0.6f;
        auto lum = 0.6f;
        for (float i = v_min; i <= v_max; i += steps_size)
        {
            hue = 0.4f - (i / (v_max - v_min)) / 2.0f;
            sat = (*value < i ? 0.0f : 0.6f);
            lum = (*value < i ? 0.0f : 0.6f);
            draw_list->AddRectFilled(ImVec2(pos.x, y), ImVec2(pos.x + size.x, y - (stepHeight * steps_size - 1)), static_cast<ImU32>(ImColor::HSV(hue, sat, lum)));
            y = pos.y + size.y - (i * stepHeight);
        }
        if (stack && count)
        {
            draw_list->AddLine(ImVec2(pos.x, pos.y + size.y - (*stack * stepHeight)), ImVec2(pos.x + size.x, pos.y + size.y - (*stack * stepHeight)), IM_COL32_WHITE, 2.f);
        }
    }
    else
    {
        float stepWidth = size.x / (v_max - v_min + 1);
        auto x = pos.x;
        auto hue = 0.4f;
        auto sat = 0.6f;
        auto lum = 0.6f;
        for (float i = v_min; i <= v_max; i += steps_size)
        {
            hue = 0.4f - (i / (v_max - v_min)) / 2.0f;
            sat = (*value < i ? 0.0f : 0.6f);
            lum = (*value < i ? 0.0f : 0.6f);
            draw_list->AddRectFilled(ImVec2(x, pos.y), ImVec2(x + (stepWidth * steps_size - 1), pos.y + size.y), static_cast<ImU32>(ImColor::HSV(hue, sat, lum)));
            x = pos.x + (i * stepWidth);
        }
        if (stack && count)
        {
            draw_list->AddLine(ImVec2(pos.x + (*stack * stepWidth), pos.y), ImVec2(pos.x + (*stack * stepWidth), pos.y + size.y), IM_COL32_WHITE, 2.f);
        }
    }
}

bool ImGui::Fader(const char *label, const ImVec2 &size, int *v, const int v_min, const int v_max, const char *format, float power)
{
    ImGuiDataType data_type = ImGuiDataType_S32;
    ImGuiWindow *window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext &g = *GImGui;
    const ImGuiStyle &style = g.Style;
    const ImGuiID id = window->GetID(label);

    const ImVec2 label_size = CalcTextSize(label, nullptr, true);
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + size);
    const ImRect bb(frame_bb.Min, frame_bb.Max + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));

    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(frame_bb, id))
        return false;

    IM_ASSERT(data_type >= 0 && data_type < ImGuiDataType_COUNT);
    if (format == nullptr)
        format = "%d";

    const bool hovered = ItemHoverable(frame_bb, id);
    if ((hovered && g.IO.MouseClicked[0]) || g.NavActivateId == id || g.NavActivateInputId == id)
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        //        g.ActiveIdAllowNavDirFlags = (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    // Draw frame
    const ImU32 frame_col = GetColorU32(g.ActiveId == id ? ImGuiCol_FrameBgActive : g.HoveredId == id ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg);
    RenderNavHighlight(frame_bb, id);
    RenderFrame(frame_bb.Min, frame_bb.Max, frame_col, true, g.Style.FrameRounding);

    // Slider behavior
    ImRect grab_bb;
    const bool value_changed = SliderBehavior(frame_bb, id, data_type, v, &v_min, &v_max, format, power, ImGuiSliderFlags_Vertical, &grab_bb);
    if (value_changed)
        MarkItemEdited(id);

    ImRect gutter;
    gutter.Min = grab_bb.Min;
    gutter.Max = ImVec2(frame_bb.Max.x - 2.0f, frame_bb.Max.y - 2.0f);
    auto w = ((gutter.Max.x - gutter.Min.x) - 4.0f) / 2.0f;
    gutter.Min.x += w;
    gutter.Max.x -= w;
    window->DrawList->AddRectFilled(gutter.Min, gutter.Max, GetColorU32(ImGuiCol_ButtonActive), style.GrabRounding);

    // Render grab
    window->DrawList->AddRectFilled(grab_bb.Min, grab_bb.Max, GetColorU32(ImGuiCol_Text), style.GrabRounding);

    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    // For the vertical slider we allow centered text to overlap the frame padding
    char value_buf[64];
    sprintf(value_buf, format, int(*v * power));
    const char *value_buf_end = value_buf + strlen(value_buf);
    RenderTextClipped(ImVec2(frame_bb.Min.x, frame_bb.Min.y + style.FramePadding.y), frame_bb.Max, value_buf, value_buf_end, nullptr, ImVec2(0.5f, 0.0f));
    if (label_size.x > 0.0f)
        RenderText(ImVec2(frame_bb.Max.x + style.ItemInnerSpacing.x, frame_bb.Min.y + style.FramePadding.y), label);

    return value_changed;
}

///////////////////////////////////////////////////////////////////////////////
// New Knob controllor 
static void draw_circle(ImVec2 center, float _size, bool filled, int segments, float radius, ImGui::ColorSet& color)
{
    float circle_radius = _size * radius;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImU32 _color = ImGui::GetColorU32(ImGui::IsItemActive() ? color.active : ImGui::IsItemHovered(0) ? color.hovered : color.base);
    if (filled)
        draw_list->AddCircleFilled(center, circle_radius, _color, segments);
    else
        draw_list->AddCircle(center, circle_radius, _color, segments);
}

static void bezier_arc(ImVec2 center, ImVec2 start, ImVec2 end, ImVec2& c1, ImVec2 & c2)
{
    float ax = start[0] - center[0];
    float ay = start[1] - center[1];
    float bx = end[0] - center[0];
    float by = end[1] - center[1];
    float q1 = ax * ax + ay * ay;
    float q2 = q1 + ax * bx + ay * by;
    float k2 = (4.0 / 3.0) * (sqrt(2.0 * q1 * q2) - q2) / (ax * by - ay * bx);
    c1 = ImVec2(center[0] + ax - k2 * ay, center[1] + ay + k2 * ax);
    c2 = ImVec2(center[0] + bx + k2 * by, center[1] + by - k2 * bx);
}

static void draw_arc1(ImVec2 center, float radius, float start_angle, float end_angle, float thickness, ImU32 color, int num_segments)
{
    ImVec2 start = {center[0] + cos(start_angle) * radius, center[1] + sin(start_angle) * radius};
    ImVec2 end = {center[0] + cos(end_angle) * radius, center[1] + sin(end_angle) * radius};
    ImVec2 c1, c2;
    bezier_arc(center, start, end, c1, c2);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    draw_list->AddBezierCubic(start, c1, c2, end, color, thickness, num_segments);
}

static void draw_arc(ImVec2 center, float radius, float start_angle, float end_angle, float thickness, ImU32 color, int num_segments, int8_t bezier_count)
{
    float overlap = thickness * radius * 0.00001 * IM_PI;
    float delta = end_angle - start_angle;
    float bez_step = 1.0 / (float)bezier_count;
    float mid_angle = start_angle + overlap;
    for (int i = 0; i < bezier_count - 1; i++)
    {
        float mid_angle2 = delta * bez_step + mid_angle;
        draw_arc1(center, radius, mid_angle - overlap, mid_angle2 + overlap, thickness, color, num_segments);
        mid_angle = mid_angle2;
    }
    draw_arc1(center, radius, mid_angle - overlap, end_angle, thickness, color, num_segments);
}

static void draw_arc(ImVec2 center, float _radius, float _size, float radius, float start_angle, float end_angle, int segments, int8_t bezier_count, ImGui::ColorSet& color)
{
    float track_radius = _radius * radius;
    float track_size = _size * radius * 0.5 + 0.0001;
    ImU32 _color = ImGui::GetColorU32(ImGui::IsItemActive() ? color.active : ImGui::IsItemHovered(0) ? color.hovered : color.base);
    draw_arc(center, track_radius, start_angle, end_angle, track_size, _color, segments, bezier_count);
}

static void draw_dot(ImVec2 center, float _radius, float _size, float radius, float _angle, bool filled, int segments, ImGui::ColorSet& color)
{
    float dot_size = _size * radius;
    float dot_radius = _radius * radius;
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImU32 _color = ImGui::GetColorU32(ImGui::IsItemActive() ? color.active : ImGui::IsItemHovered(0) ? color.hovered : color.base);
    if (filled)
        draw_list->AddCircleFilled(
                ImVec2(center[0] + cos(_angle) * dot_radius, center[1] + sin(_angle) * dot_radius),
                dot_size, _color, segments);
    else
        draw_list->AddCircle(
                ImVec2(center[0] + cos(_angle) * dot_radius, center[1] + sin(_angle) * dot_radius),
                dot_size, _color, segments);
}

static void draw_tick(ImVec2 center, float radius, float start, float end, float width, float _angle, ImGui::ColorSet& color)
{
    float tick_start = start * radius;
    float tick_end = end * radius;
    float _angle_cos = cos(_angle);
    float _angle_sin = sin(_angle);
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImU32 _color = ImGui::GetColorU32(ImGui::IsItemActive() ? color.active : ImGui::IsItemHovered(0) ? color.hovered : color.base);
    draw_list->AddLine(
            ImVec2(center[0] + _angle_cos * tick_end, center[1] + _angle_sin * tick_end),
            ImVec2(center[0] + _angle_cos * tick_start, center[1] + _angle_sin * tick_start),
            _color,
            width * radius);
}

bool ImGui::Knob(char const *label, float *p_value, float v_min, float v_max, float v_step, float v_default, float size,
                ColorSet circle_color, ColorSet wiper_color, ColorSet track_color, ColorSet tick_color,
                ImGuiKnobType type, char const *format, int tick_steps)
{
    ImGuiStyle &style = ImGui::GetStyle();
    float line_height = ImGui::GetTextLineHeight();
    ImGuiIO &io = ImGui::GetIO();
    ImVec2 ItemSize = ImVec2(size, size + line_height * 2 + style.ItemInnerSpacing.y * 2 + 4);
    std::string ViewID = "###" + std::string(label) + "_KNOB_VIEW_CONTORL_";
    ImGui::BeginChild(ViewID.c_str(), ItemSize, false, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove);
    bool showLabel = label[0] != '#' && label[1] != '#' && label[0] != '\0';
    float radius = std::fmin(ItemSize.x, ItemSize.y) / 2.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
    auto textSize = CalcTextSize(label);
    bool is_no_limit = isnan(v_min) || isnan(v_max);
    if (showLabel)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddText(ImVec2(pos.x + ((ItemSize.x / 2) - (textSize.x / 2)), pos.y + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);
        center.y += line_height + 4;
    }

    ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2 + line_height));

    float step = isnan(v_step) ? (v_max - v_min) / 200.f : v_step;
    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemActive();
    if (is_active && io.MouseDelta.y != 0.0f)
    {
        *p_value -= io.MouseDelta.y * step;
        if (!is_no_limit)
        {
            if (*p_value < v_min)
                *p_value = v_min;
            if (*p_value > v_max)
                *p_value = v_max;
        }
        value_changed = true;
    }

    if (is_active && ImGui::IsMouseDoubleClicked(0))
    {
        if (*p_value != v_default)
        {
            *p_value = v_default;
            value_changed = true;
        }
    }

    float angle_min = IM_PI * 0.75;
    float angle_max = IM_PI * 2.25;
    float t = (*p_value - v_min) / (v_max - v_min);
    if (is_no_limit)
    {
        angle_min = -IM_PI * 0.5;
        angle_max = IM_PI * 1.5;
        t = *p_value;
    }
    float angle = angle_min + (angle_max - angle_min) * t;

    switch (type)
    {
        case IMKNOB_TICK:
            draw_circle(center, 0.7, true, 32, radius, circle_color);
            draw_tick(center, radius, 0.4, 0.7, 0.1, angle, wiper_color);
        break;
        case IMKNOB_TICK_DOT:
            draw_circle(center, 0.85, true, 32, radius, circle_color);
            draw_dot(center, 0.6, 0.12, radius, angle, true, 12, wiper_color);
        break;
        case IMKNOB_TICK_WIPER:
            draw_circle(center, 0.6, true, 32, radius, circle_color);
            if (!is_no_limit)
            {
                draw_arc(center, 0.85, 0.25, radius, angle_min, angle_max, 16, 2, track_color);
                if (t > 0.01)
                    draw_arc(center, 0.85, 0.27, radius, angle_min, angle, 16, 2, wiper_color);
            }
            else
            {
                draw_circle(center, 0.75, false, 32, radius, circle_color);
            }
            draw_tick(center, radius, 0.4, 0.6, 0.1, angle, wiper_color);
        break;
        case IMKNOB_WIPER:
            draw_circle(center, 0.7, true, 32, radius, circle_color);
            if (!is_no_limit)
            {
                draw_arc(center, 0.8, 0.41, radius, angle_min, angle_max, 16, 2, track_color);
                if (t > 0.01)
                    draw_arc(center, 0.8, 0.43, radius, angle_min, angle, 16, 2, wiper_color);
            }
            else
            {
                draw_circle(center, 0.9, false, 32, radius, circle_color);
                draw_tick(center, radius, 0.75, 0.9, 0.1, angle, wiper_color);
            }
        break;
        case IMKNOB_WIPER_TICK:
            draw_circle(center, 0.6, true, 32, radius, circle_color);
            if (!is_no_limit)
            {
                draw_arc(center, 0.85, 0.41, radius, angle_min, angle_max, 16, 2, track_color);
            }
            else
            {
                draw_circle(center, 0.75, false, 32, radius, circle_color);
                draw_circle(center, 0.9, false, 32, radius, circle_color);
            }
            draw_tick(center, radius, 0.75, 0.9, 0.1, angle, wiper_color);
        break;
        case IMKNOB_WIPER_DOT:
            draw_circle(center, 0.6, true, 32, radius, circle_color);
            if (!is_no_limit)
            {
                draw_arc(center, 0.85, 0.41, radius, angle_min, angle_max, 16, 2, track_color);
            }
            else
            {
                draw_circle(center, 0.80, false, 32, radius, circle_color);
                draw_circle(center, 0.95, false, 32, radius, circle_color);
            }
            draw_dot(center, 0.85, 0.1, radius, angle, true, 12, wiper_color);
        break;
        case IMKNOB_WIPER_ONLY:
            if (!is_no_limit)
            {
                draw_arc(center, 0.8, 0.41, radius, angle_min, angle_max, 32, 2, track_color);
                if (t > 0.01)
                    draw_arc(center, 0.8, 0.43, radius, angle_min, angle, 16, 2, wiper_color);
            }
            else
            {
                draw_circle(center, 0.75, false, 32, radius, circle_color);
                draw_circle(center, 0.90, false, 32, radius, circle_color);
                draw_tick(center, radius, 0.75, 0.9, 0.1, angle, wiper_color);
            }
        break;
        case IMKNOB_STEPPED_TICK:
            for (int i = 0; i < tick_steps; i++)
            {
                float a = (float)i / (float)(tick_steps - 1);
                float angle = angle_min + (angle_max - angle_min) * a;
                draw_tick(center, radius, 0.7, 0.9, 0.04, angle, tick_color);
            }
            draw_circle(center, 0.6, true, 32, radius, circle_color);
            draw_tick(center, radius, 0.4, 0.7, 0.1, angle, wiper_color);
        break;
        case IMKNOB_STEPPED_DOT:
            for (int i = 0; i < tick_steps; i++)
            {
                float a = (float)i / (float)(tick_steps - 1);
                float angle = angle_min + (angle_max - angle_min) * a;
                draw_tick(center, radius, 0.7, 0.9, 0.04, angle, tick_color);
            }
            draw_circle(center, 0.6, true, 32, radius, circle_color);
            draw_dot(center, 0.4, 0.12, radius, angle, true, 12, wiper_color);
        break;
        case IMKNOB_SPACE:
            draw_circle(center, 0.3 - t * 0.1, true, 16, radius, circle_color);
            if (t > 0.01 && !is_no_limit)
            {
                draw_arc(center, 0.4, 0.15, radius, angle_min - 1.0, angle - 1.0, 16, 2, wiper_color);
                draw_arc(center, 0.6, 0.15, radius, angle_min + 1.0, angle + 1.0, 16, 2, wiper_color);
                draw_arc(center, 0.8, 0.15, radius, angle_min + 3.0, angle + 3.0, 16, 2, wiper_color);
            }
        break;
        default:
        break;
    }

    ImGui::PushItemWidth(size);
    std::string DragID = "###" + std::string(label) + "_KNOB_DRAG_CONTORL_";
    if (is_no_limit)
    {
        ImGui::DragFloat(DragID.c_str(), p_value, step, FLT_MIN, FLT_MAX, format);
    }
    else
    {
        ImGui::DragFloat(DragID.c_str(), p_value, step, v_min, v_max, format);
    }
    ImGui::PopItemWidth();
    ImGui::EndChild();
    return value_changed;
}

void ImGui::RoundProgressBar(float radius, float *p_value, float v_min, float v_max, ColorSet bar_color, ColorSet progress_color, ColorSet text_color)
{
    char label[20] = {0};
    ImVec4 base_color = ImVec4(0.f, 0.f, 0.f, 1.f);
    ColorSet back_color = {base_color, base_color, base_color};
    float percentage = (*p_value - v_min) / (v_max - v_min) * 100.f;
    sprintf(label, "%02.1f%%", percentage);
    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImVec2 center = ImVec2(pos.x + radius, pos.y + radius);
    ImGui::InvisibleButton("##round_progress_bar", ImVec2(radius * 2, radius * 2));
    float angle_min = IM_PI * 0.75;
    float angle_max = IM_PI * 2.25;
    float t = percentage / 100.0;
    angle_min = -IM_PI * 0.5;
    angle_max = IM_PI * 1.5;
    t = percentage / 100.f;
    float angle = angle_min + (angle_max - angle_min) * t;
    draw_circle(center, 0.95, true, 32, radius, bar_color);
    draw_circle(center, 0.80, true, 32, radius, back_color);
    for (float s = angle_min; s < angle; s += IM_PI / 72)
        draw_dot(center, 0.87, 0.1, radius, s, true, 12, progress_color);
    draw_dot(center, 0.87, 0.1, radius, angle, true, 12, progress_color);

    
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    const char* text_end = label + strlen(label);
    auto draw_size = ImGui::CalcTextSize(label);
    float font_scale = radius / draw_size.x;
    draw_size *= font_scale;
    ImGui::PushStyleVar(ImGuiStyleVar_TexGlyphOutlineWidth, 1.f);
    ImGui::PushStyleVar(ImGuiStyleVar_TexGlyphShadowOffset, ImVec2(3.0f, 3.0f));
    ImGui::PushStyleColor(ImGuiCol_TexGlyphShadow, bar_color.base);
    draw_list->AddText(ImGui::GetFont(), ImGui::GetFontSize() * font_scale, center - draw_size * ImVec2(0.5, 0.6), ImGui::GetColorU32(text_color.base), label, text_end);
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
}

// Splitter
bool ImGui::Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size)
{
	using namespace ImGui;
	ImGuiContext& g = *GImGui;
	ImGuiWindow* window = g.CurrentWindow;
	ImGuiID id = window->GetID("##Splitter");
	ImRect bb;
	bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
	bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
	return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 1.0, 0.01);
}

// ToggleButton
void ImGui::ToggleButton(const char* str_id, bool* v)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;

    float t = *v ? 1.0f : 0.0f;

    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id))
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg;
    if (ImGui::IsItemHovered())
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
    else
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}

bool ImGui::ToggleButton(const char *str_id, bool *v, const ImVec2 &size)
{
    bool valueChange = false;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImGui::InvisibleButton(str_id, size);
    if (ImGui::IsItemClicked())
    {
        *v = !*v;
        valueChange = true;
    }

    ImU32 col_tint = ImGui::GetColorU32((*v ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_Border)));
    ImU32 col_bg = ImGui::GetColorU32(ImGui::GetColorU32(ImGuiCol_WindowBg));
    if (ImGui::IsItemHovered())
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    }
    if (ImGui::IsItemActive() || *v)
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_Button);
    }

    draw_list->AddRectFilled(pos, pos + size, ImGui::GetColorU32(col_bg));

    auto textSize = ImGui::CalcTextSize(str_id);
    draw_list->AddText(ImVec2(pos.x + (size.x - textSize.x) / 2, pos.y), col_tint, str_id);

    return valueChange;
}

bool ImGui::BulletToggleButton(const char* label, bool* v, ImVec2 &pos, ImVec2 &size)
{
    bool valueChange = false;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 old_pos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(pos);
    ImGui::InvisibleButton(label, size);
    if (ImGui::IsItemClicked())
    {
        *v = !*v;
        valueChange = true;
    }
    pos += size / 2;
    if (*v)
    {
        draw_list->AddCircleFilled(pos, draw_list->_Data->FontSize * 0.20f, IM_COL32(255, 0, 0, 255), 8);
    }
    else
    {
        draw_list->AddCircleFilled(pos, draw_list->_Data->FontSize * 0.20f, IM_COL32(128, 128, 128, 255), 8);
    }
    ImGui::SetCursorScreenPos(old_pos);
    return valueChange;
}

// CheckButton
bool ImGui::CheckButton(const char* label, bool* pvalue, bool useSmallButton, float checkedStateAlphaMult) {
    bool rv = false;
    const bool tmp = pvalue ? *pvalue : false;
    if (tmp)
    {
        ImVec4 CheckButtonColor = ImVec4(1.0, 1.0, 1.0, checkedStateAlphaMult);
        ImVec4 CheckButtonHoveredColor = ImVec4(1.0, 1.0, 1.0, checkedStateAlphaMult);
        ImVec4 CheckButtonActiveColor = ImVec4(1.0, 1.0, 1.0, checkedStateAlphaMult);
        ImGui::PushStyleColor(ImGuiCol_Button,CheckButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,CheckButtonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,CheckButtonActiveColor);
    }
    if (useSmallButton) {if (ImGui::SmallButton(label)) {if (pvalue) *pvalue=!(*pvalue);rv=true;}}
    else if (ImGui::Button(label)) {if (pvalue) *pvalue=!(*pvalue);rv=true;}
    if (tmp) ImGui::PopStyleColor(3);
    return rv;
}

// ColoredButtonV1: code posted by @ocornut here: https://github.com/ocornut/imgui/issues/4722
// [Button rounding depends on the FrameRounding Style property (but can be overridden with the last argument)]
bool ImGui::ColoredButton(const char* label, const ImVec2& size_arg, ImU32 text_color, ImU32 bg_color_1, ImU32 bg_color_2,float frame_rounding_override)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = ImGui::CalcTextSize(label, NULL, true);

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = ImGui::CalcItemSize(size_arg, label_size.x + style.FramePadding.x * 2.0f, label_size.y + style.FramePadding.y * 2.0f);

    const ImRect bb(pos, pos + size);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;

    ImGuiButtonFlags flags = ImGuiButtonFlags_None;
    if (g.LastItemData.InFlags & ImGuiItemFlags_ButtonRepeat)
        flags |= ImGuiButtonFlags_Repeat;

    bool hovered, held;
    bool pressed = ImGui::ButtonBehavior(bb, id, &hovered, &held, flags);

    // Render
    const bool is_gradient = bg_color_1 != bg_color_2;
    if (held || hovered)
    {
        // Modify colors (ultimately this can be prebaked in the style)
        float h_increase = (held && hovered) ? 0.02f : 0.02f;
        float v_increase = (held && hovered) ? 0.20f : 0.07f;

        ImVec4 bg1f = ImGui::ColorConvertU32ToFloat4(bg_color_1);
        ImGui::ColorConvertRGBtoHSV(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
        bg1f.x = ImMin(bg1f.x + h_increase, 1.0f);
        bg1f.z = ImMin(bg1f.z + v_increase, 1.0f);
        ImGui::ColorConvertHSVtoRGB(bg1f.x, bg1f.y, bg1f.z, bg1f.x, bg1f.y, bg1f.z);
        bg_color_1 = GetColorU32(bg1f);
        if (is_gradient)
        {
            ImVec4 bg2f = ImGui::ColorConvertU32ToFloat4(bg_color_2);
            ImGui::ColorConvertRGBtoHSV(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
            bg2f.z = ImMin(bg2f.z + h_increase, 1.0f);
            bg2f.z = ImMin(bg2f.z + v_increase, 1.0f);
            ImGui::ColorConvertHSVtoRGB(bg2f.x, bg2f.y, bg2f.z, bg2f.x, bg2f.y, bg2f.z);
            bg_color_2 = ImGui::GetColorU32(bg2f);
        }
        else
        {
            bg_color_2 = bg_color_1;
        }
    }
    ImGui::RenderNavHighlight(bb, id);

#if 0
    // V1 : faster but prevents rounding
    window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max, bg_color_1, bg_color_1, bg_color_2, bg_color_2);
    if (g.Style.FrameBorderSize > 0.0f)
        window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_Border), 0.0f, 0, g.Style.FrameBorderSize);
#endif

    // V2
    const float frameRounding = frame_rounding_override>=0.f ? frame_rounding_override : g.Style.FrameRounding;
    int vert_start_idx = window->DrawList->VtxBuffer.Size;
    window->DrawList->AddRectFilled(bb.Min, bb.Max, bg_color_1, frameRounding);
    int vert_end_idx = window->DrawList->VtxBuffer.Size;
    if (is_gradient)
        ImGui::ShadeVertsLinearColorGradientKeepAlpha(window->DrawList, vert_start_idx, vert_end_idx, bb.Min, bb.GetBL(), bg_color_1, bg_color_2);
    if (g.Style.FrameBorderSize > 0.0f)
        window->DrawList->AddRect(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_Border), frameRounding, 0, g.Style.FrameBorderSize);

    if (g.LogEnabled)
        ImGui::LogSetNextTextDecoration("[", "]");
    ImGui::PushStyleColor(ImGuiCol_Text, text_color);
    ImGui::RenderTextClipped(bb.Min + style.FramePadding, bb.Max - style.FramePadding, label, NULL, &label_size, style.ButtonTextAlign, &bb);
    ImGui::PopStyleColor();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed;
}

// ProgressBar
float ImGui::ProgressBar(const char *optionalPrefixText, float value, const float minValue, const float maxValue, 
                        const char *format, const ImVec2 &sizeOfBarWithoutTextInPixels, 
                        const ImVec4 &colorLeft, const ImVec4 &colorRight, const ImVec4 &colorBorder)
{
    if (value<minValue) value=minValue;
    else if (value>maxValue) value = maxValue;
    const float valueFraction = (maxValue==minValue) ? 1.0f : ((value-minValue)/(maxValue-minValue));
    const bool needsPercConversion = strstr(format,"%%")!=NULL;

    ImVec2 size = sizeOfBarWithoutTextInPixels;
    if (size.x<=0) size.x = ImGui::GetWindowWidth()*0.25f;
    if (size.y<=0) size.y = ImGui::GetTextLineHeightWithSpacing(); // or without

    const ImFontAtlas* fontAtlas = ImGui::GetIO().Fonts;

    if (optionalPrefixText && strlen(optionalPrefixText)>0)
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s",optionalPrefixText);
        ImGui::SameLine();
    }

    if (valueFraction>0)
    {
        ImGui::Image(fontAtlas->TexID,ImVec2(size.x*valueFraction,size.y), fontAtlas->TexUvWhitePixel,fontAtlas->TexUvWhitePixel,colorLeft,colorBorder);
    }
    if (valueFraction<1)
    {
        if (valueFraction>0) ImGui::SameLine(0,0);
        ImGui::Image(fontAtlas->TexID,ImVec2(size.x*(1.f-valueFraction),size.y), fontAtlas->TexUvWhitePixel,fontAtlas->TexUvWhitePixel,colorRight,colorBorder);
    }
    ImGui::SameLine();

    ImGui::Text(format,needsPercConversion ? (valueFraction*100.f+0.0001f) : value);
    return valueFraction;
}


// Color bar and ring
void ImGui::DrawHueBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float alpha, float gamma, float offset)
{
	auto HueFunc = [alpha, offset](float const tt) -> ImU32
	{
		float t;
		if (tt - offset < 0.0f)
			t = ImFmod(1.0f + (tt - offset), 1.0f);
		else
			t = ImFmod(tt - offset, 1.0f);
		float r, g, b;
		ImGui::ColorConvertHSVtoRGB(t, 1.0f, 1.0f, r, g, b);
		int const ur = static_cast<int>(255.0f * r);
		int const ug = static_cast<int>(255.0f * g);
		int const ub = static_cast<int>(255.0f * b);
		int const ua = static_cast<int>(255.0f * alpha);
		return IM_COL32(ur, ug, ub, ua);
	};
	ImGui::DrawColorBandEx< true >(pDrawList, vpos, size, HueFunc, division, gamma);
}

void ImGui::DrawHueBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float colorStartRGB[3], float alpha, float gamma)
{
	float h, s, v;
	ImGui::ColorConvertRGBtoHSV(colorStartRGB[0], colorStartRGB[1], colorStartRGB[2], h, s, v);
	ImGui::DrawHueBand(pDrawList, vpos, size, division, alpha, gamma, h);
}

void ImGui::DrawLumianceBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma)
{
	float h, s, v;
	ImGui::ColorConvertRGBtoHSV(color.x, color.y, color.z, h, s, v);
	auto LumianceFunc = [h, s, v](float const t) -> ImU32
	{
		float r, g, b;
		ImGui::ColorConvertHSVtoRGB(h, s, ImLerp(0.0f, v, t), r, g, b);
		int const ur = static_cast<int>(255.0f * r);
		int const ug = static_cast<int>(255.0f * g);
		int const ub = static_cast<int>(255.0f * b);
		return IM_COL32(ur, ug, ub, 255);
	};
	ImGui::DrawColorBandEx< true >(pDrawList, vpos, size, LumianceFunc, division, gamma);
}

void ImGui::DrawSaturationBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma)
{
	float h, s, v;
	ImGui::ColorConvertRGBtoHSV(color.x, color.y, color.z, h, s, v);
	auto SaturationFunc = [h, s, v](float const t) -> ImU32
	{
		float r, g, b;
		ImGui::ColorConvertHSVtoRGB(h, ImLerp(0.0f, 1.0f, t) * s, ImLerp(0.5f, 1.0f, t) * v, r, g, b);
		int const ur = static_cast<int>(255.0f * r);
		int const ug = static_cast<int>(255.0f * g);
		int const ub = static_cast<int>(255.0f * b);
		return IM_COL32(ur, ug, ub, 255);
	};
	ImGui::DrawColorBandEx< true >(pDrawList, vpos, size, SaturationFunc, division, gamma);
}

void ImGui::DrawContrastBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, ImVec4 const& color)
{
    ImGui::DrawColorBandEx< true >(pDrawList, vpos, size,
	[size, color](float t)
	{
		int seg = t * size.x;
        float v = ((seg & 1) == 0) ? 1.0 : (1 - t);
		return IM_COL32(v * color.x * 255.f, v * color.y * 255.f, v * color.z * 255.f, color.w * 255.f);
	}, size.x, 1.f);
}

bool ImGui::ColorRing(const char* label, float thickness, int split)
{
	ImGuiID const iID = ImGui::GetID(label);
	ImGui::PushID(iID);
	ImVec2 curPos = ImGui::GetCursorScreenPos();
	float const width = ImGui::GetContentRegionAvail().x;
	float const height = width;
	ImGui::InvisibleButton("##Zone", ImVec2(width, height), 0);
	float radius = width * 0.5f;
	const float dAngle = 2.0f * IM_PI / ((float)split);
	float angle = 2.0f * IM_PI / 3.0f;
	ImVec2 offset = curPos + ImVec2(radius, radius);
	ImVec2 const uv = ImGui::GetFontTexUvWhitePixel();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
	pDrawList->PrimReserve(split * 6, split * 4);
	for (int i = 0; i < split; ++i)
	{
		float x0 = radius * ImCos(angle);
		float y0 = radius * ImSin(angle);
		float x1 = radius * ImCos(angle + dAngle);
		float y1 = radius * ImSin(angle + dAngle);
		float x2 = (radius - thickness) * ImCos(angle + dAngle);
		float y2 = (radius - thickness) * ImSin(angle + dAngle);
		float x3 = (radius - thickness) * ImCos(angle);
		float y3 = (radius - thickness) * ImSin(angle);
		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx));
		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx + 1));
		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx + 2));
		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx));
		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx + 2));
		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx + 3));
		float r0, g0, b0;
		float r1, g1, b1;
		ImGui::ColorConvertHSVtoRGB(((float)i) / ((float)(split - 1)), 1.0f, 1.0f, r0, g0, b0);
		ImGui::ColorConvertHSVtoRGB(((float)((i + 1)%split)) / ((float)(split - 1)), 1.0f, 1.0f, r1, g1, b1);
		pDrawList->PrimWriteVtx(offset + ImVec2(x0, y0), uv, IM_COL32(r0 * 255, g0 * 255, b0 * 255, 255));
		pDrawList->PrimWriteVtx(offset + ImVec2(x1, y1), uv, IM_COL32(r1 * 255, g1 * 255, b1 * 255, 255));
		pDrawList->PrimWriteVtx(offset + ImVec2(x2, y2), uv, IM_COL32(r1 * 255, g1 * 255, b1 * 255, 255));
		pDrawList->PrimWriteVtx(offset + ImVec2(x3, y3), uv, IM_COL32(r0 * 255, g0 * 255, b0 * 255, 255));
		angle += dAngle;
	}
	ImGui::PopID();
	return false;
}

static void HueSelectorEx(char const* label, ImVec2 const size, float* hueCenter, float* hueWidth, float* featherLeft, float* featherRight, float defaultVal, float ui_zoom, ImU32 triangleColor, int division, float alpha, float hideHueAlpha, float offset)
{
    ImGuiIO &io = ImGui::GetIO();
	ImGuiID const iID = ImGui::GetID(label);
	ImGui::PushID(iID);
	ImVec2 curPos = ImGui::GetCursorScreenPos();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    ImGui::InvisibleButton("##ZoneHueLineSlider", size);
	ImGui::DrawHueBand(pDrawList, curPos, size, division, alpha, 1.0f, offset);
	float center = ImClamp(ImFmod(*hueCenter + offset, 1.0f), 0.0f, 1.0f - 1e-4f);
	float width = ImClamp(*hueWidth, 0.0f, 0.5f - 1e-4f);
	float featherL = ImClamp(*featherLeft, 0.0f, 0.5f - 1e-4f);
	float featherR = ImClamp(*featherRight, 0.0f, 0.5f - 1e-4f);
	float xCenter = curPos.x + center * size.x;
	if (width == 0.0f)
	{
		pDrawList->AddLine(ImVec2(xCenter, curPos.y), ImVec2(xCenter, curPos.y + size.y), IM_COL32(0, 0, 0, 255));
	}
	else
	{
		ImGui::DrawColorDensityPlotEx< true >(pDrawList,
		    [hueAlpha = hideHueAlpha, center, width, left = featherL, right = featherR](float const xx, float const) -> ImU32
		    {
		    	float x = ImFmod(xx, 1.0f);
		    	float val;
		    	if (x < center - width && x > center - width - left)
		    	{
		    		val = ImClamp((center * (-1 + hueAlpha) + left + width + x - hueAlpha * (width + x)) / left, hueAlpha, 1.0f);
		    	}
		    	else if (x < center + width + right && x > center + width)
		    	{
		    		val = ImClamp((center - center * hueAlpha + right + width - hueAlpha * width + (-1 + hueAlpha) * x) / right, hueAlpha, 1.0f);
		    	}
		    	else if (x > center - width - left && x < center + width + right)
		    	{
		    		val = 1.0f;
		    	}
		    	else if (center + width + right > 1.0f)
		    	{
		    		val = ImClamp((center - center * hueAlpha + right + width - hueAlpha * width + (-1 + hueAlpha) * (x + 1.0f)) / right, hueAlpha, 1.0f);
		    	}
		    	else if (center - width - left < 0.0f)
		    	{
		    		val = ImClamp((center * (-1 + hueAlpha) + left + width + x - 1.0f - hueAlpha * (width + x - 1.0f)) / left, hueAlpha, 1.0f);
		    	}
		    	else
		    	{
		    		val = hueAlpha;
		    	}
		    	return IM_COL32(0, 0, 0, ImPow(1.0f - val, 1.0f / 2.2f) * 255);
		    }, 0.0f, 1.0f, 0.0f, 0.0f, curPos, size, division, 1);
	}
    if (ImGui::IsItemHovered())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            auto diff = io.MouseDelta.x * ui_zoom / size.x;
            *hueCenter += diff;
            *hueCenter = ImClamp(*hueCenter, 0.f, 1.f);
        }
        if (io.MouseWheel < -FLT_EPSILON)
        {
            *hueWidth *= 0.9;
        }
        if (io.MouseWheel > FLT_EPSILON)
        {
            *hueWidth *= 1.1;
            if (*hueWidth > 0.5)
                *hueWidth = 0.5;
        }
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            *hueCenter = defaultVal;
        }
    }

    const float arrowWidth = pDrawList->_Data->FontSize;
    float arrowOffset = curPos.x + *hueCenter * size.x;
    ImGui::Dummy(ImVec2(0, arrowWidth / 2));
    ImGui::RenderArrow(pDrawList, ImVec2(arrowOffset - arrowWidth / 2, curPos.y + size.y), IM_COL32(255,255,0,255), ImGuiDir_Up);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << *hueCenter;
    std::string value_str = oss.str();
    ImVec2 str_size = ImGui::CalcTextSize(value_str.c_str(), nullptr, true);
    pDrawList->AddText(ImVec2(curPos.x + size.x / 2 - str_size.x * 0.5f, curPos.y + size.y / 2 - arrowWidth / 2), IM_COL32(255,255,0,255), value_str.c_str());
	ImGui::PopID();
}

void ImGui::HueSelector(char const* label, ImVec2 const size, float* hueCenter, float* hueWidth, float* featherLeft, float* featherRight, float defaultVal, float ui_zoom, int division, float alpha, float hideHueAlpha, float offset)
{
	HueSelectorEx(label, size, hueCenter, hueWidth, featherLeft, featherRight, defaultVal, ui_zoom, IM_COL32(255, 128, 0, 255), division, alpha, hideHueAlpha, offset);
}

void ImGui::LumianceSelector(char const* label, ImVec2 const size, float* lumCenter, float defaultVal, float ui_zoom, int division, float gamma, bool rgb_color, ImVec4 const color)
{
    ImGuiIO &io = ImGui::GetIO();
	ImGuiID const iID = ImGui::GetID(label);
	ImGui::PushID(iID);
    ImVec2 curPos = ImGui::GetCursorScreenPos();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    ImGui::InvisibleButton("##ZoneLumianceSlider", size);
    if (!rgb_color)
    {
        ImGui::DrawLumianceBand(pDrawList, curPos, size, division, color, gamma);
    }
    else
    {
        ImVec2 bar_size = ImVec2(size.x, size.y / 4);
        ImVec2 r_pos = curPos;
        ImVec2 g_pos = ImVec2(curPos.x, curPos.y + size.y / 4);
        ImVec2 b_pos = ImVec2(curPos.x, curPos.y + size.y * 2 / 4);
        ImVec2 w_pos = ImVec2(curPos.x, curPos.y + size.y * 3 / 4);
        ImGui::DrawLumianceBand(pDrawList, r_pos, bar_size, division, ImVec4(1, 0, 0, 1), gamma);
        ImGui::DrawLumianceBand(pDrawList, g_pos, bar_size, division, ImVec4(0, 1, 0, 1), gamma);
        ImGui::DrawLumianceBand(pDrawList, b_pos, bar_size, division, ImVec4(0, 0, 1, 1), gamma);
        ImGui::DrawLumianceBand(pDrawList, w_pos, bar_size, division, ImVec4(1, 1, 1, 1), gamma);
    }
    if (ImGui::IsItemHovered())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            auto diff = io.MouseDelta.x * 2 * ui_zoom / size.x;
            *lumCenter += diff;
            *lumCenter = ImClamp(*lumCenter, -1.f, 1.f);
        }
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            *lumCenter = defaultVal;
        }
    }
    const float arrowWidth = pDrawList->_Data->FontSize;
    float arrowOffset = curPos.x + (*lumCenter / 2 + 0.5) * size.x;
    ImGui::Dummy(ImVec2(0, arrowWidth / 2));
    ImGui::RenderArrow(pDrawList, ImVec2(arrowOffset - arrowWidth / 2, curPos.y + size.y), IM_COL32(255,255,0,255), ImGuiDir_Up);
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << *lumCenter;
    std::string value_str = oss.str();
    ImVec2 str_size = ImGui::CalcTextSize(value_str.c_str(), nullptr, true);
    pDrawList->AddText(ImVec2(curPos.x + size.x / 2 - str_size.x * 0.5f, curPos.y + size.y / 2 - arrowWidth / 2), IM_COL32(255,255,0,255), value_str.c_str());
	ImGui::PopID();
}

void ImGui::SaturationSelector(char const* label, ImVec2 const size, float* satCenter, float defaultVal, float ui_zoom, int division, float gamma, bool rgb_color, ImVec4 const color)
{
    ImGuiIO &io = ImGui::GetIO();
	ImGuiID const iID = ImGui::GetID(label);
	ImGui::PushID(iID);
    ImVec2 curPos = ImGui::GetCursorScreenPos();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    ImGui::InvisibleButton("##ZoneSaturationSlider", size);
    if (!rgb_color)
    {
        ImGui::DrawSaturationBand(pDrawList, curPos, size, division, color, gamma);
    }
    else
    {
        ImVec2 bar_size = ImVec2(size.x, size.y / 4);
        ImVec2 r_pos = curPos;
        ImVec2 g_pos = ImVec2(curPos.x, curPos.y + size.y / 4);
        ImVec2 b_pos = ImVec2(curPos.x, curPos.y + size.y * 2 / 4);
        ImVec2 w_pos = ImVec2(curPos.x, curPos.y + size.y * 3 / 4);
        ImGui::DrawSaturationBand(pDrawList, r_pos, bar_size, division, ImVec4(1, 0, 0, 1), gamma);
        ImGui::DrawSaturationBand(pDrawList, g_pos, bar_size, division, ImVec4(0, 1, 0, 1), gamma);
        ImGui::DrawSaturationBand(pDrawList, b_pos, bar_size, division, ImVec4(0, 0, 1, 1), gamma);
        ImGui::DrawSaturationBand(pDrawList, w_pos, bar_size, division, ImVec4(1, 1, 1, 1), gamma);
    }
    if (ImGui::IsItemHovered())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            auto diff = io.MouseDelta.x * 2 * ui_zoom / size.x;
            *satCenter += diff;
            *satCenter = ImClamp(*satCenter, -1.f, 1.f);
        }
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            *satCenter = defaultVal;
        }
    }
    const float arrowWidth = pDrawList->_Data->FontSize;
    float arrowOffset = curPos.x + (*satCenter / 2 + 0.5) * size.x;
    ImGui::Dummy(ImVec2(0, arrowWidth / 2));
    ImGui::RenderArrow(pDrawList, ImVec2(arrowOffset - arrowWidth / 2, curPos.y + size.y), IM_COL32(255,255,0,255), ImGuiDir_Up);
	std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << *satCenter;
    std::string value_str = oss.str();
    ImVec2 str_size = ImGui::CalcTextSize(value_str.c_str(), nullptr, true);
    pDrawList->AddText(ImVec2(curPos.x + size.x / 2 - str_size.x * 0.5f, curPos.y + size.y / 2 - arrowWidth / 2), IM_COL32(255,255,0,255), value_str.c_str());
    ImGui::PopID();
}

void ImGui::ContrastSelector(char const* label, ImVec2 const size, float* conCenter, float defaultVal, float ui_zoom, bool rgb_color, ImVec4 const color)
{
    ImGuiIO &io = ImGui::GetIO();
	ImGuiID const iID = ImGui::GetID(label);
	ImGui::PushID(iID);
    ImVec2 curPos = ImGui::GetCursorScreenPos();
	ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    ImGui::InvisibleButton("##ZoneContrastSlider", size);
    if (!rgb_color)
    {
        ImGui::DrawContrastBand(pDrawList, curPos, size, color);
    }
    else
    {
        ImVec2 bar_size = ImVec2(size.x, size.y / 4);
        ImVec2 r_pos = curPos;
        ImVec2 g_pos = ImVec2(curPos.x, curPos.y + size.y / 4);
        ImVec2 b_pos = ImVec2(curPos.x, curPos.y + size.y * 2 / 4);
        ImVec2 w_pos = ImVec2(curPos.x, curPos.y + size.y * 3 / 4);
        ImGui::DrawContrastBand(pDrawList, r_pos, bar_size, ImVec4(1, 0, 0, 1));
        ImGui::DrawContrastBand(pDrawList, g_pos, bar_size, ImVec4(0, 1, 0, 1));
        ImGui::DrawContrastBand(pDrawList, b_pos, bar_size, ImVec4(0, 0, 1, 1));
        ImGui::DrawContrastBand(pDrawList, w_pos, bar_size, ImVec4(1, 1, 1, 1));
    }
    if (ImGui::IsItemHovered())
    {
        if (ImGui::IsMouseDragging(ImGuiMouseButton_Left))
        {
            auto diff = io.MouseDelta.x * 4 * ui_zoom / size.x;
            *conCenter += diff;
            *conCenter = ImClamp(*conCenter, 0.f, 4.f);
        }
        if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
        {
            *conCenter = defaultVal;
        }
    }
    const float arrowWidth = pDrawList->_Data->FontSize;
    float arrowOffset = curPos.x + (*conCenter / 4) * size.x;
    ImGui::Dummy(ImVec2(0, arrowWidth / 2));
    ImGui::RenderArrow(pDrawList, ImVec2(arrowOffset - arrowWidth / 2, curPos.y + size.y), IM_COL32(255,255,0,255), ImGuiDir_Up);
	std::ostringstream oss;
    oss << std::fixed << std::setprecision(2) << *conCenter;
    std::string value_str = oss.str();
    ImVec2 str_size = ImGui::CalcTextSize(value_str.c_str(), nullptr, true);
    pDrawList->AddText(ImVec2(curPos.x + size.x / 2 - str_size.x * 0.5f, curPos.y + size.y / 2 - arrowWidth / 2), IM_COL32(0,0,0,255), value_str.c_str());
    ImGui::PopID();
}

// imgInspect
inline void histogram(const int width, const int height, const unsigned char* const bits)
{
    unsigned int count[4][256] = {0};
    const unsigned char* ptrCols = bits;
    ImGui::InvisibleButton("histogram", ImVec2(256, 128));
    for (int l = 0; l < height * width; l++)
    {
        count[0][*ptrCols++]++;
        count[1][*ptrCols++]++;
        count[2][*ptrCols++]++;
        count[3][*ptrCols++]++;
    }
    unsigned int maxv = count[0][0];
    unsigned int* pCount = &count[0][0];
    for (int i = 0; i < 3 * 256; i++, pCount++)
    {
        maxv = (maxv > *pCount) ? maxv : *pCount;
    }
    ImDrawList* drawList = ImGui::GetWindowDrawList();
    const ImVec2 rmin = ImGui::GetItemRectMin();
    const ImVec2 rmax = ImGui::GetItemRectMax();
    const ImVec2 size = ImGui::GetItemRectSize();
    const float hFactor = size.y / float(maxv);
    for (int i = 0; i <= 10; i++)
    {
        float ax = rmin.x + (size.x / 10.f) * float(i);
        float ay = rmin.y + (size.y / 10.f) * float(i);
        drawList->AddLine(ImVec2(rmin.x, ay), ImVec2(rmax.x, ay), 0x80808080);
        drawList->AddLine(ImVec2(ax, rmin.y), ImVec2(ax, rmax.y), 0x80808080);
    }
    const float barWidth = (size.x / 256.f);
    for (int j = 0; j < 256; j++)
    {
        // pixel count << 2 + color index(on 2 bits)
        uint32_t cols[3] = {(count[0][j] << 2), (count[1][j] << 2) + 1, (count[2][j] << 2) + 2};
        if (cols[0] > cols[1])
            ImSwap(cols[0], cols[1]);
        if (cols[1] > cols[2])
            ImSwap(cols[1], cols[2]);
        if (cols[0] > cols[1])
            ImSwap(cols[0], cols[1]);
        float heights[3];
        uint32_t colors[3];
        uint32_t currentColor = 0xFFFFFFFF;
        for (int i = 0; i < 3; i++)
        {
            heights[i] = rmax.y - (cols[i] >> 2) * hFactor;
            colors[i] = currentColor;
            currentColor -= 0xFF << ((cols[i] & 3) * 8);
        }
        float currentHeight = rmax.y;
        const float left = rmin.x + barWidth * float(j);
        const float right = left + barWidth;
        for (int i = 0; i < 3; i++)
        {
            if (heights[i] >= currentHeight)
            {
                continue;
            }
            drawList->AddRectFilled(ImVec2(left, currentHeight), ImVec2(right, heights[i]), colors[i]);
            currentHeight = heights[i];
        }
    }
}
inline void drawNormal(ImDrawList* draw_list, const ImRect& rc, float x, float y)
{
    draw_list->AddCircle(rc.GetCenter(), rc.GetWidth() / 2.f, 0x20AAAAAA, 24, 1.f);
    draw_list->AddCircle(rc.GetCenter(), rc.GetWidth() / 4.f, 0x20AAAAAA, 24, 1.f);
    draw_list->AddLine(rc.GetCenter(), rc.GetCenter() + ImVec2(x, y) * rc.GetWidth() / 2.f, 0xFF0000FF, 2.f);
}
void ImGui::ImageInspect(const int width,
                        const int height,
                        const unsigned char* const bits,
                        ImVec2 mouseUVCoord,
                        ImVec2 displayedTextureSize,
                        bool histogram_full,
                        int zoom_size)
{
    ImGui::BeginTooltip();
    ImGui::BeginGroup();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    static const float zoomRectangleWidth = 80.f;
    // bitmap zoom
    ImGui::InvisibleButton("AnotherInvisibleMan", ImVec2(zoomRectangleWidth, zoomRectangleWidth));
    const ImRect pickRc(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    draw_list->AddRectFilled(pickRc.Min, pickRc.Max, 0xFF000000);
    static int zoomSize = zoom_size / 2;
    uint32_t zoomData[(zoomSize * 2 + 1) * (zoomSize * 2 + 1)];
    const float quadWidth = zoomRectangleWidth / float(zoomSize * 2 + 1);
    const ImVec2 quadSize(quadWidth, quadWidth);
    const int basex = ImClamp(int(mouseUVCoord.x * width), zoomSize, width - zoomSize);
    const int basey = ImClamp(int(mouseUVCoord.y * height), zoomSize, height - zoomSize);
    for (int y = -zoomSize; y <= zoomSize; y++)
    {
        for (int x = -zoomSize; x <= zoomSize; x++)
        {
            uint32_t texel = ((uint32_t*)bits)[(basey - y) * width + x + basex];
            ImVec2 pos = pickRc.Min + ImVec2(float(x + zoomSize), float(zoomSize - y)) * quadSize;
            draw_list->AddRectFilled(pos, pos + quadSize, texel);
        }
    }
    //ImGui::SameLine();
    // center quad
    const ImVec2 pos = pickRc.Min + ImVec2(float(zoomSize), float(zoomSize)) * quadSize;
    draw_list->AddRect(pos, pos + quadSize, 0xFF0000FF, 0.f, 15, 2.f);
    // normal direction
    ImGui::InvisibleButton("AndOneMore", ImVec2(zoomRectangleWidth, zoomRectangleWidth));
    ImRect normRc(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
    for (int y = -zoomSize; y <= zoomSize; y++)
    {
        for (int x = -zoomSize; x <= zoomSize; x++)
        {
            uint32_t texel = ((uint32_t*)bits)[(basey - y) * width + x + basex];
            const ImVec2 posQuad = normRc.Min + ImVec2(float(x + zoomSize), float(zoomSize - y)) * quadSize;
            //draw_list->AddRectFilled(pos, pos + quadSize, texel);
            const float nx = float(texel & 0xFF) / 128.f - 1.f;
            const float ny = float((texel & 0xFF00)>>8) / 128.f - 1.f;
            const ImRect rc(posQuad, posQuad + quadSize);
            drawNormal(draw_list, rc, nx, ny);
        }
    }
    ImGui::EndGroup();
    ImGui::SameLine();
    ImGui::BeginGroup();
    uint32_t texel = ((uint32_t*)bits)[basey * width + basex];
    ImVec4 color = ImColor(texel);
    ImVec4 colHSV;
    ImGui::ColorConvertRGBtoHSV(color.x, color.y, color.z, colHSV.x, colHSV.y, colHSV.z);
    ImGui::Text("U %1.3f V %1.3f", mouseUVCoord.x, mouseUVCoord.y);
    ImGui::Text("Coord %d %d", int(mouseUVCoord.x * width), int(mouseUVCoord.y * height));
    ImGui::Separator();
    ImGui::Text("R 0x%02x  G 0x%02x  B 0x%02x", int(color.x * 255.f), int(color.y * 255.f), int(color.z * 255.f));
    ImGui::Text("R %1.3f G %1.3f B %1.3f", color.x, color.y, color.z);
    ImGui::Separator();
    ImGui::Text(
        "H 0x%02x  S 0x%02x  V 0x%02x", int(colHSV.x * 255.f), int(colHSV.y * 255.f), int(colHSV.z * 255.f));
    ImGui::Text("H %1.3f S %1.3f V %1.3f", colHSV.x, colHSV.y, colHSV.z);
    ImGui::Separator();
    ImGui::Text("Alpha 0x%02x", int(color.w * 255.f));
    ImGui::Text("Alpha %1.3f", color.w);
    ImGui::Separator();
    ImGui::Text("Size %d, %d", int(displayedTextureSize.x), int(displayedTextureSize.y));
    ImGui::EndGroup();
    if (histogram_full)
    {
        histogram(width, height, bits);
    }
    else
    {
        for (int y = -zoomSize; y <= zoomSize; y++)
        {
            for (int x = -zoomSize; x <= zoomSize; x++)
            {
                uint32_t texel = ((uint32_t*)bits)[(basey - y) * width + x + basex];
                zoomData[(y + zoomSize) * zoomSize * 2 + x + zoomSize] = texel;
            }
        }
        histogram(zoomSize * 2, zoomSize * 2, (const unsigned char*)zoomData);
    }
    ImGui::EndTooltip();
}
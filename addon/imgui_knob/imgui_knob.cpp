#include "imgui_knob.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"
#include <iostream>
#include <cmath>
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

bool ImGui::Knob(char const *label, float *p_value, float v_min, float v_max, float v_default, float size,
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
    if (showLabel)
    {
        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        draw_list->AddText(ImVec2(pos.x + ((ItemSize.x / 2) - (textSize.x / 2)), pos.y + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);
        center.y += line_height + 4;
    }

    ImGui::InvisibleButton(label, ImVec2(radius * 2, radius * 2 + line_height));

    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
    bool is_hovered = ImGui::IsItemActive();
    if (is_active && io.MouseDelta.y != 0.0f)
    {
        float step = (v_max - v_min) / 200.0f;
        *p_value -= io.MouseDelta.y * step;
        if (*p_value < v_min)
            *p_value = v_min;
        if (*p_value > v_max)
            *p_value = v_max;
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
            draw_arc(center, 0.85, 0.25, radius, angle_min, angle_max, 16, 2, track_color);
            if (t > 0.01)
                draw_arc(center, 0.85, 0.27, radius, angle_min, angle, 16, 2, wiper_color);
            draw_tick(center, radius, 0.4, 0.6, 0.1, angle, wiper_color);
        break;
        case IMKNOB_WIPER:
            draw_circle(center, 0.7, true, 32, radius, circle_color);
            draw_arc(center, 0.8, 0.41, radius, angle_min, angle_max, 16, 2, track_color);
            if (t > 0.01)
                draw_arc(center, 0.8, 0.43, radius, angle_min, angle, 16, 2, wiper_color);
        break;
        case IMKNOB_WIPER_TICK:
            draw_circle(center, 0.6, true, 32, radius, circle_color);
            draw_arc(center, 0.85, 0.41, radius, angle_min, angle_max, 16, 2, track_color);
            draw_tick(center, radius, 0.75, 0.9, 0.1, angle, wiper_color);
        break;
        case IMKNOB_WIPER_DOT:
            draw_circle(center, 0.6, true, 32, radius, circle_color);
            draw_arc(center, 0.85, 0.41, radius, angle_min, angle_max, 16, 2, track_color);
            draw_dot(center, 0.85, 0.1, radius, angle, true, 12, wiper_color);
        break;
        case IMKNOB_WIPER_ONLY:
            draw_arc(center, 0.8, 0.41, radius, angle_min, angle_max, 32, 2, track_color);
            if (t > 0.01)
                draw_arc(center, 0.8, 0.43, radius, angle_min, angle, 16, 2, wiper_color);
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
            if (t > 0.01)
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
    ImGui::DragFloat(DragID.c_str(), p_value, (v_max - v_min) / 200.0, v_min, v_max, format);
    ImGui::PopItemWidth();
    ImGui::EndChild();
    return value_changed;
}

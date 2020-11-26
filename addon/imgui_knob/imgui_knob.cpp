#include "imgui_knob.h"

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
bool ImGui::SliderBehaviorT(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, TYPE* v, const TYPE v_min, const TYPE v_max, const char* format, float power, ImGuiSliderFlags flags, ImRect* out_grab_bb)
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
            v_new = RoundScalarWithFormatT<TYPE,SIGNEDTYPE>(format, data_type, v_new);

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
    case ImGuiDataType_S8:  { ImS32 v32 = (ImS32)*(ImS8*)v;  bool r = SliderBehaviorT<ImS32, ImS32, float >(bb, id, ImGuiDataType_S32, &v32, *(const ImS8*)v_min,  *(const ImS8*)v_max,  format, power, flags, out_grab_bb); if (r) *(ImS8*)v  = (ImS8)v32;  return r; }
    case ImGuiDataType_U8:  { ImU32 v32 = (ImU32)*(ImU8*)v;  bool r = SliderBehaviorT<ImU32, ImS32, float >(bb, id, ImGuiDataType_U32, &v32, *(const ImU8*)v_min,  *(const ImU8*)v_max,  format, power, flags, out_grab_bb); if (r) *(ImU8*)v  = (ImU8)v32;  return r; }
    case ImGuiDataType_S16: { ImS32 v32 = (ImS32)*(ImS16*)v; bool r = SliderBehaviorT<ImS32, ImS32, float >(bb, id, ImGuiDataType_S32, &v32, *(const ImS16*)v_min, *(const ImS16*)v_max, format, power, flags, out_grab_bb); if (r) *(ImS16*)v = (ImS16)v32; return r; }
    case ImGuiDataType_U16: { ImU32 v32 = (ImU32)*(ImU16*)v; bool r = SliderBehaviorT<ImU32, ImS32, float >(bb, id, ImGuiDataType_U32, &v32, *(const ImU16*)v_min, *(const ImU16*)v_max, format, power, flags, out_grab_bb); if (r) *(ImU16*)v = (ImU16)v32; return r; }
    case ImGuiDataType_S32:
        IM_ASSERT(*(const ImS32*)v_min >= IM_S32_MIN/2 && *(const ImS32*)v_max <= IM_S32_MAX/2);
        return SliderBehaviorT<ImS32, ImS32, float >(bb, id, data_type, (ImS32*)v,  *(const ImS32*)v_min,  *(const ImS32*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_U32:
        IM_ASSERT(*(const ImU32*)v_max <= IM_U32_MAX/2);
        return SliderBehaviorT<ImU32, ImS32, float >(bb, id, data_type, (ImU32*)v,  *(const ImU32*)v_min,  *(const ImU32*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_S64:
        IM_ASSERT(*(const ImS64*)v_min >= IM_S64_MIN/2 && *(const ImS64*)v_max <= IM_S64_MAX/2);
        return SliderBehaviorT<ImS64, ImS64, double>(bb, id, data_type, (ImS64*)v,  *(const ImS64*)v_min,  *(const ImS64*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_U64:
        IM_ASSERT(*(const ImU64*)v_max <= IM_U64_MAX/2);
        return SliderBehaviorT<ImU64, ImS64, double>(bb, id, data_type, (ImU64*)v,  *(const ImU64*)v_min,  *(const ImU64*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_Float:
        IM_ASSERT(*(const float*)v_min >= -FLT_MAX/2.0f && *(const float*)v_max <= FLT_MAX/2.0f);
        return SliderBehaviorT<float, float, float >(bb, id, data_type, (float*)v,  *(const float*)v_min,  *(const float*)v_max,  format, power, flags, out_grab_bb);
    case ImGuiDataType_Double:
        IM_ASSERT(*(const double*)v_min >= -DBL_MAX/2.0f && *(const double*)v_max <= DBL_MAX/2.0f);
        return SliderBehaviorT<double,double,double>(bb, id, data_type, (double*)v, *(const double*)v_min, *(const double*)v_max, format, power, flags, out_grab_bb);
    case ImGuiDataType_COUNT: break;
    }
    IM_ASSERT(0);
    return false;
}

void ImGui::UvMeter(char const *label, ImVec2 const &size, int *value, int v_min, int v_max)
{
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImVec2 pos = ImGui::GetCursorScreenPos();

    ImGui::InvisibleButton(label, size);

    float stepHeight = (v_max - v_min + 1) / size.y;
    auto y = pos.y + size.y;
    auto hue = 0.4f;
    auto sat = 0.6f;
    for (int i = v_min; i <= v_max; i += 5)
    {
        hue = 0.4f - (static_cast<float>(i) / static_cast<float>(v_max - v_min)) / 2.0f;
        sat = (*value < i ? 0.0f : 0.6f);
        draw_list->AddRectFilled(ImVec2(pos.x, y), ImVec2(pos.x + size.x, y - (stepHeight * 4)), static_cast<ImU32>(ImColor::HSV(hue, sat, 0.6f)));
        y = pos.y + size.y - (i * stepHeight);
    }
}

bool ImGui::Knob(char const *label, float *p_value, float v_min, float v_max, ImVec2 const &size, char const *tooltip)
{
    bool showLabel = label[0] != '#' && label[1] != '#' && label[0] != '\0';

    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec2 s(size.x - 4, size.y - 4);

    float radius_outer = std::fmin(s.x, s.y) / 2.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos = ImVec2(pos.x + 2, pos.y + 2);
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);

    float line_height = ImGui::GetTextLineHeight();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.70f;
    float ANGLE_MAX = 3.141592f * 2.30f;

    if (s.x != 0.0f && s.y != 0.0f)
    {
        center.x = pos.x + (s.x / 2.0f);
        center.y = pos.y + (s.y / 2.0f);
        ImGui::InvisibleButton(label, ImVec2(s.x, s.y + (showLabel ? line_height + style.ItemInnerSpacing.y : 0)));
    }
    else
    {
        ImGui::InvisibleButton(label, ImVec2(radius_outer * 2, radius_outer * 2 + (showLabel ? line_height + style.ItemInnerSpacing.y : 0)));
    }
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

    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * (*p_value - v_min) / (v_max - v_min);
    float angle_cos = cosf(angle);
    float angle_sin = sinf(angle);

    draw_list->AddCircleFilled(center, radius_outer * 0.7f, ImGui::GetColorU32(ImGuiCol_Button), 16);
    draw_list->PathArcTo(center, radius_outer, ANGLE_MIN, ANGLE_MAX, 16);
    draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBg), false, 3.0f);
    draw_list->AddLine(
        ImVec2(center.x + angle_cos * (radius_outer * 0.35f), center.y + angle_sin * (radius_outer * 0.35f)),
        ImVec2(center.x + angle_cos * (radius_outer * 0.7f), center.y + angle_sin * (radius_outer * 0.7f)),
        ImGui::GetColorU32(ImGuiCol_SliderGrabActive), 2.0f);
    draw_list->PathArcTo(center, radius_outer, ANGLE_MIN, angle + 0.02f, 16);
    draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_SliderGrabActive), false, 3.0f);

    if (showLabel)
    {
        auto textSize = CalcTextSize(label);
        draw_list->AddText(ImVec2(pos.x + ((size.x / 2) - (textSize.x / 2)), pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);
    }

    if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - (line_height * 2) - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        if (tooltip != nullptr)
        {
            ImGui::Text("%s\nValue : %.3f", tooltip, static_cast<double>(*p_value));
        }
        else if (showLabel)
        {
            ImGui::Text("%s %.3f", label, static_cast<double>(*p_value));
        }
        else
        {
            ImGui::Text("%.3f", static_cast<double>(*p_value));
        }
        ImGui::EndTooltip();
    }

    return value_changed;
}

bool ImGui::KnobUchar(char const *label, unsigned char *p_value, unsigned char v_min, unsigned char v_max, ImVec2 const &size, char const *tooltip)
{
    bool showLabel = label[0] != '#' && label[1] != '#' && label[0] != '\0';

    ImGuiIO &io = ImGui::GetIO();
    ImGuiStyle &style = ImGui::GetStyle();
    ImVec2 s(size.x - 4, size.y - 4);

    float radius_outer = std::fmin(s.x, s.y) / 2.0f;
    ImVec2 pos = ImGui::GetCursorScreenPos();
    pos = ImVec2(pos.x, pos.y + 2);
    ImVec2 center = ImVec2(pos.x + radius_outer, pos.y + radius_outer);

    float line_height = ImGui::GetFrameHeight();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float ANGLE_MIN = 3.141592f * 0.70f;
    float ANGLE_MAX = 3.141592f * 2.30f;

    if (s.x != 0.0f && s.y != 0.0f)
    {
        center.x = pos.x + (s.x / 2.0f);
        center.y = pos.y + (s.y / 2.0f);
        ImGui::InvisibleButton(label, ImVec2(s.x, s.y + (showLabel ? line_height + style.ItemInnerSpacing.y : 0)));
    }
    else
    {
        ImGui::InvisibleButton(label, ImVec2(radius_outer * 2, radius_outer * 2 + (showLabel ? line_height + style.ItemInnerSpacing.y : 0)));
    }
    bool value_changed = false;
    bool is_active = ImGui::IsItemActive();
#if 0
    if (is_active)
    {
        std::cout << "knob active: " << io.MouseDelta.y << std::endl;
    }
#endif
    bool is_hovered = ImGui::IsItemActive();
    if (is_active && io.MouseDelta.y != 0.0f)
    {
        int step = (v_max - v_min) / 127;
        int newVal = static_cast<int>(*p_value - io.MouseDelta.y * step);
        if (newVal < v_min)
            newVal = v_min;
        if (newVal > v_max)
            newVal = v_max;
        *p_value = static_cast<unsigned char>(newVal);
        value_changed = true;
    }

    float angle = ANGLE_MIN + (ANGLE_MAX - ANGLE_MIN) * (*p_value - v_min) / (v_max - v_min);
    float angle_cos = cosf(angle);
    float angle_sin = sinf(angle);

    draw_list->AddCircleFilled(center, radius_outer * 0.7f, ImGui::GetColorU32(ImGuiCol_Button), 16);
    draw_list->PathArcTo(center, radius_outer, ANGLE_MIN, ANGLE_MAX, 16);
    draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_FrameBg), false, 3.0f);
    draw_list->AddLine(
        ImVec2(center.x + angle_cos * (radius_outer * 0.35f), center.y + angle_sin * (radius_outer * 0.35f)),
        ImVec2(center.x + angle_cos * (radius_outer * 0.7f), center.y + angle_sin * (radius_outer * 0.7f)),
        ImGui::GetColorU32(ImGuiCol_Text), 2.0f);
    draw_list->PathArcTo(center, radius_outer, ANGLE_MIN, angle + 0.02f, 16);
    draw_list->PathStroke(ImGui::GetColorU32(ImGuiCol_SliderGrabActive), false, 3.0f);

    if (showLabel)
    {
        auto textSize = CalcTextSize(label);
        draw_list->AddText(ImVec2(pos.x + ((size.x / 2) - (textSize.x / 2)), pos.y + radius_outer * 2 + style.ItemInnerSpacing.y), ImGui::GetColorU32(ImGuiCol_Text), label);
    }

    if (is_active || is_hovered)
    {
        ImGui::SetNextWindowPos(ImVec2(pos.x - style.WindowPadding.x, pos.y - (line_height * 2) - style.ItemInnerSpacing.y - style.WindowPadding.y));
        ImGui::BeginTooltip();
        if (tooltip != nullptr)
        {
            ImGui::Text("%s\nValue : %d", tooltip, static_cast<unsigned int>(*p_value));
        }
        else if (showLabel)
        {
            ImGui::Text("%s %d", label, static_cast<unsigned int>(*p_value));
        }
        else
        {
            ImGui::Text("%d", static_cast<unsigned int>(*p_value));
        }
        ImGui::EndTooltip();
    }
    else if (ImGui::IsItemHovered() && tooltip != nullptr)
    {
        ImGui::ShowTooltipOnHover(tooltip);
    }

    return value_changed;
}

bool ImGui::DropDown(char const *label, unsigned char &value, char const *const names[], unsigned int nameCount, char const *tooltip)
{
    bool value_changed = false;

    auto current_effect_item = names[value];
    if (ImGui::BeginCombo(label, current_effect_item, ImGuiComboFlags_HeightLarge))
    {
        for (unsigned char n = 0; n < nameCount; n++)
        {
            bool is_selected = (current_effect_item == names[n]);
            if (ImGui::Selectable(names[n], is_selected))
            {
                current_effect_item = names[n];
                value = n;
                value_changed = true;
            }
        }

        ImGui::EndCombo();
    }
    ImGui::ShowTooltipOnHover(tooltip == nullptr ? label : tooltip);

    return value_changed;
}

bool ImGui::ImageToggleButton(const char *str_id, bool *v, ImTextureID user_texture_id, const ImVec2 &size)
{
    bool valueChange = false;

    const ImGuiStyle &style = ImGui::GetStyle();
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    float height = size.y + (style.FramePadding.y * 2);
    float width = size.x + (style.FramePadding.x * 2);

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (v != nullptr && ImGui::IsItemClicked())
    {
        *v = !*v;
        valueChange = true;
    }

    ImU32 col_tint = ImGui::GetColorU32((v == nullptr || *v ? ImGui::GetColorU32(ImVec4(1, 1, 1, 1)) : ImGui::GetColorU32(ImGuiCol_TextDisabled)));
    ImU32 col_bg = ImGui::GetColorU32(ImGui::GetColorU32(ImGuiCol_Button));
    if (ImGui::IsItemHovered())
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    }
    if (ImGui::IsItemActive())
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_ButtonActive);
    }

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg);
    draw_list->AddImage(user_texture_id, p, ImVec2(p.x + width, p.y + height), ImVec2(0, 0), ImVec2(1, 1), GetColorU32(col_tint));

    return valueChange;
}

bool ImGui::ToggleButtonWithCheckbox(const char *str_id, bool *v, bool *checked, const ImVec2 &size)
{
    bool valueChange = false;

    PushID(str_id);

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    const float square_sz = GetFrameHeight();

    if (Checkbox("##ButtonCheck", checked))
    {
        valueChange = true;
    }

    SameLine();

    ImGui::InvisibleButton("##InvisibleButton", ImVec2(size.x, square_sz));
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

    draw_list->AddRectFilled(pos, pos + ImVec2(size.x + square_sz, square_sz), GetColorU32(col_bg));

    auto textSize = CalcTextSize(str_id);
    draw_list->AddText(ImVec2(pos.x + (size.x + square_sz - textSize.x) / 2, pos.y), col_tint, str_id);

    PopID();

    return valueChange;
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

    draw_list->AddRectFilled(pos, pos + size, GetColorU32(col_bg));

    auto textSize = CalcTextSize(str_id);
    draw_list->AddText(ImVec2(pos.x + (size.x - textSize.x) / 2, pos.y), col_tint, str_id);

    return valueChange;
}

void ImGui::ShowTooltipOnHover(char const *tooltip)
{
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::Text("%s", tooltip);
        ImGui::EndTooltip();
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
    if ((hovered && g.IO.MouseClicked[0]) || g.NavActivateId == id || g.NavInputId == id)
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

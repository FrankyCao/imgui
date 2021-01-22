#ifndef IMGUI_KNOB_H
#define IMGUI_KNOB_H

#include <functional>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

namespace ImGui {
enum ImGuiKnobType
{
    IMKNOB_TICK = 0,
    IMKNOB_TICK_DOT,
    IMKNOB_TICK_WIPER,
    IMKNOB_WIPER,
    IMKNOB_WIPER_TICK,
    IMKNOB_WIPER_DOT,
    IMKNOB_WIPER_ONLY,
    IMKNOB_STEPPED_TICK,
    IMKNOB_STEPPED_DOT,
    IMKNOB_SPACE,
};

struct ColorSet 
{
    ImVec4 base;
    ImVec4 hovered;
    ImVec4 active;
};

template<typename T, typename SIGNED_T>                     IMGUI_API T     RoundScalarWithFormatKnobT(const char* format, ImGuiDataType data_type, T v);
template<typename T, typename SIGNED_T, typename FLOAT_T>   IMGUI_API bool  SliderBehaviorKnobT(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, T* v, T v_min, T v_max, const char* format, float power, ImGuiSliderFlags flags, ImRect* out_grab_bb);
template<typename T, typename FLOAT_T>                      IMGUI_API float SliderCalcRatioFromValueT(ImGuiDataType data_type, T v, T v_min, T v_max, float power, float linear_zero_pos);

IMGUI_API bool SliderBehavior(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, void* p_v, const void* p_min, const void* p_max, const char* format, float power, ImGuiSliderFlags flags, ImRect* out_grab_bb);

IMGUI_API void UvMeter(char const *label, ImVec2 const &size, int *value, int v_min, int v_max);

IMGUI_API bool Knob(char const *label, float *p_value, float v_min, float v_max, float v_default, float size,
                    ColorSet circle_color, ColorSet wiper_color, ColorSet track_color, ColorSet tick_color,
                    ImGuiKnobType type = IMKNOB_WIPER, char const *format = nullptr, int tick_steps = 0);

IMGUI_API bool Fader(const char* label, const ImVec2& size, int* v, const int v_min, const int v_max, const char* format = nullptr, float power = 1.0f);

} // namespace ImGui


#endif // IMGUI_KNOB_H

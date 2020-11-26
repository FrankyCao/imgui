#ifndef IMGUI_KNOB_H
#define IMGUI_KNOB_H

#include <functional>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

namespace ImGui {
template<typename T, typename SIGNED_T, typename FLOAT_T>   IMGUI_API bool  SliderBehaviorT(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, T* v, T v_min, T v_max, const char* format, float power, ImGuiSliderFlags flags, ImRect* out_grab_bb);
template<typename T, typename FLOAT_T>                      IMGUI_API float SliderCalcRatioFromValueT(ImGuiDataType data_type, T v, T v_min, T v_max, float power, float linear_zero_pos);

IMGUI_API bool SliderBehavior(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, void* p_v, const void* p_min, const void* p_max, const char* format, float power, ImGuiSliderFlags flags, ImRect* out_grab_bb);

IMGUI_API void UvMeter(char const *label, ImVec2 const &size, int *value, int v_min, int v_max);

IMGUI_API bool Knob(char const *label, float *p_value,
                    float v_min, float v_max,
                    ImVec2 const &size, char const *tooltip = nullptr);

IMGUI_API bool KnobUchar(char const *label, unsigned char *p_value,
                        unsigned char v_min, unsigned char v_max,
                        ImVec2 const &size, char const *tooltip = nullptr);

IMGUI_API bool DropDown(char const *label, unsigned char &value, char const *const names[], unsigned int nameCount, char const *tooltip = nullptr);

IMGUI_API bool ImageToggleButton(const char *str_id, bool *v, ImTextureID user_texture_id, const ImVec2 &size);

IMGUI_API bool ToggleButtonWithCheckbox(const char *str_id, bool *on, bool *checked, const ImVec2 &size);

IMGUI_API bool ToggleButton(const char *str_id, bool *v, const ImVec2 &size);

IMGUI_API void ShowTooltipOnHover(char const *tooltip);

IMGUI_API bool Fader(const char* label, const ImVec2& size, int* v, const int v_min, const int v_max, const char* format = nullptr, float power = 1.0f);

} // namespace ImGui

#endif // IMGUI_KNOB_H

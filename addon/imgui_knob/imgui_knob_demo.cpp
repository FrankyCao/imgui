#include <imgui.h>
#undef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
//-----------------------------------------------------------------------------------------------------------------
#include "imgui_knob.h"

namespace ImGui
{
void ShowKnobDemoWindow()
{
    static float val = 0.5, val_default = 0.5;
    float t = (float)ImGui::GetTime();
    float h = abs(sin(t * 0.2));
    float s = abs(sin(t * 0.1)) * 0.5 + 0.4;
    ImVec4 base_color = ImVec4(0.f, 0.f, 0.f, 1.f), active_color = ImVec4(0.f, 0.f, 0.f, 1.f), hovered_color = ImVec4(0.f, 0.f, 0.f, 1.f);
    ImGui::ColorConvertHSVtoRGB(h, s, 0.5f, base_color.x, base_color.y, base_color.z);
    ImGui::ColorConvertHSVtoRGB(h, s, 0.6f, active_color.x, active_color.y, active_color.z);
    ImGui::ColorConvertHSVtoRGB(h, s, 0.7f, hovered_color.x, hovered_color.y, hovered_color.z);
    ImVec4 highlight_base_color = ImVec4(0.f, 0.f, 0.f, 1.f), highlight_active_color = ImVec4(0.f, 0.f, 0.f, 1.f), highlight_hovered_color = ImVec4(0.f, 0.f, 0.f, 1.f);
    ImGui::ColorConvertHSVtoRGB(h, s, 0.75f, highlight_base_color.x, highlight_base_color.y, highlight_base_color.z);
    ImGui::ColorConvertHSVtoRGB(h, s, 0.95f, highlight_active_color.x, highlight_active_color.y, highlight_active_color.z);
    ImGui::ColorConvertHSVtoRGB(h, s, 1.0f, highlight_hovered_color.x, highlight_hovered_color.y, highlight_hovered_color.z);
    ImVec4 lowlight_base_color = ImVec4(0.f, 0.f, 0.f, 1.f), lowlight_active_color = ImVec4(0.f, 0.f, 0.f, 1.f), lowlight_hovered_color = ImVec4(0.f, 0.f, 0.f, 1.f);
    ImGui::ColorConvertHSVtoRGB(h, s, 0.2f, lowlight_base_color.x, lowlight_base_color.y, lowlight_base_color.z);
    ImGui::ColorConvertHSVtoRGB(h, s, 0.3f, lowlight_active_color.x, lowlight_active_color.y, lowlight_active_color.z);
    ImGui::ColorConvertHSVtoRGB(h, s, 0.4f, lowlight_hovered_color.x, lowlight_hovered_color.y, lowlight_hovered_color.z);
    ImVec4 tick_base_color = ImVec4(0.8f, 0.8f, 0.8f, 1.f), tick_active_color = ImVec4(1.f, 1.f, 1.f, 1.f), tick_hovered_color = ImVec4(1.f, 1.f, 1.f, 1.f);
    ColorSet circle_color = {base_color, active_color, hovered_color};
    ColorSet wiper_color = {highlight_base_color, highlight_active_color, highlight_hovered_color};
    ColorSet track_color = {lowlight_base_color, lowlight_active_color, lowlight_hovered_color};
    ColorSet tick_color = {tick_base_color, tick_active_color, tick_hovered_color};

    float knob_size = 80.f;
    ImGui::Knob("##Tick", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_TICK, "%.03fdB");
    ImGui::SameLine();
    ImGui::Knob("TickDot", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_TICK_DOT, "%.03fdB");
    ImGui::SameLine();
    ImGui::Knob("TickWiper", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_TICK_WIPER, "%.03fdB");
    ImGui::SameLine();
    ImGui::Knob("Wiper", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_WIPER, "%.03fdB");
    ImGui::SameLine();
    ImGui::Knob("WiperTick", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_WIPER_TICK, "%.03fdB");
    ImGui::SameLine();
    ImGui::Knob("WiperDot", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_WIPER_DOT, "%.03fdB");
    ImGui::SameLine();
    ImGui::Knob("WiperOnly", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_WIPER_ONLY, "%.03fdB");
    ImGui::SameLine();
    ImGui::Knob("SteppedTick", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_STEPPED_TICK, "%.03fdB", 10);
    ImGui::SameLine();
    ImGui::Knob("SteppedDot", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_STEPPED_DOT, "%.03fdB", 10);
    ImGui::SameLine();
    ImGui::Knob("Space", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_SPACE, "%.03fdB");

    int idb = val * 80;
    ImGui::Fader("##mastervol", ImVec2(20, 80), &idb, 0, 80, "%d", 1.0f); ImGui::ShowTooltipOnHover("Slide.");
    ImGui::SameLine();
    static int stack = 0;
    static int count = 0;
    ImGui::UvMeter("##vuvr", ImVec2(10, 80), &idb, 0, 80, 20); ImGui::ShowTooltipOnHover("Vertical Uv meters.");
    ImGui::UvMeter("##huvr", ImVec2(80, 10), &idb, 0, 80, 20, &stack, &count); ImGui::ShowTooltipOnHover("Horizon Uv meters width stack effect.");
}
} // namespace ImGui

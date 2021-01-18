#pragma once
namespace ImGui
{
    // Progress Indicators
    IMGUI_API bool          BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
    IMGUI_API bool          Spinner(const char* label, float radius, int thickness, const ImU32& color);
} // namespace ImGui
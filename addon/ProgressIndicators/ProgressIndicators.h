#pragma once
namespace ImGui
{
    // Progress Indicators
    IMGUI_API bool          BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
    IMGUI_API bool          Spinner(const char* label, float radius, int thickness, const ImU32& color);
    IMGUI_API void          LoadingIndicatorCircle(const char* label, const float indicator_radius,
                                                    const ImVec4& main_color, const ImVec4& backdrop_color,
                                                    const int circle_count, const float speed);
} // namespace ImGui
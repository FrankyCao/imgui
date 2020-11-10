#pragma once
namespace ImGui
{
    // Toggle Button
    IMGUI_API void          ToggleButton(const char* str_id, bool* v);

    // Progress Indicators
    IMGUI_API bool          BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
    IMGUI_API bool          Spinner(const char* label, float radius, int thickness, const ImU32& color);
    IMGUI_API void          LoadingIndicatorCircle(const char* label, const float indicator_radius,
                                                    const ImVec4& main_color, const ImVec4& backdrop_color,
                                                    const int circle_count, const float speed);
    
    // Spin
    IMGUI_API bool          SpinScaler(const char* label, ImGuiDataType data_type, void* data_ptr, const void* step, const void* step_fast, const char* format, ImGuiInputTextFlags flags);
	IMGUI_API bool          SpinInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
	IMGUI_API bool          SpinFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	IMGUI_API bool          SpinDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);

    // Splitter
    IMGUI_API bool          Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);

} // namespace ImGui

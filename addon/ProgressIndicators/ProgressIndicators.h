#pragma once
namespace ImGui
{
    // Progress Indicators
    IMGUI_API bool      BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col);
    IMGUI_API bool      Spinner(const char* label, float radius, int thickness, const ImU32& color);

    // Slider 2D and Slider 3D 
    IMGUI_API bool      InputVec2(char const* pLabel, ImVec2* pValue, ImVec2 const vMinValue, ImVec2 const vMaxValue, float const fScale = 1.0f);
    IMGUI_API bool      InputVec3(char const* pLabel, ImVec4* pValue, ImVec4 const vMinValue, ImVec4 const vMaxValue, float const fScale = 1.0f);
    IMGUI_API bool      SliderScalar2D(char const* pLabel, float* fValueX, float* fValueY, const float fMinX, const float fMaxX, const float fMinY, const float fMaxY, float const fZoom = 1.0f);
    IMGUI_API bool      SliderScalar3D(char const* pLabel, float* pValueX, float* pValueY, float* pValueZ, float const fMinX, float const fMaxX, float const fMinY, float const fMaxY, float const fMinZ, float const fMaxZ, float const fScale = 1.0f);

    // Bezier Widget
    IMGUI_API int       Bezier(const char *label, float P[5]);
    IMGUI_API float     BezierValue(float dt01, float P[4]);

    // Spin
    IMGUI_API bool      SpinScaler(const char* label, ImGuiDataType data_type, void* data_ptr, const void* step, const void* step_fast, const char* format, ImGuiInputTextFlags flags);
	IMGUI_API bool      SpinInt(const char* label, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
	IMGUI_API bool      SpinFloat(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);
	IMGUI_API bool      SpinDouble(const char* label, double* v, double step = 0.0, double step_fast = 0.0, const char* format = "%.6f", ImGuiInputTextFlags flags = 0);

#if IMGUI_BUILD_EXAMPLE
    // Demo
    IMGUI_API void      ShowBezierDemo();
#endif
} // namespace ImGui

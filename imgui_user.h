#pragma once
namespace ImGui
{
    // Toggle Button
    IMGUI_API void          ToggleButton(const char* str_id, bool* v);

#if 0
    // Slider Scaler
    bool SliderScalar2D(char const* pLabel, float* fValueX, float* fValueY, const float fMinX, const float fMaxX, const float fMinY, const float fMaxY, float const fZoom = 1.0f);
    bool SliderScalar3D(char const* pLabel, float* pValueX, float* pValueY, float* pValueZ, float const fMinX, float const fMaxX, float const fMinY, float const fMaxY, float const fMinZ, float const fMaxZ, float const fScale = 1.0f);
    bool InputVec2(char const* pLabel, ImVec2* pValue, ImVec2 const vMinValue, ImVec2 const vMaxValue, float const fScale = 1.0f);
    bool InputVec3(char const* pLabel, ImVec4* pValue, ImVec4 const vMinValue, ImVec4 const vMaxValue, float const fScale = 1.0f);
#endif
} // namespace ImGui

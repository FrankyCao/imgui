#ifndef IMGUI_WIDGET_H
#define IMGUI_WIDGET_H

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

IMGUI_API void UvMeter(char const *label, ImVec2 const &size, int *value, int v_min, int v_max, int steps = 10, int* stack = nullptr, int* count = nullptr);
IMGUI_API void UvMeter(char const *label, ImVec2 const &size, float *value, float v_min, float v_max, int steps = 10, float* stack = nullptr, int* count = nullptr);

IMGUI_API bool Knob(char const *label, float *p_value, float v_min, float v_max, float v_step, float v_default, float size,
                    ColorSet circle_color, ColorSet wiper_color, ColorSet track_color, ColorSet tick_color,
                    ImGuiKnobType type = IMKNOB_WIPER, char const *format = nullptr, int tick_steps = 0);

IMGUI_API bool Fader(const char* label, const ImVec2& size, int* v, const int v_min, const int v_max, const char* format = nullptr, float power = 1.0f);

IMGUI_API void RoundProgressBar(float radius, float *p_value, float v_min, float v_max, ColorSet bar_color, ColorSet progress_color, ColorSet text_color);

// Splitter
IMGUI_API bool Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);

// ToggleButton
IMGUI_API void ToggleButton(const char* str_id, bool* v);
IMGUI_API bool ToggleButton(const char *str_id, bool *v, const ImVec2 &size);
IMGUI_API bool BulletToggleButton(const char* label,bool* v, ImVec2 &pos, ImVec2 &size);

// CheckButton
IMGUI_API bool CheckButton(const char* label, bool* pvalue, bool useSmallButton = false, float checkedStateAlphaMult = 0.5f);

// ColoredButtonV1: code posted by @ocornut here: https://github.com/ocornut/imgui/issues/4722
// [Button rounding depends on the FrameRounding Style property (but can be overridden with the last argument)]
IMGUI_API bool ColoredButton(const char* label, const ImVec2& size, ImU32 text_color, ImU32 bg_color_1, ImU32 bg_color_2, float frame_rounding_override=-1.f);

// new ProgressBar
// Please note that you can tweak the "format" argument if you want to add a prefix (or a suffix) piece of text to the text that appears at the right of the bar.
// returns the value "fraction" in 0.f-1.f.
// It does not need any ID.
IMGUI_API float ProgressBar(const char* optionalPrefixText,float value,const float minValue=0.f,const float maxValue=1.f,const char* format="%1.0f%%",const ImVec2& sizeOfBarWithoutTextInPixels=ImVec2(-1,-1),
                const ImVec4& colorLeft=ImVec4(0,1,0,0.8),const ImVec4& colorRight=ImVec4(0,0.4,0,0.8),const ImVec4& colorBorder=ImVec4(0.25,0.25,1.0,1));


// Slider 2D and Slider 3D 
IMGUI_API bool InputVec2(char const* pLabel, ImVec2* pValue, ImVec2 const vMinValue, ImVec2 const vMaxValue, float const fScale = 1.0f);
IMGUI_API bool InputVec3(char const* pLabel, ImVec4* pValue, ImVec4 const vMinValue, ImVec4 const vMaxValue, float const fScale = 1.0f);
IMGUI_API bool SliderScalar2D(char const* pLabel, float* fValueX, float* fValueY, const float fMinX, const float fMaxX, const float fMinY, const float fMaxY, float const fZoom = 1.0f);
IMGUI_API bool SliderScalar3D(char const* pLabel, float* pValueX, float* pValueY, float* pValueZ, float const fMinX, float const fMaxX, float const fMinY, float const fMaxY, float const fMinZ, float const fMaxZ, float const fScale = 1.0f);

IMGUI_API bool RangeSelect2D(char const* pLabel, float* pCurMinX, float* pCurMinY, float* pCurMaxX, float* pCurMaxY, float const fBoundMinX, float const fBoundMinY, float const fBoundMaxX, float const fBoundMaxY, float const fScale /*= 1.0f*/);
IMGUI_API bool RangeSelectVec2(const char* pLabel, ImVec2* pCurMin, ImVec2* pCurMax, ImVec2 const vBoundMin, ImVec2 const vBoundMax, float const fScale /*= 1.0f*/);

// Bezier Widget
IMGUI_API bool  BezierSelect(const char *label, float P[5]);    // P[4] is curve presets(0 - 24)
IMGUI_API float BezierValue(float dt01, float P[4], int step = 0);


// Color Processing
// func: ImU32(*func)(float const x, float const y)
template < typename Type >
inline
Type	ScaleFromNormalized(Type const x, Type const newMin, Type const newMax)
{
	return x * (newMax - newMin) + newMin;
}
template < bool IsBilinear, typename FuncType >
inline
void DrawColorDensityPlotEx(ImDrawList* pDrawList, FuncType func, float minX, float maxX, float minY, float maxY, ImVec2 position, ImVec2 size, int resolutionX, int resolutionY)
{
	ImVec2 const uv = ImGui::GetFontTexUvWhitePixel();

	float const sx = size.x / ((float)resolutionX);
	float const sy = size.y / ((float)resolutionY);

	float const dy = 1.0f / ((float)resolutionY);
	float const dx = 1.0f / ((float)resolutionX);
	float const hdx = 0.5f / ((float)resolutionX);
	float const hdy = 0.5f / ((float)resolutionY);

	for (int i = 0; i < resolutionX; ++i)
	{
		float x0;
		float x1;
		if (IsBilinear)
		{
			x0 = ScaleFromNormalized(((float)i + 0) * dx, minX, maxX);
			x1 = ScaleFromNormalized(((float)i + 1) * dx, minX, maxX);
		}
		else
		{
			x0 = ScaleFromNormalized(((float)i + 0) * dx + hdx, minX, maxX);
		}

		for (int j = 0; j < resolutionY; ++j)
		{
			float y0;
			float y1;
			if (IsBilinear)
			{
				y0 = ScaleFromNormalized(((float)(j + 0) * dy), maxY, minY);
				y1 = ScaleFromNormalized(((float)(j + 1) * dy), maxY, minY);
			}
			else
			{
				y0 = ScaleFromNormalized(((float)(j + 0) * dy + hdy), maxY, minY);
			}

			ImU32 const col00 = func(x0, y0);
			if (IsBilinear)
			{
				ImU32 const col01 = func(x0, y1);
				ImU32 const col10 = func(x1, y0);
				ImU32 const col11 = func(x1, y1);
				pDrawList->AddRectFilledMultiColor(	position + ImVec2(sx * (i + 0), sy * (j + 0)),
													position + ImVec2(sx * (i + 1), sy * (j + 1)),
													col00, col10, col11, col01);
			}
			else
			{
				pDrawList->AddRectFilledMultiColor(	position + ImVec2(sx * (i + 0), sy * (j + 0)),
													position + ImVec2(sx * (i + 1), sy * (j + 1)),
													col00, col00, col00, col00);
			}
		}
	}
}
// func: ImU32(*func)(float const t)
template <bool IsBilinear, typename FuncType>
inline
void	DrawColorBandEx(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, FuncType func, int division, float gamma)
{
	float const width = size.x;
	float const height = size.y;

	float const fSlice = static_cast<float>(division);

	ImVec2 dA(vpos);
	ImVec2 dB(vpos.x + width / fSlice, vpos.y + height);

	ImVec2 const dD(ImVec2(width / fSlice, 0));

	auto curColor =	[gamma, &func](float const x, float const)
					{
						return func(ImPow(x, gamma));
					};

	DrawColorDensityPlotEx< IsBilinear >(pDrawList, curColor, 0.0f, 1.0f, 0.0f, 0.0f, vpos, size, division, 1);
}
template <bool IsBilinear, typename FuncType>
inline
void	DrawColorRingEx(ImDrawList* pDrawList, ImVec2 const curPos, ImVec2 const size, float thickness_, FuncType func, int division, float colorOffset)
{
	float const radius = ImMin(size.x, size.y) * 0.5f;

	float const dAngle = -2.0f * IM_PI / ((float)division);
	float angle = 0; //2.0f * IM_PI / 3.0f;

	ImVec2 offset = curPos + ImVec2(radius, radius);
	if (size.x < size.y)
	{
		offset.y += 0.5f * (size.y - size.x);
	}
	else if (size.x > size.y)
	{
		offset.x += 0.5f * (size.x - size.y);
	}

	float const thickness = ImSaturate(thickness_) * radius;

	ImVec2 const uv = ImGui::GetFontTexUvWhitePixel();
	pDrawList->PrimReserve(division * 6, division * 4);
	for (int i = 0; i < division; ++i)
	{
		float x0 = radius * ImCos(angle);
		float y0 = radius * ImSin(angle);

		float x1 = radius * ImCos(angle + dAngle);
		float y1 = radius * ImSin(angle + dAngle);

		float x2 = (radius - thickness) * ImCos(angle + dAngle);
		float y2 = (radius - thickness) * ImSin(angle + dAngle);

		float x3 = (radius - thickness) * ImCos(angle);
		float y3 = (radius - thickness) * ImSin(angle);

		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx));
		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx + 1));
		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx + 2));

		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx));
		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx + 2));
		pDrawList->PrimWriteIdx((ImDrawIdx)(pDrawList->_VtxCurrentIdx + 3));

		float const t0 = fmodf(colorOffset + ((float)i) / ((float)division), 1.0f);
		ImU32 const uCol0 = func(t0);

		if (IsBilinear)
		{
			float const t1 = fmodf(colorOffset + ((float)(i + 1)) / ((float)division), 1.0f);
			ImU32 const uCol1 = func(t1);
			pDrawList->PrimWriteVtx(offset + ImVec2(x0, y0), uv, uCol0);
			pDrawList->PrimWriteVtx(offset + ImVec2(x1, y1), uv, uCol1);
			pDrawList->PrimWriteVtx(offset + ImVec2(x2, y2), uv, uCol1);
			pDrawList->PrimWriteVtx(offset + ImVec2(x3, y3), uv, uCol0);
		}
		else
		{
			pDrawList->PrimWriteVtx(offset + ImVec2(x0, y0), uv, uCol0);
			pDrawList->PrimWriteVtx(offset + ImVec2(x1, y1), uv, uCol0);
			pDrawList->PrimWriteVtx(offset + ImVec2(x2, y2), uv, uCol0);
			pDrawList->PrimWriteVtx(offset + ImVec2(x3, y3), uv, uCol0);
		}
		angle += dAngle;
	}
}

IMGUI_API void DrawHueBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float alpha, float gamma, float offset = 0.0f);
IMGUI_API void DrawHueBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, float colorStartRGB[3], float alpha, float gamma);
IMGUI_API void DrawLumianceBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma);
IMGUI_API void DrawSaturationBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma);
IMGUI_API void DrawContrastBand(ImDrawList* pDrawList, ImVec2 const vpos, ImVec2 const size, ImVec4 const& color);
IMGUI_API bool ColorRing(const char* label, float thickness, int split);

// Color Selector
IMGUI_API void HueSelector(char const* label, ImVec2 const size, float* hueCenter, float* hueWidth, float* featherLeft, float* featherRight, float defaultVal, float ui_zoom = 1.0f, int division = 32, float alpha = 1.0f, float hideHueAlpha = 0.75f, float offset = 0.0f);
IMGUI_API void LumianceSelector(char const* label, ImVec2 const size, float* lumCenter, float defaultVal, float ui_zoom = 1.0f, int division = 32, float gamma = 1.f, bool rgb_color = false, ImVec4 const color = ImVec4(1, 1, 1, 1));
IMGUI_API void SaturationSelector(char const* label, ImVec2 const size, float* satCenter, float defaultVal, float ui_zoom = 1.0f, int division = 32, float gamma = 1.f, bool rgb_color = false, ImVec4 const color = ImVec4(1, 1, 1, 1));
IMGUI_API void ContrastSelector(char const* label, ImVec2 const size, float* conCenter, float defaultVal, float ui_zoom = 1.0f, bool rgb_color = false, ImVec4 const color = ImVec4(1, 1, 1, 1));
IMGUI_API void BalanceSelector(char const* label, ImVec2 const size, ImVec4 * rgba, ImVec4 defaultVal, float ui_zoom = 1.0f, int division = 128, float thickness = 1.0f, float colorOffset = 0);

// https://github.com/CedricGuillemet/imgInspect
/*
//example
Image pickerImage;
ImGui::ImageButton(pickerImage.textureID, ImVec2(pickerImage.mWidth, pickerImage.mHeight));
ImRect rc = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
ImVec2 mouseUVCoord = (io.MousePos - rc.Min) / rc.GetSize();
mouseUVCoord.y = 1.f - mouseUVCoord.y;
if (io.KeyShift && io.MouseDown[0] && mouseUVCoord.x >= 0.f && mouseUVCoord.y >= 0.f)
{
        int width = pickerImage.mWidth;
        int height = pickerImage.mHeight;

        ImGuiHelper::ImageInspect(width, height, pickerImage.GetBits(), mouseUVCoord, displayedTextureSize);
}
*/
IMGUI_API void ImageInspect(const int width,
                            const int height,
                            const unsigned char* const bits,
                            ImVec2 mouseUVCoord,
                            ImVec2 displayedTextureSize,
                            bool histogram_full = false,
                            int zoom_size = 8);

// Demo Window
#if IMGUI_BUILD_EXAMPLE
IMGUI_API void ShowExtraWidgetDemoWindow();
#endif
} // namespace ImGui


#endif // IMGUI_WIDGET_H

#ifndef IMGUI_WIDGET_H
#define IMGUI_WIDGET_H

#include <functional>
#include <vector>
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <immat.h>

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
IMGUI_API bool ToggleButton(const char* str_id, bool* v);
IMGUI_API bool ToggleButton(const char *str_id, bool *v, const ImVec2 &size);
IMGUI_API bool BulletToggleButton(const char* label,bool* v, ImVec2 &pos, ImVec2 &size);

// RotateButton
IMGUI_API bool RotateButton(const char* label, const ImVec2& size_arg, int rotate = 0);

// Input with int64
IMGUI_API bool InputInt64(const char* label, int64_t* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);

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

// ProgressBar with 0 as center
IMGUI_API void ProgressBarPanning(float fraction, const ImVec2& size_arg = ImVec2(-FLT_MIN, 0));

// new PlotEx
IMGUI_API int   PlotEx(ImGuiPlotType plot_type, const char* label, float (*values_getter)(void* data, int idx), void* data, int values_count, int values_offset, const char* overlay_text, float scale_min, float scale_max, ImVec2 frame_size, bool b_tooltops = true, bool b_comband = false);
IMGUI_API void  PlotLines(const char* label, const float* values, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), int stride = sizeof(float), bool b_tooltips = true, bool b_comband = false);
IMGUI_API void  PlotLines(const char* label, float(*values_getter)(void* data, int idx), void* data, int values_count, int values_offset = 0, const char* overlay_text = NULL, float scale_min = FLT_MAX, float scale_max = FLT_MAX, ImVec2 graph_size = ImVec2(0, 0), bool b_tooltips = true, bool b_comband = false);

// new menu item
IMGUI_API bool  MenuItemEx(const char* label, const char* icon, const char* shortcut = NULL, bool selected = false, bool enabled = true, const char* subscript = nullptr);
IMGUI_API bool  MenuItem(const char* label, const char* shortcut, bool selected, bool enabled, const char* subscript);  

// Slider 2D and Slider 3D 
IMGUI_API bool InputVec2(char const* pLabel, ImVec2* pValue, ImVec2 const vMinValue, ImVec2 const vMaxValue, float const fScale = 1.0f);
IMGUI_API bool InputVec3(char const* pLabel, ImVec4* pValue, ImVec4 const vMinValue, ImVec4 const vMaxValue, float const fScale = 1.0f);
IMGUI_API bool SliderScalar2D(char const* pLabel, float* fValueX, float* fValueY, const float fMinX, const float fMaxX, const float fMinY, const float fMaxY, float const fZoom = 1.0f);
IMGUI_API bool SliderScalar3D(char const* pLabel, float* pValueX, float* pValueY, float* pValueZ, float const fMinX, float const fMaxX, float const fMinY, float const fMaxY, float const fMinZ, float const fMaxZ, float const fScale = 1.0f);

IMGUI_API bool RangeSelect2D(char const* pLabel, float* pCurMinX, float* pCurMinY, float* pCurMaxX, float* pCurMaxY, float const fBoundMinX, float const fBoundMinY, float const fBoundMaxX, float const fBoundMaxY, float const fScale /*= 1.0f*/);
IMGUI_API bool RangeSelectVec2(const char* pLabel, ImVec2* pCurMin, ImVec2* pCurMax, ImVec2 const vBoundMin, ImVec2 const vBoundMax, float const fScale /*= 1.0f*/);

// Bezier Widget
IMGUI_API bool  BezierSelect(const char *label, const ImVec2 size, float P[5]);    // P[4] is curve presets(0 - 24)
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

IMGUI_API void DrawHueBand(ImVec2 const vpos, ImVec2 const size, int division, float alpha, float gamma, float offset = 0.0f);
IMGUI_API void DrawHueBand(ImVec2 const vpos, ImVec2 const size, int division, float colorStartRGB[3], float alpha, float gamma);
IMGUI_API void DrawLumianceBand(ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma);
IMGUI_API void DrawSaturationBand(ImVec2 const vpos, ImVec2 const size, int division, ImVec4 const& color, float gamma);
IMGUI_API void DrawContrastBand(ImVec2 const vpos, ImVec2 const size, ImVec4 const& color);
IMGUI_API bool ColorRing(const char* label, float thickness, int split);

// Color Selector
IMGUI_API void HueSelector(char const* label, ImVec2 const size, float* hueCenter, float* hueWidth, float* featherLeft, float* featherRight, float defaultVal, float ui_zoom = 1.0f, int division = 32, float alpha = 1.0f, float hideHueAlpha = 0.75f, float offset = 0.0f);
IMGUI_API void LumianceSelector(char const* label, ImVec2 const size, float* lumCenter, float defaultVal, float ui_zoom = 1.0f, int division = 32, float gamma = 1.f, bool rgb_color = false, ImVec4 const color = ImVec4(1, 1, 1, 1));
IMGUI_API void GammaSelector(char const* label, ImVec2 const size, float* gammaCenter, float defaultVal, float vmin, float vmax, float ui_zoom = 1.0f, int division = 32);
IMGUI_API void SaturationSelector(char const* label, ImVec2 const size, float* satCenter, float defaultVal, float ui_zoom = 1.0f, int division = 32, float gamma = 1.f, bool rgb_color = false, ImVec4 const color = ImVec4(1, 1, 1, 1));
IMGUI_API void ContrastSelector(char const* label, ImVec2 const size, float* conCenter, float defaultVal, float ui_zoom = 1.0f, bool rgb_color = false, ImVec4 const color = ImVec4(1, 1, 1, 1));
IMGUI_API void TemperatureSelector(char const* label, ImVec2 const size, float* tempCenter, float defaultVal, float vmin, float vmax, float ui_zoom = 1.0f, int division = 32);
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
IMGUI_API void ShowImKalmanDemoWindow();
IMGUI_API void ShowImFFTDemoWindow();
IMGUI_API void ShowImSTFTDemoWindow();
#endif
} // namespace ImGui

namespace ImGui
{
// Extensions to ImDrawList
IMGUI_API void ImDrawListAddConvexPolyFilledWithVerticalGradient(ImDrawList* dl, const ImVec2* points, const int points_count, ImU32 colTop, ImU32 colBot, float miny=-1.f, float maxy=-1.f);
IMGUI_API void ImDrawListPathFillWithVerticalGradientAndStroke(ImDrawList* dl, const ImU32& fillColorTop, const ImU32& fillColorBottom, const ImU32& strokeColor, bool strokeClosed=false, float strokeThickness = 1.0f, float miny=-1.f, float maxy=-1.f);
IMGUI_API void ImDrawListPathFillAndStroke(ImDrawList* dl,const ImU32& fillColor,const ImU32& strokeColor,bool strokeClosed=false, float strokeThickness = 1.0f);
IMGUI_API void ImDrawListAddRect(ImDrawList* dl,const ImVec2& a, const ImVec2& b,const ImU32& fillColor,const ImU32& strokeColor,float rounding = 0.0f, int rounding_corners = 0,float strokeThickness = 1.0f);
IMGUI_API void ImDrawListAddRectWithVerticalGradient(ImDrawList* dl,const ImVec2& a, const ImVec2& b,const ImU32& fillColorTop,const ImU32& fillColorBottom,const ImU32& strokeColor,float rounding = 0.0f, int rounding_corners = 0,float strokeThickness = 1.0f);
IMGUI_API void ImDrawListAddRectWithVerticalGradient(ImDrawList* dl,const ImVec2& a, const ImVec2& b,const ImU32& fillColor,float fillColorGradientDeltaIn0_05,const ImU32& strokeColor,float rounding = 0.0f, int rounding_corners = 0,float strokeThickness = 1.0f);
IMGUI_API void ImDrawListPathArcTo(ImDrawList* dl,const ImVec2& centre,const ImVec2& radii, float amin, float amax, int num_segments = 10);
IMGUI_API void ImDrawListAddEllipse(ImDrawList* dl,const ImVec2& centre, const ImVec2& radii,const ImU32& fillColor,const ImU32& strokeColor,int num_segments = 12,float strokeThickness = 1.f);
IMGUI_API void ImDrawListAddEllipseWithVerticalGradient(ImDrawList* dl, const ImVec2& centre, const ImVec2& radii, const ImU32& fillColorTop, const ImU32& fillColorBottom, const ImU32& strokeColor, int num_segments = 12, float strokeThickness = 1.f);
IMGUI_API void ImDrawListAddCircle(ImDrawList* dl,const ImVec2& centre, float radius,const ImU32& fillColor,const ImU32& strokeColor,int num_segments = 12,float strokeThickness = 1.f);
IMGUI_API void ImDrawListAddCircleWithVerticalGradient(ImDrawList* dl, const ImVec2& centre, float radius, const ImU32& fillColorTop, const ImU32& fillColorBottom, const ImU32& strokeColor, int num_segments = 12, float strokeThickness = 1.f);
// Overload of ImDrawList::addPolyLine(...) that takes offset and scale:
IMGUI_API void ImDrawListAddPolyLine(ImDrawList *dl,const ImVec2* polyPoints,int numPolyPoints,ImU32 strokeColor=IM_COL32_WHITE,float strokeThickness=1.f,bool strokeClosed=false, const ImVec2 &offset=ImVec2(0,0), const ImVec2& scale=ImVec2(1,1));
IMGUI_API void ImDrawListAddConvexPolyFilledWithHorizontalGradient(ImDrawList *dl, const ImVec2 *points, const int points_count, ImU32 colLeft, ImU32 colRight, float minx=-1.f, float maxx=-1.f);
IMGUI_API void ImDrawListPathFillWithHorizontalGradientAndStroke(ImDrawList *dl, const ImU32 &fillColorLeft, const ImU32 &fillColorRight, const ImU32 &strokeColor, bool strokeClosed=false, float strokeThickness = 1.0f, float minx=-1.f,float maxx=-1.f);
IMGUI_API void ImDrawListAddRectWithHorizontalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColorLeft, const ImU32 &fillColoRight, const ImU32 &strokeColor, float rounding = 0.0f, int rounding_corners = 0, float strokeThickness = 1.0f);
IMGUI_API void ImDrawListAddEllipseWithHorizontalGradient(ImDrawList *dl, const ImVec2 &centre, const ImVec2 &radii, const ImU32 &fillColorLeft, const ImU32 &fillColorRight, const ImU32 &strokeColor, int num_segments = 12, float strokeThickness = 1.0f);
IMGUI_API void ImDrawListAddCircleWithHorizontalGradient(ImDrawList *dl, const ImVec2 &centre, float radius, const ImU32 &fillColorLeft, const ImU32 &fillColorRight, const ImU32 &strokeColor, int num_segments = 12, float strokeThickness = 1.0f);
IMGUI_API void ImDrawListAddRectWithHorizontalGradient(ImDrawList *dl, const ImVec2 &a, const ImVec2 &b, const ImU32 &fillColor, float fillColorGradientDeltaIn0_05, const ImU32 &strokeColor, float rounding = 0.0f, int rounding_corners = 0, float strokeThickness = 1.0f);
// Add Dashed line or circle
IMGUI_API void ImDrawListAddLineDashed(ImDrawList *dl, const ImVec2& a, const ImVec2& b, ImU32 col, float thickness = 1.0f, unsigned int segments = 10, unsigned int on_segments = 1, unsigned int off_segments = 1);
IMGUI_API void ImDrawListAddCircleDashed(ImDrawList *dl, const ImVec2& centre, float radius, ImU32 col, int num_segments = 12, float thickness = 1.0f, int on_segments = 1, int off_segments = 1);
IMGUI_API void ImDrawListPathArcToDashedAndStroke(ImDrawList *dl, const ImVec2& centre, float radius, float a_min, float a_max, ImU32 col, float thickness = 1.0f, int num_segments = 10, int on_segments = 1, int off_segments = 1);
// Add Rotate Image
IMGUI_API void ImDrawListAddImageRotate(ImDrawList *dl, ImTextureID tex_id, ImVec2 pos, ImVec2 size, float angle, ImU32 board_col = IM_COL32(0, 0, 0, 255));
} // namespace ImGui

namespace ImGui
{
// Vertical Text Helper
IMGUI_API ImVec2    ImCalcVerticalTextSize(const char* text, const char* text_end = NULL, bool hide_text_after_double_hash = false, float wrap_width = -1.0f);
IMGUI_API void      ImRenderTextVertical(const ImFont* font,ImDrawList* draw_list, float size, ImVec2 pos, ImU32 col, const ImVec4& clip_rect, const char* text_begin,  const char* text_end = NULL, float wrap_width = 0.0f, bool cpu_fine_clip = false, bool rotateCCW = false);
IMGUI_API void      ImAddTextVertical(ImDrawList* drawList,const ImFont* font, float font_size, const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL, float wrap_width = 0.0f, const ImVec4* cpu_fine_clip_rect = NULL,bool rotateCCW = false);
IMGUI_API void      ImAddTextVertical(ImDrawList* drawList,const ImVec2& pos, ImU32 col, const char* text_begin, const char* text_end = NULL,bool rotateCCW = false);
IMGUI_API void      ImRenderTextVerticalClipped(const ImVec2& pos_min, const ImVec2& pos_max, const char* text, const char* text_end, const ImVec2* text_size_if_known, const ImVec2& align = ImVec2(0.0f,0.0f), const ImVec2* clip_min = NULL, const ImVec2* clip_max = NULL,bool rotateCCW = false);
} //namespace ImGui

namespace ImGui
{
// Posted by @alexsr here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
IMGUI_API void      LoadingIndicatorCircle(const char* label, float indicatorRadiusFactor=1.f,
                                        const ImVec4* pOptionalMainColor=NULL, const ImVec4* pOptionalBackdropColor=NULL,
                                        int circle_count=8, const float speed=1.f);
// Posted by @zfedoran here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
IMGUI_API void      LoadingIndicatorCircle2(const char* label, float indicatorRadiusFactor=1.f, float indicatorRadiusThicknessFactor=1.f, const ImVec4* pOptionalColor=NULL);
} // namespace ImGui

namespace ImGui
{
class IMGUI_API Piano {
public:
    int key_states[256] = {0};
    void up(int key);
    void draw_keyboard(ImVec2 size, bool input = false);
    void down(int key, int velocity);
    void reset();
};
} // namespace ImGui

namespace ImGui
{

IMGUI_API void  ImSpectrogram(const ImMat& in_mat, ImMat& out_mat, int window = 512, bool stft = false, int hope = 128);

} // namespace ImGui
#endif // IMGUI_WIDGET_H

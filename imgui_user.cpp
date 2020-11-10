#include "imgui.h"
#ifndef IMGUI_DISABLE
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"
#include <string>
#include <cmath>
#include <ctype.h>      // toupper
#if defined(_MSC_VER) && _MSC_VER <= 1500 // MSVC 2008 or earlier
#include <stddef.h>     // intptr_t
#else
#include <stdint.h>     // intptr_t
#endif

//-------------------------------------------------------------------------
void ImGui::ToggleButton(const char* str_id, bool* v)
{
    ImVec2 p = ImGui::GetCursorScreenPos();
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    float height = ImGui::GetFrameHeight();
    float width = height * 1.55f;
    float radius = height * 0.50f;

    ImGui::InvisibleButton(str_id, ImVec2(width, height));
    if (ImGui::IsItemClicked())
        *v = !*v;

    float t = *v ? 1.0f : 0.0f;

    ImGuiContext& g = *GImGui;
    float ANIM_SPEED = 0.08f;
    if (g.LastActiveId == g.CurrentWindow->GetID(str_id))
    {
        float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
        t = *v ? (t_anim) : (1.0f - t_anim);
    }

    ImU32 col_bg;
    if (ImGui::IsItemHovered())
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
    else
        col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

    draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
    draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
}

//-------------------------------------------------------------------------

bool ImGui::BufferingBar(const char* label, float value,  const ImVec2& size_arg, const ImU32& bg_col, const ImU32& fg_col)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return false;
    
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size = size_arg;
    size.x -= style.FramePadding.x * 2;
    
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ImGui::ItemSize(bb, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, id))
        return false;
    
    // Render
    const float circleStart = size.x * 0.7f;
    const float circleEnd = size.x;
    const float circleWidth = circleEnd - circleStart;
    
    window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart, bb.Max.y), bg_col);
    window->DrawList->AddRectFilled(bb.Min, ImVec2(pos.x + circleStart*value, bb.Max.y), fg_col);
    
    const float t = g.Time;
    const float r = size.y / 2;
    const float speed = 1.5f;
    
    const float a = speed*0;
    const float b = speed*0.333f;
    const float c = speed*0.666f;
    
    const float o1 = (circleWidth+r) * (t+a - speed * (int)((t+a) / speed)) / speed;
    const float o2 = (circleWidth+r) * (t+b - speed * (int)((t+b) / speed)) / speed;
    const float o3 = (circleWidth+r) * (t+c - speed * (int)((t+c) / speed)) / speed;
    
    window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o1, bb.Min.y + r), r, bg_col);
    window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o2, bb.Min.y + r), r, bg_col);
    window->DrawList->AddCircleFilled(ImVec2(pos.x + circleEnd - o3, bb.Min.y + r), r, bg_col);
    return true;
}

//-------------------------------------------------------------------------
bool ImGui::Spinner(const char* label, float radius, int thickness, const ImU32& color) 
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    
    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size((radius )*2, (radius + style.FramePadding.y)*2);
    
    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return false;
    
    // Render
    window->DrawList->PathClear();
    
    int num_segments = 30;
    int start = abs(ImSin(g.Time*1.8f)*(num_segments-5));
    
    const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
    const float a_max = IM_PI*2.0f * ((float)num_segments-3) / (float)num_segments;
    const ImVec2 centre = ImVec2(pos.x+radius, pos.y+radius+style.FramePadding.y);
    
    for (int i = 0; i < num_segments; i++) {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a+g.Time*8) * radius,
                                            centre.y + ImSin(a+g.Time*8) * radius));
    }
    window->DrawList->PathStroke(color, false, thickness);
    return true;
}

//-------------------------------------------------------------------------
void ImGui::LoadingIndicatorCircle(const char* label, const float indicator_radius,
                                const ImVec4& main_color, const ImVec4& backdrop_color,
                                const int circle_count, const float speed)
{
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    ImGuiStyle& style = ImGui::GetStyle();
    const ImGuiID id = window->GetID(label);

    const ImVec2 pos = window->DC.CursorPos;
    const float circle_radius = indicator_radius / 10.0f;
    const ImRect bb(pos, ImVec2(pos.x + indicator_radius * 2.0f,
                                pos.y + indicator_radius * 2.0f));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return;

    const float t = g.Time;
    const auto degree_offset = 2.0f * IM_PI / circle_count;
    for (int i = 0; i < circle_count; ++i) {
        const auto x = indicator_radius * std::sin(degree_offset * i);
        const auto y = indicator_radius * std::cos(degree_offset * i);
        const auto growth = std::max(0.0f, std::sin(t * speed - i * degree_offset));
        ImVec4 color;
        color.x = main_color.x * growth + backdrop_color.x * (1.0f - growth);
        color.y = main_color.y * growth + backdrop_color.y * (1.0f - growth);
        color.z = main_color.z * growth + backdrop_color.z * (1.0f - growth);
        color.w = 1.0f;
        window->DrawList->AddCircleFilled(ImVec2(pos.x + indicator_radius + x,
                                                pos.y + indicator_radius - y),
                                                circle_radius + growth * circle_radius,
                                                GetColorU32(color));
    }
}

//-------------------------------------------------------------------------
bool ImGui::SpinScaler(const char* label, ImGuiDataType data_type, void* data_ptr, const void* step, const void* step_fast, const char* format, ImGuiInputTextFlags flags)
{
	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	ImGuiStyle& style = g.Style;

	if (format == NULL)
		format = DataTypeGetInfo(data_type)->PrintFmt;

	char buf[64];
	DataTypeFormatString(buf, IM_ARRAYSIZE(buf), data_type, data_ptr, format);

	bool value_changed = false;
	if ((flags & (ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsScientific)) == 0)
		flags |= ImGuiInputTextFlags_CharsDecimal;
	flags |= ImGuiInputTextFlags_AutoSelectAll;
	flags |= ImGuiInputTextFlags_NoMarkEdited;  // We call MarkItemEdited() ourselve by comparing the actual data rather than the string.

	if (step != NULL)
	{
		const float button_size = GetFrameHeight();

		BeginGroup(); // The only purpose of the group here is to allow the caller to query item data e.g. IsItemActive()
		PushID(label);
		SetNextItemWidth(ImMax(1.0f, CalcItemWidth() - (button_size + style.ItemInnerSpacing.x) * 2));
		if (InputText("", buf, IM_ARRAYSIZE(buf), flags)) // PushId(label) + "" gives us the expected ID from outside point of view
			value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, data_ptr, format);

		// Step buttons
		const ImVec2 backup_frame_padding = style.FramePadding;
		style.FramePadding.x = style.FramePadding.y;
		ImGuiButtonFlags button_flags = ImGuiButtonFlags_Repeat | ImGuiButtonFlags_DontClosePopups;
		if (flags & ImGuiInputTextFlags_ReadOnly)
			button_flags |= ImGuiButtonFlags_Disabled;
		SameLine(0, style.ItemInnerSpacing.x);

        // start diffs
		float frame_height = GetFrameHeight();
		float arrow_size = std::floor(frame_height * .45f);
		float arrow_spacing = frame_height - 2.0f * arrow_size;

		BeginGroup();
		PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{g.Style.ItemSpacing.x, arrow_spacing});

		// save/change font size to draw arrow buttons correctly
		float org_font_size = GetDrawListSharedData()->FontSize;
		GetDrawListSharedData()->FontSize = arrow_size;

		if (ArrowButtonEx("+", ImGuiDir_Up, ImVec2(arrow_size, arrow_size), button_flags))
		{
			DataTypeApplyOp(data_type, '+', data_ptr, data_ptr, g.IO.KeyCtrl && step_fast ? step_fast : step);
			value_changed = true;
		}

		if (ArrowButtonEx("-", ImGuiDir_Down, ImVec2(arrow_size, arrow_size), button_flags))
		{
			DataTypeApplyOp(data_type, '-', data_ptr, data_ptr, g.IO.KeyCtrl && step_fast ? step_fast : step);
			value_changed = true;
		}

		// restore font size
		GetDrawListSharedData()->FontSize = org_font_size;

		PopStyleVar(1);
		EndGroup();
        // end diffs

		const char* label_end = FindRenderedTextEnd(label);
		if (label != label_end)
		{
			SameLine(0, style.ItemInnerSpacing.x);
			TextEx(label, label_end);
		}
		style.FramePadding = backup_frame_padding;

		PopID();
		EndGroup();
	}
	else
	{
		if (InputText(label, buf, IM_ARRAYSIZE(buf), flags))
			value_changed = DataTypeApplyOpFromText(buf, g.InputTextState.InitialTextA.Data, data_type, data_ptr, format);
	}
	if (value_changed)
		MarkItemEdited(window->DC.LastItemId);

	return value_changed;
}

bool ImGui::SpinInt(const char* label, int* v, int step, int step_fast, ImGuiInputTextFlags flags)
{
	// Hexadecimal input provided as a convenience but the flag name is awkward. Typically you'd use InputText() to parse your own data, if you want to handle prefixes.
	const char* format = (flags & ImGuiInputTextFlags_CharsHexadecimal) ? "%08X" : "%d";
	return SpinScaler(label, ImGuiDataType_S32, (void*)v, (void*)(step>0 ? &step : NULL), (void*)(step_fast>0 ? &step_fast : NULL), format, flags);
}

bool ImGui::SpinFloat(const char* label, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags)
{
	flags |= ImGuiInputTextFlags_CharsScientific;
	return SpinScaler(label, ImGuiDataType_Float, (void*)v, (void*)(step>0.0f ? &step : NULL), (void*)(step_fast>0.0f ? &step_fast : NULL), format, flags);
}

bool ImGui::SpinDouble(const char* label, double* v, double step, double step_fast, const char* format, ImGuiInputTextFlags flags)
{
	flags |= ImGuiInputTextFlags_CharsScientific;
	return SpinScaler(label, ImGuiDataType_Double, (void*)v, (void*)(step>0.0 ? &step : NULL), (void*)(step_fast>0.0 ? &step_fast : NULL), format, flags);
}

//-------------------------------------------------------------------------
bool ImGui::Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size)
{
    using namespace ImGui;
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = g.CurrentWindow;
    ImGuiID id = window->GetID("##Splitter");
    ImRect bb;
    bb.Min = window->DC.CursorPos + (split_vertically ? ImVec2(*size1, 0.0f) : ImVec2(0.0f, *size1));
    bb.Max = bb.Min + CalcItemSize(split_vertically ? ImVec2(thickness, splitter_long_axis_size) : ImVec2(splitter_long_axis_size, thickness), 0.0f, 0.0f);
    return SplitterBehavior(bb, id, split_vertically ? ImGuiAxis_X : ImGuiAxis_Y, size1, size2, min_size1, min_size2, 0.0f);
}

#if 0
bool ImGui::SliderScalar2D(char const* pLabel, float* fValueX, float* fValueY, const float fMinX, const float fMaxX, const float fMinY, const float fMaxY, float const fZoom /*= 1.0f*/)
{
    assert(fMinX < fMaxX);
    assert(fMinY < fMaxY);
    
    ImGuiID const iID = ImGui::GetID(pLabel);
    
    ImVec2 const vSizeSubstract = ImGui::CalcTextSize(std::to_string(1.0f).c_str()) * 1.1f;
    
    float const vSizeFull = (ImGui::GetWindowContentRegionWidth() - vSizeSubstract.x)*fZoom;
    ImVec2 const vSize(vSizeFull, vSizeFull);
    
    float const fHeightOffset = ImGui::GetTextLineHeight();
    ImVec2 const vHeightOffset(0.0f, fHeightOffset);
    
    ImVec2 vPos = GetCursorScreenPos();
    ImRect oRect(vPos + vHeightOffset, vPos + vSize + vHeightOffset);
    
    ImGui::Text("%s", pLabel);
    
    ImGui::PushID(iID);
    
    ImU32 const uFrameCol = ImGui::GetColorU32(ImGuiCol_FrameBg);
    
    ImVec2 const vOriginPos = ImGui::GetCursorScreenPos();
    ImGui::RenderFrame(oRect.Min, oRect.Max, uFrameCol, false, 0.0f);
    
    float const fDeltaX = fMaxX - fMinX;
    float const fDeltaY = fMaxY - fMinY;
    
    bool bModified = false;
    ImVec2 const vSecurity(15.0f, 15.0f);
    if (ImGui::IsMouseHoveringRect(oRect.Min - vSecurity, oRect.Max + vSecurity) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        ImVec2 const vCursorPos = ImGui::GetMousePos() - oRect.Min;
        
        *fValueX = vCursorPos.x/(oRect.Max.x - oRect.Min.x)*fDeltaX + fMinX;
        *fValueY = fDeltaY - vCursorPos.y/(oRect.Max.y - oRect.Min.y)*fDeltaY + fMinY;
        
        bModified = true;
    }
    
    *fValueX = std::min(std::max(*fValueX, fMinX), fMaxX);
    *fValueY = std::min(std::max(*fValueY, fMinY), fMaxY);
    
    float const fScaleX = (*fValueX - fMinX)/fDeltaX;
    float const fScaleY = 1.0f - (*fValueY - fMinY)/fDeltaY;
    
    constexpr float fCursorOff = 10.0f;
    float const fXLimit = fCursorOff/oRect.GetWidth();
    float const fYLimit = fCursorOff/oRect.GetHeight();
    
    ImVec2 const vCursorPos((oRect.Max.x - oRect.Min.x)*fScaleX + oRect.Min.x, (oRect.Max.y - oRect.Min.y)*fScaleY + oRect.Min.y);
    
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    
    ImVec4 const vBlue	(  70.0f/255.0f, 102.0f/255.0f, 230.0f/255.0f, 1.0f ); // TODO: choose from style
    ImVec4 const vOrange( 255.0f/255.0f, 128.0f/255.0f,  64.0f/255.0f, 1.0f ); // TODO: choose from style
    
    ImS32 const uBlue	= ImGui::GetColorU32(vBlue);
    ImS32 const uOrange	= ImGui::GetColorU32(vOrange);
    
    constexpr float fBorderThickness	= 2.0f;
    constexpr float fLineThickness		= 3.0f;
    constexpr float fHandleRadius		= 7.0f;
    constexpr float fHandleOffsetCoef	= 2.0f;
    
    // Cursor
    pDrawList->AddCircleFilled(vCursorPos, 5.0f, uBlue, 16);
    
    // Vertical Line
    if (fScaleY > 2.0f*fYLimit)
        pDrawList->AddLine(ImVec2(vCursorPos.x, oRect.Min.y + fCursorOff), ImVec2(vCursorPos.x, vCursorPos.y - fCursorOff), uOrange, fLineThickness);
    if (fScaleY < 1.0f - 2.0f*	fYLimit)
        pDrawList->AddLine(ImVec2(vCursorPos.x, oRect.Max.y - fCursorOff), ImVec2(vCursorPos.x, vCursorPos.y + fCursorOff), uOrange, fLineThickness);
    
    // Horizontal Line
    if (fScaleX > 2.0f*fXLimit)
        pDrawList->AddLine(ImVec2(oRect.Min.x + fCursorOff, vCursorPos.y), ImVec2(vCursorPos.x - fCursorOff, vCursorPos.y), uOrange, fLineThickness);
    if (fScaleX < 1.0f - 2.0f*fYLimit)
        pDrawList->AddLine(ImVec2(oRect.Max.x - fCursorOff, vCursorPos.y), ImVec2(vCursorPos.x + fCursorOff, vCursorPos.y), uOrange, fLineThickness);
    
    char pBufferX[16];
    char pBufferY[16];
    ImFormatString(pBufferX, IM_ARRAYSIZE(pBufferX), "%.5f", *(float const*)fValueX);
    ImFormatString(pBufferY, IM_ARRAYSIZE(pBufferY), "%.5f", *(float const*)fValueY);
    
    ImU32 const uTextCol = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);
    
    ImGui::SetWindowFontScale(0.75f);
    
    ImVec2 const vXSize = ImGui::CalcTextSize(pBufferX);
    ImVec2 const vYSize = ImGui::CalcTextSize(pBufferY);
    
    ImVec2 const vHandlePosX = ImVec2(vCursorPos.x, oRect.Max.y + vYSize.x*0.5f);
    ImVec2 const vHandlePosY = ImVec2(oRect.Max.x + fHandleOffsetCoef * fCursorOff + vYSize.x, vCursorPos.y);
    
    if (ImGui::IsMouseHoveringRect(vHandlePosX - ImVec2(fHandleRadius, fHandleRadius) - vSecurity, vHandlePosX + ImVec2(fHandleRadius, fHandleRadius) + vSecurity) &&
        ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        ImVec2 const vCursorPos = ImGui::GetMousePos() - oRect.Min;
        
        *fValueX = vCursorPos.x/(oRect.Max.x - oRect.Min.x)*fDeltaX + fMinX;
        
        bModified = true;
    }
    else if (ImGui::IsMouseHoveringRect(vHandlePosY - ImVec2(fHandleRadius, fHandleRadius) - vSecurity, vHandlePosY + ImVec2(fHandleRadius, fHandleRadius) + vSecurity) &&
            ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        ImVec2 const vCursorPos = ImGui::GetMousePos() - oRect.Min;
        
        *fValueY = fDeltaY - vCursorPos.y/(oRect.Max.y - oRect.Min.y)*fDeltaY + fMinY;
        
        bModified = true;
    }
    
    pDrawList->AddText(
                    ImVec2(
                            std::min(std::max(vCursorPos.x - vXSize.x*0.5f, oRect.Min.x), oRect.Min.x + vSize.x - vXSize.x),
                            oRect.Max.y + fCursorOff),
                    uTextCol,
                    pBufferX);
    pDrawList->AddText(
                    ImVec2(oRect.Max.x + fCursorOff, std::min(std::max(vCursorPos.y - vYSize.y*0.5f, oRect.Min.y),
                                                                oRect.Min.y + vSize.y - vYSize.y)),
                    uTextCol,
                    pBufferY);
    ImGui::SetWindowFontScale(1.0f);
    
    // Borders::Right
    pDrawList->AddCircleFilled(ImVec2(oRect.Max.x, vCursorPos.y), 2.0f, uOrange, 3);
    // Handle Right::Y
    pDrawList->AddNgonFilled(ImVec2(oRect.Max.x + fHandleOffsetCoef*fCursorOff + vYSize.x, vCursorPos.y), fHandleRadius, uBlue, 4);
    if (fScaleY > fYLimit)
        pDrawList->AddLine(ImVec2(oRect.Max.x, oRect.Min.y), ImVec2(oRect.Max.x, vCursorPos.y - fCursorOff), uBlue, fBorderThickness);
    if (fScaleY < 1.0f - fYLimit)
        pDrawList->AddLine(ImVec2(oRect.Max.x, oRect.Max.y), ImVec2(oRect.Max.x, vCursorPos.y + fCursorOff), uBlue, fBorderThickness);
    // Borders::Top
    pDrawList->AddCircleFilled(ImVec2(vCursorPos.x, oRect.Min.y), 2.0f, uOrange, 3);
    if (fScaleX > fXLimit)
        pDrawList->AddLine(ImVec2(oRect.Min.x, oRect.Min.y), ImVec2(vCursorPos.x - fCursorOff, oRect.Min.y), uBlue, fBorderThickness);
    if (fScaleX < 1.0f - fXLimit)
        pDrawList->AddLine(ImVec2(oRect.Max.x, oRect.Min.y), ImVec2(vCursorPos.x + fCursorOff, oRect.Min.y), uBlue, fBorderThickness);
    // Borders::Left
    pDrawList->AddCircleFilled(ImVec2(oRect.Min.x, vCursorPos.y), 2.0f, uOrange, 3);
    if (fScaleY > fYLimit)
        pDrawList->AddLine(ImVec2(oRect.Min.x, oRect.Min.y), ImVec2(oRect.Min.x, vCursorPos.y - fCursorOff), uBlue, fBorderThickness);
    if (fScaleY < 1.0f - fYLimit)
        pDrawList->AddLine(ImVec2(oRect.Min.x, oRect.Max.y), ImVec2(oRect.Min.x, vCursorPos.y + fCursorOff), uBlue, fBorderThickness);
    // Borders::Bottom
    pDrawList->AddCircleFilled(ImVec2(vCursorPos.x, oRect.Max.y), 2.0f, uOrange, 3);
    // Handle Bottom::X
    pDrawList->AddNgonFilled(ImVec2(vCursorPos.x, oRect.Max.y + vXSize.y*2.0f), fHandleRadius, uBlue, 4);
    if (fScaleX > fXLimit)
        pDrawList->AddLine(ImVec2(oRect.Min.x, oRect.Max.y), ImVec2(vCursorPos.x - fCursorOff, oRect.Max.y), uBlue, fBorderThickness);
    if (fScaleX < 1.0f - fXLimit)
        pDrawList->AddLine(ImVec2(oRect.Max.x, oRect.Max.y), ImVec2(vCursorPos.x + fCursorOff, oRect.Max.y), uBlue, fBorderThickness);
    
    ImGui::PopID();
    
    ImGui::Dummy(vHeightOffset);
    ImGui::Dummy(vHeightOffset);
    ImGui::Dummy(vSize);
    
    char pBufferID[64];
    ImFormatString(pBufferID, IM_ARRAYSIZE(pBufferID), "Values##%d", *(ImS32 const*)&iID);
    
    if (ImGui::CollapsingHeader(pBufferID))
    {
        float const fSpeedX = fDeltaX/64.0f;
        float const fSpeedY = fDeltaY/64.0f;
        
        char pBufferXID[64];
        ImFormatString(pBufferXID, IM_ARRAYSIZE(pBufferXID), "X##%d", *(ImS32 const*)&iID);
        char pBufferYID[64];
        ImFormatString(pBufferYID, IM_ARRAYSIZE(pBufferYID), "Y##%d", *(ImS32 const*)&iID);
        
        bModified |= ImGui::DragScalar(pBufferXID, ImGuiDataType_Float, fValueX, fSpeedX, &fMinX, &fMaxX);
        bModified |= ImGui::DragScalar(pBufferYID, ImGuiDataType_Float, fValueY, fSpeedY, &fMinY, &fMaxY);
    }
    
    return bModified;
}

bool ImGui::SliderScalar3D(char const* pLabel, float* pValueX, float* pValueY, float* pValueZ, float const fMinX, float const fMaxX, float const fMinY, float const fMaxY, float const fMinZ, float const fMaxZ, float const fScale /*= 1.0f*/)
{
    assert(fMinX < fMaxX);
    assert(fMinY < fMaxY);
    assert(fMinZ < fMaxZ);
    
    ImGuiID const iID = ImGui::GetID(pLabel);
    
    ImVec2 const vSizeSubstract = ImGui::CalcTextSize(std::to_string(1.0f).c_str()) * 1.1f;
    
    float const vSizeFull = ImGui::GetWindowContentRegionWidth();
    float const fMinSize = (vSizeFull - vSizeSubstract.x*0.5f)*fScale*0.75f;
    ImVec2 const vSize(fMinSize, fMinSize);
    
    float const fHeightOffset = ImGui::GetTextLineHeight();
    ImVec2 const vHeightOffset(0.0f, fHeightOffset);
    
    ImVec2 vPos = GetCursorScreenPos();
    ImRect oRect(vPos + vHeightOffset, vPos + vSize + vHeightOffset);
    
    ImGui::Text("%s", pLabel);
    
    ImGui::PushID(iID);
    
    ImU32 const uFrameCol	= ImGui::GetColorU32(ImGuiCol_FrameBg) | 0xFF000000;
    ImU32 const uFrameCol2	= ImGui::GetColorU32(ImGuiCol_FrameBgActive);
    
    float& fX = *pValueX;
    float& fY = *pValueY;
    float& fZ = *pValueZ;
    
    float const fDeltaX = fMaxX - fMinX;
    float const fDeltaY = fMaxY - fMinY;
    float const fDeltaZ = fMaxZ - fMinZ;
    
    ImVec2 const vOriginPos = ImGui::GetCursorScreenPos();
    
    ImDrawList* pDrawList = ImGui::GetWindowDrawList();
    
    float const fX3 = vSize.x/3.0f;
    float const fY3 = vSize.y/3.0f;
    
    ImVec2 const vStart = oRect.Min;
    
    ImVec2 aPositions[] = {
        ImVec2(vStart.x,			vStart.y + fX3),
        ImVec2(vStart.x,			vStart.y + vSize.y),
        ImVec2(vStart.x + 2.0f*fX3,	vStart.y + vSize.y),
        ImVec2(vStart.x + vSize.x,	vStart.y + 2.0f*fY3),
        ImVec2(vStart.x + vSize.x,	vStart.y),
        ImVec2(vStart.x + fX3,		vStart.y)
    };
    
    pDrawList->AddPolyline(&aPositions[0], 6, uFrameCol2, true, 1.0f);
    
    // Cube Shape
    pDrawList->AddLine(
                    oRect.Min + ImVec2(0.0f, vSize.y),
                    oRect.Min + ImVec2(fX3, 2.0f*fY3), uFrameCol2, 1.0f);
    pDrawList->AddLine(
                    oRect.Min + ImVec2(fX3, 2.0f*fY3),
                    oRect.Min + ImVec2(vSize.x, 2.0f*fY3), uFrameCol2, 1.0f);
    pDrawList->AddLine(
                    oRect.Min + ImVec2(fX3, 0.0f),
                    oRect.Min + ImVec2(fX3, 2.0f*fY3), uFrameCol2, 1.0f);
    
    ImGui::PopID();
    
    constexpr float fDragZOffsetX = 64.0f;
    
    ImRect oZDragRect(ImVec2(vStart.x + 2.0f*fX3 + fDragZOffsetX, vStart.y + 2.0f*fY3), ImVec2(vStart.x + vSize.x + fDragZOffsetX, vStart.y + vSize.y));
    
    ImVec2 const vMousePos = ImGui::GetMousePos();
    ImVec2 const vSecurity(15.0f, 15.0f);
    ImVec2 const vDragStart	(oZDragRect.Min.x, oZDragRect.Max.y);
    ImVec2 const vDragEnd	(oZDragRect.Max.x, oZDragRect.Min.y);
    bool bModified = false;
    if (ImGui::IsMouseHoveringRect(oZDragRect.Min - vSecurity, oZDragRect.Max + vSecurity) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        if (DistToSegmentSqr(vMousePos, vDragStart, vDragEnd) < 100.0f) // 100 is arbitrary threshold
        {
            float const fMaxDist	= std::sqrt(Dist2(vDragStart, vDragEnd));
            float const fDist		= std::max(std::min(std::sqrt(DistOnSegmentSqr(vMousePos, vDragStart, vDragEnd))/fMaxDist, 1.0f), 0.0f);
            
            fZ = fDist*fDeltaZ*fDist + fMinZ;
            
            bModified = true;
        }
    }
    
    float const fScaleZ = (fZ - fMinZ)/fDeltaZ;
    
    ImVec2 const vRectStart	(vStart.x, vStart.y + fX3);
    ImVec2 const vRectEnd	(vStart.x + fX3, vStart.y);
    ImRect const oXYDrag((vRectEnd - vRectStart)*fScaleZ + vRectStart,
                        (vRectEnd - vRectStart)*fScaleZ + vRectStart + ImVec2(2.0f*fX3, 2.0f*fY3));
    if (ImGui::IsMouseHoveringRect(oXYDrag.Min - vSecurity, oXYDrag.Max + vSecurity) && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        ImVec2 const vLocalPos = ImGui::GetMousePos() - oXYDrag.Min;
        
        fX = vLocalPos.x/(oXYDrag.Max.x - oXYDrag.Min.x)*fDeltaX + fMinX;
        fY = fDeltaY - vLocalPos.y/(oXYDrag.Max.y - oXYDrag.Min.y)*fDeltaY + fMinY;
        
        bModified = true;
    }
    
    fX = std::min(std::max(fX, fMinX), fMaxX);
    fY = std::min(std::max(fY, fMinY), fMaxY);
    fZ = std::min(std::max(fZ, fMinZ), fMaxZ);
    
    float const fScaleX = (fX - fMinX)/fDeltaX;
    float const fScaleY = 1.0f - (fY - fMinY)/fDeltaY;
    
    ImVec4 const vBlue	(  70.0f/255.0f, 102.0f/255.0f, 230.0f/255.0f, 1.0f );
    ImVec4 const vOrange( 255.0f/255.0f, 128.0f/255.0f,  64.0f/255.0f, 1.0f );
    
    ImS32 const uBlue	= ImGui::GetColorU32(vBlue);
    ImS32 const uOrange	= ImGui::GetColorU32(vOrange);
    
    constexpr float fBorderThickness	= 2.0f; // TODO: move as Style
    constexpr float fLineThickness		= 3.0f; // TODO: move as Style
    constexpr float fHandleRadius		= 7.0f; // TODO: move as Style
    constexpr float fHandleOffsetCoef	= 2.0f; // TODO: move as Style
    
    pDrawList->AddLine(
                        vDragStart,
                        vDragEnd, uFrameCol2, 1.0f);
    pDrawList->AddRectFilled(
                            oXYDrag.Min, oXYDrag.Max, uFrameCol);
    
    constexpr float fCursorOff = 10.0f;
    float const fXLimit = fCursorOff/oXYDrag.GetWidth();
    float const fYLimit = fCursorOff/oXYDrag.GetHeight();
    float const fZLimit = fCursorOff/oXYDrag.GetWidth();
    
    char pBufferX[16];
    char pBufferY[16];
    char pBufferZ[16];
    ImFormatString(pBufferX, IM_ARRAYSIZE(pBufferX), "%.5f", *(float const*)&fX);
    ImFormatString(pBufferY, IM_ARRAYSIZE(pBufferY), "%.5f", *(float const*)&fY);
    ImFormatString(pBufferZ, IM_ARRAYSIZE(pBufferZ), "%.5f", *(float const*)&fZ);
    
    ImU32 const uTextCol = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Text]);
    
    ImVec2 const vCursorPos((oXYDrag.Max.x - oXYDrag.Min.x)*fScaleX + oXYDrag.Min.x, (oXYDrag.Max.y - oXYDrag.Min.y)*fScaleY + oXYDrag.Min.y);
    
    ImGui::SetWindowFontScale(0.75f);
    
    ImVec2 const vXSize = ImGui::CalcTextSize(pBufferX);
    ImVec2 const vYSize = ImGui::CalcTextSize(pBufferY);
    ImVec2 const vZSize = ImGui::CalcTextSize(pBufferZ);
    
    ImVec2 const vTextSlideXMin	= oRect.Min + ImVec2(0.0f, vSize.y);
    ImVec2 const vTextSlideXMax	= oRect.Min + ImVec2(2.0f*fX3, vSize.y);
    ImVec2 const vXTextPos((vTextSlideXMax - vTextSlideXMin)*fScaleX + vTextSlideXMin);
    
    ImVec2 const vTextSlideYMin	= oRect.Min + ImVec2(vSize.x, 2.0f*fY3);
    ImVec2 const vTextSlideYMax	= oRect.Min + ImVec2(vSize.x, 0.0f);
    ImVec2 const vYTextPos((vTextSlideYMax - vTextSlideYMin)*(1.0f - fScaleY) + vTextSlideYMin);
    
    ImVec2 const vTextSlideZMin = vDragStart + ImVec2(fCursorOff, fCursorOff);
    ImVec2 const vTextSlideZMax = vDragEnd   + ImVec2(fCursorOff, fCursorOff);
    ImVec2 const vZTextPos((vTextSlideZMax - vTextSlideZMin)*fScaleZ + vTextSlideZMin);
    
    ImVec2 const vHandlePosX = vXTextPos + ImVec2(0.0f, vXSize.y + fHandleOffsetCoef*fCursorOff);
    ImVec2 const vHandlePosY = vYTextPos + ImVec2(vYSize.x + (fHandleOffsetCoef + 1.0f)*fCursorOff, 0.0f);
    
    if (ImGui::IsMouseHoveringRect(vHandlePosX - ImVec2(fHandleRadius, fHandleRadius) - vSecurity, vHandlePosX + ImVec2(fHandleRadius, fHandleRadius) + vSecurity) &&
        ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        float const fCursorPosX = ImGui::GetMousePos().x - vStart.x;
        
        fX = fDeltaX*fCursorPosX/(2.0f*fX3) + fMinX;
        
        bModified = true;
    }
    else if (ImGui::IsMouseHoveringRect(vHandlePosY - ImVec2(fHandleRadius, fHandleRadius) - vSecurity, vHandlePosY + ImVec2(fHandleRadius, fHandleRadius) + vSecurity) &&
            ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        float const fCursorPosY = ImGui::GetMousePos().y - vStart.y;
        
        fY = fDeltaY*(1.0f - fCursorPosY/(2.0f*fY3)) + fMinY;
        
        bModified = true;
    }
    
    pDrawList->AddText(
                    ImVec2(
                            std::min(std::max(vXTextPos.x - vXSize.x*0.5f, vTextSlideXMin.x), vTextSlideXMax.x - vXSize.x),
                            vXTextPos.y + fCursorOff),
                    uTextCol,
                    pBufferX);
    pDrawList->AddText(
                    ImVec2(
                            vYTextPos.x + fCursorOff,
                            std::min(std::max(vYTextPos.y - vYSize.y*0.5f, vTextSlideYMax.y), vTextSlideYMin.y - vYSize.y)),
                    uTextCol,
                    pBufferY);
    pDrawList->AddText(
                        vZTextPos,
                        uTextCol,
                        pBufferZ);
    ImGui::SetWindowFontScale(1.0f);
    
    // Handles
    pDrawList->AddNgonFilled(vHandlePosX, fHandleRadius, uBlue, 4);
    pDrawList->AddNgonFilled(vHandlePosY, fHandleRadius, uBlue, 4);
    
    // Vertical Line
    if (fScaleY > 2.0f*fYLimit)
        pDrawList->AddLine(ImVec2(vCursorPos.x, oXYDrag.Min.y + fCursorOff), ImVec2(vCursorPos.x, vCursorPos.y - fCursorOff), uOrange, fLineThickness);
    if (fScaleY < 1.0f - 2.0f*	fYLimit)
        pDrawList->AddLine(ImVec2(vCursorPos.x, oXYDrag.Max.y - fCursorOff), ImVec2(vCursorPos.x, vCursorPos.y + fCursorOff), uOrange, fLineThickness);
    
    // Horizontal Line
    if (fScaleX > 2.0f*fXLimit)
        pDrawList->AddLine(ImVec2(oXYDrag.Min.x + fCursorOff, vCursorPos.y), ImVec2(vCursorPos.x - fCursorOff, vCursorPos.y), uOrange, fLineThickness);
    if (fScaleX < 1.0f - 2.0f*fYLimit)
        pDrawList->AddLine(ImVec2(oXYDrag.Max.x - fCursorOff, vCursorPos.y), ImVec2(vCursorPos.x + fCursorOff, vCursorPos.y), uOrange, fLineThickness);
    
    // Line To Text
    // X
    if (fScaleZ > 2.0f*fZLimit)
        pDrawList->AddLine(
                            ImVec2(vCursorPos.x - 0.5f*fCursorOff, oXYDrag.Max.y + 0.5f*fCursorOff),
                            ImVec2(vXTextPos.x + 0.5f*fCursorOff, vXTextPos.y - 0.5f*fCursorOff), uOrange, fLineThickness
                            );
    else
        pDrawList->AddLine(
                            ImVec2(vCursorPos.x, oXYDrag.Max.y),
                            ImVec2(vXTextPos.x, vXTextPos.y), uOrange, 1.0f
                            );
    pDrawList->AddCircleFilled(vXTextPos, 2.0f, uOrange, 3);
    // Y
    if (fScaleZ < 1.0f - 2.0f*fZLimit)
        pDrawList->AddLine(
                            ImVec2(oXYDrag.Max.x + 0.5f*fCursorOff, vCursorPos.y - 0.5f*fCursorOff),
                            ImVec2(vYTextPos.x - 0.5f*fCursorOff, vYTextPos.y + 0.5f*fCursorOff), uOrange, fLineThickness
                            );
    else
        pDrawList->AddLine(
                            ImVec2(oXYDrag.Max.x, vCursorPos.y),
                            ImVec2(vYTextPos.x, vYTextPos.y), uOrange, 1.0f
                            );
    pDrawList->AddCircleFilled(vYTextPos, 2.0f, uOrange, 3);
    
    // Borders::Right
    pDrawList->AddCircleFilled(ImVec2(oXYDrag.Max.x, vCursorPos.y), 2.0f, uOrange, 3);
    if (fScaleY > fYLimit)
        pDrawList->AddLine(ImVec2(oXYDrag.Max.x, oXYDrag.Min.y), ImVec2(oXYDrag.Max.x, vCursorPos.y - fCursorOff), uBlue, fBorderThickness);
    if (fScaleY < 1.0f - fYLimit)
        pDrawList->AddLine(ImVec2(oXYDrag.Max.x, oXYDrag.Max.y), ImVec2(oXYDrag.Max.x, vCursorPos.y + fCursorOff), uBlue, fBorderThickness);
    // Borders::Top
    pDrawList->AddCircleFilled(ImVec2(vCursorPos.x, oXYDrag.Min.y), 2.0f, uOrange, 3);
    if (fScaleX > fXLimit)
        pDrawList->AddLine(ImVec2(oXYDrag.Min.x, oXYDrag.Min.y), ImVec2(vCursorPos.x - fCursorOff, oXYDrag.Min.y), uBlue, fBorderThickness);
    if (fScaleX < 1.0f - fXLimit)
        pDrawList->AddLine(ImVec2(oXYDrag.Max.x, oXYDrag.Min.y), ImVec2(vCursorPos.x + fCursorOff, oXYDrag.Min.y), uBlue, fBorderThickness);
    // Borders::Left
    pDrawList->AddCircleFilled(ImVec2(oXYDrag.Min.x, vCursorPos.y), 2.0f, uOrange, 3);
    if (fScaleY > fYLimit)
        pDrawList->AddLine(ImVec2(oXYDrag.Min.x, oXYDrag.Min.y), ImVec2(oXYDrag.Min.x, vCursorPos.y - fCursorOff), uBlue, fBorderThickness);
    if (fScaleY < 1.0f - fYLimit)
        pDrawList->AddLine(ImVec2(oXYDrag.Min.x, oXYDrag.Max.y), ImVec2(oXYDrag.Min.x, vCursorPos.y + fCursorOff), uBlue, fBorderThickness);
    // Borders::Bottom
    pDrawList->AddCircleFilled(ImVec2(vCursorPos.x, oXYDrag.Max.y), 2.0f, uOrange, 3);
    if (fScaleX > fXLimit)
        pDrawList->AddLine(ImVec2(oXYDrag.Min.x, oXYDrag.Max.y), ImVec2(vCursorPos.x - fCursorOff, oXYDrag.Max.y), uBlue, fBorderThickness);
    if (fScaleX < 1.0f - fXLimit)
        pDrawList->AddLine(ImVec2(oXYDrag.Max.x, oXYDrag.Max.y), ImVec2(vCursorPos.x + fCursorOff, oXYDrag.Max.y), uBlue, fBorderThickness);
    
    pDrawList->AddLine(
                        oRect.Min + ImVec2(0.0f, fY3),
                        oRect.Min + ImVec2(2.0f*fX3, fY3), uFrameCol2, 1.0f);
    pDrawList->AddLine(
                        oRect.Min + ImVec2(2.0f*fX3, fY3),
                        oRect.Min + ImVec2(vSize.x, 0.0f), uFrameCol2, 1.0f);
    pDrawList->AddLine(
                        oRect.Min + ImVec2(2.0f*fX3, fY3),
                        oRect.Min + ImVec2(2.0f*fX3, vSize.y), uFrameCol2, 1.0f);
    
    // Cursor
    pDrawList->AddCircleFilled(vCursorPos, 5.0f, uBlue, 16);
    // CursorZ
    pDrawList->AddNgonFilled((vDragEnd - vDragStart)*fScaleZ + vDragStart, fHandleRadius, uBlue, 4);
    
    ImGui::Dummy(vHeightOffset);
    ImGui::Dummy(vHeightOffset*1.25f);
    ImGui::Dummy(vSize);
    
    char pBufferID[64];
    ImFormatString(pBufferID, IM_ARRAYSIZE(pBufferID), "Values##%d", *(ImS32 const*)&iID);
    
    if (ImGui::CollapsingHeader(pBufferID))
    {
        float const fMoveDeltaX = fDeltaX/64.0f; // Arbitrary
        float const fMoveDeltaY = fDeltaY/64.0f; // Arbitrary
        float const fMoveDeltaZ = fDeltaZ/64.0f; // Arbitrary
        
        char pBufferXID[64];
        ImFormatString(pBufferXID, IM_ARRAYSIZE(pBufferXID), "X##%d", *(ImS32 const*)&iID);
        char pBufferYID[64];
        ImFormatString(pBufferYID, IM_ARRAYSIZE(pBufferYID), "Y##%d", *(ImS32 const*)&iID);
        char pBufferZID[64];
        ImFormatString(pBufferZID, IM_ARRAYSIZE(pBufferZID), "Z##%d", *(ImS32 const*)&iID);
        
        bModified |= ImGui::DragScalar(pBufferXID, ImGuiDataType_Float, &fX, fMoveDeltaX, &fMinX, &fMaxX);
        bModified |= ImGui::DragScalar(pBufferYID, ImGuiDataType_Float, &fY, fMoveDeltaY, &fMinY, &fMaxY);
        bModified |= ImGui::DragScalar(pBufferZID, ImGuiDataType_Float, &fZ, fMoveDeltaZ, &fMinZ, &fMaxZ);
    }
    
    return bModified;
}

bool ImGui::InputVec2(char const* pLabel, ImVec2* pValue, ImVec2 const vMinValue, ImVec2 const vMaxValue, float const fScale /*= 1.0f*/)
{
    return SliderScalar2D(pLabel, &pValue->x, &pValue->y, vMinValue.x, vMaxValue.x, vMinValue.y, vMaxValue.y, fScale);
}

bool ImGui::InputVec3(char const* pLabel, ImVec4* pValue, ImVec4 const vMinValue, ImVec4 const vMaxValue, float const fScale /*= 1.0f*/)
{
    return SliderScalar3D(pLabel, &pValue->x, &pValue->y, &pValue->z, vMinValue.x, vMaxValue.x, vMinValue.y, vMaxValue.y, vMinValue.z, vMaxValue.z, fScale);
}
#endif // need DistToSegmentSqr/DistOnSegmentSqr/Dist2

#endif // #ifndef IMGUI_DISABLE
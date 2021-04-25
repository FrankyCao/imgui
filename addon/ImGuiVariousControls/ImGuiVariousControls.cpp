//- Common Code For All Addons needed just to ease inclusion as separate files in user code ----------------------
#include <imgui.h>
#undef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
//-----------------------------------------------------------------------------------------------------------------

#include "ImGuiVariousControls.h"

namespace ImGui {

#ifndef IMGUIHELPER_H_
// Posted by Omar in one post. It might turn useful...
bool IsItemActiveLastFrame()    {
    ImGuiContext& g = *GImGui;
    if (g.ActiveIdPreviousFrame)
	return g.ActiveIdPreviousFrame== GImGui->CurrentWindow->DC.LastItemId;
    return false;
}
bool IsItemJustReleased()   {
    return IsItemActiveLastFrame() && !ImGui::IsItemActive();
}
#endif //IMGUIHELPER_H_

static float GetWindowFontScale() {
    //ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();
    return window->FontWindowScale;
}

static bool CheckButton(const char* label,bool* pvalue,bool useSmallButton,float checkedStateAlphaMult=0.5f) {
    bool rv = false;
    const bool tmp = pvalue ? *pvalue : false;
    if (tmp) {
        const ImGuiStyle& style(ImGui::GetStyle());
        ImVec4 CheckButtonColor = style.Colors[ImGuiCol_Button];                CheckButtonColor.w*=checkedStateAlphaMult;
        ImVec4 CheckButtonHoveredColor = style.Colors[ImGuiCol_ButtonHovered];  CheckButtonHoveredColor.w*=checkedStateAlphaMult;
        ImVec4 CheckButtonActiveColor = style.Colors[ImGuiCol_ButtonActive];    CheckButtonActiveColor.w*=checkedStateAlphaMult;

        ImGui::PushStyleColor(ImGuiCol_Button,CheckButtonColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered,CheckButtonHoveredColor);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,CheckButtonActiveColor);
    }
    if (useSmallButton) {if (ImGui::SmallButton(label)) {if (pvalue) *pvalue=!(*pvalue);rv=true;}}
    else if (ImGui::Button(label)) {if (pvalue) *pvalue=!(*pvalue);rv=true;}
    if (tmp) ImGui::PopStyleColor(3);
    return rv;
}
bool CheckButton(const char* label,bool* pvalue) {return CheckButton(label,pvalue,false);}
bool SmallCheckButton(const char* label,bool* pvalue) {return CheckButton(label,pvalue,true);}

void ToggleButton(const char* str_id, bool* v)
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

bool ToggleButton(const char *str_id, bool *v, const ImVec2 &size)
{
    bool valueChange = false;

    ImVec2 pos = ImGui::GetCursorScreenPos();
    ImDrawList *draw_list = ImGui::GetWindowDrawList();

    ImGui::InvisibleButton(str_id, size);
    if (ImGui::IsItemClicked())
    {
        *v = !*v;
        valueChange = true;
    }

    ImU32 col_tint = ImGui::GetColorU32((*v ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_Border)));
    ImU32 col_bg = ImGui::GetColorU32(ImGui::GetColorU32(ImGuiCol_WindowBg));
    if (ImGui::IsItemHovered())
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    }
    if (ImGui::IsItemActive() || *v)
    {
        col_bg = ImGui::GetColorU32(ImGuiCol_Button);
    }

    draw_list->AddRectFilled(pos, pos + size, GetColorU32(col_bg));

    auto textSize = CalcTextSize(str_id);
    draw_list->AddText(ImVec2(pos.x + (size.x - textSize.x) / 2, pos.y), col_tint, str_id);

    return valueChange;
}

bool BulletToggleButton(const char* label, bool* v, ImVec2 &pos, ImVec2 &size)
{
    bool valueChange = false;

    ImDrawList *draw_list = ImGui::GetWindowDrawList();
    ImVec2 old_pos = ImGui::GetCursorScreenPos();
    ImGui::SetCursorScreenPos(pos);
    ImGui::InvisibleButton(label, size);
    if (ImGui::IsItemClicked())
    {
        *v = !*v;
        valueChange = true;
    }
    pos += size / 2;
    if (*v)
    {
        draw_list->AddCircleFilled(pos, draw_list->_Data->FontSize * 0.20f, IM_COL32(255, 0, 0, 255), 8);
    }
    else
    {
        draw_list->AddCircleFilled(pos, draw_list->_Data->FontSize * 0.20f, IM_COL32(128, 128, 128, 255), 8);
    }
    ImGui::SetCursorScreenPos(old_pos);
    return valueChange;
}

float ProgressBar(const char *optionalPrefixText, float value, const float minValue, const float maxValue, const char *format, const ImVec2 &sizeOfBarWithoutTextInPixels, const ImVec4 &colorLeft, const ImVec4 &colorRight, const ImVec4 &colorBorder)    {
    if (value<minValue) value=minValue;
    else if (value>maxValue) value = maxValue;
    const float valueFraction = (maxValue==minValue) ? 1.0f : ((value-minValue)/(maxValue-minValue));
    const bool needsPercConversion = strstr(format,"%%")!=NULL;

    ImVec2 size = sizeOfBarWithoutTextInPixels;
    if (size.x<=0) size.x = ImGui::GetWindowWidth()*0.25f;
    if (size.y<=0) size.y = ImGui::GetTextLineHeightWithSpacing(); // or without

    const ImFontAtlas* fontAtlas = ImGui::GetIO().Fonts;

    if (optionalPrefixText && strlen(optionalPrefixText)>0) {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("%s",optionalPrefixText);
        ImGui::SameLine();
    }

    if (valueFraction>0)   {
        ImGui::Image(fontAtlas->TexID,ImVec2(size.x*valueFraction,size.y), fontAtlas->TexUvWhitePixel,fontAtlas->TexUvWhitePixel,colorLeft,colorBorder);
    }
    if (valueFraction<1)   {
        if (valueFraction>0) ImGui::SameLine(0,0);
        ImGui::Image(fontAtlas->TexID,ImVec2(size.x*(1.f-valueFraction),size.y), fontAtlas->TexUvWhitePixel,fontAtlas->TexUvWhitePixel,colorRight,colorBorder);
    }
    ImGui::SameLine();

    ImGui::Text(format,needsPercConversion ? (valueFraction*100.f+0.0001f) : value);
    return valueFraction;
}

void TestProgressBar()  {    
    const float time = ((float)(((unsigned int) (ImGui::GetTime()*1000.f))%50000)-25000.f)/25000.f;
    float progress=(time>0?time:-time);
    // No IDs needed for ProgressBars:
    ImGui::ProgressBar("ProgressBar",progress);
    ImGui::ProgressBar("ProgressBar",1.f-progress);
    ImGui::ProgressBar("",500+progress*1000,500,1500,"%4.0f (absolute value in [500,1500] and fixed bar size)",ImVec2(150,-1));
    ImGui::ProgressBar("",500+progress*1000,500,1500,"%3.0f%% (same as above, but with percentage and new colors)",ImVec2(150,-1),ImVec4(0.7,0.7,1,1),ImVec4(0.05,0.15,0.5,0.8),ImVec4(0.8,0.8,0,1));
    // This one has just been added to ImGui:
    //char txt[48]="";sprintf(txt,"%3d%% (ImGui default progress bar)",(int)(progress*100));
    //ImGui::ProgressBar(progress,ImVec2(0,0),txt);
}


int PopupMenuSimple(bool &open, const char **pEntries, int numEntries, const char *optionalTitle, int *pOptionalHoveredEntryOut, int startIndex, int endIndex, bool reverseItems, const char *scrollUpEntryText, const char *scrollDownEntryText)   {
    int selectedEntry = -1;
    if (pOptionalHoveredEntryOut) *pOptionalHoveredEntryOut=-1;
    if (!open) return selectedEntry;
    if (numEntries==0 || !pEntries) {
        open = false;
        return selectedEntry;
    }

    float fs = 1.f;
#   ifdef IMGUI_INCLUDE_IMGUI_USER_INL
    fs = ImGui::GetWindowFontScale();   // Internal to <imgui.cpp>
#   endif //   IMGUI_INCLUDE_IMGUI_USER_INL

    if (!open) return selectedEntry;
    ImGui::PushID(&open);   // or pEntries ??
    //ImGui::BeginPopup(&open);
    ImGui::OpenPopup("MyOwnPopupSimpleMenu");
    if (ImGui::BeginPopup("MyOwnPopupSimpleMenu"))  {
        if (optionalTitle) {ImGui::Text("%s",optionalTitle);ImGui::Separator();}
        if (startIndex<0) startIndex=0;
        if (endIndex<0) endIndex = numEntries-1;
        if (endIndex>=numEntries) endIndex = numEntries-1;
        const bool needsScrolling = (endIndex-startIndex+1)<numEntries;
        if (scrollUpEntryText && needsScrolling) {
            ImGui::SetWindowFontScale(fs*0.75f);
            if (reverseItems ? (endIndex+1<numEntries) : (startIndex>0))    {
                const int entryIndex = reverseItems ? -3 : -2;
                if (ImGui::Selectable(scrollUpEntryText, false))  {
                    selectedEntry = entryIndex;//open = false;    // Hide menu
                }
                else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = entryIndex;
            }
            else ImGui::Text(" ");
            ImGui::SetWindowFontScale(fs);
        }
        if (!reverseItems)  {
            for (int i = startIndex; i <= endIndex; i++)    {
                const char* entry = pEntries[i];
                if (!entry || strlen(entry)==0) ImGui::Separator();
                else {
                    if (ImGui::Selectable(entry, false))  {
                        selectedEntry = i;open = false;    // Hide menu
                    }
                    else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = i;
                }
            }
        }
        else {
            for (int i = endIndex; i >= startIndex; i--)    {
                const char* entry = pEntries[i];
                if (!entry || strlen(entry)==0) ImGui::Separator();
                else {
                    if (ImGui::Selectable(entry, false))  {
                        selectedEntry = i;open = false;    // Hide menu
                    }
                    else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = i;
                }

            }
        }
        if (scrollDownEntryText && needsScrolling) {
            const float fs = ImGui::GetWindowFontScale();      // Internal to <imgui.cpp>
            ImGui::SetWindowFontScale(fs*0.75f);
            if (reverseItems ? (startIndex>0) : (endIndex+1<numEntries))    {
                const int entryIndex = reverseItems ? -2 : -3;
                if (ImGui::Selectable(scrollDownEntryText, false))  {
                    selectedEntry = entryIndex;//open = false;    // Hide menu
                }
                else if (pOptionalHoveredEntryOut && ImGui::IsItemHovered()) *pOptionalHoveredEntryOut = entryIndex;
            }
            else ImGui::Text(" ");
            ImGui::SetWindowFontScale(fs);
        }
        if (open)   // close menu when mouse goes away
        {
            const float d = 10;
            ImVec2 pos = ImGui::GetWindowPos();pos.x-=d;pos.y-=d;
            ImVec2 size = ImGui::GetWindowSize();size.x+=2.f*d;size.y+=2.f*d;
            const ImVec2& mousePos = ImGui::GetIO().MousePos;
            if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) open = false;
        }
    }
    ImGui::EndPopup();
    ImGui::PopID();

    return selectedEntry;    
}

int PopupMenuSimpleCopyCutPasteOnLastItem(bool readOnly) {
    static bool open = false;
    static const char* entries[] = {"Copy","Cut","","Paste"};   // "" is separator
    //open|=ImGui::Button("Show Popup Menu Simple");                    // BUTTON
    open|= ImGui::GetIO().MouseClicked[1] && ImGui::IsItemHovered(); // RIGHT CLICK
    int selectedEntry = PopupMenuSimple(open,entries,readOnly?1:4);
    if (selectedEntry>2) selectedEntry = 2; // Normally separator takes out one space
    return selectedEntry;
    // About "open": when user exits popup-menu, "open" becomes "false".
    // Please set it to "true" to display it again (we do it using open|=[...])
}


int PopupMenuSimple(PopupMenuSimpleParams &params, const char **pTotalEntries, int numTotalEntries, int numAllowedEntries, bool reverseItems, const char *optionalTitle, const char *scrollUpEntryText, const char *scrollDownEntryText)    {
    if (numAllowedEntries<1 || numTotalEntries==0) {params.open=false;return -1;}
    if (params.endIndex==-1) params.endIndex=reverseItems ? numTotalEntries-1 : numAllowedEntries-1;
    if (params.startIndex==-1) params.startIndex=params.endIndex-numAllowedEntries+1;

    const int oldHoveredEntry = params.hoveredEntry;
    params.selectedEntry = PopupMenuSimple(params.open,pTotalEntries,numTotalEntries,optionalTitle,&params.hoveredEntry,params.startIndex,params.endIndex,reverseItems,scrollUpEntryText,scrollDownEntryText);

    if (params.hoveredEntry<=-2 || params.selectedEntry<=-2)   {
        if (oldHoveredEntry!=params.hoveredEntry) params.scrollTimer = ImGui::GetTime();
        const float newTime = ImGui::GetTime();
        if (params.selectedEntry<=-2 || (newTime - params.scrollTimer > 0.4f))    {
            params.scrollTimer = newTime;
            if (params.hoveredEntry==-2 || params.selectedEntry==-2)   {if (params.startIndex>0) {--params.startIndex;--params.endIndex;}}
            else if (params.hoveredEntry==-3 || params.selectedEntry==-3) {if (params.endIndex<numTotalEntries-1) {++params.startIndex;++params.endIndex;}}

        }
    }
    if (!params.open && params.resetScrollingWhenRestart) {
        params.endIndex=reverseItems ? numTotalEntries-1 : numAllowedEntries-1;
        params.startIndex=params.endIndex-numAllowedEntries+1;
    }
    return params.selectedEntry;
}

void TestPopupMenuSimple(const char *scrollUpEntryText, const char *scrollDownEntryText) {
    // Recent Files-like menu
    static const char* recentFileList[] = {"filename01","filename02","filename03","filename04","filename05","filename06","filename07","filename08","filename09","filename10"};

    static PopupMenuSimpleParams pmsParams;
    pmsParams.open|= ImGui::GetIO().MouseClicked[1];// RIGHT CLICK
    const int selectedEntry = PopupMenuSimple(pmsParams,recentFileList,(int) sizeof(recentFileList)/sizeof(recentFileList[0]),5,true,"RECENT FILES",scrollUpEntryText,scrollDownEntryText);
    if (selectedEntry>=0) {
        // Do something: clicked entries[selectedEntry]
    }
}

inline static void ClampColor(ImVec4& color)    {
    float* pf;
    pf = &color.x;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
    pf = &color.y;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
    pf = &color.z;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
    pf = &color.w;if (*pf<0) *pf=0;if (*pf>1) *pf=1;
}

// Based on the code from: https://github.com/benoitjacquier/imgui
inline static bool ColorChooserInternal(ImVec4 *pColorOut,bool supportsAlpha,bool showSliders,ImGuiWindowFlags extra_flags=0,bool* pisAnyItemActive=NULL,float windowWidth = 180/*,bool isCombo = false*/)    {
    bool colorSelected = false;
    if (pisAnyItemActive) *pisAnyItemActive=false;
    //const bool isCombo = (extra_flags&ImGuiWindowFlags_ComboBox);

    ImVec4 color = pColorOut ? *pColorOut : ImVec4(0,0,0,1);
    if (!supportsAlpha) color.w=1.f;

    ImGuiContext& g = *GImGui;
    const float smallWidth = windowWidth/9.f;

    static const ImU32 black = ColorConvertFloat4ToU32(ImVec4(0,0,0,1));
    static const ImU32 white = ColorConvertFloat4ToU32(ImVec4(1,1,1,1));
    static float hue, sat, val;

    ImGui::ColorConvertRGBtoHSV( color.x, color.y, color.z, hue, sat, val );


    ImGuiWindow* colorWindow = GetCurrentWindow();

    const float quadSize = windowWidth - smallWidth - colorWindow->WindowPadding.x*2;
    //if (isCombo) ImGui::SetCursorPosX(ImGui::GetCursorPos().x+colorWindow->WindowPadding.x);
    // Hue Saturation Value
    if (ImGui::BeginChild("ValueSaturationQuad##ValueSaturationQuadColorChooser", ImVec2(quadSize, quadSize), false,extra_flags ))
    //ImGui::BeginGroup();
    {
        const int step = 5;
        ImVec2 pos = ImVec2(0, 0);
        ImGuiWindow* window = GetCurrentWindow();

        ImVec4 c00(1, 1, 1, 1);
        ImVec4 c10(1, 1, 1, 1);
        ImVec4 c01(1, 1, 1, 1);
        ImVec4 c11(1, 1, 1, 1);
        for (int y = 0; y < step; y++) {
            for (int x = 0; x < step; x++) {
                float s0 = (float)x / (float)step;
                float s1 = (float)(x + 1) / (float)step;
                float v0 = 1.0 - (float)(y) / (float)step;
                float v1 = 1.0 - (float)(y + 1) / (float)step;


                ImGui::ColorConvertHSVtoRGB(hue, s0, v0, c00.x, c00.y, c00.z);
                ImGui::ColorConvertHSVtoRGB(hue, s1, v0, c10.x, c10.y, c10.z);
                ImGui::ColorConvertHSVtoRGB(hue, s0, v1, c01.x, c01.y, c01.z);
                ImGui::ColorConvertHSVtoRGB(hue, s1, v1, c11.x, c11.y, c11.z);

                window->DrawList->AddRectFilledMultiColor(window->Pos + pos, window->Pos + pos + ImVec2(quadSize / step, quadSize / step),
                                                          ImGui::ColorConvertFloat4ToU32(c00),
                                                          ImGui::ColorConvertFloat4ToU32(c10),
                                                          ImGui::ColorConvertFloat4ToU32(c11),
                                                          ImGui::ColorConvertFloat4ToU32(c01));

                pos.x += quadSize / step;
            }
            pos.x = 0;
            pos.y += quadSize / step;
        }

        window->DrawList->AddCircle(window->Pos + ImVec2(sat, 1-val)*quadSize, 4, val<0.5f?white:black, 4);

        const ImGuiID id = window->GetID("ValueSaturationQuad");
        ImRect bb(window->Pos, window->Pos + window->Size);
        bool hovered, held;
        /*bool pressed = */ImGui::ButtonBehavior(bb, id, &hovered, &held, ImGuiButtonFlags_NoKeyModifiers);///*false,*/ false);
        if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (held)   {
            ImVec2 pos = g.IO.MousePos - window->Pos;
            sat = ImSaturate(pos.x / (float)quadSize);
            val = 1 - ImSaturate(pos.y / (float)quadSize);
            ImGui::ColorConvertHSVtoRGB(hue, sat, val, color.x, color.y, color.z);
            colorSelected = true;
        }

    }
    ImGui::EndChild();	// ValueSaturationQuad
    //ImGui::EndGroup();

    ImGui::SameLine();

    //if (isCombo) ImGui::SetCursorPosX(ImGui::GetCursorPos().x+colorWindow->WindowPadding.x+quadSize);

    //Vertical tint
    if (ImGui::BeginChild("Tint##TintColorChooser", ImVec2(smallWidth, quadSize), false,extra_flags))
    //ImGui::BeginGroup();
    {
        const int step = 8;
        const int width = (int)smallWidth;
        ImGuiWindow* window = GetCurrentWindow();
        ImVec2 pos(0, 0);
        ImVec4 c0(1, 1, 1, 1);
        ImVec4 c1(1, 1, 1, 1);
        for (int y = 0; y < step; y++) {
            float tint0 = (float)(y) / (float)step;
            float tint1 = (float)(y + 1) / (float)step;
            ImGui::ColorConvertHSVtoRGB(tint0, 1.0, 1.0, c0.x, c0.y, c0.z);
            ImGui::ColorConvertHSVtoRGB(tint1, 1.0, 1.0, c1.x, c1.y, c1.z);

            window->DrawList->AddRectFilledMultiColor(window->Pos + pos, window->Pos + pos + ImVec2(width, quadSize / step),
                                                      ColorConvertFloat4ToU32(c0),
                                                      ColorConvertFloat4ToU32(c0),
                                                      ColorConvertFloat4ToU32(c1),
                                                      ColorConvertFloat4ToU32(c1));

            pos.y += quadSize / step;
        }

        window->DrawList->AddCircle(window->Pos + ImVec2(smallWidth*0.5f, hue*quadSize), 4, black, 4);
        //window->DrawList->AddLine(window->Pos + ImVec2(0, hue*quadSize), window->Pos + ImVec2(width, hue*quadSize), ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)));
        bool hovered, held;
        const ImGuiID id = window->GetID("Tint");
        ImRect bb(window->Pos, window->Pos + window->Size);
        /*bool pressed = */ButtonBehavior(bb, id, &hovered, &held,ImGuiButtonFlags_NoKeyModifiers);// /*false,*/ false);
        if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        if (held)
        {

            ImVec2 pos = g.IO.MousePos - window->Pos;
            hue = ImClamp( pos.y / (float)quadSize, 0.0f, 1.0f );
            ImGui::ColorConvertHSVtoRGB( hue, sat, val, color.x, color.y, color.z );
            colorSelected = true;
        }

    }
    ImGui::EndChild(); // "Tint"
    //ImGui::EndGroup();

    if (showSliders)
    {
        //Sliders
        //ImGui::PushItemHeight();
        //if (isCombo) ImGui::SetCursorPos(ImVec2(ImGui::GetCursorPos().x+colorWindow->WindowPadding.x,ImGui::GetCursorPos().y+colorWindow->WindowPadding.y+quadSize));
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Sliders");
        static bool useHsvSliders = false;
        static const char* btnNames[2] = {"to HSV","to RGB"};
        const int index = useHsvSliders?1:0;
        ImGui::SameLine();
        if (ImGui::SmallButton(btnNames[index])) useHsvSliders=!useHsvSliders;

        ImGui::Separator();
        const ImVec2 sliderSize = /*isCombo ? ImVec2(-1,quadSize) : */ImVec2(-1,-1);
        if (ImGui::BeginChild("Sliders##SliderColorChooser", sliderSize, false,extra_flags))
        {


            {
                int r = ImSaturate( useHsvSliders ? hue : color.x )*255.f;
                int g = ImSaturate( useHsvSliders ? sat : color.y )*255.f;
                int b = ImSaturate( useHsvSliders ? val : color.z )*255.f;
                int a = ImSaturate( color.w )*255.f;

                static const char* names[2][3]={{"R","G","B"},{"H","S","V"}};
                bool sliderMoved = false;
                sliderMoved|= ImGui::SliderInt(names[index][0], &r, 0, 255);
                sliderMoved|= ImGui::SliderInt(names[index][1], &g, 0, 255);
                sliderMoved|= ImGui::SliderInt(names[index][2], &b, 0, 255);
                sliderMoved|= (supportsAlpha && ImGui::SliderInt("A", &a, 0, 255));
                if (sliderMoved)
                {
                    colorSelected = true;
                    color.x = (float)r/255.f;
                    color.y = (float)g/255.f;
                    color.z = (float)b/255.f;
                    if (useHsvSliders)  ImGui::ColorConvertHSVtoRGB(color.x,color.y,color.z,color.x,color.y,color.z);
                    if (supportsAlpha) color.w = (float)a/255.f;
                }
                //ColorConvertRGBtoHSV(s_color.x, s_color.y, s_color.z, tint, sat, val);*/
                if (pisAnyItemActive) *pisAnyItemActive|=sliderMoved;
            }


        }
        ImGui::EndChild();
    }

    if (colorSelected && pColorOut) *pColorOut = color;

    return colorSelected;
}

// Based on the code from: https://github.com/benoitjacquier/imgui
bool ColorChooser(bool* open,ImVec4 *pColorOut,bool supportsAlpha)   {
    static bool lastOpen = false;
    static const ImVec2 windowSize(175,285);

    if (open && !*open) {lastOpen=false;return false;}
    if (open && *open && *open!=lastOpen) {
        ImGui::SetNextWindowPos(ImGui::GetCursorScreenPos());
        ImGui::SetNextWindowSize(windowSize);
        lastOpen=*open;
    }

    //ImGui::OpenPopup("Color Chooser##myColorChoserPrivate");

    bool colorSelected = false;

    ImGuiWindowFlags WindowFlags = 0;
    //WindowFlags |= ImGuiWindowFlags_NoTitleBar;
    WindowFlags |= ImGuiWindowFlags_NoResize;
    //WindowFlags |= ImGuiWindowFlags_NoMove;
    WindowFlags |= ImGuiWindowFlags_NoScrollbar;
    WindowFlags |= ImGuiWindowFlags_NoCollapse;
    WindowFlags |= ImGuiWindowFlags_NoScrollWithMouse;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,2));

    if (open) ImGui::SetNextWindowFocus();
    //if (ImGui::BeginPopupModal("Color Chooser##myColorChoserPrivate",open,WindowFlags))
    //if (ImGui::Begin("Color Chooser##myColorChoserPrivate",open,windowSize,-1.f,WindowFlags)) // Old API
    ImGui::SetNextWindowSize(windowSize, ImGuiCond_FirstUseEver);
    if (ImGui::Begin("Color Chooser##myColorChoserPrivate",open,WindowFlags))
    {
        colorSelected = ColorChooserInternal(pColorOut,supportsAlpha,true);

        //ImGui::EndPopup();
    }
    ImGui::End();

    ImGui::PopStyleVar(2);

    return colorSelected;

}

// Based on the code from: https://github.com/benoitjacquier/imgui
bool ColorCombo(const char* label,ImVec4 *pColorOut,bool supportsAlpha,float width,bool closeWhenMouseLeavesIt)    {
    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float itemWidth = width>=0 ? width : ImGui::CalcItemWidth();
    const ImVec2 label_size = ImGui::CalcTextSize(label);
    const float color_quad_size = (g.FontSize + style.FramePadding.x);
    const float arrow_size = (g.FontSize + style.FramePadding.x * 2.0f);
    ImVec2 totalSize = ImVec2(label_size.x+color_quad_size+arrow_size, label_size.y) + style.FramePadding*2.0f;
    if (totalSize.x < itemWidth) totalSize.x = itemWidth;
    const ImRect frame_bb(window->DC.CursorPos, window->DC.CursorPos + totalSize);
    const ImRect total_bb(frame_bb.Min, frame_bb.Max);// + ImVec2(label_size.x > 0.0f ? style.ItemInnerSpacing.x + label_size.x : 0.0f, 0.0f));
    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id)) return false;
    const float windowWidth = frame_bb.Max.x - frame_bb.Min.x - style.FramePadding.x;


    ImVec4 color = pColorOut ? *pColorOut : ImVec4(0,0,0,1);
    if (!supportsAlpha) color.w=1.f;

    const bool hovered = ItemHoverable(frame_bb, id);

    const ImRect value_bb(frame_bb.Min, frame_bb.Max - ImVec2(arrow_size, 0.0f));
    RenderFrame(frame_bb.Min, frame_bb.Max, GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    RenderFrame(frame_bb.Min,ImVec2(frame_bb.Min.x+color_quad_size,frame_bb.Max.y), ImColor(style.Colors[ImGuiCol_Text]), true, style.FrameRounding);
    RenderFrame(ImVec2(frame_bb.Min.x+1,frame_bb.Min.y+1), ImVec2(frame_bb.Min.x+color_quad_size-1,frame_bb.Max.y-1),
                ImGui::ColorConvertFloat4ToU32(ImVec4(color.x,color.y,color.z,1.f)),
                true, style.FrameRounding);

    RenderFrame(ImVec2(frame_bb.Max.x-arrow_size, frame_bb.Min.y), frame_bb.Max, GetColorU32(hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button), true, style.FrameRounding); // FIXME-ROUNDING
    RenderArrow(window->DrawList,ImVec2(frame_bb.Max.x-arrow_size, frame_bb.Min.y) + style.FramePadding,GetColorU32(ImGuiCol_Text), ImGuiDir_Down);

    RenderTextClipped(ImVec2(frame_bb.Min.x+color_quad_size,frame_bb.Min.y) + style.FramePadding, value_bb.Max, label, NULL, NULL);

    if (hovered)
    {
        SetHoveredID(id);
        if (g.IO.MouseClicked[0])
        {
            ClearActiveID();
            if (ImGui::IsPopupOpen(id,0))
            {
                ClosePopupToLevel(g.OpenPopupStack.Size - 1,true);
            }
            else
            {
                FocusWindow(window);
                ImGui::OpenPopup(label);
            }
        }
        static ImVec4 copiedColor(1,1,1,1);
        static const ImVec4* pCopiedColor = NULL;
        if (g.IO.MouseClicked[1]) { // right-click (copy color)
            copiedColor = color;
            pCopiedColor = &copiedColor;
            //fprintf(stderr,"Copied\n");
        }
        else if (g.IO.MouseClicked[2] && pCopiedColor && pColorOut) { // middle-click (paste color)
            pColorOut->x = pCopiedColor->x;
            pColorOut->y = pCopiedColor->y;
            pColorOut->z = pCopiedColor->z;
            if (supportsAlpha) pColorOut->w = pCopiedColor->w;
            color = *pColorOut;
            //fprintf(stderr,"Pasted\n");
        }
    }

    bool value_changed = false;
    if (ImGui::IsPopupOpen(id,0))
    {
        ImRect popup_rect(ImVec2(frame_bb.Min.x, frame_bb.Max.y), ImVec2(frame_bb.Max.x, frame_bb.Max.y));
        //popup_rect.Max.y = ImMin(popup_rect.Max.y, g.IO.DisplaySize.y - style.DisplaySafeAreaPadding.y); // Adhoc height limit for Combo. Ideally should be handled in Begin() along with other popups size, we want to have the possibility of moving the popup above as well.
        ImGui::SetNextWindowPos(popup_rect.Min);
        ImGui::SetNextWindowSize(ImVec2(windowWidth,-1));//popup_rect.GetSize());
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, style.FramePadding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0);
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4,2));

        bool mustCloseCombo = false;
        const ImGuiWindowFlags flags =  0;//ImGuiWindowFlags_Modal;//ImGuiWindowFlags_ComboBox;  // ImGuiWindowFlags_ComboBox is no more available... what now ?
        if (ImGui::BeginPopup(label, flags))
        {
            bool comboItemActive = false;
            value_changed = ColorChooserInternal(pColorOut,supportsAlpha,false,flags,&comboItemActive,windowWidth/*,true*/);
            if (closeWhenMouseLeavesIt && !comboItemActive)
            {
                const float distance = g.FontSize*1.75f;//1.3334f;//24;
                //fprintf(stderr,"%1.f",distance);
                ImVec2 pos = ImGui::GetWindowPos();pos.x-=distance;pos.y-=distance;
                ImVec2 size = ImGui::GetWindowSize();
                size.x+=2.f*distance;
                size.y+=2.f*distance+windowWidth*8.f/9.f;   // problem: is seems that ImGuiWindowFlags_ComboBox does not return the full window height
                const ImVec2& mousePos = ImGui::GetIO().MousePos;
                if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) {
                    mustCloseCombo = true;
                    //fprintf(stderr,"Leaving ColorCombo: pos(%1f,%1f) size(%1f,%1f)\n",pos.x,pos.y,size.x,size.y);
                }
            }
            ImGui::EndPopup();
        }
        if (mustCloseCombo && ImGui::IsPopupOpen(id,0)) {
            ClosePopupToLevel(g.OpenPopupStack.Size - 1,true);
        }
        ImGui::PopStyleVar(3);
    }
    return value_changed;
}


// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
struct MultilineScrollState {
    // Input.
    float scrollRegionX;
    float scrollX;
    ImGuiStorage *storage;
    const char* textToPasteInto;
    int actionToPerformCopyCutSelectAllFrom1To3;

    // Output.
    bool newScrollPositionAvailable;
    float newScrollX;
    int CursorPos;
    int SelectionStart; //                                      // Read (== to SelectionEnd when no selection)
    int SelectionEnd;   //                                      // Read
};
// Based on the code from: https://github.com/Roflraging (see https://github.com/ocornut/imgui/issues/383)
static int MultilineScrollCallback(ImGuiInputTextCallbackData *data) {
    //static int cnt=0;fprintf(stderr,"MultilineScrollCallback (%d)\n",++cnt);
    MultilineScrollState *scrollState = (MultilineScrollState *)data->UserData;

    ImGuiID cursorId = ImGui::GetID("cursor");
    int oldCursorIndex = scrollState->storage->GetInt(cursorId, 0);

    if (oldCursorIndex != data->CursorPos)  {
        int begin = data->CursorPos;

        while ((begin > 0) && (data->Buf[begin - 1] != '\n'))   {
            --begin;
        }

        float cursorOffset = ImGui::CalcTextSize(data->Buf + begin, data->Buf + data->CursorPos).x;
        float SCROLL_INCREMENT = scrollState->scrollRegionX * 0.25f;

        if (cursorOffset < scrollState->scrollX)    {
            scrollState->newScrollPositionAvailable = true;
            scrollState->newScrollX = cursorOffset - SCROLL_INCREMENT; if (scrollState->newScrollX<0) scrollState->newScrollX=0;
        }
        else if ((cursorOffset - scrollState->scrollRegionX) >= scrollState->scrollX)   {
            scrollState->newScrollPositionAvailable = true;
            scrollState->newScrollX = cursorOffset - scrollState->scrollRegionX + SCROLL_INCREMENT;
        }
    }

    scrollState->storage->SetInt(cursorId, data->CursorPos);

    scrollState->CursorPos = data->CursorPos;
    if (data->SelectionStart<=data->SelectionEnd) {scrollState->SelectionStart = data->SelectionStart;scrollState->SelectionEnd = data->SelectionEnd;}
    else {scrollState->SelectionStart = data->SelectionEnd;scrollState->SelectionEnd = data->SelectionStart;}

    return 0;
}

bool ImageButtonWithText(ImTextureID texId,const char* label,const ImVec2& imageSize, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
    return false;

    ImVec2 size = imageSize;
    if (size.x<=0 && size.y<=0) {size.x=size.y=ImGui::GetTextLineHeightWithSpacing();}
    else {
        if (size.x<=0)          size.x=size.y;
        else if (size.y<=0)     size.y=size.x;
        size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImGuiID id = window->GetID(label);
    const ImVec2 textSize = ImGui::CalcTextSize(label,NULL,true);
    const bool hasText = textSize.x>0;

    const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
    const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
    const ImVec2 totalSizeWithoutPadding(size.x+innerSpacing+textSize.x,size.y>textSize.y ? size.y : textSize.y);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding*2);
    ImVec2 start(0,0);
    start = window->DC.CursorPos + padding;if (size.y<textSize.y) start.y+=(textSize.y-size.y)*.5f;
    const ImRect image_bb(start, start + size);
    start = window->DC.CursorPos + padding;start.x+=size.x+innerSpacing;if (size.y>textSize.y) start.y+=(size.y-textSize.y)*.5f;
    ItemSize(bb);
    if (!ItemAdd(bb, id))
    return false;

    bool hovered=false, held=false;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    // Render
    const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
    RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
    if (bg_col.w > 0.0f)
    window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

    window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv0, uv1, GetColorU32(tint_col));

    if (textSize.x>0) ImGui::RenderText(start,label);
    return pressed;
}

#ifndef NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE
// Now this struct cannot be used without stb_image.h anymore, even if no gif support is required,
// because it uses STBI_MALLOC and STBI_FREE
struct AnimatedImageInternal {
    protected:

    int w,h,frames;
    unsigned char* buffer;                    // Allocated with STBI_MALLOC: thus stb_image.h is always required now
    ImVector<float> delays;                   // Currently in cs (but now stb_image.h gives us ms)
    ImTextureID persistentTexId;              // This will be used when all frames can fit into a single texture (very good for performance and memory)
    int numFramesPerRowInPersistentTexture,numFramesPerColInPersistentTexture;
    bool hoverModeIfSupported;
    bool persistentTexIdIsNotOwned;
    mutable bool isAtLeastOneWidgetInHoverMode;  // internal

    mutable int lastFrameNum;
    mutable float delay;
    mutable float timer;
    mutable ImTextureID texId;
    mutable ImVec2 uvFrame0,uvFrame1;   // used by persistentTexId
    mutable int lastImGuiFrameUpdate;

    inline void updateTexture() const   {
        // fix updateTexture() to use persistentTexID when necessary
        IM_ASSERT(AnimatedImage::GenerateOrUpdateTextureCb!=NULL);	// Please use ImGui::AnimatedImage::SetGenerateOrUpdateTextureCallback(...) before calling this method
        if (frames<=0) return;
        else if (frames==1) {
            if (!texId) AnimatedImage::GenerateOrUpdateTextureCb(texId,w,h,4,buffer,false,false,false,false,false);
            return;
        }

        // These two lines sync animation in multiple items:
        if (texId && lastImGuiFrameUpdate==ImGui::GetFrameCount()) return;
        lastImGuiFrameUpdate=ImGui::GetFrameCount();
        if (hoverModeIfSupported && !isAtLeastOneWidgetInHoverMode) {
            // reset animation here:
            timer=-1.f;lastFrameNum=-1;delay=0;
            //calculateTexCoordsForFrame(0,uvFrame0,uvFrame1);
        }
        isAtLeastOneWidgetInHoverMode = false;

        float lastDelay = delay;
        if (timer>0) {
            delay = ImGui::GetTime()*100.f-timer;
            if (delay<0) timer = -1.f;
        }
        if (timer<0) {timer = ImGui::GetTime()*100.f;delay=0.f;}

        const int imageSz = 4 * w * h;
        IM_ASSERT(sizeof(unsigned short)==2*sizeof(unsigned char));
        bool changed = false;
        float frameTime=0.f;
        bool forceUpdate = false;
        if (lastFrameNum<0) {forceUpdate=true;lastFrameNum=0;}
        for (int i=lastFrameNum;i<frames;i++)   {
            frameTime = delays[i];
            //fprintf(stderr,"%d/%d) %1.2f\n",i,frames,frameTime);
            if (delay <= lastDelay+frameTime) {
                changed = (i!=lastFrameNum || !texId);
                lastFrameNum = i;
                if (changed || forceUpdate)    {
                    if (!persistentTexId) AnimatedImage::GenerateOrUpdateTextureCb(texId,w,h,4,&buffer[imageSz*i],false,false,false,false,false);
                    else {
                        texId = persistentTexId;
                        // calculate uvFrame0 and uvFrame1 here based on 'i' and numFramesPerRowInPersistentTexture,numFramesPerColInPersistentTexture
                        calculateTexCoordsForFrame(i,uvFrame0,uvFrame1);
                    }
                }
                //fprintf(stderr,"%d/%d) %1.2f %1.2f %1.2f\n",i,frames,frameTime,delay,lastDelay);
                delay = lastDelay;
                return;
            }
            lastDelay+=frameTime;
            if (i==frames-1) i=-1;
        }

    }

#   ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    struct ScopedFileContent {
        stbi_uc* gif_buffer;
        int gif_buffer_size;
        static stbi_uc* GetFileContent(const char *filePath,int* size_out)   {
            stbi_uc* f_data = NULL;FILE* f=NULL;long f_size=-1;size_t f_size_read=0;*size_out=0;
            if (!filePath || (f = ImFileOpen(filePath, "rb")) == NULL) return NULL;
            if (fseek(f, 0, SEEK_END) ||  (f_size = ftell(f)) == -1 || fseek(f, 0, SEEK_SET))  {fclose(f);return NULL;}
            f_data = (stbi_uc*) STBI_MALLOC(f_size);
            f_size_read = f_size>0 ? fread(f_data, 1, f_size, f) : 0;
            fclose(f);
            if (f_size_read == 0 || f_size_read!=(size_t)f_size)  {STBI_FREE(f_data);return NULL;}
            *size_out=(int)f_size;
            return f_data;
        }
        ScopedFileContent(const char* filePath) {gif_buffer=GetFileContent(filePath,&gif_buffer_size);}
        ~ScopedFileContent() {if (gif_buffer) {STBI_FREE(gif_buffer);gif_buffer=NULL;} gif_buffer_size=0;}
    };
#   endif //IMGUIVARIOUSCONTROLS_NO_STDIO

    public:
    AnimatedImageInternal()  {buffer=NULL;persistentTexIdIsNotOwned=false;texId=persistentTexId=NULL;clear();}
    ~AnimatedImageInternal()  {clear();persistentTexIdIsNotOwned=false;}
#	ifndef STBI_NO_GIF
#   ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    AnimatedImageInternal(char const *filename,bool useHoverModeIfSupported=false)  {buffer=NULL;persistentTexIdIsNotOwned = false;texId=persistentTexId=NULL;load(filename,useHoverModeIfSupported);}
#   endif //IMGUIVARIOUSCONTROLS_NO_STDIO
    AnimatedImageInternal(const unsigned char* memory_gif,int memory_gif_size,bool useHoverModeIfSupported=false)  {buffer=NULL;persistentTexIdIsNotOwned = false;texId=persistentTexId=NULL;load_from_memory(memory_gif,memory_gif_size,useHoverModeIfSupported);}
#	endif //STBI_NO_GIF
    AnimatedImageInternal(ImTextureID myTexId,int animationImageWidth,int animationImageHeight,int numFrames,int numFramesPerRowInTexture,int numFramesPerColumnInTexture,float delayDetweenFramesInCs,bool useHoverMode=false) {
        buffer=NULL;persistentTexIdIsNotOwned = false;texId=persistentTexId=NULL;
        create(myTexId,animationImageWidth,animationImageHeight,numFrames,numFramesPerRowInTexture,numFramesPerColumnInTexture,delayDetweenFramesInCs,useHoverMode);
    }
    void clear() {
        w=h=frames=lastFrameNum=0;delay=0.f;timer=-1.f;
        if (buffer) {STBI_FREE(buffer);buffer=NULL;} delays.clear();
        numFramesPerRowInPersistentTexture = numFramesPerColInPersistentTexture = 0;
        uvFrame0.x=uvFrame0.y=0;uvFrame1.x=uvFrame1.y=1;
        lastImGuiFrameUpdate = -1;hoverModeIfSupported=isAtLeastOneWidgetInHoverMode = false;
        if (texId || persistentTexId) IM_ASSERT(AnimatedImage::FreeTextureCb!=NULL);   // Please use ImGui::AnimatedImage::SetFreeTextureCallback(...)
        if (texId) {if (texId!=persistentTexId) AnimatedImage::FreeTextureCb(texId);texId=NULL;}
        if (persistentTexId)  {if (!persistentTexIdIsNotOwned) AnimatedImage::FreeTextureCb(persistentTexId);persistentTexId=NULL;}
    }
#	ifndef STBI_NO_GIF
#   ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    bool load(char const *filename,bool useHoverModeIfSupported=false)  {
        ScopedFileContent fc(filename);
        return (fc.gif_buffer && load_from_memory(fc.gif_buffer,fc.gif_buffer_size,useHoverModeIfSupported));
    }
#   endif //ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    bool load_from_memory(const unsigned char* gif_buffer,int gif_buffer_size,bool useHoverModeIfSupported=false)  {
        clear();hoverModeIfSupported = false;

        int c=0, *int_delays=NULL;
        buffer = stbi_load_gif_from_memory(gif_buffer,gif_buffer_size,&int_delays,&w,&h,&frames,&c,4);
        if (!buffer || frames<=0 || !int_delays) {clear();return false;}
        //fprintf(stderr,"w=%d h=%d z=%d c=%d\n",w,h,frames,c);

        // copy int_delays into delays
        delays.resize(frames);
        for (int i=0;i<frames;i++) {
            delays[i] = ((float) int_delays[i])*0.1f;   // cs, whereas int_delays is ms
            //fprintf(stderr,"int_delays[%d] = %d;\n",i,int_delays[i]);
        }
        STBI_FREE(int_delays);int_delays=NULL;

        if (AnimatedImage::MaxPersistentTextureSize.x>0 && AnimatedImage::MaxPersistentTextureSize.y>0)	{
            // code path that checks 'MaxPersistentTextureSize' and puts all into a single texture (rearranging the buffer)
            ImVec2 textureSize = AnimatedImage::MaxPersistentTextureSize;
            int maxNumFramesPerRow = (int)textureSize.x/(int) w;
            int maxNumFramesPerCol = (int)textureSize.y/(int) h;
            int maxNumFramesInATexture = maxNumFramesPerRow * maxNumFramesPerCol;
            int cnt = 0;
            ImVec2 lastValidTextureSize(0,0);
            while (maxNumFramesInATexture>=frames)	{
                // Here we just halve the 'textureSize', so that, if it fits, we save further texture space
                lastValidTextureSize = textureSize;
                if (cnt%2==0) textureSize.y = textureSize.y/2;
                else textureSize.x = textureSize.x/2;
                maxNumFramesPerRow = (int)textureSize.x/(int)w;
                maxNumFramesPerCol = (int)textureSize.y/(int)h;
                maxNumFramesInATexture = maxNumFramesPerRow * maxNumFramesPerCol;
                ++cnt;
            }
            if (cnt>0)  {
                textureSize=lastValidTextureSize;
                maxNumFramesPerRow = (int)textureSize.x/(int)w;
                maxNumFramesPerCol = (int)textureSize.y/(int)h;
                maxNumFramesInATexture = maxNumFramesPerRow * maxNumFramesPerCol;
            }

            if (maxNumFramesInATexture>=frames)	{
                numFramesPerRowInPersistentTexture = maxNumFramesPerRow;
                numFramesPerColInPersistentTexture = maxNumFramesPerCol;

                rearrangeBufferForPersistentTexture();

                // generate persistentTexture,delete buffer
                IM_ASSERT(AnimatedImage::GenerateOrUpdateTextureCb!=NULL);	// Please use ImGui::AnimatedImage::SetGenerateOrUpdateTextureCallback(...) before calling this method
                AnimatedImage::GenerateOrUpdateTextureCb(persistentTexId,w*maxNumFramesPerRow,h*maxNumFramesPerCol,4,buffer,false,false,false,false,false);
                STBI_FREE(buffer);buffer=NULL;

                hoverModeIfSupported = useHoverModeIfSupported;
                //fprintf(stderr,"%d x %d (%d x %d)\n",numFramesPerRowInPersistentTexture,numFramesPerColInPersistentTexture,(int)textureSize.x,(int)textureSize.y);

                if (hoverModeIfSupported) delays[0] = 0.f;  // Otherwise when we start hovering, we usually get an unwanted delay
            }
        }

        return true;
    }
#	endif //STBI_NO_GIF
    bool create(ImTextureID myTexId,int animationImageWidth,int animationImageHeight,int numFrames,int numFramesPerRowInTexture,int numFramesPerColumnInTexture,float delayDetweenFramesInCs,bool useHoverMode=false)   {
        clear();
        persistentTexIdIsNotOwned = false;
        IM_ASSERT(myTexId);
        IM_ASSERT(animationImageWidth>0 && animationImageHeight>0);
        IM_ASSERT(numFrames>0);
        IM_ASSERT(delayDetweenFramesInCs>0);
        IM_ASSERT(numFramesPerRowInTexture*numFramesPerColumnInTexture>=numFrames);
        if (!myTexId || animationImageWidth<=0 || animationImageHeight<=0
                || numFrames<=0 || delayDetweenFramesInCs<=0 || (numFramesPerRowInTexture*numFramesPerColumnInTexture<numFrames))
            return false;
        persistentTexId = myTexId;
        persistentTexIdIsNotOwned = true;
        w = animationImageWidth;
        h = animationImageHeight;
        frames = numFrames;
        numFramesPerRowInPersistentTexture = numFramesPerRowInTexture;
        numFramesPerColInPersistentTexture = numFramesPerColumnInTexture;
        delays.resize(frames);
        for (int i=0;i<frames;i++) delays[i] = delayDetweenFramesInCs;
        hoverModeIfSupported = useHoverMode;
        return true;
    }

    inline bool areAllFramesInASingleTexture() const {return persistentTexId!=NULL;}
    void render(ImVec2 size=ImVec2(0,0), const ImVec2& uv0=ImVec2(0,0), const ImVec2& uv1=ImVec2(1,1), const ImVec4& tint_col=ImVec4(1,1,1,1), const ImVec4& border_col=ImVec4(0,0,0,0)) const  {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return;
        if (size.x==0) size.x=w;
        else if (size.x<0) size.x=-size.x*w;
        if (size.y==0) size.y=h;
        else if (size.y<0) size.y=-size.y*h;
        size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;

        ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size);
        if (border_col.w > 0.0f)
            bb.Max += ImVec2(2,2);
        ItemSize(bb);
        if (!ItemAdd(bb, 0))
            return;

        updateTexture();

        ImVec2 uv_0 = uv0;
        ImVec2 uv_1 = uv1;
        if (persistentTexId) {
            bool hovered = true;	// to fall back when useHoverModeIfSupported == false;
            if (hoverModeIfSupported) {
                hovered = ImGui::IsItemHovered();
                if (hovered) isAtLeastOneWidgetInHoverMode = true;
            }
            if (hovered)	{
                const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
                uv_0 = uvFrame0 + uv0*uvFrameDelta;
                uv_1 = uvFrame0 + uv1*uvFrameDelta;
            }
            else {
                // We must use frame zero here:
                ImVec2 uvFrame0,uvFrame1;
                calculateTexCoordsForFrame(0,uvFrame0,uvFrame1);
                const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
                uv_0 = uvFrame0 + uv0*uvFrameDelta;
                uv_1 = uvFrame0 + uv1*uvFrameDelta;
            }
        }
        if (border_col.w > 0.0f)
        {
            window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(border_col), 0.0f);
            window->DrawList->AddImage(texId, bb.Min+ImVec2(1,1), bb.Max-ImVec2(1,1), uv_0, uv_1, GetColorU32(tint_col));
        }
        else
        {
            window->DrawList->AddImage(texId, bb.Min, bb.Max, uv_0, uv_1, GetColorU32(tint_col));
        }
    }
    bool renderAsButton(const char* label,ImVec2 size=ImVec2(0,0), const ImVec2& uv0 = ImVec2(0,0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1)) const {
        ImGuiWindow* window = GetCurrentWindow();
        if (window->SkipItems)
            return false;

        if (size.x==0) size.x=w;
        else if (size.x<0) size.x=-size.x*w;
        if (size.y==0) size.y=h;
        else if (size.y<0) size.y=-size.y*h;
        size*=window->FontWindowScale*ImGui::GetIO().FontGlobalScale;

        ImGuiContext& g = *GImGui;
        const ImGuiStyle& style = g.Style;

        // Default to using texture ID as ID. User can still push string/integer prefixes.
        // We could hash the size/uv to create a unique ID but that would prevent the user from animating UV.
        ImGui::PushID((void *)this);
        const ImGuiID id = window->GetID(label);
        ImGui::PopID();

        const ImVec2 textSize = ImGui::CalcTextSize(label,NULL,true);
        const bool hasText = textSize.x>0;

        const float innerSpacing = hasText ? ((frame_padding >= 0) ? (float)frame_padding : (style.ItemInnerSpacing.x)) : 0.f;
        const ImVec2 padding = (frame_padding >= 0) ? ImVec2((float)frame_padding, (float)frame_padding) : style.FramePadding;
        const ImVec2 totalSizeWithoutPadding(size.x+innerSpacing+textSize.x,size.y>textSize.y ? size.y : textSize.y);
        const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + totalSizeWithoutPadding + padding*2);
        ImVec2 start(0,0);
        start = window->DC.CursorPos + padding;if (size.y<textSize.y) start.y+=(textSize.y-size.y)*.5f;
        const ImRect image_bb(start, start + size);
        start = window->DC.CursorPos + padding;start.x+=size.x+innerSpacing;if (size.y>textSize.y) start.y+=(size.y-textSize.y)*.5f;
        ItemSize(bb);
        if (!ItemAdd(bb, id))
            return false;

        bool hovered=false, held=false;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);

        updateTexture();

        ImVec2 uv_0 = uv0;
        ImVec2 uv_1 = uv1;
        if (hovered && hoverModeIfSupported) isAtLeastOneWidgetInHoverMode = true;
        if ((persistentTexId && hoverModeIfSupported && hovered) || !persistentTexId || !hoverModeIfSupported) {
            const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
            uv_0 = uvFrame0 + uv0*uvFrameDelta;
            uv_1 = uvFrame0 + uv1*uvFrameDelta;
        }
        else {
            // We must use frame zero here:
            ImVec2 uvFrame0,uvFrame1;
            calculateTexCoordsForFrame(0,uvFrame0,uvFrame1);
            const ImVec2 uvFrameDelta = uvFrame1 - uvFrame0;
            uv_0 = uvFrame0 + uv0*uvFrameDelta;
            uv_1 = uvFrame0 + uv1*uvFrameDelta;
        }


        // Render
        const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
        RenderFrame(bb.Min, bb.Max, col, true, ImClamp((float)ImMin(padding.x, padding.y), 0.0f, style.FrameRounding));
        if (bg_col.w > 0.0f)
            window->DrawList->AddRectFilled(image_bb.Min, image_bb.Max, GetColorU32(bg_col));

        window->DrawList->AddImage(texId, image_bb.Min, image_bb.Max, uv_0, uv_1, GetColorU32(tint_col));

        if (hasText) ImGui::RenderText(start,label);
        return pressed;
    }


    inline int getWidth() const {return w;}
    inline int getHeight() const {return h;}
    inline int getNumFrames() const {return frames;}
    inline ImTextureID getTexture() const {return texId;}

    private:
    AnimatedImageInternal(const AnimatedImageInternal& ) {}
    void operator=(const AnimatedImageInternal& ) {}
    void rearrangeBufferForPersistentTexture()  {
        const int newBufferSize = w*numFramesPerRowInPersistentTexture*h*numFramesPerColInPersistentTexture*4;

        // BUFFER: frames images one below each other: size: 4*w x (h*frames)
        // TMP:    frames images numFramesPerRowInPersistentTexture * (4*w) x (h*numFramesPerColInPersistentTexture)

        const int strideSz = w*4;
        const int frameSz = strideSz*h;
        unsigned char* tmp = (unsigned char*) STBI_MALLOC(newBufferSize);
        IM_ASSERT(tmp);

        unsigned char*          pw = tmp;
        const unsigned char*    pr = buffer;

        int frm=0,colSz=0;
        while (frm<frames)	{
            for (int y = 0; y<h;y++)    {
                pr=&buffer[frm*frameSz + y*strideSz];
                colSz = numFramesPerRowInPersistentTexture>(frames-frm)?(frames-frm):numFramesPerRowInPersistentTexture;
                for (int col = 0; col<colSz;col++)    {
                    memcpy(pw,pr,strideSz);
                    pr+=frameSz;
                    pw+=strideSz;
                }
                if (colSz<numFramesPerRowInPersistentTexture) {
                    for (int col = colSz;col<numFramesPerRowInPersistentTexture;col++)    {
                        memset(pw,0,strideSz);
                        pw+=strideSz;
                    }
                }
            }
            frm+=colSz;
        }

        //-----------------------------------------------------------------------
        STBI_FREE(buffer);buffer=tmp;tmp=NULL;

#       ifdef DEBUG_OUT_TEXTURE
        stbi_write_png("testOutputPng.png", w*numFramesPerRowInPersistentTexture,h*numFramesPerColInPersistentTexture, 4, &buffer[0], w*numFramesPerRowInPersistentTexture*4);
#       undef DEBUG_OUT_TEXTURE
#       endif //DEBUG_OUT_TEXTURE
    }
    void calculateTexCoordsForFrame(int frm,ImVec2& uv0Out,ImVec2& uv1Out) const    {
        uv0Out=ImVec2((float)(frm%numFramesPerRowInPersistentTexture)/(float)numFramesPerRowInPersistentTexture,(float)(frm/numFramesPerRowInPersistentTexture)/(float)numFramesPerColInPersistentTexture);
        uv1Out=ImVec2(uv0Out.x+1.f/(float)numFramesPerRowInPersistentTexture,uv0Out.y+1.f/(float)numFramesPerColInPersistentTexture);
    }

};

AnimatedImage::FreeTextureDelegate AnimatedImage::FreeTextureCb = &ImDestroyTexture;
AnimatedImage::GenerateOrUpdateTextureDelegate AnimatedImage::GenerateOrUpdateTextureCb = &ImGenerateOrUpdateTexture;

ImVec2 AnimatedImage::MaxPersistentTextureSize(2048,2048);

#ifndef STBI_NO_GIF
#ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
AnimatedImage::AnimatedImage(const char *gif_filepath, bool useHoverModeIfSupported)    {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal(gif_filepath,useHoverModeIfSupported);
}
#endif //IMGUIVARIOUSCONTROLS_NO_STDIO
AnimatedImage::AnimatedImage(const unsigned char* gif_buffer,int gif_buffer_size,bool useHoverModeIfSupported)  {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal(gif_buffer,gif_buffer_size,useHoverModeIfSupported);
}
#endif //STBI_NO_GIF
AnimatedImage::AnimatedImage(ImTextureID myTexId, int animationImageWidth, int animationImageHeight, int numFrames, int numFramesPerRowInTexture, int numFramesPerColumnInTexture, float delayBetweenFramesInCs, bool useHoverMode) {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal(myTexId,animationImageWidth,animationImageHeight,numFrames,numFramesPerRowInTexture,numFramesPerColumnInTexture,delayBetweenFramesInCs,useHoverMode);
}
AnimatedImage::AnimatedImage()  {
    ptr = (AnimatedImageInternal*) ImGui::MemAlloc(sizeof(AnimatedImageInternal));
    IM_PLACEMENT_NEW(ptr) AnimatedImageInternal();
}
AnimatedImage::~AnimatedImage() {
    clear();
    ptr->~AnimatedImageInternal();
    ImGui::MemFree(ptr);ptr=NULL;
}
void AnimatedImage::clear() {ptr->clear();}
void AnimatedImage::render(ImVec2 size, const ImVec2 &uv0, const ImVec2 &uv1, const ImVec4 &tint_col, const ImVec4 &border_col) const   {ptr->render(size,uv0,uv1,tint_col,border_col);}
bool AnimatedImage::renderAsButton(const char *label, ImVec2 size, const ImVec2 &uv0, const ImVec2 &uv1, int frame_padding, const ImVec4 &bg_col, const ImVec4 &tint_col)   {return ptr->renderAsButton(label,size,uv0,uv1,frame_padding,bg_col,tint_col);}
#ifndef STBI_NO_GIF
#ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
bool AnimatedImage::load(const char *gif_filepath, bool useHoverModeIfSupported)    {return ptr->load(gif_filepath,useHoverModeIfSupported);}
#endif //IMGUIVARIOUSCONTROLS_NO_STDIO
bool AnimatedImage::load_from_memory(const unsigned char* gif_buffer,int gif_buffer_size,bool useHoverModeIfSupported)  {return ptr->load_from_memory(gif_buffer,gif_buffer_size,useHoverModeIfSupported);}
#endif //STBI_NO_GIF
bool AnimatedImage::create(ImTextureID myTexId, int animationImageWidth, int animationImageHeight, int numFrames, int numFramesPerRowInTexture, int numFramesPerColumnInTexture, float delayBetweenFramesInCs, bool useHoverMode)   {return ptr->create(myTexId,animationImageWidth,animationImageHeight,numFrames,numFramesPerRowInTexture,numFramesPerColumnInTexture,delayBetweenFramesInCs,useHoverMode);}
int AnimatedImage::getWidth() const {return ptr->getWidth();}
int AnimatedImage::getHeight() const    {return ptr->getHeight();}
int AnimatedImage::getNumFrames() const {return ptr->getNumFrames();}
bool AnimatedImage::areAllFramesInASingleTexture() const    {return ptr->areAllFramesInASingleTexture();}
#endif //NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE


/*
    inline ImVec2 mouseToPdfRelativeCoords(const ImVec2 &mp) const {
       return ImVec2((mp.x+cursorPosAtStart.x-startPos.x)*(uv1.x-uv0.x)/zoomedImageSize.x+uv0.x,
               (mp.y+cursorPosAtStart.y-startPos.y)*(uv1.y-uv0.y)/zoomedImageSize.y+uv0.y);
    }
    inline ImVec2 pdfRelativeToMouseCoords(const ImVec2 &mp) const {
        return ImVec2((mp.x-uv0.x)*(zoomedImageSize.x)/(uv1.x-uv0.x)+startPos.x-cursorPosAtStart.x,(mp.y-uv0.y)*(zoomedImageSize.y)/(uv1.y-uv0.y)+startPos.y-cursorPosAtStart.y);
    }
*/
bool ImageZoomAndPan(ImTextureID user_texture_id, const ImVec2& size,float aspectRatio,float& zoom,ImVec2& zoomCenter,int panMouseButtonDrag,int resetZoomAndPanMouseButton,const ImVec2& zoomMaxAndZoomStep)
{
    bool rv = false;
    ImGuiWindow* window = GetCurrentWindow();
    if (!window || window->SkipItems) return rv;
    ImVec2 curPos = ImGui::GetCursorPos();
    const ImVec2 wndSz(size.x>0 ? size.x : ImGui::GetWindowSize().x-curPos.x,size.y>0 ? size.y : ImGui::GetWindowSize().y-curPos.y);

    IM_ASSERT(wndSz.x!=0 && wndSz.y!=0 && zoom!=0);

    // Here we use the whole size (although it can be partially empty)
    ImRect bb(window->DC.CursorPos, ImVec2(window->DC.CursorPos.x + wndSz.x,window->DC.CursorPos.y + wndSz.y));
    ItemSize(bb);
    if (!ItemAdd(bb, 0)) return rv;

    ImVec2 imageSz = wndSz;
    ImVec2 remainingWndSize(0,0);
    if (aspectRatio!=0) {
        const float wndAspectRatio = wndSz.x/wndSz.y;
        if (aspectRatio >= wndAspectRatio) {imageSz.y = imageSz.x/aspectRatio;remainingWndSize.y = wndSz.y - imageSz.y;}
        else {imageSz.x = imageSz.y*aspectRatio;remainingWndSize.x = wndSz.x - imageSz.x;}
    }

    if (ImGui::IsItemHovered()) {
        const ImGuiIO& io = ImGui::GetIO();
        if (io.MouseWheel!=0) {
            if (io.KeyCtrl) {
                const float zoomStep = zoomMaxAndZoomStep.y;
                const float zoomMin = 1.f;
                const float zoomMax = zoomMaxAndZoomStep.x;
                if (io.MouseWheel < 0) {zoom/=zoomStep;if (zoom<zoomMin) zoom=zoomMin;}
                else {zoom*=zoomStep;if (zoom>zoomMax) zoom=zoomMax;}
                rv = true;
                /*if (io.FontAllowUserScaling) {
                    // invert effect:
                    // Zoom / Scale window
                    ImGuiContext& g = *GImGui;
                    ImGuiWindow* window = g.HoveredWindow;
                    float new_font_scale = ImClamp(window->FontWindowScale - g.IO.MouseWheel * 0.10f, 0.50f, 2.50f);
                    float scale = new_font_scale / window->FontWindowScale;
                    window->FontWindowScale = new_font_scale;

                    const ImVec2 offset = window->Size * (1.0f - scale) * (g.IO.MousePos - window->Pos) / window->Size;
                    window->Pos += offset;
                    window->PosFloat += offset;
                    window->Size *= scale;
                    window->SizeFull *= scale;
                }*/
            }
            else  {
                const bool scrollDown = io.MouseWheel <= 0;
                const float zoomFactor = .5/zoom;
                if ((!scrollDown && zoomCenter.y > zoomFactor) || (scrollDown && zoomCenter.y <  1.f - zoomFactor))  {
                    const float slideFactor = zoomMaxAndZoomStep.y*0.1f*zoomFactor;
                    if (scrollDown) {
                        zoomCenter.y+=slideFactor;///(imageSz.y*zoom);
                        if (zoomCenter.y >  1.f - zoomFactor) zoomCenter.y =  1.f - zoomFactor;
                    }
                    else {
                        zoomCenter.y-=slideFactor;///(imageSz.y*zoom);
                        if (zoomCenter.y < zoomFactor) zoomCenter.y = zoomFactor;
                    }
                    rv = true;
                }
            }
        }
        if (io.MouseClicked[resetZoomAndPanMouseButton]) {zoom=1.f;zoomCenter.x=zoomCenter.y=.5f;rv = true;}
        if (ImGui::IsMouseDragging(panMouseButtonDrag,1.f))   {
            zoomCenter.x-=io.MouseDelta.x/(imageSz.x*zoom);
            zoomCenter.y-=io.MouseDelta.y/(imageSz.y*zoom);
            rv = true;
            ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
        }
    }

    const float zoomFactor = .5/zoom;
    if (rv) {
        if (zoomCenter.x < zoomFactor) zoomCenter.x = zoomFactor;
        else if (zoomCenter.x > 1.f - zoomFactor) zoomCenter.x = 1.f - zoomFactor;
        if (zoomCenter.y < zoomFactor) zoomCenter.y = zoomFactor;
        else if (zoomCenter.y > 1.f - zoomFactor) zoomCenter.y = 1.f - zoomFactor;
    }

    ImVec2 uvExtension(2.f*zoomFactor,2.f*zoomFactor);
    if (remainingWndSize.x > 0) {
        const float remainingSizeInUVSpace = 2.f*zoomFactor*(remainingWndSize.x/imageSz.x);
        const float deltaUV = uvExtension.x;
        const float remainingUV = 1.f-deltaUV;
        if (deltaUV<1) {
            float adder = (remainingUV < remainingSizeInUVSpace ? remainingUV : remainingSizeInUVSpace);
            uvExtension.x+=adder;
            remainingWndSize.x-= adder * zoom * imageSz.x;
            imageSz.x+=adder * zoom * imageSz.x;

            if (zoomCenter.x < uvExtension.x*.5f) zoomCenter.x = uvExtension.x*.5f;
            else if (zoomCenter.x > 1.f - uvExtension.x*.5f) zoomCenter.x = 1.f - uvExtension.x*.5f;
        }
    }
    if (remainingWndSize.y > 0) {
        const float remainingSizeInUVSpace = 2.f*zoomFactor*(remainingWndSize.y/imageSz.y);
        const float deltaUV = uvExtension.y;
        const float remainingUV = 1.f-deltaUV;
        if (deltaUV<1) {
            float adder = (remainingUV < remainingSizeInUVSpace ? remainingUV : remainingSizeInUVSpace);
            uvExtension.y+=adder;
            remainingWndSize.y-= adder * zoom * imageSz.y;
            imageSz.y+=adder * zoom * imageSz.y;

            if (zoomCenter.y < uvExtension.y*.5f) zoomCenter.y = uvExtension.y*.5f;
            else if (zoomCenter.y > 1.f - uvExtension.y*.5f) zoomCenter.y = 1.f - uvExtension.y*.5f;
        }
    }

    ImVec2 uv0((zoomCenter.x-uvExtension.x*.5f),(zoomCenter.y-uvExtension.y*.5f));
    ImVec2 uv1((zoomCenter.x+uvExtension.x*.5f),(zoomCenter.y+uvExtension.y*.5f));


    /* // Here we use just the window size, but then ImGui::IsItemHovered() should be moved below this block. How to do it?
    ImVec2 startPos=window->DC.CursorPos;
    startPos.x+= remainingWndSize.x*.5f;
    startPos.y+= remainingWndSize.y*.5f;
    ImVec2 endPos(startPos.x+imageSz.x,startPos.y+imageSz.y);
    ImRect bb(startPos, endPos);
    ItemSize(bb);
    if (!ItemAdd(bb, NULL)) return rv;*/

    ImVec2 startPos=bb.Min,endPos=bb.Max;
    startPos.x+= remainingWndSize.x*.5f;
    startPos.y+= remainingWndSize.y*.5f;
    endPos.x = startPos.x + imageSz.x;
    endPos.y = startPos.y + imageSz.y;

    window->DrawList->AddImage(user_texture_id, startPos, endPos, uv0, uv1);

    return rv;
}

int DefaultInputTextAutoCompletionCallback(ImGuiInputTextCallbackData *data) {
    InputTextWithAutoCompletionData& mad = *((InputTextWithAutoCompletionData*) data->UserData);
    if (mad.newTextToSet.size()>0 && mad.newTextToSet[0]!='\0') {
        data->DeleteChars(0,data->BufTextLen);
        data->InsertChars(0,&mad.newTextToSet[0]);
        mad.newTextToSet[0]='\0';
    }
    if      (data->EventKey==ImGuiKey_DownArrow) ++mad.deltaTTItems;
    else if (data->EventKey==ImGuiKey_UpArrow) --mad.deltaTTItems;
    else if (data->EventKey==ImGuiKey_Tab) {mad.tabPressed=true;}
    return 0;
}
float InputTextWithAutoCompletionData::Opacity = 0.6f;
int InputTextWithAutoCompletionData::HelperGetItemInsertionPosition(const char* txt,bool (*items_getter)(void*, int, const char**), int items_count, void* user_data,bool* item_is_already_present_out) {
    if (item_is_already_present_out) *item_is_already_present_out=false;
    if (!txt || txt[0]=='\0' || !items_getter || items_count<0) return -1;
    const char* itxt = NULL;int cmp = 0;
    for (int i=0;i<items_count;i++) {
        if (items_getter(user_data,i,&itxt))   {
            if ((cmp=strcmp(itxt,txt))>=0)  {
                if (item_is_already_present_out && cmp==0) *item_is_already_present_out=true;
                return i;
            }
        }
    }
    return items_count;
}

int InputTextWithAutoCompletionData::HelperInsertItem(const char* txt,bool (*items_getter)(void*, int, const char**),bool (*items_inserter)(void*, int,const char*), int items_count, void* user_data,bool* item_is_already_present_out) {
    if (!txt || txt[0]=='\0' || !items_getter || !items_inserter || items_count<0) return -1;
    bool alreadyPresent=false;
    if (!item_is_already_present_out) item_is_already_present_out=&alreadyPresent;
    const int itemPosition = HelperGetItemInsertionPosition(txt,items_getter,items_count,user_data,item_is_already_present_out);
    if (!(*item_is_already_present_out) && itemPosition>=0) {
        if (items_inserter(user_data,itemPosition,txt)) return itemPosition;
        else return -1;
    }
    return itemPosition;
}

bool InputTextWithAutoCompletion(const char* label, char* buf, size_t buf_size, InputTextWithAutoCompletionData* pAutocompletion_data, bool (*autocompletion_items_getter)(void*, int, const char**), int autocompletion_items_size, void* autocompletion_user_data, int num_visible_autocompletion_items) {
    IM_ASSERT(pAutocompletion_data);
    IM_ASSERT(autocompletion_items_getter);
    InputTextWithAutoCompletionData& ad = *pAutocompletion_data;
    ad.inited = true;
    const ImGuiInputTextFlags itFlags = (!(ad.newTextToSet.size()>0 && ad.newTextToSet[0]!='\0')) ?
                (ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_AutoSelectAll) :
                (ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CallbackAlways);
    const ImVec2 cursorScreenPos = ImGui::GetCursorScreenPos();
    const bool rv = ImGui::InputText(label,buf,buf_size,itFlags|ad.additionalFlags,DefaultInputTextAutoCompletionCallback,(void*)&ad);
    if (rv) {
        // return pressed
        ad.itemPositionOfReturnedText=ad.itemIndexOfReturnedText=-1;
        if (strlen(buf)>0)  {
            const char* txt=NULL;
            int itemPlacement = 0,comp = 0, alreadyPresentIndex = -1;
            for (int i=0;i<autocompletion_items_size;i++) {
                if (autocompletion_items_getter(autocompletion_user_data,i,&txt))   {
                    comp = strcmp(buf,txt);
                    if (comp>0) ++itemPlacement;
                    else if (comp==0) {
                        alreadyPresentIndex=i;
                        break;    // already present
                    }
                }
            }
            if (alreadyPresentIndex>=0)	{ad.itemIndexOfReturnedText=alreadyPresentIndex;}
            else {ad.itemPositionOfReturnedText=itemPlacement;}
        }
        return rv;
    }
    bool inputTextActive = ImGui::IsItemActive();
    //ImGui::SameLine();ImGui::Text(label);
    if (inputTextActive && autocompletion_items_size>0) {
        const int numItems = autocompletion_items_size;
        if (buf[0]!='\0' && numItems>0) {
            const int buffersize = strlen(buf);
            if (ad.bufTextLen!=buffersize) {
                ad.bufTextLen=buffersize;
                ad.deltaTTItems = 0;    // We reset the UP/DOWN offset whe text changes
            }

            int selectedTTItemIndex = numItems-1;
            const char* txt=NULL;
            // We need to fetch the selectedTTItemIndex here
            if (ad.lastSelectedTTItemIndex>=0 && ad.lastSelectedTTItemIndex<numItems && autocompletion_items_getter(autocompletion_user_data,ad.lastSelectedTTItemIndex,&txt))   {
                // Speed up branch (we start our search from previous frame: ad.lastSelectedTTItemIndex
                int i = ad.lastSelectedTTItemIndex;
                //int cnt = 0;
                if (strcmp(txt,buf)<0)  {
                    while (i<numItems) {
                        if (autocompletion_items_getter(autocompletion_user_data,++i,&txt))   {
                            if (strcmp(txt,buf)>=0)  {selectedTTItemIndex=i;break;}
                        }
                        //++cnt;
                    }
                    if (i>=numItems) selectedTTItemIndex=numItems-1;
                }
                else {
                    while (i>=0) {
                        if (autocompletion_items_getter(autocompletion_user_data,i-1,&txt))   {
                            if (strcmp(txt,buf)<0)  {selectedTTItemIndex=i;break;}
                        }
                        --i;
                        //++cnt;
                    }
                    if (i<0) selectedTTItemIndex=0;
                }
                //static int oldCnt=10000000;if (cnt!=oldCnt) {fprintf(stderr,"cnt=%d\n",cnt);oldCnt=cnt;}
            }
            else {
                // Normal (slow) branch
                for (int i=0;i<numItems;i++) {
                    if (autocompletion_items_getter(autocompletion_user_data,i,&txt))   {
                        if (strcmp(txt,buf)>=0)  {selectedTTItemIndex=i;break;}
                    }
                }
            }
            if (selectedTTItemIndex + ad.deltaTTItems>=numItems) ad.deltaTTItems=numItems-selectedTTItemIndex-1;
            else if (selectedTTItemIndex + ad.deltaTTItems<0) ad.deltaTTItems=-selectedTTItemIndex;
            ad.lastSelectedTTItemIndex=selectedTTItemIndex+=ad.deltaTTItems;
            if (ad.tabPressed)  {
                if (selectedTTItemIndex<numItems) {
                    const char* selectedTTItemText=NULL;
                    if (!autocompletion_items_getter(autocompletion_user_data,selectedTTItemIndex,&selectedTTItemText))   {
                        IM_ASSERT(true);
                    }
                    IM_ASSERT(selectedTTItemText && strlen(selectedTTItemText)>0);
                    const size_t len = strlen(selectedTTItemText);
                    ad.newTextToSet.resize(len+1);
                    strcpy(&ad.newTextToSet[0],selectedTTItemText);
                }
                ad.deltaTTItems=0;
            }

            const int MaxNumTooltipItems = num_visible_autocompletion_items>0 ? num_visible_autocompletion_items : 7;
            const float textLineHeightWithSpacing = ImGui::GetTextLineHeightWithSpacing();
            const ImVec2 storedCursorScreenPos = ImGui::GetCursorScreenPos();
            const int MaxNumItemsBelowInputText = (ImGui::GetIO().DisplaySize.y - (storedCursorScreenPos.y+textLineHeightWithSpacing))/textLineHeightWithSpacing;
            const int MaxNumItemsAboveInputText = storedCursorScreenPos.y/textLineHeightWithSpacing;
            int numTTItems = numItems>MaxNumTooltipItems?MaxNumTooltipItems:numItems;
            bool useUpperScreen = false;
            if (numTTItems>MaxNumItemsBelowInputText && MaxNumItemsBelowInputText<MaxNumItemsAboveInputText)    {
                useUpperScreen = true;
                if (numTTItems>MaxNumItemsAboveInputText) numTTItems = MaxNumItemsAboveInputText;
            }
            else if (numTTItems>MaxNumItemsBelowInputText) numTTItems=MaxNumItemsBelowInputText;

            const int numTTItemsHalf = numTTItems/2;
            int firstTTItemIndex = selectedTTItemIndex-numTTItemsHalf;
            if (selectedTTItemIndex+numTTItemsHalf>=numItems) firstTTItemIndex = numItems-numTTItems;
            if (firstTTItemIndex<0) firstTTItemIndex=0;

            const ImVec2 inputTextBoxSize = ImGui::GetItemRectSize();
            float labelWidth = ImGui::CalcTextSize(label,NULL,true).x;
            if (labelWidth>0.0f) labelWidth+=ImGui::GetStyle().ItemInnerSpacing.x;            
            const ImVec2 ttWindowSize(ImVec2(inputTextBoxSize.x-labelWidth,numTTItems*textLineHeightWithSpacing));
            const ImVec2 newCursorScreenPos(cursorScreenPos.x,cursorScreenPos.y+(useUpperScreen ? (-numTTItems*textLineHeightWithSpacing) : (textLineHeightWithSpacing)));
            ImGui::SetCursorScreenPos(newCursorScreenPos);
            ImGui::SetNextWindowPos(newCursorScreenPos);
            ImGui::SetNextWindowSize(ttWindowSize);
            const float xPadding = textLineHeightWithSpacing*0.5f;
            ImGuiWindow* window = ImGui::GetCurrentWindowRead();
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding,0);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding,ImVec2(0,0));

            //if (ImGui::Begin("##TooltipAutocomplete", NULL,ttWindowSize,InputTextWithAutoCompletionData::Opacity,ImGuiWindowFlags_Tooltip|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse))  // Old API
            ImGui::SetNextWindowSize(ttWindowSize, ImGuiCond_FirstUseEver);
            if (InputTextWithAutoCompletionData::Opacity>=0.f) ImGui::SetNextWindowBgAlpha(InputTextWithAutoCompletionData::Opacity);
            if (ImGui::Begin("##TooltipAutocomplete", NULL,ImGuiWindowFlags_Tooltip|ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize|ImGuiWindowFlags_NoSavedSettings|ImGuiWindowFlags_NoScrollbar|ImGuiWindowFlags_NoScrollWithMouse))
            {
                // We must always use newCursorScreenPos when mnually drawing inside this window
                if (!window) window = ImGui::GetCurrentWindowRead();
                for (int i=firstTTItemIndex,iSz=firstTTItemIndex+numTTItems;i<iSz;i++) {
                    const ImVec2 start(newCursorScreenPos.x,newCursorScreenPos.y+(i-firstTTItemIndex)*textLineHeightWithSpacing);
                    if (i==ad.currentAutocompletionItemIndex) {
                        ImVec2 end(start.x+ttWindowSize.x-1,start.y+textLineHeightWithSpacing);
                        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Header]);
                        ImGui::GetWindowDrawList()->AddRectFilled(start,end,col,0,0);
                        //ImGui::GetWindowDrawList()->AddRect(start,end,ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_HeaderActive]),0,ImDrawFlags_RoundCornersNone,1.f);

                        ImGui::PushStyleColor(ImGuiCol_Text,ImGui::GetStyle().Colors[ImGuiCol_TextDisabled]);
                    }
                    if (i==selectedTTItemIndex) {
                        ImVec2 end(start.x+ttWindowSize.x-1,start.y+textLineHeightWithSpacing);
                        ImU32 col = ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_Button]);
                        ImGui::GetWindowDrawList()->AddRectFilled(start,end,col,0,0);
                        ImGui::GetWindowDrawList()->AddRect(start,end,ImGui::ColorConvertFloat4ToU32(ImGui::GetStyle().Colors[ImGuiCol_ButtonActive]),0,ImDrawFlags_RoundCornersNone,1.f);

                    }
                    const bool ok = autocompletion_items_getter(autocompletion_user_data,i,&txt);
                    ImGui::RenderText(ImVec2(start.x+xPadding,start.y+window->DC.PrevLineTextBaseOffset*0.5f),ok ? txt : "unknown");    // Not sure about +window->DC.PrevLineTextBaseOffset*0.5f
                    if (i==ad.currentAutocompletionItemIndex) ImGui::PopStyleColor();
                }
            }
            ImGui::End();
            ImGui::PopStyleVar(2);
            ImGui::SetCursorScreenPos(storedCursorScreenPos);   // restore


            // Debug:
            //ImGui::SetTooltip("autocompletionEntries.size()=%d\nbufData.currentAutocompletionItemIndex=%d\nad.deltaTTItems=%d\nfirstTTItemIndex=%d\nnumTTItems=%d",autocompletion_items_size,ad.currentAutocompletionItemIndex,ad.deltaTTItems,firstTTItemIndex,numTTItems);

        }
        else ad.deltaTTItems = 0;
    }
    ad.tabPressed=false;
    return rv;
}

char InputComboWithAutoCompletionData::ButtonCharcters[3][5] = {ICON_MD_ADD, ICON_MD_EDIT, ICON_MD_DELETE};//{"+","r","x"};
char InputComboWithAutoCompletionData::ButtonTooltips[3][128] = {"add","rename","delete"};

bool InputComboWithAutoCompletion(const char* label, int *current_item, size_t autocompletion_buffer_size, InputComboWithAutoCompletionData* pAutocompletion_data,
                                  bool (*items_getter)(void*, int, const char**),       // gets item at position ... (cannot be NULL)
                                  bool (*items_inserter)(void*, int,const char*),       // inserts item at position ... (cannot be NULL)
                                  bool (*items_deleter)(void*, int),                    // deletes item at position ... (can be NULL)
                                  bool (*items_renamer)(void *, int, int, const char *),// deletes item at position, and inserts renamed item at new position  ... (can be NULL)
                                  int items_count, void* user_data, int num_visible_items)   {

    IM_ASSERT(pAutocompletion_data);
    IM_ASSERT(items_getter);
    IM_ASSERT(items_inserter);
    IM_ASSERT(autocompletion_buffer_size>1);
    bool rv = false;
    InputComboWithAutoCompletionData* pad = pAutocompletion_data;
    pad->inited = true;if (current_item) pad->currentAutocompletionItemIndex=*current_item;
    pad->itemHovered = pad->itemActive = false;
    if (pad->buf.size()<(int)autocompletion_buffer_size) {
        pad->buf.resize(autocompletion_buffer_size);pad->buf[0]='\0';
    }
    ImGui::PushID(pAutocompletion_data);
    if (!pad->inputTextShown) {
        // ImGui::Combo(...) here
        const bool aValidItemIsSelected = items_count>0 && pad->currentAutocompletionItemIndex>=0 && pad->currentAutocompletionItemIndex<items_count;
        const bool hasDeleteButton = items_deleter && aValidItemIsSelected;
        const bool hasRenameButton = items_renamer && aValidItemIsSelected;
        const float singleButtonPadding = ImGui::GetStyle().FramePadding.x * 2.0f;
        const float addButtonWidth      = singleButtonPadding + ImGui::CalcTextSize(InputComboWithAutoCompletionData::ButtonCharcters[0]).x;
        const float renameButtonWidth   = singleButtonPadding + ImGui::CalcTextSize(InputComboWithAutoCompletionData::ButtonCharcters[1]).x;
        const float deleteButtonWidth   = singleButtonPadding + ImGui::CalcTextSize(InputComboWithAutoCompletionData::ButtonCharcters[2]).x;
        const float buttonsWidth = addButtonWidth + (hasRenameButton ? renameButtonWidth : 0.f) + (hasDeleteButton ? deleteButtonWidth : 0.f);
        const ImGuiWindow* window = ImGui::GetCurrentWindowRead();
        float comboWidth = (window->DC.ItemWidthStack.size()>0 ? window->DC.ItemWidthStack[window->DC.ItemWidthStack.size()-1] : window->ItemWidthDefault);
        bool noButtons = false;
        if (comboWidth>buttonsWidth) comboWidth-=buttonsWidth;
        else noButtons=true;

        // Combo
        ImGui::PushItemWidth(comboWidth);
        rv= ImGui::Combo("###icwac",current_item,items_getter,user_data,items_count,num_visible_items);
        ImGui::PopItemWidth();
        if (rv && current_item)  pad->currentAutocompletionItemIndex=*current_item;
        const bool comboHovered = ImGui::IsItemHovered();
        const bool comboActive = ImGui::IsItemActive();
        pad->itemHovered|=comboHovered;
        pad->itemActive|=comboActive;
        bool mustEnterEditMode = comboHovered && ImGui::IsMouseClicked(1);
        // Buttons
        if (!noButtons) {
            ImGui::SameLine(0,0);
            mustEnterEditMode|=ImGui::Button(InputComboWithAutoCompletionData::ButtonCharcters[0]);
            if (InputComboWithAutoCompletionData::ButtonTooltips[0][0]!='\0' && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",InputComboWithAutoCompletionData::ButtonTooltips[0]);
            if (mustEnterEditMode) {
                ++pad->inputTextShown;
                if (pad->buf.size()<(int)autocompletion_buffer_size) {
                    pad->buf.resize(autocompletion_buffer_size);
                    pad->buf[0]='\0';
                }
            }
            if (hasRenameButton)   {
                ImGui::SameLine(0,0);
                if (ImGui::Button(InputComboWithAutoCompletionData::ButtonCharcters[1])) {
                    ++pad->inputTextShown;pad->isRenaming = true;
                    const char* textToHighlight = NULL;
                    if (items_getter(user_data,pad->currentAutocompletionItemIndex,&textToHighlight) && textToHighlight) {
                        const int len = (int) strlen(textToHighlight);
                        if (len>0 && len<(int)autocompletion_buffer_size)    {
                            strcpy(&pad->buf[0],textToHighlight);
                        }
                    }
                }
                if (InputComboWithAutoCompletionData::ButtonTooltips[1][0]!='\0' && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",InputComboWithAutoCompletionData::ButtonTooltips[1]);
            }
            if (hasDeleteButton)   {
                ImGui::SameLine(0,0);
                if (ImGui::Button(InputComboWithAutoCompletionData::ButtonCharcters[2]) && items_deleter(user_data,pad->currentAutocompletionItemIndex)) {
                    rv = true;
                    if (ImGui::GetIO().KeyShift) {
                        const int num_items = items_count -1;
                        --pad->currentAutocompletionItemIndex;
                        if (pad->currentAutocompletionItemIndex<0) pad->currentAutocompletionItemIndex=num_items-1;
                    }
                    else pad->currentAutocompletionItemIndex=-1;
                    if (current_item) *current_item = pad->currentAutocompletionItemIndex;
                }
                if (InputComboWithAutoCompletionData::ButtonTooltips[2][0]!='\0' && ImGui::IsItemHovered()) ImGui::SetTooltip("%s",InputComboWithAutoCompletionData::ButtonTooltips[2]);
            }
        }
        // Label
        ImGui::SameLine();
        //ImGui::Text("%s",label);    // This doesn't cut "##". We must add all this cucumberson and error-prone code to workaround this (for correct alignment and isHovered detection):
        const ImVec2 label_size = CalcTextSize(label, NULL, true);
        if (label_size.x>0) ImGui::RenderText(ImVec2(window->DC.CursorPos.x,window->DC.CursorPos.y+window->DC.CurrLineTextBaseOffset),label);
        const ImRect label_bb(window->DC.CursorPos,ImVec2(window->DC.CursorPos.x + label_size.x, window->DC.CursorPos.y + label_size.y + ImGui::GetStyle().FramePadding.y*2.0f));
        ImGui::ItemSize(label_bb, ImGui::GetStyle().FramePadding.y);
        const ImGuiID labelID = 0;  // is this allowed ?
        if (ImGui::ItemAdd(label_bb, labelID)) pad->itemHovered|=ItemHoverable(label_bb, labelID);
    }
    else {
        // ImGui::InputText(...) here
        if (pad->inputTextShown==1) {++pad->inputTextShown;ImGui::SetKeyboardFocusHere(0);}
        const bool enter_pressed = InputTextWithAutoCompletion(label,&pad->buf[0],autocompletion_buffer_size,pad,items_getter,items_count,user_data,num_visible_items);
        const bool inputTextHovered = ImGui::IsItemHovered();
        const bool inputTextActive = ImGui::IsItemActive();
        pad->itemHovered|=inputTextHovered;
        pad->itemActive|=inputTextActive;
        bool mustAllowFurtherEditing = false;
        if (enter_pressed) {
            if (pad->buf[0]!='\0')  {
                if (pad->isRenaming) {
                    if (pad->getItemPositionOfReturnedText()>=0)    {
                        const int oldItemPosition = pad->currentAutocompletionItemIndex;
                        const int newItemPositionInOldList = pad->getItemPositionOfReturnedText();
                        int newItemPositionInNewList = newItemPositionInOldList;
                        if (oldItemPosition<newItemPositionInOldList) newItemPositionInNewList=newItemPositionInOldList-1;
                        if (items_renamer(user_data,oldItemPosition,newItemPositionInNewList,&pad->buf[0])) {
                            pad->currentAutocompletionItemIndex=newItemPositionInNewList;
                            if (current_item) *current_item = pad->currentAutocompletionItemIndex;
                            rv = true;
                            pad->buf[0]='\0';
                            pad->inputTextShown = mustAllowFurtherEditing ? 1 : 0;
                            pad->isRenaming = false;
                        }
                    }
                }
                else {
                    if (pad->getItemIndexOfReturnedText()>=0) {
                        pad->currentAutocompletionItemIndex=pad->getItemIndexOfReturnedText();
                        if (current_item) *current_item = pad->currentAutocompletionItemIndex;
                        rv = true;
                    }
                    else if (pad->getItemPositionOfReturnedText()>=0 && items_inserter(user_data,pad->getItemPositionOfReturnedText(),&pad->buf[0])) {
                        pad->currentAutocompletionItemIndex=pad->getItemPositionOfReturnedText();
                        if (current_item) *current_item = pad->currentAutocompletionItemIndex;
                        rv = true;
                    }
                }
                if (ImGui::GetIO().KeyShift && !pad->isRenaming) mustAllowFurtherEditing=true;
                pad->buf[0]='\0';
                pad->inputTextShown = mustAllowFurtherEditing ? 1 : 0;
                pad->isRenaming = false;
            }

        }
        if ((ImGui::IsItemActiveLastFrame() && !inputTextActive) || (inputTextHovered && ImGui::IsMouseClicked(1)))	{
            pad->inputTextShown = mustAllowFurtherEditing ? 1 : 0;
            pad->buf[0]='\0';
            pad->isRenaming = false;
        }
    }
    ImGui::PopID();
    return rv;
}

}   // ImGui namespace

namespace ImGui {

// Start PasswordDrawer (code based on ImGui::ImageButton(...))==========================================================
inline static ImU32 PasswordDrawerFadeAlpha(ImU32 color,int fragment,int numFragments) {
    int a = (IM_COL32_A_MASK&color) >> IM_COL32_A_SHIFT;
    a = (int) (a*(float)(numFragments-fragment)/(float)numFragments);
    return (color & ~IM_COL32_A_MASK) | (a << IM_COL32_A_SHIFT);
}
inline static ImU32 PasswordDrawerLighten(ImU32 color,int amount) {
    int rgba[4] = {(unsigned char) (color>>IM_COL32_R_SHIFT),
		     (unsigned char) (color>>IM_COL32_G_SHIFT),
		     (unsigned char) (color>>IM_COL32_B_SHIFT),
		     (unsigned char) (color>>IM_COL32_A_SHIFT)};
    for (int i=0;i<3;i++) rgba[0]=ImClamp(rgba[i]+amount,0,255);
    return IM_COL32(rgba[0],rgba[1],rgba[2],rgba[3]);
}
bool PasswordDrawer(char *password, int passwordSize,ImGuiPasswordDrawerFlags flags,const float size,const ImU32 colors[7])   {
    IM_ASSERT(passwordSize>4);

    const ImU32 defaultColors[7] = {
        IM_COL32(45,148,129,255),   // bg TL
        IM_COL32(29,86,103,255),    // bg TR
        IM_COL32(62,48,99,255),     // bg BL
        IM_COL32(78,62,107,255),    // bg BR
        IM_COL32(219,216,164,240),  // circles and quads
        IM_COL32(219,216,124,240),  // filled circles
        IM_COL32(32,32,0,164)       // lines
    };

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems || passwordSize<5) return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Default to using password as ID. User can still push string/integer prefixes.
    PushID((void *)password);
    const ImGuiID id = window->GetID("#password_drawer");
    PopID();

    const float itemWidth = size<=0 ? ImGui::CalcItemWidth() : size;
    const ImVec2 size1(itemWidth,itemWidth);
    const ImRect bb(window->DC.CursorPos, window->DC.CursorPos + size1);
    const ImRect image_bb(window->DC.CursorPos, window->DC.CursorPos + size1);
    ItemSize(bb);
    if (!ItemAdd(bb, id)) return false;

    bool hovered=false, held=false, editable = !(flags&ImGuiPasswordDrawerFlags_ReadOnly);
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);

    const ImU32* pColors = colors ? colors : defaultColors;
    const bool border = true;
    if (editable || (!hovered && !pressed && !held))
	window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max ,pColors[0],pColors[1],pColors[3],pColors[2]);
    else {
	ImU32 bgColors[4];
	for (int i=0;i<4;i++) bgColors[i] = PasswordDrawerLighten(pColors[i],(pressed||held) ? 60 : 30);
	window->DrawList->AddRectFilledMultiColor(bb.Min, bb.Max ,bgColors[0],bgColors[1],bgColors[3],bgColors[2]);
    }
    if (border && window->WindowBorderSize)   {
        window->DrawList->AddRect(bb.Min+ImVec2(1,1), bb.Max+ImVec2(1,1), GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding);
        window->DrawList->AddRect(bb.Min, bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
    }

    if (!editable) {
	//pressed|=held;
	held=false;
    }

    static ImGuiID draggingState = 0;
    if (draggingState==0 && held) {
        password[0]='\0';   // reset
        draggingState = id;
    }
    const bool mouseIsHoveringRect = ImGui::IsMouseHoveringRect(image_bb.Min,image_bb.Max);

    // Now we start the control logic
    static const int numRowsSquared[5] = {4,9,16,25,36};
    int passwordLen = strlen(password);
    int numRows = 0;char minChar='1';
    for (int i=4;i>=0;--i) {
        if (passwordSize>numRowsSquared[i]) {
            numRows = 2+i;
	    if (passwordLen>numRowsSquared[i]) passwordLen=numRowsSquared[i]; // Max 36
            break;
        }
    }

    const ImGuiIO& io = ImGui::GetIO();
    unsigned char selected[37]={0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
    for (int l=0;l<passwordLen;l++) {
        const int ch = (const int) (password[l]-minChar);
        if (ch<passwordSize) {
            selected[ch]=(unsigned char) (l+1);
        }
    }


    // Here we draw the circles (and optional Quads)
    const float imageQuadWidth = image_bb.GetWidth()/(float)numRows;
    const float radius = imageQuadWidth*0.25f;
    const float radiusSquared = radius*radius;
    const float quadHalfSize = radius*0.175f;
    int num_segments = radius*(42.f/100.f);
    if (num_segments<5) num_segments=5;
    else if (num_segments>24) num_segments = 24;
    float thickness = radius*0.05f;
    ImVec2 center(0,0),tmp(0,0);int cnt=0;
    unsigned char charToAdd = 0;
    for (int row=0;row<numRows;row++)   {
        center.y = bb.Min.y+(imageQuadWidth*row)+imageQuadWidth*0.5f;
        for (int col=0;col<numRows;col++)   {
            center.x = bb.Min.x+(imageQuadWidth*col)+imageQuadWidth*0.5f;

            const unsigned char isSelected = selected[cnt];
            if (isSelected) {
                if (flags&ImGuiPasswordDrawerFlags_NoFilledCircles)  {
                    // Normal Drawing
                    window->DrawList->AddCircle(center,radius,pColors[4],num_segments,thickness);
                }
                else {
                    // Filled Drawing
                    window->DrawList->AddCircleFilled(center,radius,PasswordDrawerFadeAlpha(pColors[5],isSelected-1,passwordLen),num_segments);
                }
                const ImDrawFlags corners_flags = pColors[(flags&ImGuiPasswordDrawerFlags_NoLines)] ? ImDrawFlags_RoundCornersBottomRight : ImDrawFlags_RoundCornersNone;
                window->DrawList->AddRectFilled(ImVec2(center.x-quadHalfSize,center.y-quadHalfSize),ImVec2(center.x+quadHalfSize,center.y+quadHalfSize),corners_flags);
            }
            else {
                window->DrawList->AddCircle(center,radius,pColors[4],num_segments,thickness);
                window->DrawList->AddRectFilled(ImVec2(center.x-quadHalfSize,center.y-quadHalfSize),ImVec2(center.x+quadHalfSize,center.y+quadHalfSize),pColors[4]);
                if (held && mouseIsHoveringRect && draggingState==id && charToAdd==0 && (passwordLen==0 || password[passwordLen-1]!=(char)(minChar+cnt)))   {
                    tmp.x = io.MousePos.x-center.x;tmp.y = io.MousePos.y-center.y;
                    //if (tmp.x>-radius && tmp.x<radius && tmp.y>-radius && tmp.y<radius) // This line can be commented out (is it faster or not?)
                    {
                        tmp.x*=tmp.x;tmp.y*=tmp.y;
                        if (tmp.x+tmp.y<radiusSquared) charToAdd = minChar+cnt+1;
                    }
                }
            }
            ++cnt;
        }
    }

    // Draw lines:
    if (!(flags&ImGuiPasswordDrawerFlags_NoLines))  {
        const float lineThickness = radius*0.35f;
        ImVec2 lastCenter(-1,-1);
        for (int l=0;l<passwordLen;l++) {
            const int ch = (const int) (password[l]-minChar);
            if (ch<passwordSize) {
                const int row = ch/numRows;
                const int col = ch%numRows;
                center.x = bb.Min.x+(imageQuadWidth*col)+imageQuadWidth*0.5f;
                center.y = bb.Min.y+(imageQuadWidth*row)+imageQuadWidth*0.5f;
                if (l>0) {
                    window->DrawList->AddLine(lastCenter,center,pColors[6],lineThickness);
                }
                lastCenter = center;
            }
        }
	if (editable && passwordLen>0 && mouseIsHoveringRect && passwordLen<numRows*numRows) {
            // Draw last live line
            window->DrawList->AddLine(lastCenter,io.MousePos,pColors[6],lineThickness);
        }
    }

    if (editable && charToAdd>0) {
        password[passwordLen] = (char)(charToAdd-1);
        password[passwordLen+1]='\0';
    }


    if (draggingState==id && !held) {
        // end
        draggingState = 0;
	if (editable) return passwordLen>0;
    }

    return editable ? 0 : pressed;
}
// End PasswordDrawer ===================================================================================================

// Start CheckboxFlags ==============================================================================================
unsigned int CheckboxFlags(const char* label,unsigned int* flags,int numFlags,int numRows,int numColumns,unsigned int flagAnnotations,int* itemHoveredOut,const unsigned int* pFlagsValues)
{
    unsigned int itemPressed = 0;
    if (itemHoveredOut) *itemHoveredOut=-1;
    if (numRows*numColumns*numFlags==0) return itemPressed;

    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) return itemPressed;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImVec2 label_size = CalcTextSize(label, NULL, true);

    int numItemsPerRow = numFlags/numRows;if (numFlags%numRows) ++numItemsPerRow;
    int numItemsPerGroup = numItemsPerRow/numColumns;if (numItemsPerRow%numColumns) ++numItemsPerGroup;
    numItemsPerRow = numItemsPerGroup * numColumns;

    ImGui::BeginGroup();
    const ImVec2 startCurPos = window->DC.CursorPos;
    ImVec2 curPos = startCurPos, maxPos = startCurPos;
    const ImVec2 checkSize(label_size.y,label_size.y);
    const float pad = ImMax(1.0f, (float)(int)(checkSize.x / 6.0f));
    unsigned int j=0;int groupItemCnt = 0, groupCnt = 0;
    const bool buttonPressed = ImGui::IsMouseClicked(0);

    ImU32 annColor;int annSegments;
    float annThickness,annCenter,annRadius;
    if (flagAnnotations)    {
        annColor = GetColorU32(ImGuiCol_Button);
        annSegments = (int) (checkSize.x * 0.4f);if (annSegments<3) annSegments=3;
        annThickness = checkSize.x / 6.0f;
        annCenter = (float)(int)(checkSize.x * 0.5f);
        annRadius = (float)(int)(checkSize.x / 4.0f);
    }

    for (int i=0;i<numFlags;i++) {
        j = pFlagsValues ? pFlagsValues[i] : (1<<i);
        ImGui::PushID(i);
        ImGuiID itemID = window->GetID(label);
        ImGui::PopID();

        bool v = ((*flags & j) == j);
        const bool vNotif =  ((flagAnnotations & j) == j);
        ImRect bb(curPos,curPos + checkSize);
        ItemSize(bb, style.FramePadding.y);

        if (!ItemAdd(bb, itemID)) break;

        bool hovered=false, held= false, pressed = false;
        // pressed = ButtonBehavior(bb, itemID, &hovered, &held);   // Too slow!
        hovered = ItemHoverable(bb, itemID);pressed =  buttonPressed && hovered;   // Way faster
        if (pressed) {
            v = !v;
            if (!ImGui::GetIO().KeyShift) *flags=0;
            if (v)  *flags |= j;
            else    *flags &= ~j;
            itemPressed = j;
        }
        if (itemHoveredOut && hovered) *itemHoveredOut=i;

        RenderFrame(bb.Min, bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), true, style.FrameRounding);

        if (v)  {
            window->DrawList->AddRectFilled(bb.Min+ImVec2(pad,pad), bb.Max-ImVec2(pad,pad), GetColorU32(ImGuiCol_CheckMark), style.FrameRounding);
            if (vNotif) window->DrawList->AddCircle(bb.Min+ImVec2(annCenter,annCenter), annRadius, annColor,annSegments,annThickness);
        }
        else if (vNotif) window->DrawList->AddCircle(bb.Min+ImVec2(annCenter,annCenter), annRadius, annColor,annSegments,annThickness);

        // Nope: LogRenderedText is static inside imgui.cpp => we remove it, so that imguivariouscontrols.h/cpp can be used on their own too
        //if (g.LogEnabled) LogRenderedText(&bb.Min, v ? "[x]" : "[ ]");

        curPos.x+=checkSize.x;
        if (maxPos.x<curPos.x) maxPos.x=curPos.x;
        if (i<numFlags-1) {
            ++groupCnt;
            if (++groupItemCnt==numItemsPerGroup) {
                groupItemCnt=0;
                curPos.x+=style.FramePadding.y*2;
                if (groupCnt>=numItemsPerRow) {
                    groupCnt=0;
                    curPos.x = startCurPos.x;
                    curPos.y+=checkSize.y;
                    if (maxPos.y<curPos.y) maxPos.y=curPos.y;
                }
                else {
                    ImGui::SameLine(0,0);
                }
            }
            else ImGui::SameLine(0,0);
        }

    }
    ImGui::EndGroup();


    if (label_size.x > 0) {
        const float spacing = style.ItemInnerSpacing.x;
        SameLine(0, spacing);
        const ImVec2 textStart(maxPos.x+spacing,startCurPos.y);
        const ImRect text_bb(textStart, textStart + label_size);


        ImGuiID itemID = window->GetID(label);
        ItemSize(text_bb, style.FramePadding.y*numRows);
        if (ItemAdd(text_bb, itemID)) RenderText(text_bb.Min, label);
    }

    return itemPressed;
}
// End CheckboxFlags =================================================================================================

// Start CheckBoxStyled ==============================================================================================
bool CheckboxStyled(const char* label, bool* v,const ImU32* pOptionalEightColors,const ImVec2& checkBoxScale,float checkBoxRounding)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    IM_ASSERT(checkBoxScale.x>0 && checkBoxScale.y>0);
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 label_size = CalcTextSize(label, NULL, true);
    const ImVec2 check_size(label_size.y*2.5f*checkBoxScale.x,label_size.y);

    const ImRect check_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(check_size.x + style.FramePadding.x*2, check_size.y + style.FramePadding.y*2)); // We want a square shape to we use Y twice
    ItemSize(check_bb, style.FramePadding.y);

    ImRect total_bb = check_bb;
    if (label_size.x > 0) SameLine(0, style.ItemInnerSpacing.x);
    const ImRect text_bb(window->DC.CursorPos + ImVec2(0,style.FramePadding.y), window->DC.CursorPos + ImVec2(0,style.FramePadding.y) + label_size);
    if (label_size.x > 0)
    {
        ItemSize(ImVec2(text_bb.GetWidth(), check_bb.GetHeight()), style.FramePadding.y);
        total_bb = ImRect(ImMin(check_bb.Min, text_bb.Min), ImMax(check_bb.Max, text_bb.Max));
    }

    if (!ItemAdd(total_bb, id))
        return false;

    bool hovered, held;
    bool pressed = ButtonBehavior(total_bb, id, &hovered, &held);
    static float timeBegin = -1.f;
    static ImGuiID timeID = 0;
    static const float timeActionLength = 0.2f;
    if (pressed) {
        *v = !(*v); // change state soon
        if (timeID==id) {
            // Fine tuning for the case when user clicks on the same checkbox twice quickly
            float elapsedTime = ImGui::GetTime()-timeBegin;
            if (elapsedTime>timeActionLength) timeBegin = ImGui::GetTime();   // restart
            else {
                // We must invert the time, tweaking timeBegin
                const float newElapsedTime = timeActionLength-elapsedTime;
                timeBegin= ImGui::GetTime()-newElapsedTime;
            }
        }
        else {
            timeID = id;
            timeBegin = ImGui::GetTime();
        }
    }

    // Widget Look Here ================================================================
    float t = 0.f;    // In [0,1] 0 = OFF 1 = ON
    bool animationActive = false;
    if (timeID==id) {
        float elapsedTime = ImGui::GetTime()-timeBegin;
        if (elapsedTime>timeActionLength) {timeBegin=-1;timeID=0;}
        else {
            t = 1.f-elapsedTime/timeActionLength;
            animationActive = t>0;
        }
    }
    if (*v) t = 1.f-t;
    if (t<0) t=0;
    else if (t>1) t=1;
    const float check_bb_height = check_bb.GetHeight();
    const float innerFrameHeight = check_bb_height*0.5f*(checkBoxScale.y<=2.f?checkBoxScale.y:2.f);
    const float heightDelta = (check_bb_height-innerFrameHeight)*0.5f;
    const float check_bb_width = check_bb.GetWidth();
    float widthFraction = check_bb_width*t;    
    float rounding = checkBoxRounding<0 ? style.WindowRounding : checkBoxRounding;//style.FrameRounding;
    rounding*=innerFrameHeight*0.065f;if (rounding>16.f) rounding = 16.f;
    ImRect innerFrame0(ImVec2(check_bb.Min.x,check_bb.Min.y+heightDelta),ImVec2(check_bb.Min.x+widthFraction,check_bb.Max.y-heightDelta));
    ImRect innerFrame1(ImVec2(check_bb.Min.x+widthFraction,check_bb.Min.y+heightDelta),ImVec2(check_bb.Max.x,check_bb.Max.y-heightDelta));
    if (t>0) {
	ImU32 fillColor0 = pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[5] : pOptionalEightColors[4]) :
        (GetColorU32((held || hovered) ? ImGuiCol_ButtonHovered : ImGuiCol_Button));
        window->DrawList->AddRectFilled(innerFrame0.Min, innerFrame0.Max, fillColor0, rounding, t<1 ? ImDrawFlags_RoundCornersLeft : ImDrawFlags_RoundCornersAll);
    }
    if (t<1) {
	ImU32 fillColor1 = pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[7] : pOptionalEightColors[6]) :
        (GetColorU32((held || hovered) ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg));
        window->DrawList->AddRectFilled(innerFrame1.Min, innerFrame1.Max, fillColor1, rounding, t>0 ?  ImDrawFlags_RoundCornersRight : ImDrawFlags_RoundCornersAll);
    }
    if (style.FrameBorderSize)   {
        ImRect innerFrame(innerFrame0.Min,innerFrame1.Max);
        window->DrawList->AddRect(innerFrame.Min+ImVec2(1,1), innerFrame.Max+ImVec2(1,1), GetColorU32(ImGuiCol_BorderShadow), rounding);
        window->DrawList->AddRect(innerFrame.Min, innerFrame.Max, GetColorU32(ImGuiCol_Border), rounding);
    }
    int numSegments = (int)(check_bb_height*0.8f);if (numSegments<3) numSegments=3;else if (numSegments>24) numSegments = 24;
    float radius = check_bb_height*0.5f;
    if (widthFraction<radius) widthFraction=radius;
    else if (widthFraction>check_bb_width-radius) widthFraction=check_bb_width-radius;
    ImVec2 center(check_bb.Min.x+widthFraction,check_bb.Min.y+check_bb_height*0.5f);
    // All the 4 circle colors will be forced to have A = 255
    const ImGuiCol defaultCircleColorOn =	    ImGuiCol_Text;
    const ImGuiCol defaultCircleColorOnHovered =    ImGuiCol_Text;
    const ImGuiCol defaultCircleColorOff =	    ImGuiCol_TextDisabled;
    const ImGuiCol defaultCircleColorOffHovered =   ImGuiCol_TextDisabled;
    if (!animationActive) {
	if (*v) {
	    ImU32 circleColorOn = pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[1] : pOptionalEightColors[0]) :
			(GetColorU32((held || hovered) ? defaultCircleColorOnHovered : defaultCircleColorOn));
	    int col = (circleColorOn & ~IM_COL32_A_MASK) | (0xFF << IM_COL32_A_SHIFT);
	    window->DrawList->AddCircleFilled(center,radius,col,numSegments);
	}
	else {
	    ImU32 circleColorOff = pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[3] : pOptionalEightColors[2]) :
			(GetColorU32((held || hovered) ? defaultCircleColorOffHovered : defaultCircleColorOff));
	    int col = (circleColorOff & ~IM_COL32_A_MASK) | (0xFF << IM_COL32_A_SHIFT);
	    window->DrawList->AddCircleFilled(center,radius,col,numSegments);
	}
    }
    else {
	int col1 = (int) (pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[1] : pOptionalEightColors[0]) :
			(GetColorU32((held || hovered) ? defaultCircleColorOnHovered : defaultCircleColorOn)));
	int col0 = (int) (pOptionalEightColors ? ((held || hovered) ? pOptionalEightColors[3] : pOptionalEightColors[2]) :
			(GetColorU32((held || hovered) ? defaultCircleColorOffHovered : defaultCircleColorOff)));
	int r = ImLerp((col0 >> IM_COL32_R_SHIFT) & 0xFF, (col1 >> IM_COL32_R_SHIFT) & 0xFF, t);
	int g = ImLerp((col0 >> IM_COL32_G_SHIFT) & 0xFF, (col1 >> IM_COL32_G_SHIFT) & 0xFF, t);
	int b = ImLerp((col0 >> IM_COL32_B_SHIFT) & 0xFF, (col1 >> IM_COL32_B_SHIFT) & 0xFF, t);
	int col = (r << IM_COL32_R_SHIFT) | (g << IM_COL32_G_SHIFT) | (b << IM_COL32_B_SHIFT) | (0xFF << IM_COL32_A_SHIFT);
	window->DrawList->AddCircleFilled(center,radius,col,numSegments);
    }
    // ==================================================================================

    //if (g.LogEnabled) LogRenderedText(&text_bb.Min, *v ? "[x]" : "[ ]");
    if (label_size.x > 0.0f)    RenderText(text_bb.Min, label);

    return pressed;
}
bool CheckboxStyledFlags(const char* label, unsigned int* flags, unsigned int flags_value, const ImU32 *pOptionalEightColors, const ImVec2 &checkBoxScale,float checkBoxRounding)  {
    bool v = ((*flags & flags_value) == flags_value);
    bool pressed = CheckboxStyled(label, &v, pOptionalEightColors, checkBoxScale, checkBoxRounding);
    if (pressed)    {
        if (v)  *flags |= flags_value;
        else    *flags &= ~flags_value;
    }
    return pressed;
}
// End CheckBoxStyled ================================================================================================

// Posted by @alexsr here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
void LoadingIndicatorCircle(const char* label, float indicatorRadiusFactor,
                                   const ImVec4* pOptionalMainColor, const ImVec4* pOptionalBackdropColor,
                                   int circle_count,const float speed) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems) {
        return;
    }

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);
    const ImGuiStyle& style = GetStyle();

    if (circle_count<=0) circle_count = 12;
    if (indicatorRadiusFactor<=0.f) indicatorRadiusFactor = 1.f;
    if (!pOptionalMainColor)        pOptionalMainColor = &style.Colors[ImGuiCol_Button];
    if (!pOptionalBackdropColor)    pOptionalBackdropColor = &style.Colors[ImGuiCol_ButtonHovered];

    const float lineHeight = GetTextLineHeight(); // or GetTextLineHeight() or GetTextLineHeightWithSpacing() ?
    float indicatorRadiusPixels = indicatorRadiusFactor*lineHeight*0.5f;

    const ImVec2 pos = window->DC.CursorPos;
    const float circle_radius = indicatorRadiusPixels / 8.f;
    indicatorRadiusPixels-= 2.0f*circle_radius;
    const ImRect bb(pos, ImVec2(pos.x + indicatorRadiusPixels*2.f+4.f*circle_radius,
                                pos.y + indicatorRadiusPixels*2.f+4.f*circle_radius));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id)) {
        return;
    }
    const float base_num_segments = circle_radius*1.f;
    const double t = g.Time;
    const float degree_offset = 2.0f * IM_PI / circle_count;
    for (int i = 0; i < circle_count; ++i) {
        const float sinx = -ImSin(degree_offset * i);
        const float cosx = ImCos(degree_offset * i);
        const float growth = ImMax(0.0f, ImSin((float)(t*(double)(speed*3.0f)-(double)(i*degree_offset))));
        ImVec4 color;
        color.x = pOptionalMainColor->x * growth + pOptionalBackdropColor->x * (1.0f - growth);
        color.y = pOptionalMainColor->y * growth + pOptionalBackdropColor->y * (1.0f - growth);
        color.z = pOptionalMainColor->z * growth + pOptionalBackdropColor->z * (1.0f - growth);
        color.w = 1.0f;
        float grown_circle_radius = circle_radius*(1.0f + growth);
        int num_segments = (int)(base_num_segments*grown_circle_radius);
        if (num_segments<4) num_segments=4;
        window->DrawList->AddCircleFilled(ImVec2(pos.x+2.f*circle_radius + indicatorRadiusPixels*(1.0f+sinx),
                                                 pos.y+2.f*circle_radius + indicatorRadiusPixels*(1.0f+cosx)),
                                                 grown_circle_radius,
                                                 GetColorU32(color),num_segments);
    }
}

// Posted by @zfedoran here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
void LoadingIndicatorCircle2(const char* label,float indicatorRadiusFactor, float indicatorRadiusThicknessFactor, const ImVec4* pOptionalColor) {
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    if (indicatorRadiusFactor<=0.f) indicatorRadiusFactor = 1.f;
    if (indicatorRadiusThicknessFactor<=0.f) indicatorRadiusThicknessFactor = 1.f;
    if (!pOptionalColor)    pOptionalColor = &style.Colors[ImGuiCol_Button];
    const ImU32 color = GetColorU32(*pOptionalColor);

    const float lineHeight = GetTextLineHeight(); // or GetTextLineHeight() or GetTextLineHeightWithSpacing() ?
    float indicatorRadiusPixels = indicatorRadiusFactor*lineHeight*0.5f;
    float indicatorThicknessPixels = indicatorRadiusThicknessFactor*indicatorRadiusPixels*0.6f;
    if (indicatorThicknessPixels>indicatorThicknessPixels*0.4f) indicatorThicknessPixels=indicatorThicknessPixels*0.4f;
    indicatorRadiusPixels-=indicatorThicknessPixels;

    ImVec2 pos = window->DC.CursorPos;
    ImVec2 size(indicatorRadiusPixels*2.f, (indicatorRadiusPixels + style.FramePadding.y)*2.f);

    const ImRect bb(pos, ImVec2(pos.x + size.x, pos.y + size.y));
    ItemSize(bb, style.FramePadding.y);
    if (!ItemAdd(bb, id))
        return;

    // Render
    window->DrawList->PathClear();



    //int num_segments = indicatorRadiusPixels/8.f;
    //if (num_segments<4) num_segments=4;

    int num_segments = 30;

    int start = abs(ImSin(g.Time*1.8f)*(num_segments-5));

    const float a_min = IM_PI*2.0f * ((float)start) / (float)num_segments;
    const float a_max = IM_PI*2.0f * ((float)num_segments-3) / (float)num_segments;

    const ImVec2 centre = ImVec2(pos.x+indicatorRadiusPixels, pos.y+indicatorRadiusPixels+style.FramePadding.y);

    for (int i = 0; i < num_segments; i++) {
        const float a = a_min + ((float)i / (float)num_segments) * (a_max - a_min);
        window->DrawList->PathLineTo(ImVec2(centre.x + ImCos(a+g.Time*8) * indicatorRadiusPixels,
                                            centre.y + ImSin(a+g.Time*8) * indicatorRadiusPixels));
    }

    window->DrawList->PathStroke(color, false, indicatorThicknessPixels);
}


} // namespace ImGui

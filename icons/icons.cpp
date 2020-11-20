#include "imgui.h"

bool ImGui::LoadInternalIcons(ImFontAtlas* atlas)
{
    // FileDialog icons
    static const ImWchar icons_ranges[] = { ICON_MIN_IGFD, ICON_MAX_IGFD, 0 };
	ImFontConfig icons_config; icons_config.MergeMode = true; icons_config.PixelSnapH = true;
	atlas->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFD, 15.0f, &icons_config, icons_ranges);
    
    return true;
}
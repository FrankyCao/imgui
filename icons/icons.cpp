#include "imgui.h"

bool ImGui::LoadInternalIcons(ImFontAtlas* atlas)
{
    ImFontConfig icons_config; 
    icons_config.MergeMode = true; 
    icons_config.PixelSnapH = true;
    // FileDialog Icons
    static const ImWchar icons_ranges[] = { ICON_MIN_IGFD, ICON_MAX_IGFD, 0 };
	atlas->AddFontFromMemoryCompressedBase85TTF(FONT_ICON_BUFFER_NAME_IGFD, 16.0f, &icons_config, icons_ranges);
    
    // Audio Icons
    static const ImWchar fad_icons_ranges[] = { ICON_MIN_FAD, ICON_MAX_FAD, 0 };
	atlas->AddFontFromMemoryCompressedBase85TTF(fontaudio_compressed_data_base85, 16.0f, &icons_config, fad_icons_ranges);

    // Awesome brands Icons
    static const ImWchar fab_icons_ranges[] = { ICON_MIN_FAB, ICON_MAX_FAB, 0 };
	atlas->AddFontFromMemoryCompressedBase85TTF(fa_brands_compressed_data_base85, 16.0f, &icons_config, fab_icons_ranges);

    // Awesome 4 Icons
    static const ImWchar fa4_icons_ranges[] = { ICON_MIN_FA4, ICON_MAX_FA4, 0 };
	atlas->AddFontFromMemoryCompressedBase85TTF(webfont_compressed_data_base85, 16.0f, &icons_config, fa4_icons_ranges);

    // Awesome 5 Icons
    static const ImWchar fa5_icons_ranges[] = { ICON_MIN_FA5, ICON_MAX_FA5, 0 };
	atlas->AddFontFromMemoryCompressedBase85TTF(fa_regular_compressed_data_base85, 16.0f, &icons_config, fa5_icons_ranges);
    atlas->AddFontFromMemoryCompressedBase85TTF(fa_solid_compressed_data_base85, 16.0f, &icons_config, fa5_icons_ranges);

    // Fork Awesome Icons
    static const ImWchar fk_icons_ranges[] = { ICON_MIN_FK, ICON_MAX_FK, 0 };
	atlas->AddFontFromMemoryCompressedBase85TTF(fork_webfont_compressed_data_base85, 16.0f, &icons_config, fk_icons_ranges);

    // Fork Material Design Icons
    static const ImWchar md_icons_ranges[] = { ICON_MIN_MD, ICON_MAX_MD, 0 };
	atlas->AddFontFromMemoryCompressedBase85TTF(MaterialIcons_compressed_data_base85, 16.0f, &icons_config, md_icons_ranges);
    return true;
}
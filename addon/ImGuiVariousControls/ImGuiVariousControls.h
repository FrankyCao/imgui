// This software is provided 'as-is', without any express or implied
// warranty.  In no event will the authors be held liable for any damages
// arising from the use of this software.
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it
// freely, subject to the following restrictions:
// 1. The origin of this software must not be misrepresented; you must not
//    claim that you wrote the original software. If you use this software
//    in a product, an acknowledgment in the product documentation would be
//    appreciated but is not required.
// 2. Altered source versions must be plainly marked as such, and must not be
//    misrepresented as being the original software.
// 3. This notice may not be removed or altered from any source distribution.


#ifndef IMGUIVARIOUSCONTROLS_H_
#define IMGUIVARIOUSCONTROLS_H_

#ifndef IMGUI_API
#include <imgui.h>
#endif //IMGUI_API
#include <imgui_helper.h>

// USAGE
/*
#include "imguivariouscontrols.h"

// inside a ImGui::Window:
ImGui::TestProgressBar();

ImGui::TestPopupMenuSimple();
*/



namespace ImGui {

#ifndef IMGUIHELPER_H_
IMGUI_API bool IsItemActiveLastFrame();
IMGUI_API bool IsItemJustReleased();
#endif //IMGUIHELPER_H_

IMGUI_API bool CheckButton(const char* label,bool* pvalue);
IMGUI_API bool SmallCheckButton(const char* label,bool* pvalue);
IMGUI_API void ToggleButton(const char* str_id, bool* v);
IMGUI_API bool ToggleButton(const char *str_id, bool *v, const ImVec2 &size);
IMGUI_API bool BulletToggleButton(const char* label,bool* v, ImVec2 &pos, ImVec2 &size);

// Please note that you can tweak the "format" argument if you want to add a prefix (or a suffix) piece of text to the text that appears at the right of the bar.
// returns the value "fraction" in 0.f-1.f.
// It does not need any ID.
IMGUI_API float ProgressBar(const char* optionalPrefixText,float value,const float minValue=0.f,const float maxValue=1.f,const char* format="%1.0f%%",const ImVec2& sizeOfBarWithoutTextInPixels=ImVec2(-1,-1),
                const ImVec4& colorLeft=ImVec4(0,1,0,0.8),const ImVec4& colorRight=ImVec4(0,0.4,0,0.8),const ImVec4& colorBorder=ImVec4(0.25,0.25,1.0,1));

IMGUI_API void TestProgressBar();

// Single column popup menu without icon support. It disappears when the mouse goes away.
// Returns -1 when no entries has been selected.
// Optional scrollUpEntryText returns index of -2,scrollDownEntryText -3 (but they must be manually handled by the user)
IMGUI_API int PopupMenuSimple(bool& open, const char** pEntries, int numEntries, const char* optionalTitle=NULL, int* pOptionalHoveredEntryOut=NULL, int startIndex=0, int endIndex=-1, bool reverseItems=false, const char* scrollUpEntryText=NULL, const char* scrollDownEntryText=NULL);

// returns -1 if nothing has been chosen, 0 if copy has been clicked, 1 if cut has been clicked and 2 if paste has been clicked
IMGUI_API int PopupMenuSimpleCopyCutPasteOnLastItem(bool readOnly=false);

class PopupMenuSimpleParams {
public:
    bool open;
    int getSelectedEntry() const {return selectedEntry;}    // optional (use PopupMenuSimple(...) return value)
//protected:
    int selectedEntry;
    int hoveredEntry;
    int endIndex;
    int startIndex;
    float scrollTimer;
    bool resetScrollingWhenRestart;
public:
    PopupMenuSimpleParams(bool _resetScrollingWhenRestart=true)
    : open(false),selectedEntry(-1),hoveredEntry(-1),endIndex(-1),startIndex(-1),scrollTimer(ImGui::GetTime()),resetScrollingWhenRestart(_resetScrollingWhenRestart)
    {}
    //int PopupMenuSimple(PopupMenuSimpleParams& params,const char** pTotalEntries,int numTotalEntries,int numAllowedEntries,bool reverseItems,const char* optionalTitle,const char* scrollUpEntryText,const char* scrollDownEntryText);
};

IMGUI_API int PopupMenuSimple(PopupMenuSimpleParams& params,const char** pTotalEntries,int numTotalEntries,int numAllowedEntries,bool reverseItems=false,const char* optionalTitle=NULL,const char* scrollUpEntryText="   ^   ",const char* scrollDownEntryText="   v   ");

IMGUI_API void TestPopupMenuSimple(const char* scrollUpEntryText="   ^   ",const char* scrollDownEntryText="   v   ");

// Single column popup menu with icon support. It disappears when the mouse goes away. Never tested.
// User is supposed to create a static instance of it, add entries once, and then call "render()".
class PopupMenu {
protected:
// TODO: Merge IconData into PopupMenuEntry
    struct IconData {
        ImTextureID user_texture_id;
        ImVec2 uv0;
        ImVec2 uv1;
        ImVec4 bg_col;
        ImVec4 tint_col;
        IconData(ImTextureID _user_texture_id=NULL,const ImVec2& _uv0 = ImVec2(0,0),const ImVec2& _uv1 = ImVec2(1,1),const ImVec4& _bg_col = ImVec4(0,0,0,1),const ImVec4& _tint_col = ImVec4(1,1,1,1))
            : user_texture_id(_user_texture_id),uv0(_uv0),uv1(_uv1),bg_col(_bg_col),tint_col(_tint_col)
        {}
        IconData(const IconData& o) {*this = o;}
        inline int compareTo(const IconData& o) const {
            if ((size_t) user_texture_id < (size_t) o.user_texture_id) return -1;
            if (user_texture_id==o.user_texture_id) {
                if (uv0.y < o.uv0.y) return -1;
                if (uv0.y == o.uv0.y)   {
                    if (uv0.x < o.uv0.x) return -1;
                    if (uv0.x == o.uv0.x) return 0;
                }
            }
            return 1;
        }
        const IconData& operator=(const IconData& o) {
            user_texture_id = o.user_texture_id;
            uv0 = o.uv0; uv1 = o.uv1; bg_col = o.bg_col; tint_col = o.tint_col;
            return *this;
        }
    };
public:    
    struct PopupMenuEntry : public IconData  {
    public:
        enum {
            MAX_POPUP_MENU_ENTRY_TEXT_SIZE = 512
        };
        char text[MAX_POPUP_MENU_ENTRY_TEXT_SIZE];
        bool selectable;
        PopupMenuEntry(const char* _text=NULL,bool _selectable=false,ImTextureID _user_texture_id=NULL,const ImVec2& _uv0 = ImVec2(0,0),const ImVec2& _uv1 = ImVec2(1,1),const ImVec4& _bg_col = ImVec4(0,0,0,1),const ImVec4& _tint_col = ImVec4(1,1,1,1))
            :  IconData(_user_texture_id,_uv0,_uv1,_bg_col,_tint_col),selectable(_selectable)
        {
            if (_text)   {
                IM_ASSERT(strlen(_text)<MAX_POPUP_MENU_ENTRY_TEXT_SIZE);
                strcpy(text,_text);
            }
            else text[0]='\0';
        }
        PopupMenuEntry(const PopupMenuEntry& o) : IconData(o) {*this = o;}
        inline int compareTo(const PopupMenuEntry& o) const {
            if (text[0]=='\0')  {
                if (o.text[0]!='\0') return 1;
            }
            else if (o.text[0]=='\0') return -1;
            const int c = strcmp(text,o.text);
            if (c!=0) return c;
            if ((size_t) user_texture_id < (size_t) o.user_texture_id) return -1;
            if (user_texture_id==o.user_texture_id) {
                if (uv0.y < o.uv0.y) return -1;
                if (uv0.y == o.uv0.y)   {
                    if (uv0.x < o.uv0.x) return -1;
                    if (uv0.x == o.uv0.x) return 0;
                }
            }
            return 1;
        }
        const PopupMenuEntry& operator=(const PopupMenuEntry& o) {
            IconData::operator=(o);
            selectable = o.selectable;
            if (o.text[0]!='\0') strcpy(text,o.text);
            else text[0]='\0';
            return *this;
        }
    };

mutable int selectedEntry;  // of last frame. otherwise -1
ImVector <PopupMenuEntry> entries;  // should be protected, but maybe the user wants to modify it at runtime: in case inherit from this class

void addEntryTitle(const char* text,bool addSeparator=true) {
    entries.push_back(PopupMenuEntry(text,false));
    if (addSeparator) addEntrySeparator();
}
void addEntrySeparator() {
    entries.push_back(PopupMenuEntry(NULL,false));
}
void addEntry(const char* _text,ImTextureID _user_texture_id=NULL,const ImVec2& _uv0 = ImVec2(0,0),const ImVec2& _uv1 = ImVec2(1,1),const ImVec4& _bg_col = ImVec4(0,0,0,1),const ImVec4& _tint_col = ImVec4(1,1,1,1))  {
    entries.push_back(PopupMenuEntry(_text,true,_user_texture_id,_uv0,_uv1,_bg_col,_tint_col));
}

// of last frame. otherwise -1
int getSelectedEntry() const {return selectedEntry;}

// please set "open" to "true" when starting popup.
// When the menu closes, you have open==false and as a return value "selectedEntry"
// The returned "selectedEntry" (and "getSelectedEntry()") are !=-1 only at the exact frame the menu entry is selected.
int render(bool& open) const    {
    selectedEntry = -1;
    if (!open) return selectedEntry;
    const int numEntries = (int) entries.size();
    if (numEntries==0) {
        open = false;
        return selectedEntry;
    }    

    static const ImVec4 transparentColor(1,1,1,0);   
    ImGui::PushStyleColor(ImGuiCol_Button,transparentColor);
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered,ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
    ImVec2 iconSize;iconSize.x = iconSize.y = ImGui::GetTextLineHeight();

    ImGui::PushID(&entries);
    //ImGui::BeginPopup(&open);
    ImGui::OpenPopup("MyOwnMenu");
    if (ImGui::BeginPopup("MyOwnMenu")) {
        bool imageClicked = false;
        for (int i = 0; i < numEntries; i++)    {
            const PopupMenuEntry& entry = entries[i];
            imageClicked = false;
            if (entry.user_texture_id) {
                imageClicked = ImGui::ImageButton((void*)entry.user_texture_id,iconSize,entry.uv0,entry.uv1,0,entry.bg_col,entry.tint_col) && entry.selectable;
                ImGui::SameLine();
            }
            if (strlen(entry.text)==0) ImGui::Separator();
            else if (entry.selectable)  {
                if (ImGui::Selectable(entry.text, false) || imageClicked)  {
                    selectedEntry = i;
                    open = false;    // Hide menu
                }
            }
            else ImGui::Text("%s",entry.text);
        }
        if (open)   // close menu when mouse goes away
        {
            ImVec2 pos = ImGui::GetWindowPos();pos.x-=5;pos.y-=5;
            ImVec2 size = ImGui::GetWindowSize();size.x+=10;size.y+=10;
            const ImVec2& mousePos = ImGui::GetIO().MousePos;
            if (mousePos.x<pos.x || mousePos.y<pos.y || mousePos.x>pos.x+size.x || mousePos.y>pos.y+size.y) open = false;
        }
    }
    ImGui::EndPopup();
    ImGui::PopID();
    ImGui::PopStyleColor(2);

    return selectedEntry;
}

bool isEmpty() const {return entries.size()==0;}

};

// Based on the code from: https://github.com/benoitjacquier/imgui
IMGUI_API bool ColorChooser(bool* open,ImVec4* pColorOut=NULL, bool supportsAlpha=true);
// Based on the code from: https://github.com/benoitjacquier/imgui
IMGUI_API bool ColorCombo(const char* label,ImVec4 *pColorOut=NULL,bool supportsAlpha=false,float width=0.f,bool closeWhenMouseLeavesIt=true);

// label is used as id
// <0 frame_padding uses default frame padding settings. 0 for no padding
IMGUI_API bool ImageButtonWithText(ImTextureID texId,const char* label,const ImVec2& imageSize=ImVec2(0,0), const ImVec2& uv0 = ImVec2(0,0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));

#ifndef NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE
// One instance per image can feed multiple widgets
struct AnimatedImage {
    // TODO: load still images as fallback and load gifs from memory
    public:
    typedef void (*FreeTextureDelegate)(ImTextureID texid);
    typedef void (*GenerateOrUpdateTextureDelegate)(ImTextureID& imtexid,int width,int height,int channels,const unsigned char* pixels,bool useMipmapsIfPossible,bool wraps,bool wrapt,bool,bool);
    void SetFreeTextureCallback(FreeTextureDelegate freeTextureCb) {FreeTextureCb=freeTextureCb;}
    void SetGenerateOrUpdateTextureCallback(GenerateOrUpdateTextureDelegate generateOrUpdateTextureCb) {GenerateOrUpdateTextureCb=generateOrUpdateTextureCb;}

#	ifndef STBI_NO_GIF
#   ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    IMGUI_API AnimatedImage(char const *gif_filepath,bool useHoverModeIfSupported=false); // 'hoverMode' is supported only if all frames fit 'MaxPersistentTextureSize'
#   endif //IMGUIVARIOUSCONTROLS_NO_STDIO
    IMGUI_API AnimatedImage(const unsigned char* gif_buffer,int gif_buffer_size,bool useHoverModeIfSupported=false); // 'hoverMode' is supported only if all frames fit 'MaxPersistentTextureSize'
#	endif //STBI_NO_GIF
    IMGUI_API AnimatedImage(ImTextureID myTexId,int animationImageWidth,int animationImageHeight,int numFrames,int numFramesPerRowInTexture,int numFramesPerColumnInTexture,float delayBetweenFramesInCs,bool useHoverMode=false); // 'hoverMode' always available. 'myTexId' is yours.
    IMGUI_API AnimatedImage();    // You'll need to manually call 'load' o 'create'
    IMGUI_API ~AnimatedImage();   // calls 'clear'
    IMGUI_API void clear();   // releases the textures that are created inside the class

    // Main methods
    IMGUI_API void render(ImVec2 size=ImVec2(0,0), const ImVec2& uv0=ImVec2(0,0), const ImVec2& uv1=ImVec2(1,1), const ImVec4& tint_col=ImVec4(1,1,1,1), const ImVec4& border_col=ImVec4(0,0,0,0)) const;
    IMGUI_API bool renderAsButton(const char* label,ImVec2 size=ImVec2(0,0), const ImVec2& uv0 = ImVec2(0,0),  const ImVec2& uv1 = ImVec2(1,1), int frame_padding = -1, const ImVec4& bg_col = ImVec4(0,0,0,0), const ImVec4& tint_col = ImVec4(1,1,1,1));    // <0 frame_padding uses default frame padding settings. 0 for no padding

    // Less useful methods
#	ifndef STBI_NO_GIF
#   ifndef IMGUIVARIOUSCONTROLS_NO_STDIO
    IMGUI_API bool load(char const *gif_filepath,bool useHoverModeIfSupported=false); // 'hoverMode' is supported only if all frames fit 'MaxPersistentTextureSize'
#   endif //IMGUIVARIOUSCONTROLS_NO_STDIO
    IMGUI_API bool load_from_memory(const unsigned char* gif_buffer,int gif_buffer_size,bool useHoverModeIfSupported=false);  // 'hoverMode' is supported only if all frames fit 'MaxPersistentTextureSize'
#	endif //STBI_NO_GIF
    IMGUI_API bool create(ImTextureID myTexId,int animationImageWidth,int animationImageHeight,int numFrames,int numFramesPerRowInTexture,int numFramesPerColumnInTexture,float delayBetweenFramesInCs,bool useHoverMode=false); // 'hoverMode' always available. 'myTexId' is yours.
    IMGUI_API int getWidth() const;
    IMGUI_API int getHeight() const;
    IMGUI_API int getNumFrames() const;
    IMGUI_API bool areAllFramesInASingleTexture() const;  // when true, 'hoverMode' was available in ctr/load/create (but it can't change at runtime)

    static ImVec2 MaxPersistentTextureSize;   // 2048,2048 (Enlarge the buffer if needed for 'hoverMode': but using smaller animated images and less frames is better)

    private:
    AnimatedImage(const AnimatedImage& ) {}     // Actually maybe we could allow some of these for containers...
    void operator=(const AnimatedImage& ) {}
    static FreeTextureDelegate FreeTextureCb;
    static GenerateOrUpdateTextureDelegate GenerateOrUpdateTextureCb;
    friend struct AnimatedImageInternal;
    struct AnimatedImageInternal* ptr;
};
#endif //NO_IMGUIVARIOUSCONTROLS_ANIMATEDIMAGE

// zoomCenter is panning in [(0,0),(1,1)]
// returns true if some user interaction have been processed
IMGUI_API bool ImageZoomAndPan(ImTextureID user_texture_id, const ImVec2& size,float aspectRatio,float& zoom,ImVec2& zoomCenter,int panMouseButtonDrag=1,int resetZoomAndPanMouseButton=2,const ImVec2& zoomMaxAndZoomStep=ImVec2(16.f,1.025f));

class InputTextWithAutoCompletionData  {
public:
    // auto completion:
    int deltaTTItems;                           // modified by UP/DOWN keys
    bool tabPressed;                            // triggers autocompletion
    ImVector<char> newTextToSet;                // needed because ImGui does not allow changing an InputText(...) buffer directly, while it's active
    int itemPositionOfReturnedText;
    int itemIndexOfReturnedText;
    int additionalFlags,bufTextLen,lastSelectedTTItemIndex;
    bool inited;                                // turns true the first time a method that use this class is called

public:
    int currentAutocompletionItemIndex;         // completely user-side (if!=-1, that item is displayed in a different way in the autocompletion menu)
    InputTextWithAutoCompletionData(ImGuiInputTextFlags _additionalFlags=0,int _currentAutocompletionItemIndex=-1) : deltaTTItems(0),tabPressed(false),itemPositionOfReturnedText(-1),itemIndexOfReturnedText(-1),
    additionalFlags(_additionalFlags&(ImGuiInputTextFlags_CharsDecimal|ImGuiInputTextFlags_CharsHexadecimal|ImGuiInputTextFlags_CharsNoBlank|ImGuiInputTextFlags_CharsUppercase)),bufTextLen(-1),lastSelectedTTItemIndex(-1),
    inited(false),currentAutocompletionItemIndex(_currentAutocompletionItemIndex) {}

    bool isInited() const {return inited;}      // added just for my laziness (to init elements inside DrawGL() of similiar)
    int getItemPositionOfReturnedText() const {return itemPositionOfReturnedText;}  // usable only after "return" is pressed: it returns the item position at which the newly entered text can be inserted, or -1
    int getItemIndexOfReturnedText() const {return itemIndexOfReturnedText;}        // usable only after "return" is pressed: it returns the item index that exactly matches the newly entered text, or -1

    static float Opacity;   // 0.6f;

    //friend bool InputTextWithAutoCompletion(const char* label, char* buf, size_t buf_size,InputTextWithAutoCompletionData* pAutocompletion_data, bool (*autocompletion_items_getter)(void*, int, const char**), int autocompletion_items_size, void* autocompletion_user_data, int num_visible_autocompletion_items);
    //friend int DefaultInputTextAutoCompletionCallback(ImGuiInputTextCallbackData *data);

    // Some useful helper methods
    IMGUI_API static int HelperGetItemInsertionPosition(const char* txt,bool (*items_getter)(void*, int, const char**), int items_count, void* user_data=NULL,bool* item_is_already_present_out=NULL);
    IMGUI_API static int HelperInsertItem(const char* txt,bool (*items_getter)(void*, int, const char**),bool (*items_inserter)(void*, int,const char*), int items_count, void* user_data=NULL,bool* item_is_already_present_out=NULL);
};
IMGUI_API bool InputTextWithAutoCompletion(const char* label, char* buf, size_t buf_size, InputTextWithAutoCompletionData* pAutocompletion_data, bool (*autocompletion_items_getter)(void*, int, const char**), int autocompletion_items_size, void* autocompletion_user_data=NULL, int num_visible_autocompletion_items=-1);


class InputComboWithAutoCompletionData : public InputTextWithAutoCompletionData {
public:
    int inputTextShown;
    ImVector<char> buf;
    bool isRenaming;
    bool itemHovered,itemActive;
public:
    InputComboWithAutoCompletionData(ImGuiInputTextFlags additionalInputTextFlags=0) : InputTextWithAutoCompletionData(additionalInputTextFlags),inputTextShown(0),isRenaming(false),itemHovered(false),itemActive(false) {}

    bool isInited() const {return inited;}              // added just for my laziness (to init elements inside DrawGL() of similiar)
    bool isItemHovered() const {return itemHovered;}    // well, this widget is made of 2 widgets with buttons, so ImGui::IsItemHovered() does not always work
    bool isItemActive() const {return itemActive;}      // well, this widget is made of 2 widgets with buttons, so ImGui::IsItemActive() does not always work
    bool isInputTextVisibleNextCall() const {return inputTextShown!=0;}
    const char* getInputTextBuffer() const {return buf.size()>0 ? &buf[0] : NULL;}

    static char ButtonCharcters[3][5];  // = {"+","r","x"};
    static char ButtonTooltips[3][128]; // = {"add","rename","delete"};

    //friend bool InputComboWithAutoCompletion(const char* label, int *current_item, size_t autocompletion_buffer_size, InputComboWithAutoCompletionData* pAutocompletion_data,
    //                              bool (*items_getter)(void*, int, const char**),       // gets item at position ... (cannot be NULL)
    //                              bool (*items_inserter)(void*, int,const char*),       // inserts item at position ... (cannot be NULL)
    //                              bool (*items_deleter)(void*, int),                    // deletes item at position ... (can be NULL)
    //                              bool (*items_renamer)(void *, int, int, const char *),// deletes item at position, and inserts renamed item at new position  ... (can be NULL)
    //                              int items_count, void* user_data, int num_visible_items);
};
IMGUI_API bool InputComboWithAutoCompletion(const char* label,int* current_item,size_t autocompletion_buffer_size,InputComboWithAutoCompletionData* pAutocompletion_data,
    bool (*items_getter)(void*, int, const char**),  // gets item at position ... (cannot be NULL)
    bool (*items_inserter)(void*, int,const char*),  // inserts item at position ... (cannot be NULL)
    bool (*items_deleter)(void*, int),               // deletes item at position ... (can be NULL)
    bool (*items_renamer)(void*,int,int,const char*),// deletes item at position, and inserts renamed item at new position  ... (can be NULL)
    int items_count, void* user_data=NULL, int num_visible_items=-1);

// mobile lock control: a very unsafe way of using password
// passwordSize: must be: gridSize*gridSize+1, where gridSize is in [2,6]
// size: is the width and height of the widget in pixels
// colors: please copy defaultColors in the definition (.cpp file), and modify it as needed.
typedef int ImGuiPasswordDrawerFlags;
IMGUI_API bool PasswordDrawer(char* password, int passwordSize, ImGuiPasswordDrawerFlags flags=0, const float size=0, const ImU32 colors[7]=NULL);

} // namespace ImGui

enum ImGuiPasswordDrawerFlags_  {
    ImGuiPasswordDrawerFlags_ReadOnly         = 1 << 1,   // password is not touched, it's simply shown. Returns true when the (whole) widget pressed. [Note that passwordSize must still be gridSize*gridSize+1, even if the (untouched) password buffer is shorter]
    ImGuiPasswordDrawerFlags_NoFilledCircles  = 1 << 2,   // Filled circles are hidden
    ImGuiPasswordDrawerFlags_NoLines          = 1 << 4,   // Draw lines are hidden
    ImGuiPasswordDrawerFlags_Hidden           =           // Everything is hidden [to prevent someone from spotting your password]
    ImGuiPasswordDrawerFlags_NoFilledCircles|ImGuiPasswordDrawerFlags_NoLines
};


namespace ImGui {

// Experimental: CheckboxFlags(...) overload to handle multiple flags with a single call
// returns the value of the pressed flag (not the index of the check box), or zero
// flagAnnotations, when!=0, just displays a circle in the required checkboxes
// itemHoveredOut, when used, returns the index of the hovered check box (not its flag), or -1.
// pFlagsValues, when used, must be numFlags long, and must contain the flag values (not the flag indices) that the control must use.
// KNOWN BUG: When ImGui::SameLine() is used after it, the alignment is broken
IMGUI_API unsigned int CheckboxFlags(const char* label,unsigned int* flags,int numFlags,int numRows,int numColumns,unsigned int flagAnnotations=0,int* itemHoveredOut=NULL,const unsigned int* pFlagsValues=NULL);

// These just differ from the default ones for their look:
// checkBoxScale.y max is clamped to 2.f
// pOptionalEightColors are: {circle_on, circle_on_hovered, circle_off, circle_off_hovered, bg_on, bg_on_hovered, bg_off, bg_off_hovered} [The 4 circle colors will have A = 255, even if users choose otherwise]
// checkBoxRounding if negative defaults to style.WindowRounding
IMGUI_API bool CheckboxStyled(const char* label, bool* v, const ImU32 *pOptionalEightColors=NULL, const ImVec2 &checkBoxScale=ImVec2(1.f,1.f), float checkBoxRounding=-1.f);
IMGUI_API bool CheckboxStyledFlags(const char* label, unsigned int* flags, unsigned int flags_value,const ImU32 *pOptionalEightColors=NULL,const ImVec2 &checkBoxScale=ImVec2(1.f,1.f), float checkBoxRounding=-1.f);

// Posted by @alexsr here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
IMGUI_API void LoadingIndicatorCircle(const char* label, float indicatorRadiusFactor=1.f,
                                   const ImVec4* pOptionalMainColor=NULL, const ImVec4* pOptionalBackdropColor=NULL,
                                   int circle_count=8, const float speed=1.f);

// Posted by @zfedoran here: https://github.com/ocornut/imgui/issues/1901
// Sligthly modified to provide default behaviour with default args
void LoadingIndicatorCircle2(const char* label, float indicatorRadiusFactor=1.f, float indicatorRadiusThicknessFactor=1.f, const ImVec4* pOptionalColor=NULL);

// Splitter
IMGUI_API bool          Splitter(bool split_vertically, float thickness, float* size1, float* size2, float min_size1, float min_size2, float splitter_long_axis_size = -1.0f);
} // namespace ImGui

#endif

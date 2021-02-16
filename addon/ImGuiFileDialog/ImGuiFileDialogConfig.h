#pragma once

// uncomment and modify defines under for customize ImGuiFileDialog

//#define MAX_FILE_DIALOG_NAME_BUFFER 1024
//#define MAX_PATH_BUFFER_SIZE 1024

//#define USE_EXPLORATION_BY_KEYS
// this mapping by default is for GLFW but you can use another
//#include <GLFW/glfw3.h> 
// Up key for explore to the top
//#define IGFD_KEY_UP GLFW_KEY_UP
// Down key for explore to the bottom
//#define IGFD_KEY_DOWN GLFW_KEY_DOWN
// Enter key for open directory
//#define IGFD_KEY_ENTER GLFW_KEY_ENTER
// BackSpace for comming back to the last directory
//#define IGFD_KEY_BACKSPACE GLFW_KEY_BACKSPACE

// widget
// filter combobox width
//#define FILTER_COMBO_WIDTH 120.0f
// button widget use for compose path
//#define IMGUI_PATH_BUTTON ImGui::Button
// standar button
//#define IMGUI_BUTTON ImGui::Button

// locales string
#ifdef IMGUI_INTERNAL_ICONS
#define createDirButtonString ICON_IGFD_ADD
#define okButtonString ICON_IGFD_OK " OK"
#define cancelButtonString ICON_IGFD_CANCEL " Cancel"
#define resetButtonString ICON_IGFD_RESET
#define drivesButtonString ICON_IGFD_DRIVES
//#define searchString ICON_IGFD_SEARCH
#define dirEntryString ICON_IGFD_FOLDER
#define linkEntryString ICON_IGFD_LINK
#define fileEntryString ICON_IGFD_FILE
#else
#define createDirButtonString "+"
#define okButtonString " OK"
#define cancelButtonString " Cancel"
#define resetButtonString "R"
#define drivesButtonString "Drives"
//#define searchString " Search"
#define dirEntryString "[DIR] "
#define linkEntryString "[LINK] "
#define fileEntryString "[FILE] "
#endif
//#define fileNameString "File Name : "
//#define dirNameString "Directory Path :"
//#define buttonResetSearchString "Reset search"
//#define buttonDriveString "Drives"
//#define buttonResetPathString "Reset to current directory"
//#define buttonCreateDirString "Create Directory"
//#define OverWriteDialogTitleString "The file Already Exist !"
//#define OverWriteDialogMessageString "Would you like to OverWrite it ?"
#ifdef IMGUI_INTERNAL_ICONS
#define OverWriteDialogConfirmButtonString ICON_IGFD_OK " Confirm"
#define OverWriteDialogCancelButtonString ICON_IGFD_CANCEL " Cancel"
#else
#define OverWriteDialogConfirmButtonString "Confirm"
#define OverWriteDialogCancelButtonString "Cancel"
#endif

// DateTimeFormat
// see strftime functionin <ctime> for customize
// "%Y/%m/%d %H:%M" give 2021:01:22 11:47
// "%Y/%m/%d %i:%M%p" give 2021:01:22 11:45PM
//#define DateTimeFormat "%Y/%m/%d %i:%M%p"

// theses icons will appear in table headers
#define USE_CUSTOM_SORTING_ICON
#ifdef IMGUI_INTERNAL_ICONS
#define tableHeaderAscendingIcon ICON_IGFD_CHEVRON_UP
#define tableHeaderDescendingIcon ICON_IGFD_CHEVRON_DOWN
#else
#define tableHeaderAscendingIcon "A|"
#define tableHeaderDescendingIcon "D|"
#endif
#define tableHeaderFileNameString " File name"
//#define tableHeaderFileTypeString " Type"
#define tableHeaderFileSizeString " Size"
#define tableHeaderFileDateTimeString " Date"

#define USE_BOOKMARK
//#define bookmarkPaneWith 150.0f
//#define IMGUI_TOGGLE_BUTTON ToggleButton
#ifdef IMGUI_INTERNAL_ICONS
#define bookmarksButtonString ICON_IGFD_BOOKMARK
#define addBookmarkButtonString ICON_IGFD_ADD
#define removeBookmarkButtonString ICON_IGFD_REMOVE
#else
#define bookmarksButtonString "Bookmark"
#define bookmarksButtonHelpString "Bookmark"
#define addBookmarkButtonString "+"
#define removeBookmarkButtonString "-"
#endif

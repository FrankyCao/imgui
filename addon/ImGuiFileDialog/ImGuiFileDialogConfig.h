#pragma once
#include <imgui.h>
// uncomment and modify defines under for customize ImGuiFileDialog

//this options need c++17
//#define USE_STD_FILESYSTEM

//#define MAX_FILE_DIALOG_NAME_BUFFER 1024
//#define MAX_PATH_BUFFER_SIZE 1024

//#define USE_THUMBNAILS
//#if IMGUI_ICONS
//#define DisplayMode_FilesList_ButtonString ICON_IGFD_FILE_LIST
//#define DisplayMode_ThumbailsList_ButtonString ICON_IGFD_FILE_LIST_THUMBNAILS
//#endif
//the thumbnail generation use the stb_image and stb_resize lib who need to define the implementation
//btw if you already use them in your app, you can have compiler error due to "implemntation found in double"
//so uncomment these line for prevent the creation of implementation of these libs again
//#define DONT_DEFINE_AGAIN__STB_IMAGE_IMPLEMENTATION
//#define DONT_DEFINE_AGAIN__STB_IMAGE_RESIZE_IMPLEMENTATION
//#define IMGUI_RADIO_BUTTON RadioButton
//#define DisplayMode_ThumbailsList_ImageHeight 32.0f
//#define tableHeaderFileThumbnailsString "Thumbnails"
//#define DisplayMode_FilesList_ButtonString "FL"
//#define DisplayMode_FilesList_ButtonHelp "File List"
//#define DisplayMode_ThumbailsList_ButtonString "TL"
//#define DisplayMode_ThumbailsList_ButtonHelp "Thumbnails List"
// todo
//#define DisplayMode_ThumbailsGrid_ButtonString "TG"
//#define DisplayMode_ThumbailsGrid_ButtonHelp "Thumbnails Grid"


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

// by ex you can quit the dialog by pressing the key excape
//#define USE_DIALOG_EXIT_WITH_KEY
//#define IGFD_EXIT_KEY GLFW_KEY_ESCAPE

// widget
// filter combobox width
//#define FILTER_COMBO_WIDTH 120.0f
// button widget use for compose path
//#define IMGUI_PATH_BUTTON ImGui::Button
// standard button
//#define IMGUI_BUTTON ImGui::Button

// locales string
#if IMGUI_ICONS
#define createDirButtonString ICON_IGFD_ADD
#define okButtonString ICON_IGFD_OK " OK"
#define cancelButtonString ICON_IGFD_CANCEL " Cancel"
#define resetButtonString ICON_IGFD_RESET
#define drivesButtonString ICON_IGFD_DRIVES
#define dirEntryString ICON_IGFD_FOLDER
#define linkEntryString ICON_IGFD_LINK
#define fileEntryString ICON_IGFD_FILE
#define editPathButtonString ICON_IGFD_EDIT
#define OverWriteDialogConfirmButtonString ICON_IGFD_OK " Confirm"
#define OverWriteDialogCancelButtonString ICON_IGFD_CANCEL " Cancel"
#endif
//#define createDirButtonString "+"
//#define okButtonString " OK"
//#define cancelButtonString " Cancel"
//#define resetButtonString "R"
//#define drivesButtonString "Drives"
//#define editPathButtonString "E"
//#define searchString "Search"
//#define dirEntryString "[DIR] "
//#define linkEntryString "[LINK] "
//#define fileEntryString "[FILE] "
//#define fileNameString "File Name : "
//#define dirNameString "Directory Path :"
//#define buttonResetSearchString "Reset search"
//#define buttonDriveString "Drives"
//#define buttonEditPathString "Edit path\nYou can also right click on path buttons"
//#define buttonResetPathString "Reset to current directory"
//#define buttonCreateDirString "Create Directory"
//#define OverWriteDialogTitleString "The file Already Exist !"
//#define OverWriteDialogMessageString "Would you like to OverWrite it ?"
//#define OverWriteDialogConfirmButtonString "Confirm"
//#define OverWriteDialogCancelButtonString "Cancel"

// DateTimeFormat
// see strftime functionin <ctime> for customize
// "%Y/%m/%d %H:%M" give 2021:01:22 11:47
// "%Y/%m/%d %i:%M%p" give 2021:01:22 11:45PM
//#define DateTimeFormat "%Y/%m/%d %i:%M%p"

// theses icons will appear in table headers
#define USE_CUSTOM_SORTING_ICON
#if IMGUI_ICONS
#define tableHeaderAscendingIcon ICON_IGFD_CHEVRON_UP
#define tableHeaderDescendingIcon ICON_IGFD_CHEVRON_DOWN
#endif
//#define tableHeaderAscendingIcon "A|"
//#define tableHeaderDescendingIcon "D|"
//#define tableHeaderFileNameString " File name"
//#define tableHeaderFileTypeString " Type"
//#define tableHeaderFileSizeString " Size"
//#define tableHeaderFileDateTimeString " Date"
//#define fileSizeBytes "B"
//#define fileSizeKiloBytes "KB"
//#define fileSizeMegaBytes "MB"
//#define fileSizeGigaBytes "GB"
//#define fileSizeTeraBytes "TB"

// default table sort field (must be FIELD_FILENAME, FIELD_TYPE, FIELD_SIZE, FIELD_DATE or FIELD_THUMBNAILS)
//#define defaultSortField FIELD_FILENAME

// default table sort order for each field (true => Descending, false => Ascending)
//#define defaultSortOrderFilename true
//#define defaultSortOrderType true
//#define defaultSortOrderSize true
//#define defaultSortOrderDate true
//#define defaultSortOrderThumbnails true

#define USE_BOOKMARK
#if IMGUI_ICONS
#define bookmarksButtonString ICON_IGFD_BOOKMARK
#define addBookmarkButtonString ICON_IGFD_ADD
#define removeBookmarkButtonString ICON_IGFD_REMOVE
#endif
//#define bookmarkPaneWith 150.0f
//#define IMGUI_TOGGLE_BUTTON ToggleButton
//#define bookmarksButtonString "Bookmark"
//#define bookmarksButtonHelpString "Bookmark"
//#define addBookmarkButtonString "+"
//#define removeBookmarkButtonString "-"

#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui_mat.h>
#include <application.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#if IMGUI_VULKAN_SHADER
#include "imgui_impl_vulkan.h"
#endif
#if IMGUI_ADDON_IMPLOTS
#include "implot.h"
#endif
#if IMGUI_ADDON_MARKDOWN
#include "imgui_markdown.h"
#endif
#if IMGUI_ADDON_MEMORY_EDITOR
#include "imgui_memory_editor.h"
#endif
#if IMGUI_ADDON_IMNODES
#include "imnodes.h"
#endif
#if IMGUI_ADDON_NODE_GRAPH
#include "ImGuiNodeGraphEditor.h"
#endif
#if IMGUI_ADDON_TEXT_EDITOR
#include "TextEditor.h"
#endif
#if IMGUI_ADDON_FILE_DIALOG
#include "ImGuiFileDialog.h"
#endif
#if IMGUI_ADDON_FILE_SYSTEM
#include "ImGuiFileSystem.h"
#endif
#if IMGUI_ADDON_TINYFILE
#include "tinyfiledialogs.h"
#endif
#if IMGUI_ADDON_HOTKEY
#include "HotKey.h"
#endif
#if IMGUI_ADDON_ZMO
#include "ImGuizmo.h"
#endif
#if IMGUI_ADDON_ZMOQUAT
#include "imGuIZMOquat.h"
#endif
#if IMGUI_ADDON_DEAR_WIDGETS
#include "dear_widgets.h"
#endif
#if IMGUI_ADDON_DATE_CHOOSER || IMGUI_ADDON_KNOB || IMGUI_ADDON_VARIOUS || IMGUI_ADDON_DOCK || IMGUI_ADDON_TABWINDOW || IMGUI_ADDON_PROGRESSES || IMGUI_ADDON_TIMELINE || IMGUI_VULKAN_SHADER
#include "addon/addons_demo.h"
#endif
#include "Config.h"

#if IMGUI_ADDON_HOTKEY
// Init HotKey
static std::vector<ImHotKey::HotKey> hotkeys = 
{ 
    {"Layout", "Reorder nodes in a simpler layout", 0xFFFF26E0},
    {"Save", "Save the current graph", 0xFFFF1FE0},
    {"Load", "Load an existing graph file", 0xFFFF18E0},
    {"Play/Stop", "Play or stop the animation from the current graph", 0xFFFFFF3F},
    {"SetKey", "Make a new animation key with the current parameters values at the current time", 0xFFFFFF1F}
};
#endif

static inline void box(ImGui::ImMat& image, int x1, int y1, int x2, int y2, int R, int G, int B)
{
    for (int j = y1; j <= y2; j++)
    {
        for (int i = x1; i <= x2; i++)
        {
            unsigned int color = 0xFF000000 | (R << 16) | (G << 8) | B;
            image.at<unsigned int>(i, j) = color;
        }
    }
}

static inline void color_bar(ImGui::ImMat& image, int x1, int y1, int x2, int y2)
{
    const unsigned char r[8] = {255,255,0,0,255,255,0,0};
    const unsigned char g[8] = {255,255,255,255,0,0,0,0};
    const unsigned char b[8] = {255,0,255,0,255,0,255,0};
    int len = x2 - x1 + 1;
    for (int i = 0; i < 8; i++)
    {
        box(image, x1 + len * i / 8, y1, x1 + len * (i + 1) / 8 - 1, y2, r[i], g[i], b[i]);
    }
}

static inline void gray_bar(ImGui::ImMat& image, int x1,int y1,int x2,int y2,int step)
{
    int len = x2 - x1 + 1;
    for (int i = 0; i < step; i++)
    {
        box(image, x1 + len * i / step, y1, x1 + len * (i + 1) / step - 1, y2, 255 * i / step, 255 * i / step, 255 * i / step);
    }
}
class Example
{
public:
    Example() 
    {
#if IMGUI_ADDON_FILE_DIALOG
        // load file dialog resource
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
        prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());
#endif

#if IMGUI_ADDON_MEMORY_EDITOR
        // init memory edit
        mem_edit.Open = false;
        mem_edit.OptShowDataPreview = true;
        mem_edit.OptAddrDigitsCount = 8;
        data = malloc(0x400);
#endif

#if IMGUI_ADDON_IMNODES
        // Init imnodes
        std::string node_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.ini";
        std::string node_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.node";
        ImNodes::CreateContext();
        imnodes_example::NodeEditorInitialize(node_ini_path.c_str(), node_path.c_str());
#endif

#if IMGUI_ADDON_NODE_GRAPH
        // Init NodeGraphEditor
        std::string nge_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.nge.ini";
        std::string nge_style_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.style.ini";
        nge.save_node_path = nge_ini_path;
        nge.save_style_path = nge_style_path;
#endif
#if IMGUI_VULKAN_SHADER
        ImGui::PrepareVulkanDemo();
#endif
        color_bar(image, 0, 0, 255, 191);
        gray_bar(image, 0, 192, 255, 255, 13);
        ImageTexture = ImGui::ImCreateTexture(image.data, image.w, image.h);
    };
    ~Example() 
    {
#if IMGUI_ADDON_MEMORY_EDITOR
        if (data)
            free(data); 
#endif

#if IMGUI_ADDON_FILE_DIALOG
        // Store file dialog bookmark
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
        end_file_dialog_demo_window(&filedialog, bookmark_path.c_str());
#endif

#if IMGUI_ADDON_IMNODES
        // Clean Node Window
        std::string node_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.ini";
        std::string node_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.node";
        imnodes_example::NodeEditorShutdown(node_ini_path.c_str(), node_path.c_str());
        ImNodes::DestroyContext();
#endif

#if IMGUI_ADDON_DATE_CHOOSER || IMGUI_ADDON_KNOB || IMGUI_ADDON_VARIOUS || IMGUI_ADDON_DOCK || IMGUI_ADDON_TABWINDOW || IMGUI_ADDON_PROGRESSES || IMGUI_ADDON_TIMELINE
        ImGui::CleanupDemo();
#endif

#if IMGUI_ADDON_ZMO
        ImGuizmo::CleanupZMODemo();
#endif

#if IMGUI_VULKAN_SHADER
        ImGui::CleanVulkanDemo();
#endif
        if (ImageTexture) { ImGui::ImDestroyTexture(ImageTexture); ImageTexture = 0; }
    }

public:
    bool show_demo_window = true;
    bool show_another_window = false;

#if IMGUI_ADDON_FILE_DIALOG
    // init file dialog
    ImGuiFileDialog filedialog;
#endif

#if IMGUI_ADDON_FILE_SYSTEM
    // init sample file dialog
    ImGuiFs::Dialog dlg;
#endif

#if IMGUI_ADDON_MEMORY_EDITOR
    // init memory edit
    MemoryEditor mem_edit;
    void* data = nullptr;
#endif

#if IMGUI_ADDON_TEXT_EDITOR
    // Init Text Edit
    TextEditor editor;
#endif

#if IMGUI_ADDON_MARKDOWN
    // Init MarkDown
    ImGui::MarkdownConfig mdConfig;
#endif

#if IMGUI_ADDON_NODE_GRAPH
    // Init NodeGraphEditor
    ImGui::NodeGraphEditor nge;
#endif

public:
    bool show_implot_window = false;
    bool show_file_dialog_window = false;
    bool show_sample_file_dialog = false;
    bool show_tiny_file_dialog = false;
    bool show_text_edit_window = false;
    bool show_markdown_window = false;
    bool show_dock_window = false;
    bool show_tab_window = false;
    bool show_node_window = false;
    bool show_node_edit_window = false;
    bool show_addon_widget = false;
    bool show_zmo_window = false;
    bool show_quat_window = false;
    bool show_dear_widget_window = false;
public:
#if IMGUI_ADDON_MARKDOWN
    std::string get_file_contents(const char *filename);
    static ImGui::MarkdownImageData ImageCallback( ImGui::MarkdownLinkCallbackData data_ );
    static void LinkCallback( ImGui::MarkdownLinkCallbackData data_ );
    static void ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ );
#endif
#if IMGUI_VULKAN_SHADER
public:
    bool show_shader_window = false;
#endif
public:
    ImGui::ImMat image {ImGui::ImMat(256, 256, 1u, 4)};
    ImTextureID ImageTexture = 0;
};

#if IMGUI_ADDON_MARKDOWN
std::string Example::get_file_contents(const char *filename)
{
    std::string file_path = std::string(DEFAULT_DOCUMENT_PATH) + std::string(filename);
    std::ifstream infile(file_path, std::ios::in | std::ios::binary);
    if (infile.is_open())
    {
        std::ostringstream contents;
        contents << infile.rdbuf();
        infile.close();
        return(contents.str());
    }
    throw(errno);
}

ImGui::MarkdownImageData Example::ImageCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    char image_url[MAX_PATH_BUFFER_SIZE] = {0};
    strncpy(image_url, data_.link, data_.linkLength);
    // In your application you would load an image based on data_ input. Here we just use the imgui font texture.
    ImTextureID image = ImGui::GetIO().Fonts->TexID;
    // > C++14 can use ImGui::MarkdownImageData imageData{ true, false, image, ImVec2( 40.0f, 20.0f ) };

    ImGui::MarkdownImageData imageData;
    imageData.isValid =         true;
    imageData.useLinkCallback = false;
    imageData.user_texture_id = image;
    imageData.size =            ImVec2( 40.0f, 20.0f );
    return imageData;
}

void Example::LinkCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    std::string url( data_.link, data_.linkLength );
    std::string command = "open " + url;
    if( !data_.isImage )
    {
        system(command.c_str());
    }
}

void Example::ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ )
{
    // Call the default first so any settings can be overwritten by our implementation.
    // Alternatively could be called or not called in a switch statement on a case by case basis.
    // See defaultMarkdownFormatCallback definition for furhter examples of how to use it.
    ImGui::defaultMarkdownFormatCallback( markdownFormatInfo_, start_ );        
    switch( markdownFormatInfo_.type )
    {
        // example: change the colour of heading level 2
        case ImGui::MarkdownFormatType::HEADING:
        {
            if( markdownFormatInfo_.level == 2 )
            {
                if( start_ )
                {
                    ImGui::PushStyleColor( ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_TextDisabled] );
                }
                else
                {
                    ImGui::PopStyleColor();
                }
            }
            break;
        }
        default:
        {
            break;
        }
    }
}
#endif

static std::string ini_file = std::string(DEFAULT_CONFIG_PATH) + "Application_Example.ini";

const char* Application_GetName(void* handle)
{
    return "Application_Example";
}

void Application_Initialize(void** handle)
{
    srand((unsigned int)time(0));
    *handle = new Example();
    Example * example = (Example *)*handle;
#if IMGUI_ADDON_IMPLOTS
    ImPlot::CreateContext();
#endif
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = ini_file.c_str();
}

void Application_Finalize(void** handle)
{
    if (handle && *handle)
    {
        Example * example = (Example *)*handle;
        delete example;
        *handle = nullptr;
    }
#if IMGUI_ADDON_IMPLOTS
    ImPlot::DestroyContext();
#endif
}

bool Application_Frame(void* handle)
{
    bool done = false;
    auto& io = ImGui::GetIO();
    Example * example = (Example *)handle;
    if (!example)
        return true;
    // Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (example->show_demo_window)
        ImGui::ShowDemoWindow(&example->show_demo_window);

    // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &example->show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &example->show_another_window);
#if IMGUI_ADDON_IMPLOTS
        ImGui::Checkbox("ImPlot Window", &example->show_implot_window);
#endif
#if IMGUI_ADDON_FILE_DIALOG
        ImGui::Checkbox("File Dialog Window", &example->show_file_dialog_window);
#endif
#if IMGUI_ADDON_FILE_SYSTEM
        ImGui::Checkbox("Sample File Dialog", &example->show_sample_file_dialog);
#endif
#if IMGUI_ADDON_TINYFILE
        ImGui::Checkbox("Tiny File Dialog", &example->show_tiny_file_dialog);
#endif
#if IMGUI_ADDON_MEMORY_EDITOR
        ImGui::Checkbox("Memory Edit Window", &example->mem_edit.Open);
#endif
#if IMGUI_ADDON_TEXT_EDITOR
        ImGui::Checkbox("Show Text Edit Window", &example->show_text_edit_window);
#endif
#if IMGUI_ADDON_MARKDOWN
        ImGui::Checkbox("Show Markdown Window", &example->show_markdown_window);
#endif
#if IMGUI_ADDON_DOCK
        ImGui::Checkbox("Show Dock Window", &example->show_dock_window);
#endif
#if IMGUI_ADDON_TABWINDOW
        ImGui::Checkbox("Show Tab Window", &example->show_tab_window);
#endif
#if IMGUI_ADDON_IMNODES
        ImGui::Checkbox("Show Node Sample Window", &example->show_node_window);
#endif
#if IMGUI_ADDON_NODE_GRAPH
        ImGui::Checkbox("Show Node Edit Windows", &example->show_node_edit_window);
#endif
#if IMGUI_ADDON_DATE_CHOOSER || IMGUI_ADDON_KNOB || IMGUI_ADDON_VARIOUS || IMGUI_ADDON_DOCK || IMGUI_ADDON_TABWINDOW || IMGUI_ADDON_PROGRESSES || IMGUI_ADDON_TIMELINE
        ImGui::Checkbox("Show Addon Widgets", &example->show_addon_widget);
#endif
#if IMGUI_ADDON_ZMO
        ImGui::Checkbox("Show ZMO Window", &example->show_zmo_window);
#endif
#if IMGUI_ADDON_ZMOQUAT
        ImGui::Checkbox("Show ZMOQuat Window", &example->show_quat_window);
#endif
#if IMGUI_ADDON_DEAR_WIDGETS
        ImGui::Checkbox("Show DearWidgets Window", &example->show_dear_widget_window);
#endif
#if IMGUI_VULKAN_SHADER
        ImGui::Checkbox("Show Shader Window", &example->show_shader_window);
#endif
#if IMGUI_ADDON_HOTKEY
        // show hotkey window
        if (ImGui::Button("Edit Hotkeys"))
        {
            ImGui::OpenPopup("HotKeys Editor");
        }

        // Handle hotkey popup
        ImHotKey::Edit(hotkeys.data(), hotkeys.size(), "HotKeys Editor");
        int hotkey = ImHotKey::GetHotKey(hotkeys.data(), hotkeys.size());
        if (hotkey != -1)
        {
            // handle the hotkey index!
        }
#endif
        ImVec2 displayedTextureSize(256,256);
        ImGui::Image((ImTextureID)(uint64_t)example->ImageTexture, displayedTextureSize);
        {
            ImRect rc = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            ImVec2 mouseUVCoord = (io.MousePos - rc.Min) / rc.GetSize();
            if (ImGui::IsItemHovered() && mouseUVCoord.x >= 0.f && mouseUVCoord.y >= 0.f)
            {
                ImGuiHelper::ImageInspect(example->image.w, example->image.h, 
                                        (const unsigned char*)example->image.data, mouseUVCoord, 
                                        displayedTextureSize);
            }
        }
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", ImGui::GetIO().DeltaTime * 1000.f, ImGui::GetIO().Framerate);
        ImGui::Text("Frames since last input: %d", ImGui::GetIO().FrameCountSinceLastInput);
        ImGui::End();
    }

    // Show another simple window.
    if (example->show_another_window)
    {
        ImGui::Begin("Another Window", &example->show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            example->show_another_window = false;
        ImGui::End();
    }

#if IMGUI_ADDON_IMPLOTS
    // Show ImPlot simple window
    if (example->show_implot_window)
    {
        ImPlot::ShowDemoWindow(&example->show_implot_window);
    }
#endif

#if IMGUI_ADDON_FILE_DIALOG
    // Show FileDialog demo window
    if (example->show_file_dialog_window)
    {
        show_file_dialog_demo_window(&example->filedialog, &example->show_file_dialog_window);
    }
#endif

#if IMGUI_ADDON_FILE_SYSTEM
    // Show Sample FileDialog
    {
        //example->dlg.WrapMode = false;
        const char* filePath = example->dlg.chooseFileDialog(example->show_sample_file_dialog, example->dlg.getLastDirectory(), ".jpg;.jpeg;.png;.gif;.tga;.bmp", "Sample file dialog", ImVec2(400, 800), ImVec2(50, 50));
        if (strlen(filePath) > 0) 
        {
	        //fprintf(stderr,"Browsed..: %s\n",filePath);
        }
        example->show_sample_file_dialog = false;
    }
#endif

#if IMGUI_ADDON_TINYFILE
    // Show Tiny FileDialog
    if (example->show_tiny_file_dialog)
    {
        const char* filterPatterns[1] = { "*.cpp" };
        auto path = tinyfd_openFileDialog(
            "Open Source Code...",
            nullptr,
            1, filterPatterns,
            "Source Code Files (*.cpp)", 0);
        example->show_tiny_file_dialog = false;
    }
#endif

#if IMGUI_ADDON_MEMORY_EDITOR
    // Show Memory Edit window
    if (example->mem_edit.Open)
    {
        static int i = 0;
        int * test_point = (int *)example->data;
        *test_point = i; i++;
        example->mem_edit.DrawWindow("Memory Editor", example->data, 0x400, 0, &example->mem_edit.Open, 768);
    }
#endif

#if IMGUI_ADDON_TEXT_EDITOR
    // Show Text Edit Window
    if (example->show_text_edit_window)
    {
        example->editor.text_edit_demo(&example->show_text_edit_window);
    }
#endif

#if IMGUI_ADDON_MARKDOWN
    // Show Markdown Window
    if (example->show_markdown_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
        ImGui::Begin("iMarkdown window",&example->show_markdown_window, ImGuiWindowFlags_NoScrollbar);
        std::string help_doc =                   example->get_file_contents("imgui.md");
        example->mdConfig.linkCallback =         example->LinkCallback;
        example->mdConfig.tooltipCallback =      NULL;
        example->mdConfig.imageCallback =        example->ImageCallback;
        example->mdConfig.linkIcon =             ICON_FA5_LINK;
        example->mdConfig.headingFormats[0] =    { io.Fonts->Fonts[0], true };
        example->mdConfig.headingFormats[1] =    { io.Fonts->Fonts.size() > 1 ? io.Fonts->Fonts[1] : nullptr, true };
        example->mdConfig.headingFormats[2] =    { io.Fonts->Fonts.size() > 2 ? io.Fonts->Fonts[2] : nullptr, false };
        example->mdConfig.userData =             NULL;
        example->mdConfig.formatCallback =       example->ExampleMarkdownFormatCallback;
        ImGui::Markdown( help_doc.c_str(), help_doc.length(), example->mdConfig );
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_DOCK
    // Show Dock Window
    if (example->show_dock_window)
    {
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("imguidock window (= lumix engine's dock system)",&example->show_dock_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowAddonsDuckWindow();
        }
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_TABWINDOW
    // Show Tab Window
    if (example->show_tab_window)
    {
        ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Example: TabWindow", &example->show_tab_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowAddonsTabWindow();   // see its code for further info         
        }
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_IMNODES
    // Show Node Sample Window
    if (example->show_node_window)
    {
        imnodes_example::NodeEditorShow(&example->show_node_window);
    }
#endif

#if IMGUI_ADDON_NODE_GRAPH
    // Show Node Edit Window
    if (example->show_node_edit_window)
    {
        ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Example: Custom Node Graph",&example->show_node_edit_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::TestNodeGraphEditor(&example->nge);   // see its code for further info         
        }
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_DATE_CHOOSER || IMGUI_ADDON_KNOB || IMGUI_ADDON_VARIOUS || IMGUI_ADDON_DOCK || IMGUI_ADDON_TABWINDOW || IMGUI_ADDON_PROGRESSES || IMGUI_ADDON_TIMELINE
    // Show Addon Widget.
    if (example->show_addon_widget)
    {
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Addon Widget", &example->show_addon_widget);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::ShowAddonsDemoWindowWidgets();
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_ZMO
    // Show Zmo Window
    if (example->show_zmo_window)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
        ImGui::Begin("##ZMO", &example->show_zmo_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
        ImGuizmo::ShowAddonsZMOWindow();
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_ZMOQUAT
    // Show Zmo Quat Window
    if (example->show_quat_window)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1280, 900), ImGuiCond_FirstUseEver);
        ImGui::Begin("ZMOQuat", &example->show_quat_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
        ImGui::ShowGizmoDemo();
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_DEAR_WIDGETS
    // Show Dear Widgets Window
    if (example->show_dear_widget_window)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
        ImGui::Begin("##DearWidgets", &example->show_dear_widget_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
        ImWidgets::ShowDemo();
        ImGui::End();
    }
#endif

#if IMGUI_VULKAN_SHADER
    // Show Shader Window
    if (example->show_shader_window)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 850), ImGuiCond_FirstUseEver);
        ImGui::Begin("##Shader", &example->show_shader_window, ImGuiWindowFlags_NoSavedSettings);
        ImGui::ShowAddonsVulkanShaderWindow();
        ImGui::End();
    }
#endif
    return done;
}

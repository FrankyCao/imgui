# include <imgui.h>
# define IMGUI_DEFINE_MATH_OPERATORS
# include <imgui_internal.h>
# include <application.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#ifdef IMGUI_VULKAN_SHADER
#include "imgui_impl_vulkan.h"
#endif
#include "Config.h"

#ifdef IMGUI_ADDONS
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

class Example
{
public:
    Example() 
    {
#ifdef IMGUI_ADDONS
        std::string node_save_path;
        // load file dialog resource
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
        prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

        // init memory edit
        mem_edit.Open = false;
        mem_edit.OptShowDataPreview = true;
        data = malloc(0x1000);

        // Init imnodes
        std::string node_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.ini";
        std::string node_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.node";
        imnodes::Initialize();
        imnodes_sample::NodeEditorInitialize(node_ini_path.c_str(), node_path.c_str());

        // Init NodeGraphEditor
        std::string nge_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.nge.ini";
        std::string nge_style_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.style.ini";
        nge.save_node_path = nge_ini_path;
        nge.save_style_path = nge_style_path;
#ifdef IMGUI_VULKAN_SHADER
        ImGui::PrepareVulkanDemo();
#endif
#endif
    };
    ~Example() 
    {
#ifdef IMGUI_ADDONS
        if (data)
            free(data); 

        // Store file dialog bookmark
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
        end_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

        // Clean Node Window
        std::string node_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.ini";
        std::string node_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.node";
        imnodes_sample::NodeEditorShutdown(node_ini_path.c_str(), node_path.c_str());
        imnodes::Shutdown();
        ImGui::CleanupDemo();
        ImGui::CleanupZMODemo();
#ifdef IMGUI_VULKAN_SHADER
        ImGui::CleanVulkanDemo();
#endif
#endif
    }

public:
    bool show_demo_window = true;
    bool show_another_window = false;

#ifdef IMGUI_ADDONS
    // init file dialog
    ImGuiFileDialog filedialog;

    // init sample file dialog
    ImGuiFs::Dialog dlg;

    // init memory edit
    MemoryEditor mem_edit;
    void* data = nullptr;

    // Init Text Edit
    TextEditor editor;

    // Init MarkDown
    ImGui::MarkdownConfig mdConfig;

    // Init NodeGraphEditor
    ImGui::NodeGraphEditor nge;

public:
    bool show_implot_window = false;
    bool show_file_dialog_window = false;
    bool show_sample_file_dialog = false;
    bool show_text_edit_window = false;
    bool show_markdown_window = false;
    bool show_dock_window = false;
    bool show_tab_window = false;
    bool show_node_window = false;
    bool show_node_edit_window = false;
    bool show_addon_widget = false;
    bool show_zmo_window = false;

public:
    std::string get_file_contents(const char *filename);
    static ImGui::MarkdownImageData ImageCallback( ImGui::MarkdownLinkCallbackData data_ );
    static void LinkCallback( ImGui::MarkdownLinkCallbackData data_ );
    static void ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ );
#ifdef IMGUI_VULKAN_SHADER
public:
    bool show_shader_window = false;
#endif
#endif
};

#ifdef IMGUI_ADDONS
std::string Example::get_file_contents(const char *filename)
{
    std::ifstream infile(filename, std::ios::in | std::ios::binary);
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
    *handle = new Example();
    Example * example = (Example *)*handle;
#ifdef IMGUI_ADDONS
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
#ifdef IMGUI_ADDONS
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
    // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (example->show_demo_window)
        ImGui::ShowDemoWindow(&example->show_demo_window);

    // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &example->show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &example->show_another_window);
#ifdef IMGUI_ADDONS
        ImGui::Checkbox("ImPlot Window", &example->show_implot_window);
        ImGui::Checkbox("File Dialog Window", &example->show_file_dialog_window);
        ImGui::Checkbox("Sample File Dialog", &example->show_sample_file_dialog);
        ImGui::Checkbox("Memory Edit Window", &example->mem_edit.Open);
        ImGui::Checkbox("Show Text Edit Window", &example->show_text_edit_window);
        ImGui::Checkbox("Show Markdown Window", &example->show_markdown_window);
        ImGui::Checkbox("Show Dock Window", &example->show_dock_window);
        ImGui::Checkbox("Show Tab Window", &example->show_tab_window);
        ImGui::Checkbox("Show Node Sample Window", &example->show_node_window);
        ImGui::Checkbox("Show Node Edit Windows", &example->show_node_edit_window);
        ImGui::Checkbox("Show Addon Widgets", &example->show_addon_widget);
        ImGui::Checkbox("Show ZMO Window", &example->show_zmo_window);
#ifdef IMGUI_VULKAN_SHADER
        ImGui::Checkbox("Show Shader Window", &example->show_shader_window);
#endif

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
        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // 3. Show another simple window.
    if (example->show_another_window)
    {
        ImGui::Begin("Another Window", &example->show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            example->show_another_window = false;
        ImGui::End();
    }

#ifdef IMGUI_ADDONS
    // 4. Show ImPlot simple window
    if (example->show_implot_window)
    {
        ImPlot::ShowDemoWindow(&example->show_implot_window);
    }

    // 5. Show FileDialog demo window
    if (example->show_file_dialog_window)
    {
        show_file_dialog_demo_window(&example->filedialog, &example->show_file_dialog_window);
    }

    // 6. Show Sample FileDialog
    {
        //example->dlg.WrapMode = false;
        const char* filePath = example->dlg.chooseFileDialog(example->show_sample_file_dialog, example->dlg.getLastDirectory(), ".jpg;.jpeg;.png;.gif;.tga;.bmp", "Sample file dialog", ImVec2(400, 800), ImVec2(50, 50));
        if (strlen(filePath) > 0) 
        {
	        //fprintf(stderr,"Browsed..: %s\n",filePath);
        }
        example->show_sample_file_dialog = false;
    }

    // 7. Show Memory Edit window
    if (example->mem_edit.Open)
    {
        ImGui::SetNextWindowSize(ImVec2(400, 768), ImGuiCond_FirstUseEver);
        ImGui::Begin("Memory Window", &example->mem_edit.Open);
        example->mem_edit.DrawWindow("Memory Editor", example->data, 0x1000);
        ImGui::End();
    }

    // 8. Show Text Edit Window
    if (example->show_text_edit_window)
    {
        example->editor.text_edit_demo(&example->show_text_edit_window);
    }

    // 9. Show Markdown Window
    if (example->show_markdown_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
        ImGui::Begin("iMarkdown window",&example->show_markdown_window, ImGuiWindowFlags_NoScrollbar);
        std::string help_doc =                   example->get_file_contents("docs/imgui.md");
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

    // 10. Show Dock Window
    if (example->show_dock_window)
    {
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("imguidock window (= lumix engine's dock system)",&example->show_dock_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowAddonsDuckWindow();
        }
        ImGui::End();
    }

    // 11. Show Tab Window
    if (example->show_tab_window)
    {
        ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Example: TabWindow", &example->show_tab_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowAddonsTabWindow();   // see its code for further info         
        }
        ImGui::End();
    }

    // 12. Show Node Sample Window
    if (example->show_node_window)
    {
        imnodes_sample::NodeEditorShow(&example->show_node_window);
    }

    // 13. Show Node Edit Window
    if (example->show_node_edit_window)
    {
        ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Example: Custom Node Graph",&example->show_node_edit_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::TestNodeGraphEditor(&example->nge);   // see its code for further info         
        }
        ImGui::End();
    }

    // 14. Show Addon Widget.
    if (example->show_addon_widget)
    {
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Addon Widget", &example->show_addon_widget);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::ShowAddonsDemoWindowWidgets();
        ImGui::End();
    }

    // 15. Show Zmo Window
    if (example->show_zmo_window)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
        ImGui::Begin("##ZMO", &example->show_zmo_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
        ImGui::ShowAddonsZMOWindow();
        ImGui::End();
    }

#ifdef IMGUI_VULKAN_SHADER
    // 16. Show Shader Window
    if (example->show_shader_window)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(400, 850), ImGuiCond_FirstUseEver);
        ImGui::Begin("##Shader", &example->show_shader_window, ImGuiWindowFlags_NoSavedSettings);
        ImGui::ShowAddonsVulkanShaderWindow();
        ImGui::End();
    }
#endif
#endif
    return done;
}

# include <imgui.h>
# define IMGUI_DEFINE_MATH_OPERATORS
# include <imgui_internal.h>
# include <application.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>

// Init HotKey
static std::vector<ImHotKey::HotKey> hotkeys = 
{ 
    {"Layout", "Reorder nodes in a simpler layout", 0xFFFF26E0},
    {"Save", "Save the current graph", 0xFFFF1FE0},
    {"Load", "Load an existing graph file", 0xFFFF18E0},
    {"Play/Stop", "Play or stop the animation from the current graph", 0xFFFFFF3F},
    {"SetKey", "Make a new animation key with the current parameters values at the current time", 0xFFFFFF1F}
};

class Example
{
public:
    Example() 
    {
        // load file dialog resource
        igfd::prepare_file_dialog_demo_window(&filedialog);

        // init memory edit
        mem_edit.Open = false;
        mem_edit.OptShowDataPreview = true;
        data = malloc(0x1000);

        // Init imnodes
        imnodes::Initialize();
        imnodes_sample::NodeEditorInitialize();
    };
    ~Example() 
    { 
        if (data)
            free(data); 

        // Store file dialog bookmark
        igfd::end_file_dialog_demo_window(&filedialog);

        // Clean Node Window
        imnodes_sample::NodeEditorShutdown();
        imnodes::Shutdown();
    }

public:
    // init file dialog
    igfd::ImGuiFileDialog filedialog;

    // init memory edit
    MemoryEditor mem_edit;
    void* data = nullptr;
    // Init Text Edit
    TextEditor editor;
    // Init MarkDown
    ImGui::MarkdownConfig mdConfig;

public:
    bool show_demo_window = true;
    bool show_another_window = false;
    bool show_implot_window = false;
    bool show_file_dialog_window = false;
    bool show_text_edit_window = false;
    bool show_markdown_window = false;
    bool show_dock_window = false;
    bool show_node_window = false;

public:
    std::string get_file_contents(const char *filename);
    static ImGui::MarkdownImageData ImageCallback( ImGui::MarkdownLinkCallbackData data_ );
    static void LinkCallback( ImGui::MarkdownLinkCallbackData data_ );
    static void ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ );
};

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

const char* Application_GetName(void* handle)
{
    return "Application_Example";
}

void Application_Initialize(void** handle)
{
    *handle = new Example();
    Example * example = (Example *)*handle;
}

void Application_Finalize(void** handle)
{
    if (handle && *handle)
    {
        Example * example = (Example *)*handle;
        delete example;
        *handle = nullptr;
    }
}

void Application_Frame(void* handle)
{
    auto& io = ImGui::GetIO();
    Example * example = (Example *)handle;
    if (!example)
        return;
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
        ImGui::Checkbox("ImPlot Window", &example->show_implot_window);
        ImGui::Checkbox("File Dialog Window", &example->show_file_dialog_window);
        ImGui::Checkbox("Memory Edit Window", &example->mem_edit.Open);
        ImGui::Checkbox("Show Text Edit Window", &example->show_text_edit_window);
        ImGui::Checkbox("Show Markdown Window", &example->show_markdown_window);
        ImGui::Checkbox("Show Dock Window", &example->show_dock_window);
        ImGui::Checkbox("Show Node Window", &example->show_node_window);

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

    // 4. Show ImPlot simple window
    if (example->show_implot_window)
    {
        ImPlot::ShowDemoWindow(&example->show_implot_window);
    }

    // 5. Show FileDialog demo window
    if (example->show_file_dialog_window)
    {
        igfd::show_file_dialog_demo_window(&example->filedialog, &example->show_file_dialog_window);
    }

    // 6. Show Memory Edit window
    if (example->mem_edit.Open)
    {
        ImGui::Begin("Memory Window", &example->mem_edit.Open);
        example->mem_edit.DrawWindow("Memory Editor", example->data, 0x1000);
        ImGui::End();
    }

    // 7. Show Text Edit Window
    if (example->show_text_edit_window)
    {
        example->editor.text_edit_demo(&example->show_text_edit_window);
    }

    // 8. Show Markdown Window
    if (example->show_markdown_window)
    {
        std::string help_doc =                   example->get_file_contents("docs/README.md");
        example->mdConfig.linkCallback =         example->LinkCallback;
        example->mdConfig.tooltipCallback =      NULL;
        example->mdConfig.imageCallback =        example->ImageCallback;
        example->mdConfig.linkIcon =             ICON_FA5_LINK;
        example->mdConfig.headingFormats[0] =    { io.Fonts->Fonts[0], true };
        example->mdConfig.headingFormats[1] =    { io.Fonts->Fonts[1], true };
        example->mdConfig.headingFormats[2] =    { io.Fonts->Fonts[2], false };
        example->mdConfig.userData =             NULL;
        example->mdConfig.formatCallback =       example->ExampleMarkdownFormatCallback;
        ImGui::Markdown( help_doc.c_str(), help_doc.length(), example->mdConfig );
    }

    // 9. Show Dock Window
    if (example->show_dock_window)
    {
        if(ImGui::Begin("Dock Demo"))
        {
	        // dock layout by hard-coded or .ini file
            ImGui::BeginDockspace();
            if(ImGui::BeginDock("Dock 1"))
            {
                ImGui::Text("I'm Wubugui!");
            }
            ImGui::EndDock();
            if(ImGui::BeginDock("Dock 2"))
            {
                ImGui::Text("I'm BentleyBlanks!");
            }
            ImGui::EndDock();
            if(ImGui::BeginDock("Dock 3"))
            {
                ImGui::Text("I'm LonelyWaiting!");
            }
            ImGui::EndDock();
            ImGui::EndDockspace();
        }
        ImGui::End();
        // multiple dockspace supported
        if(ImGui::Begin("Dock Demo2"))
        {
            ImGui::BeginDockspace();
            if(ImGui::BeginDock("Dock 2"))
            {
                ImGui::Text("Who's your daddy?");
            }
            ImGui::EndDock();
            ImGui::EndDockspace();
        }
        ImGui::End();
    }

    // 10. Show Node Window
    if (example->show_node_window)
    {
        imnodes_sample::NodeEditorShow();
    }
}

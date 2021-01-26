#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_impl_sdl.h"
#include "imgui_impl_sdl_es2.h"
#include <stdio.h>
#include <SDL.h>
#include <GLES2/gl2.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include "Config.h"

static std::string get_file_contents(const char *filename)
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

inline ImGui::MarkdownImageData ImageCallback( ImGui::MarkdownLinkCallbackData data_ )
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

static void LinkCallback( ImGui::MarkdownLinkCallbackData data_ )
{
    std::string url( data_.link, data_.linkLength );
    std::string command = "open " + url;
    if( !data_.isImage )
    {
        system(command.c_str());
    }
}

static void ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ )
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

// Main code
int main(int, char**)
{
    // Setup SDL
    // (Some versions of SDL before <2.0.10 appears to have performance/stalling issues on a minority of Windows systems,
    // depending on whether SDL_INIT_GAMECONTROLLER is enabled or disabled.. updating to latest version of SDL is recommended!)
    if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_TIMER) != 0)
    {
        printf("Error: %s\n", SDL_GetError());
        return -1;
    }
    int window_width = 1440;
    int window_height = 960;
    float window_scale = 1.0;
#ifdef __linux__
    // Query default monitor resolution
    SDL_Rect display_bounds;
    if (SDL_GetDisplayBounds(0, &display_bounds) != 0)
    {
        fprintf(stderr, "Failed to obtain bounds of display 0: %s\n", SDL_GetError());
        return -1;
    }
    if (display_bounds.w > 1920)
    {
        window_width *= 2;
        window_height *= 2;
        window_scale *= 2;
    }
#endif
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);

    // Create window with graphics context
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_WindowFlags window_flags = (SDL_WindowFlags)(SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
    SDL_Window* window = SDL_CreateWindow("Dear ImGui SDL2+OpenGLES2 example", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, window_width, window_height, window_flags);
    if (!window)
    {
        fprintf(stderr, "Could not create window! SDL reports error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_GLContext gl_context = SDL_GL_CreateContext(window);
    if (!gl_context)
    {
        fprintf(stderr, "Could not create context! SDL reports error: %s\n", SDL_GetError());
        return -1;
    }
    SDL_GL_MakeCurrent(window, gl_context);
    SDL_GL_SetSwapInterval(1); // Enable vsync

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    std::string ini_file = std::string(DEFAULT_CONFIG_PATH) + "sdl_opengles2.ini";
    io.IniFilename = ini_file.c_str();
    io.FontGlobalScale = window_scale;

    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    io.Fonts->AddFontDefault();
    /*
    ImFontConfig font_cfg = ImFontConfig();
    font_cfg.OversampleH = 2;
    font_cfg.OversampleV = 2;
    font_cfg.PixelSnapH = true;
    font_cfg.EllipsisChar = (ImWchar)0x0085;
    font_cfg.GlyphOffset.y = 1.0f * IM_FLOOR(font_cfg.SizePixels / 16.0f);
    static ImFontGlyphRangesBuilder range;
    range.Clear();
    static ImVector<ImWchar> gr;
    gr.clear();
    //range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseSimplifiedCommon());
    range.AddRanges(ImGui::GetIO().Fonts->GetGlyphRangesChineseFull());
    range.BuildRanges(&gr);
    io.Fonts->AddFontFromFileTTF("/Users/dicky/Desktop/dks.ttf", 16.0f, &font_cfg, gr.Data);
    */
    
    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplSdlGLES2_Init(window);

    // load file dialog resource
    ImGuiFileDialog filedialog;
    std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
    prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

    // init memory edit
    MemoryEditor mem_edit;
    mem_edit.Open = false;
    mem_edit.OptShowDataPreview = true;
    size_t data_size = 0x1000;
    void* data = malloc(data_size);

    // Init Text Edit
	TextEditor editor;

    // Init MarkDown
    ImGui::MarkdownConfig mdConfig; 

    // Init imnodes
    std::string node_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.ini";
    std::string node_path = std::string(DEFAULT_CONFIG_PATH) + "nodes_save_load.node";
    imnodes::Initialize();
    imnodes_sample::NodeEditorInitialize(node_ini_path.c_str(), node_path.c_str());

    // Init HotKey
    static std::vector<ImHotKey::HotKey> hotkeys = 
    { 
        {"Layout", "Reorder nodes in a simpler layout", 0xFFFF26E0},
        {"Save", "Save the current graph", 0xFFFF1FE0},
        {"Load", "Load an existing graph file", 0xFFFF18E0},
        {"Play/Stop", "Play or stop the animation from the current graph", 0xFFFFFF3F},
        {"SetKey", "Make a new animation key with the current parameters values at the current time", 0xFFFFFF1F}
    };

    // Our state
    bool show_demo_window = true;
    bool show_another_window = false;
    bool show_implot_window = false;
    bool show_file_dialog_window = false;
    bool show_text_edit_window = false;
    bool show_markdown_window = false;
    bool show_dock_window = false;
    bool show_node_window = false;
    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    bool done = false;
    bool show = true;
    while (!done)
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            ImGui_ImplSdlGLES2_ProcessEvent(&event);
            if (event.type == SDL_QUIT)
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(window))
                done = true;
            if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_SHOWN)
            {
                show = true;
            }
            if (event.type == SDL_WINDOWEVENT && (event.window.event == SDL_WINDOWEVENT_HIDDEN || event.window.event == SDL_WINDOWEVENT_MINIMIZED))
            {
                show = false;
            }
        }
        if (!show)
        {
            continue;
        }

        // Start the Dear ImGui frame
        ImGui_ImplSdlGLES2_NewFrame(window);
        ImGui::NewFrame();

        // 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
        if (show_demo_window)
            ImGui::ShowDemoWindow(&show_demo_window);

        // 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
        {
            static float f = 0.0f;
            static int counter = 0;

            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

            ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
            ImGui::Checkbox("Another Window", &show_another_window);
            ImGui::Checkbox("ImPlot Window", &show_implot_window);
            ImGui::Checkbox("File Dialog Window", &show_file_dialog_window);
            ImGui::Checkbox("Memory Edit Window", &mem_edit.Open);
            ImGui::Checkbox("Show Text Edit Window", &show_text_edit_window);
            ImGui::Checkbox("Show Markdown Window", &show_markdown_window);
            ImGui::Checkbox("Show Dock Window", &show_dock_window);
            ImGui::Checkbox("Show Node Window", &show_node_window);

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
        if (show_another_window)
        {
            ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::Text("Hello from another window!");
            if (ImGui::Button("Close Me"))
                show_another_window = false;
            ImGui::End();
        }

        // 4. Show ImPlot simple window
        if (show_implot_window)
        {
            ImPlot::ShowDemoWindow(&show_implot_window);
        }

        // 5. Show FileDialog demo window
        if (show_file_dialog_window)
        {
            show_file_dialog_demo_window(&filedialog, &show_file_dialog_window);
        }

        // 6. Show Memory Edit window
        if (mem_edit.Open)
        {
            ImGui::Begin("Memory Window", &mem_edit.Open);
            mem_edit.DrawWindow("Memory Editor", data, data_size);
            ImGui::End();
        }

        // 7. Show Text Edit Window
        if (show_text_edit_window)
        {
            editor.text_edit_demo(&show_text_edit_window);
        }

        // 8. Show Markdown Window
        if (show_markdown_window)
        {
            std::string help_doc = get_file_contents("docs/imgui.md");
            mdConfig.linkCallback =         LinkCallback;
            mdConfig.tooltipCallback =      NULL;
            mdConfig.imageCallback =        ImageCallback;
            mdConfig.linkIcon =             ICON_FA5_LINK;
            mdConfig.headingFormats[0] =    { io.Fonts->Fonts[0], true };
            mdConfig.headingFormats[1] =    { io.Fonts->Fonts[1], true };
            mdConfig.headingFormats[2] =    { io.Fonts->Fonts[2], false };
            mdConfig.userData =             NULL;
            mdConfig.formatCallback =       ExampleMarkdownFormatCallback;
            ImGui::Markdown( help_doc.c_str(), help_doc.length(), mdConfig );
        }

        // 9. Show Dock Window
        if (show_dock_window)
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
        if (show_node_window)
        {
            imnodes_sample::NodeEditorShow();
        }

        // Rendering
        ImGui::Render();
        glViewport(0, 0, (int)io.DisplaySize.x, (int)io.DisplaySize.y);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplSdlGLES2_RenderDrawData(ImGui::GetDrawData());
        SDL_GL_SwapWindow(window);
    }
    // Cleanup memory edit resource
    if (data)
        free(data);

    // Store file dialog bookmark
    end_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

    // Clean Node Window
    imnodes_sample::NodeEditorShutdown(node_ini_path.c_str(), node_path.c_str());
    imnodes::Shutdown();

    // Cleanup
    ImGui_ImplSdlGLES2_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    SDL_GL_DeleteContext(gl_context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}

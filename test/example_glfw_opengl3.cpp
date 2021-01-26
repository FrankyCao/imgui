// Dear ImGui: standalone example application for GLFW + OpenGL 3, using programmable pipeline
// (GLFW is a cross-platform general purpose library for handling windows, inputs, OpenGL/Vulkan/Metal graphics context creation, etc.)
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include <stdio.h>
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

// About Desktop OpenGL function loaders:
//  Modern desktop OpenGL doesn't have a standard portable header file to load OpenGL function pointers.
//  Helper libraries are often used for this purpose! Here we are supporting a few common ones (gl3w, glew, glad).
//  You may use another loader/header of your choice (glext, glLoadGen, etc.), or chose to manually implement your own.
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
#include <GL/gl3w.h>            // Initialize with gl3wInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
#include <GL/glew.h>            // Initialize with glewInit()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
#include <glad/glad.h>          // Initialize with gladLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
#include <glad/gl.h>            // Initialize with gladLoadGL(...) or gladLoaderLoadGL()
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/Binding.h>  // Initialize with glbinding::Binding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
#define GLFW_INCLUDE_NONE       // GLFW including OpenGL headers causes ambiguity or multiple definition errors.
#include <glbinding/glbinding.h>// Initialize with glbinding::initialize()
#include <glbinding/gl/gl.h>
using namespace gl;
#else
#include IMGUI_IMPL_OPENGL_LOADER_CUSTOM
#endif

// Include glfw3.h after our OpenGL definitions
#include <GLFW/glfw3.h>

// [Win32] Our example includes a copy of glfw3.lib pre-compiled with VS2010 to maximize ease of testing and compatibility with old VS compilers.
// To link with VS2010-era libraries, VS2015+ requires linking with legacy_stdio_definitions.lib, which we do using this pragma.
// Your own project should not be affected, as you are likely to link with a newer binary of GLFW that is adequate for your version of Visual Studio.
#if defined(_MSC_VER) && (_MSC_VER >= 1900) && !defined(IMGUI_DISABLE_WIN32_FUNCTIONS)
#pragma comment(lib, "legacy_stdio_definitions")
#endif

static void glfw_error_callback(int error, const char* description)
{
    fprintf(stderr, "Glfw Error %d: %s\n", error, description);
}

int main(int, char**)
{
    // Setup window
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
        return 1;

    // Decide GL+GLSL versions
#ifdef __APPLE__
    // GL 3.2 + GLSL 150
    const char* glsl_version = "#version 150";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // Required on Mac
#elif defined(__arm__) || defined(__aarch64__)
    // GL 2.1 + GLSL 120
    const char* glsl_version = "#version 120";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 1);
#else
    // GL 3.0 + GLSL 130
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    //glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
#endif
    int window_width = 1440;
    int window_height = 960;
    float window_scale = 1;
#ifdef __linux__
    GLFWmonitor * main_monitor = glfwGetPrimaryMonitor();
    float xscale = 0, yscale = 0;
    glfwGetMonitorContentScale(main_monitor, &xscale, &yscale);
    if (xscale != 0 && yscale != 0)
    {
        window_width *= xscale;
        window_height *= yscale;
        window_scale = xscale;
    }
#endif

    // Create window with graphics context
    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "Dear ImGui GLFW+OpenGL3 example", NULL, NULL);
    if (window == NULL)
        return 1;
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Initialize OpenGL loader
#if defined(IMGUI_IMPL_OPENGL_LOADER_GL3W)
    bool err = gl3wInit() != 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLEW)
    bool err = glewInit() != GLEW_OK;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD)
    bool err = gladLoadGL() == 0;
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLAD2)
    bool err = gladLoadGL(glfwGetProcAddress) == 0; // glad2 recommend using the windowing library loader instead of the (optionally) bundled one.
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING2)
    bool err = false;
    glbinding::Binding::initialize();
#elif defined(IMGUI_IMPL_OPENGL_LOADER_GLBINDING3)
    bool err = false;
    glbinding::initialize([](const char* name) { return (glbinding::ProcAddress)glfwGetProcAddress(name); });
#else
    bool err = false; // If you use IMGUI_IMPL_OPENGL_LOADER_CUSTOM, your loader is likely to requires some form of initialization.
#endif
    if (err)
    {
        fprintf(stderr, "Failed to initialize OpenGL loader!\n");
        return 1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    std::string ini_file = std::string(DEFAULT_CONFIG_PATH) + "glfw_opengl3.ini";
    io.IniFilename = ini_file.c_str();
    io.FontGlobalScale = window_scale;
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.md' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

    // load file dialog resource
    ImGuiFileDialog filedialog;
    std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
    prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

    // init sample file dialog
    ImGuiFs::Dialog dlg;

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

    // Init NodeGraphEditor
    ImGui::NodeGraphEditor nge;
    std::string nge_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.nge.ini";
    std::string nge_style_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.style.ini";
    nge.save_node_path = nge_ini_path;
    nge.save_style_path = nge_style_path;

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
    bool show_sample_file_dialog = false;
    bool show_text_edit_window = false;
    bool show_markdown_window = false;
    bool show_dock_window = false;
    bool show_tab_window = false;
    bool show_node_window = false;
    bool show_node_edit_window = false;
    bool show_addon_widget = false;
    bool show_zmo_window = false;

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Main loop
    while (!glfwWindowShouldClose(window))
    {
        // Poll and handle events (inputs, window resize, etc.)
        // You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
        // - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application.
        // - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application.
        // Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
        glfwPollEvents();

        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
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
            ImGui::Checkbox("Sample File Dialog", &show_sample_file_dialog);
            ImGui::Checkbox("Memory Edit Window", &mem_edit.Open);
            ImGui::Checkbox("Show Text Edit Window", &show_text_edit_window);
            ImGui::Checkbox("Show Markdown Window", &show_markdown_window);
            ImGui::Checkbox("Show Dock Window", &show_dock_window);
            ImGui::Checkbox("Show Tab Window", &show_tab_window);
            ImGui::Checkbox("Show Node Sample Window", &show_node_window);
            ImGui::Checkbox("Show Node Edit Windows", &show_node_edit_window);
            ImGui::Checkbox("Show Addon Widgets", &show_addon_widget);
            ImGui::Checkbox("Show ImGuizmo Window", &show_zmo_window);

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

        // 6. Show Sample FileDialog
        {
            // dlg.WrapMode = false;
            const char* filePath = dlg.chooseFileDialog(show_sample_file_dialog, dlg.getLastDirectory(), ".jpg;.jpeg;.png;.gif;.tga;.bmp", "Sample file dialog", ImVec2(400, 800), ImVec2(50, 50));
            if (strlen(filePath) > 0) 
            {
	            //fprintf(stderr,"Browsed..: %s\n",filePath);
            }
            show_sample_file_dialog = false;
        }

        // 7. Show Memory Edit window
        if (mem_edit.Open)
        {
            ImGui::Begin("Memory Window", &mem_edit.Open);
            mem_edit.DrawWindow("Memory Editor", data, data_size);
            ImGui::End();
        }

        // 8. Show Text Edit Window
        if (show_text_edit_window)
        {
            editor.text_edit_demo(&show_text_edit_window);
        }

        // 9. Show Markdown Window
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

        // 10. Show Dock Window
        if (show_dock_window)
        {
            ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
            if(ImGui::Begin("imguidock window (= lumix engine's dock system)",&show_dock_window, ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::ShowAddonsDuckWindow();
            }
            ImGui::End();
        }

        // 11. Show Tab Window
        if (show_tab_window)
        {
            ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Example: TabWindow", &show_tab_window, ImGuiWindowFlags_NoScrollbar))
            {
                ImGui::ShowAddonsTabWindow();   // see its code for further info         
            }
            ImGui::End();
        }

        // 12. Show Node  Window
        if (show_node_window)
        {
            imnodes_sample::NodeEditorShow();
        }

        // 13. Show Node Edit Window
        if (show_node_edit_window)
        {
            ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
            if (ImGui::Begin("Example: Custom Node Graph",&show_node_edit_window, ImGuiWindowFlags_NoScrollbar))
            {
                std::string node_ini_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.nge.ini";
                std::string node_style_path = std::string(DEFAULT_CONFIG_PATH) + "nodeGraphEditor.style.ini";
                ImGui::TestNodeGraphEditor(&nge);   // see its code for further info         
            }
            ImGui::End();
        }

        // 14. Show Addon Widget.
        if (show_addon_widget)
        {
            ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
            ImGui::Begin("Addon Widget", &show_addon_widget);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
            ImGui::ShowAddonsDemoWindowWidgets();
            ImGui::End();
        }

        // 15. Show Zmo Window
        if (show_zmo_window)
        {
            ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
            ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
            ImGui::Begin("##ZMO", &show_zmo_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
            ImGui::ShowAddonsZMOWindow();
            ImGui::End();
        }

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
    // Cleanup memory edit resource
    if (data)
        free(data);

    // Store file dialog bookmark
    end_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

    // Clean Node Window
    imnodes_sample::NodeEditorShutdown(node_ini_path.c_str(), node_path.c_str());
    imnodes::Shutdown();

    // Cleanup Demo
    ImGui::CleanupDemo();
    ImGui::CleanupZMODemo();
    nge.clear();

    // Cleanup
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}

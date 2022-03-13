#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
#include <imgui_helper.h>
#include <application.h>
#include <fstream>
#include <sstream>
#include <string>
#include <cerrno>
#include <imgui_markdown.h>
#include <imgui_memory_editor.h>
#include <ImGuiFileDialog.h>
#include <imgui_extra_widget.h>
#include <HotKey.h>
#include <TextEditor.h>
#include <ImGuiTabWindow.h>
#if IMGUI_VULKAN_SHADER
#include <ImVulkanShader.h>
#endif
#include <immat.h>
#include "Config.h"

// Init HotKey
static std::vector<ImHotKey::HotKey> hotkeys = 
{ 
    {"Layout", "Reorder nodes in a simpler layout", 0xFFFF26E0},
    {"Save", "Save the current graph", 0xFFFF1FE0},
    {"Load", "Load an existing graph file", 0xFFFF18E0},
    {"Play/Stop", "Play or stop the animation from the current graph", 0xFFFFFF3F},
    {"SetKey", "Make a new animation key with the current parameters values at the current time", 0xFFFFFF1F}
};

static inline void box(ImGui::ImMat& image, int x1, int y1, int x2, int y2, int R, int G, int B)
{
    for (int j = y1; j <= y2; j++)
    {
        for (int i = x1; i <= x2; i++)
        {
            //unsigned int color = 0xFF000000 | (R << 16) | (G << 8) | B;
            //image.at<unsigned int>(i, j) = color;
            image.at<unsigned char>(i, j, 3) = 0xFF;
            image.at<unsigned char>(i, j, 2) = B;
            image.at<unsigned char>(i, j, 1) = G;
            image.at<unsigned char>(i, j, 0) = R;
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

#if IMGUI_VULKAN_SHADER
// VulkanShader Demo
static std::string print_result(float gflops)
{
    if (gflops == -1)
            return "  error";

    if (gflops == -233)
        return "  not supported";

    if (gflops == 0)
        return "  not tested";

    if (gflops > 1000)
        return "  " + std::to_string(gflops / 1000.0) + " TFLOPS";
    return "  " + std::to_string(gflops) + " GFLOPS";
}

static void ImVulkanTestWindow(const char* name, bool* p_open, ImGuiWindowFlags flags)
{
    ImGui::Begin(name, p_open, flags);
    static int loop_count = 200;
    static int block_count = 20;
    static int cmd_count = 1;
    static float fp32[8] = {0.f};
    static float fp32v4[8] = {0.f};
    static float fp32v8[8] = {0.f};
    static float fp16pv4[8] = {0.f};
    static float fp16pv8[8] = {0.f};
    static float fp16s[8] = {0.f};
    static float fp16sv4[8] = {0.f};
    static float fp16sv8[8] = {0.f};
    int device_count = ImGui::get_gpu_count();
    for (int i = 0; i < device_count; i++)
    {
        ImGui::VulkanDevice* vkdev = ImGui::get_gpu_device(i);
        uint32_t driver_version = vkdev->info.driver_version();
        uint32_t api_version = vkdev->info.api_version();
        int device_type = vkdev->info.type();
        std::string driver_ver = std::to_string(VK_VERSION_MAJOR(driver_version)) + "." + 
                                std::to_string(VK_VERSION_MINOR(driver_version)) + "." +
                                std::to_string(VK_VERSION_PATCH(driver_version));
        std::string api_ver =   std::to_string(VK_VERSION_MAJOR(api_version)) + "." + 
                                std::to_string(VK_VERSION_MINOR(api_version)) + "." +
                                std::to_string(VK_VERSION_PATCH(api_version));
        std::string device_name = vkdev->info.device_name();
        uint32_t gpu_memory_budget = vkdev->get_heap_budget();
        ImGui::Text("%uMB", gpu_memory_budget);
        ImGui::Text("Device[%d]", i);
        ImGui::Text("Driver:%s", driver_ver.c_str());
        ImGui::Text("   API:%s", api_ver.c_str());
        ImGui::Text("  Name:%s", device_name.c_str());
        ImGui::Text("Memory:%uMB", gpu_memory_budget);
        ImGui::Text("Device Type:%s", device_type == 0 ? "Discrete" : device_type == 1 ? "Integrated" : device_type == 2 ? "Virtual" : "CPU");
        std::string buffon_label = "Perf Test##" + std::to_string(i);
        if (ImGui::Button(buffon_label.c_str(), ImVec2(120, 20)))
        {
            int _loop_count = device_type == 0 ? loop_count : loop_count / 5;
            fp32[i]     = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 0, 0, 0);
            fp32v4[i]   = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 0, 0, 1);
            fp32v8[i]   = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 0, 0, 2);
            fp16pv4[i]  = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 1, 1, 1);
            fp16pv8[i]  = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 1, 1, 2);
            fp16s[i]    = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 2, 1, 0);
            fp16sv4[i]  = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 2, 1, 1);
            fp16sv8[i]  = ImGui::ImVulkanPeak(vkdev, _loop_count, block_count, cmd_count, 2, 1, 2);
        }
        ImGui::Text(" FP32 Scalar :%s", print_result(fp32[i]).c_str());
        ImGui::Text("   FP32 Vec4 :%s", print_result(fp32v4[i]).c_str());
        ImGui::Text("   FP32 Vec8 :%s", print_result(fp32v8[i]).c_str());
        ImGui::Text("  FP16p Vec4 :%s", print_result(fp16pv4[i]).c_str());
        ImGui::Text("  FP16p Vec8 :%s", print_result(fp16pv8[i]).c_str());
        ImGui::Text("FP16s Scalar :%s", print_result(fp16s[i]).c_str());
        ImGui::Text("  FP16s Vec4 :%s", print_result(fp16sv4[i]).c_str());
        ImGui::Text("  FP16s Vec8 :%s", print_result(fp16sv8[i]).c_str());
        
        ImGui::Separator();
    }
    ImGui::End();
}
#endif

class Example
{
public:
    Example() 
    {
        // load file dialog resource
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
        prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());

        // init memory edit
        mem_edit.Open = false;
        mem_edit.OptShowDataPreview = true;
        mem_edit.OptAddrDigitsCount = 8;
        data = malloc(0x400);
        // init color inspact
        color_bar(image, 0, 0, 255, 191);
        gray_bar(image, 0, 192, 255, 255, 13);
        ImageTexture = ImGui::ImCreateTexture(image.data, image.w, image.h);
    };
    ~Example() 
    {
        if (data)
            free(data); 

        // Store file dialog bookmark
        std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
        end_file_dialog_demo_window(&filedialog, bookmark_path.c_str());
        if (ImageTexture) { ImGui::ImDestroyTexture(ImageTexture); ImageTexture = 0; }
    }

public:
    // init file dialog
    ImGuiFileDialog filedialog;

    // init memory edit
    MemoryEditor mem_edit;
    void* data = nullptr;

    // Init MarkDown
    ImGui::MarkdownConfig mdConfig;

    // Init Colorful Text Edit
    TextEditor editor;

public:
    bool show_demo_window = true;
    bool show_viewport_fullscreen = false;
    bool show_another_window = false;

    bool show_file_dialog_window = false;
    bool show_markdown_window = false;
    bool show_widget_window = false;
    bool show_text_editor_window = false;
    bool show_tab_window = false;
   
public:

    std::string get_file_contents(const char *filename);
    static ImGui::MarkdownImageData ImageCallback( ImGui::MarkdownLinkCallbackData data_ );
    static void LinkCallback( ImGui::MarkdownLinkCallbackData data_ );
    static void ExampleMarkdownFormatCallback( const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_ );

#if IMGUI_VULKAN_SHADER
public:
    bool show_shader_window = false;
#endif
public:
    ImGui::ImMat image {ImGui::ImMat(256, 256, 4, 1u, 4)};
    ImTextureID ImageTexture = 0;
};

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

static std::string ini_file = std::string(DEFAULT_CONFIG_PATH) + "Application_Example.ini";

void Application_GetWindowProperties(ApplicationWindowProperty& property)
{
    property.name = "Application_Example";
}

void Application_SetupContext(ImGuiContext* ctx)
{
}

void Application_Initialize(void** handle)
{
    srand((unsigned int)time(0));
    *handle = new Example();
    Example * example = (Example *)*handle;
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
}

bool Application_Frame(void* handle, bool app_will_quit)
{
    bool app_done = false;
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
        if (ImGui::Checkbox("Full Screen Window", &example->show_viewport_fullscreen))
        {
            if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
            {
                auto platformio = ImGui::GetPlatformIO();
                if (platformio.Platform_FullScreen) platformio.Platform_FullScreen(ImGui::GetMainViewport(), example->show_viewport_fullscreen);
            }
            else
                Application_FullScreen(example->show_viewport_fullscreen);
        }
        ImGui::Checkbox("Another Window", &example->show_another_window);
        ImGui::Checkbox("File Dialog Window", &example->show_file_dialog_window);
        ImGui::Checkbox("Memory Edit Window", &example->mem_edit.Open);
        ImGui::Checkbox("Show Markdown Window", &example->show_markdown_window);
        ImGui::Checkbox("Show Extra Widget Window", &example->show_widget_window);
        ImGui::Checkbox("Show Text Edit Window", &example->show_text_editor_window);
        ImGui::Checkbox("Show Tab Window", &example->show_tab_window);

#if IMGUI_VULKAN_SHADER
        ImGui::Checkbox("Show Vulkan Shader Test Window", &example->show_shader_window);
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

        ImVec2 displayedTextureSize(256,256);
        ImGui::Image((ImTextureID)(uint64_t)example->ImageTexture, displayedTextureSize);
        {
            ImRect rc = ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
            ImVec2 mouseUVCoord = (io.MousePos - rc.Min) / rc.GetSize();
            if (ImGui::IsItemHovered() && mouseUVCoord.x >= 0.f && mouseUVCoord.y >= 0.f)
            {
                ImGui::ImageInspect(example->image.w, example->image.h, 
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

    // Show FileDialog demo window
    if (example->show_file_dialog_window)
    {
        show_file_dialog_demo_window(&example->filedialog, &example->show_file_dialog_window);
    }

    // Show Memory Edit window
    if (example->mem_edit.Open)
    {
        static int i = 0;
        int * test_point = (int *)example->data;
        *test_point = i; i++;
        example->mem_edit.DrawWindow("Memory Editor", example->data, 0x400, 0, &example->mem_edit.Open, 768);
    }

    // Show Markdown Window
    if (example->show_markdown_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
        ImGui::Begin("iMarkdown window",&example->show_markdown_window, ImGuiWindowFlags_NoScrollbar);
        std::string help_doc =                   example->get_file_contents("README.md");
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

    // Show Extra widget Window
    if (example->show_widget_window)
    {
        ImGui::SetNextWindowSize(ImVec2(1024, 768), ImGuiCond_FirstUseEver);
        ImGui::Begin("Extra Widget", &example->show_widget_window);
        ImGui::ShowExtraWidgetDemoWindow();
        ImGui::End();
    }

    // Show Text Edit Window
    if (example->show_text_editor_window)
    {
        example->editor.text_edit_demo(&example->show_text_editor_window);
    }

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

#if IMGUI_VULKAN_SHADER
    // Show Vulkan Shader Test Window
    if (example->show_shader_window)
    {
        ImVulkanTestWindow("ImGui Vulkan test", &example->show_shader_window, 0);
    }
#endif
    if (app_will_quit)
        app_done = true;
    return app_done;
}

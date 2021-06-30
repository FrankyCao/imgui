// Dear ImGui: standalone example application for OSX + Metal.
// If you are new to Dear ImGui, read documentation from the docs/ folder + read the top of imgui.cpp.
// Read online: https://github.com/ocornut/imgui/tree/master/docs

#import <Foundation/Foundation.h>

#if TARGET_OS_OSX
#import <Cocoa/Cocoa.h>
#else
#import <UIKit/UIKit.h>
#endif

#import <Metal/Metal.h>
#import <MetalKit/MetalKit.h>

#include "imgui.h"
#include "imgui_impl_metal.h"
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
#if IMGUI_ADDON_DOCK
#include "imgui_dock.h"
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

#if TARGET_OS_OSX
#include "imgui_impl_osx.h"
@interface AppViewController : NSViewController
@end
#else
@interface AppViewController : UIViewController
@end
#endif
#include <string>
#include <fstream>
#include <sstream>
#include "Config.h"

#if IMGUI_ADDON_MARKDOWN
static std::string get_file_contents(const char *filename)
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
#endif
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

@interface AppViewController () <MTKViewDelegate>
@property (nonatomic, readonly) MTKView *mtkView;
@property (nonatomic, strong) id <MTLDevice> device;
@property (nonatomic, strong) id <MTLCommandQueue> commandQueue;
@end

#if IMGUI_ADDON_FILE_DIALOG
ImGuiFileDialog filedialog;
#endif
#if IMGUI_ADDON_FILE_SYSTEM
ImGuiFs::Dialog dlg;
#endif
#if IMGUI_ADDON_MEMORY_EDITOR
MemoryEditor mem_edit;
size_t data_size = 0x1000;
void* data = nullptr;
#endif
#if IMGUI_ADDON_TEXT_EDITOR
TextEditor editor;
#endif
#if IMGUI_ADDON_MARKDOWN
ImGui::MarkdownConfig mdConfig; 
#endif
#if IMGUI_ADDON_NODE_GRAPH
ImGui::NodeGraphEditor nge;
#endif
//-----------------------------------------------------------------------------------
// AppViewController
//-----------------------------------------------------------------------------------

@implementation AppViewController

-(instancetype)initWithNibName:(nullable NSString *)nibNameOrNil bundle:(nullable NSBundle *)nibBundleOrNil
{
    self = [super initWithNibName:nibNameOrNil bundle:nibBundleOrNil];

    _device = MTLCreateSystemDefaultDevice();
    _commandQueue = [_device newCommandQueue];

    if (!self.device)
    {
        NSLog(@"Metal is not supported");
        abort();
    }

    // Setup Dear ImGui context
    // FIXME: This example doesn't have proper cleanup...
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
#if IMGUI_ADDON_IMPLOTS
    ImPlot::CreateContext();
#endif
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    std::string ini_file = std::string(DEFAULT_CONFIG_PATH) + "apple_metal.ini";
    io.IniFilename = ini_file.c_str();
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    // Setup Dear ImGui style
    ImGui::StyleColorsDark();
    //ImGui::StyleColorsClassic();

    // Setup Renderer backend
    ImGui_ImplMetal_Init(_device);

    // Load Fonts
    // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
    // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
    // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
    // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
    // - Read 'docs/FONTS.txt' for more instructions and details.
    // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
    //io.Fonts->AddFontDefault();
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
    //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
    //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
    //IM_ASSERT(font != NULL);

#if IMGUI_ADDON_FILE_DIALOG
    // load file dialog resource
    std::string bookmark_path = std::string(DEFAULT_CONFIG_PATH) + "bookmark.ini";
    prepare_file_dialog_demo_window(&filedialog, bookmark_path.c_str());
#endif

#if IMGUI_ADDON_MEMORY_EDITOR
    // init memory edit
    mem_edit.Open = false;
    mem_edit.OptShowDataPreview = true;
    data = malloc(data_size);
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
    return self;
}

-(MTKView *)mtkView
{
    return (MTKView *)self.view;
}

-(void)loadView
{
    self.view = [[MTKView alloc] initWithFrame:CGRectMake(0, 0, 1440, 1024)];
}

-(void)viewDidLoad
{
    [super viewDidLoad];

    self.mtkView.device = self.device;
    self.mtkView.delegate = self;

#if TARGET_OS_OSX
    // Add a tracking area in order to receive mouse events whenever the mouse is within the bounds of our view
    NSTrackingArea *trackingArea = [[NSTrackingArea alloc] initWithRect:NSZeroRect
                                                                options:NSTrackingMouseMoved | NSTrackingInVisibleRect | NSTrackingActiveAlways
                                                                  owner:self
                                                               userInfo:nil];
    [self.view addTrackingArea:trackingArea];

    // If we want to receive key events, we either need to be in the responder chain of the key view,
    // or else we can install a local monitor. The consequence of this heavy-handed approach is that
    // we receive events for all controls, not just Dear ImGui widgets. If we had native controls in our
    // window, we'd want to be much more careful than just ingesting the complete event stream.
    // To match the behavior of other backends, we pass every event down to the OS.
    NSEventMask eventMask = NSEventMaskKeyDown | NSEventMaskKeyUp | NSEventMaskFlagsChanged;
    [NSEvent addLocalMonitorForEventsMatchingMask:eventMask handler:^NSEvent * _Nullable(NSEvent *event)
    {
        ImGui_ImplOSX_HandleEvent(event, self.view);
        return event;
    }];

    ImGui_ImplOSX_Init();

#endif
}

-(void)drawInMTKView:(MTKView*)view
{
    ImGuiIO& io = ImGui::GetIO();
    io.DisplaySize.x = view.bounds.size.width;
    io.DisplaySize.y = view.bounds.size.height;

#if TARGET_OS_OSX
    CGFloat framebufferScale = view.window.screen.backingScaleFactor ?: NSScreen.mainScreen.backingScaleFactor;
#else
    CGFloat framebufferScale = view.window.screen.scale ?: UIScreen.mainScreen.scale;
#endif
    io.DisplayFramebufferScale = ImVec2(framebufferScale, framebufferScale);

    io.DeltaTime = 1 / float(view.preferredFramesPerSecond ?: 60);

    id<MTLCommandBuffer> commandBuffer = [self.commandQueue commandBuffer];

    MTLRenderPassDescriptor* renderPassDescriptor = view.currentRenderPassDescriptor;
    if (renderPassDescriptor == nil)
    {
        [commandBuffer commit];
		return;
    }

    // Start the Dear ImGui frame
    ImGui_ImplMetal_NewFrame(renderPassDescriptor);
#if TARGET_OS_OSX
    ImGui_ImplOSX_NewFrame(view);
#endif
    ImGui::NewFrame();

    // Our state (make them static = more or less global) as a convenience to keep the example terse.
    static bool show_demo_window = true;
    static bool show_another_window = false;

    static bool show_implot_window = false;
    static bool show_file_dialog_window = false;
    static bool show_sample_file_dialog = false;
    static bool show_text_edit_window = false;
    static bool show_markdown_window = false;
    static bool show_dock_window = false;
    static bool show_tab_window = false;
    static bool show_node_window = false;
    static bool show_node_edit_window = false;
    static bool show_addon_widget = false;
    static bool show_zmo_window = false;
    static bool show_quat_window = false;
    static bool show_dear_widget_window = false;

    static ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    // Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
    if (show_demo_window)
        ImGui::ShowDemoWindow(&show_demo_window);

    // Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
    {
        static float f = 0.0f;
        static int counter = 0;

        ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

        ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
        ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state
        ImGui::Checkbox("Another Window", &show_another_window);

#if IMGUI_ADDON_IMPLOTS
        ImGui::Checkbox("ImPlot Window", &show_implot_window);
#endif
#if IMGUI_ADDON_FILE_DIALOG
        ImGui::Checkbox("File Dialog Window", &show_file_dialog_window);
#endif
#if IMGUI_ADDON_FILE_SYSTEM
        ImGui::Checkbox("Sample File Dialog", &show_sample_file_dialog);
#endif
#if IMGUI_ADDON_MEMORY_EDITOR
        ImGui::Checkbox("Memory Edit Window", &mem_edit.Open);
#endif
#if IMGUI_ADDON_TEXT_EDITOR
        ImGui::Checkbox("Show Text Edit Window", &show_text_edit_window);
#endif
#if IMGUI_ADDON_MARKDOWN
        ImGui::Checkbox("Show Markdown Window", &show_markdown_window);
#endif
#if IMGUI_ADDON_DOCK
        ImGui::Checkbox("Show Dock Window", &show_dock_window);
#endif
#if IMGUI_ADDON_TABWINDOW
        ImGui::Checkbox("Show Tab Window", &show_tab_window);
#endif
#if IMGUI_ADDON_IMNODES
        ImGui::Checkbox("Show Node Sample Window", &show_node_window);
#endif
#if IMGUI_ADDON_NODE_GRAPH
        ImGui::Checkbox("Show Node Edit Windows", &show_node_edit_window);
#endif
#if IMGUI_ADDON_DATE_CHOOSER || IMGUI_ADDON_KNOB || IMGUI_ADDON_VARIOUS || IMGUI_ADDON_DOCK || IMGUI_ADDON_TABWINDOW || IMGUI_ADDON_PROGRESSES || IMGUI_ADDON_TIMELINE
        ImGui::Checkbox("Show Addon Widgets", &show_addon_widget);
#endif
#if IMGUI_ADDON_ZMO
        ImGui::Checkbox("Show ImGuizmo Window", &show_zmo_window);
#endif
#if IMGUI_ADDON_ZMOQUAT
        ImGui::Checkbox("Show ZMOQuat Window", &show_quat_window);
#endif
#if IMGUI_ADDON_DEAR_WIDGETS
        ImGui::Checkbox("Show DearWidgets Window", &show_dear_widget_window);
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

        ImGui::SliderFloat("float", &f, 0.0f, 1.0f);            // Edit 1 float using a slider from 0.0f to 1.0f
        ImGui::ColorEdit3("clear color", (float*)&clear_color); // Edit 3 floats representing a color

        if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
            counter++;
        ImGui::SameLine();
        ImGui::Text("counter = %d", counter);

        ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
        ImGui::End();
    }

    // Show another simple window.
    if (show_another_window)
    {
        ImGui::Begin("Another Window", &show_another_window);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::Text("Hello from another window!");
        if (ImGui::Button("Close Me"))
            show_another_window = false;
        ImGui::End();
    }

#if IMGUI_ADDON_IMPLOTS
    // Show ImPlot simple window
    if (show_implot_window)
    {
        ImPlot::ShowDemoWindow(&show_implot_window);
    }
#endif

#if IMGUI_ADDON_FILE_DIALOG
    // Show FileDialog demo window
    if (show_file_dialog_window)
    {
        show_file_dialog_demo_window(&filedialog, &show_file_dialog_window);
    }
#endif

#if IMGUI_ADDON_FILE_SYSTEM
    // Show Sample FileDialog
    {
        // dlg.WrapMode = false;
        const char* filePath = dlg.chooseFileDialog(show_sample_file_dialog, dlg.getLastDirectory(), ".jpg;.jpeg;.png;.gif;.tga;.bmp", "Sample file dialog", ImVec2(400, 800), ImVec2(50, 50));
        if (strlen(filePath) > 0) 
        {
	        //fprintf(stderr,"Browsed..: %s\n",filePath);
        }
        show_sample_file_dialog = false;
    }
#endif

#if IMGUI_ADDON_MEMORY_EDITOR
    // Show Memory Edit window
    if (mem_edit.Open)
    {
        mem_edit.DrawWindow("Memory Editor", data, data_size, 0, &mem_edit.Open, 768);
    }
#endif

#if IMGUI_ADDON_TEXT_EDITOR
    // Show Text Edit Window
    if (show_text_edit_window)
    {
        editor.text_edit_demo(&show_text_edit_window);
    }
#endif

#if IMGUI_ADDON_MARKDOWN
    // Show Markdown Window
    if (show_markdown_window)
    {
        std::string help_doc = get_file_contents("imgui.md");
        mdConfig.linkCallback =         LinkCallback;
        mdConfig.tooltipCallback =      NULL;
        mdConfig.imageCallback =        ImageCallback;
        mdConfig.linkIcon =             ICON_FA5_LINK;
        mdConfig.headingFormats[0] =    { io.Fonts->Fonts[0], true };
        mdConfig.headingFormats[1] =    { io.Fonts->Fonts.size() > 1 ? io.Fonts->Fonts[1] : nullptr, true };
        mdConfig.headingFormats[2] =    { io.Fonts->Fonts.size() > 2 ? io.Fonts->Fonts[2] : nullptr, false };
        mdConfig.userData =             NULL;
        mdConfig.formatCallback =       ExampleMarkdownFormatCallback;
        ImGui::Markdown( help_doc.c_str(), help_doc.length(), mdConfig );
    }
#endif

#if IMGUI_ADDON_DOCK
    // Show Dock Window
    if (show_dock_window)
    {
        ImGui::SetNextWindowSize(ImVec2(400, 300), ImGuiCond_FirstUseEver);
        if(ImGui::Begin("imguidock window (= lumix engine's dock system)",&show_dock_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowAddonsDuckWindow();
        }
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_TABWINDOW
    // Show Tab Window
    if (show_tab_window)
    {
        ImGui::SetNextWindowSize(ImVec2(700,600), ImGuiCond_FirstUseEver);
        if (ImGui::Begin("Example: TabWindow", &show_tab_window, ImGuiWindowFlags_NoScrollbar))
        {
            ImGui::ShowAddonsTabWindow();   // see its code for further info         
        }
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_IMNODES
    // Show Node Window
    if (show_node_window)
    {
        imnodes_example::NodeEditorShow();
    }
#endif

#if IMGUI_ADDON_NODE_GRAPH
    // Show Node Edit Window
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
#endif

#if IMGUI_ADDON_DATE_CHOOSER || IMGUI_ADDON_KNOB || IMGUI_ADDON_VARIOUS || IMGUI_ADDON_DOCK || IMGUI_ADDON_TABWINDOW || IMGUI_ADDON_PROGRESSES || IMGUI_ADDON_TIMELINE
    // Show Addon Widget.
    if (show_addon_widget)
    {
        ImGui::SetNextWindowSize(ImVec2(600, 400), ImGuiCond_FirstUseEver);
        ImGui::Begin("Addon Widget", &show_addon_widget);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)
        ImGui::ShowAddonsDemoWindowWidgets();
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_ZMO
    // Show Zmo Window
    if (show_zmo_window)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
        ImGui::Begin("##ZMO", &show_zmo_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
        ImGuizmo::ShowAddonsZMOWindow();
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_ZMOQUAT
    // Show Zmo Quat Window
    if (show_quat_window)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
        ImGui::Begin("##ZMOQuat", &show_quat_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
        ImGui::ShowGizmoDemo();
        ImGui::End();
    }
#endif

#if IMGUI_ADDON_DEAR_WIDGETS
    // Show Dear Widgets Window
    if (show_dear_widget_window)
    {
        ImGui::SetNextWindowPos(ImVec2(10, 10), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(1280, 800), ImGuiCond_FirstUseEver);
        ImGui::Begin("##DearWidgets", &show_dear_widget_window, ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar);
        ImWidgets::ShowDemo();
        ImGui::End();
    }
#endif

    // Rendering
    ImGui::Render();
    ImDrawData* draw_data = ImGui::GetDrawData();

    renderPassDescriptor.colorAttachments[0].clearColor = MTLClearColorMake(clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w);
    id <MTLRenderCommandEncoder> renderEncoder = [commandBuffer renderCommandEncoderWithDescriptor:renderPassDescriptor];
    [renderEncoder pushDebugGroup:@"Dear ImGui rendering"];
    ImGui_ImplMetal_RenderDrawData(draw_data, commandBuffer, renderEncoder);
    [renderEncoder popDebugGroup];
    [renderEncoder endEncoding];

	// Present
    [commandBuffer presentDrawable:view.currentDrawable];
    [commandBuffer commit];
}

-(void)mtkView:(MTKView*)view drawableSizeWillChange:(CGSize)size
{
}

//-----------------------------------------------------------------------------------
// Input processing
//-----------------------------------------------------------------------------------

#if TARGET_OS_OSX

// Forward Mouse/Keyboard events to Dear ImGui OSX backend.
// Other events are registered via addLocalMonitorForEventsMatchingMask()
-(void)mouseDown:(NSEvent *)event           { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseDown:(NSEvent *)event      { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseDown:(NSEvent *)event      { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseUp:(NSEvent *)event             { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseUp:(NSEvent *)event        { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseUp:(NSEvent *)event        { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseMoved:(NSEvent *)event          { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)mouseDragged:(NSEvent *)event        { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseMoved:(NSEvent *)event     { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)rightMouseDragged:(NSEvent *)event   { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseMoved:(NSEvent *)event     { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)otherMouseDragged:(NSEvent *)event   { ImGui_ImplOSX_HandleEvent(event, self.view); }
-(void)scrollWheel:(NSEvent *)event         { ImGui_ImplOSX_HandleEvent(event, self.view); }

#else

// This touch mapping is super cheesy/hacky. We treat any touch on the screen
// as if it were a depressed left mouse button, and we don't bother handling
// multitouch correctly at all. This causes the "cursor" to behave very erratically
// when there are multiple active touches. But for demo purposes, single-touch
// interaction actually works surprisingly well.
-(void)updateIOWithTouchEvent:(UIEvent *)event
{
    UITouch *anyTouch = event.allTouches.anyObject;
    CGPoint touchLocation = [anyTouch locationInView:self.view];
    ImGuiIO &io = ImGui::GetIO();
    io.MousePos = ImVec2(touchLocation.x, touchLocation.y);

    BOOL hasActiveTouch = NO;
    for (UITouch *touch in event.allTouches)
    {
        if (touch.phase != UITouchPhaseEnded && touch.phase != UITouchPhaseCancelled)
        {
            hasActiveTouch = YES;
            break;
        }
    }
    io.MouseDown[0] = hasActiveTouch;
}

-(void)touchesBegan:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event      { [self updateIOWithTouchEvent:event]; }
-(void)touchesMoved:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event      { [self updateIOWithTouchEvent:event]; }
-(void)touchesCancelled:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event  { [self updateIOWithTouchEvent:event]; }
-(void)touchesEnded:(NSSet<UITouch *> *)touches withEvent:(UIEvent *)event      { [self updateIOWithTouchEvent:event]; }

#endif

@end

//-----------------------------------------------------------------------------------
// AppDelegate
//-----------------------------------------------------------------------------------

#if TARGET_OS_OSX

@interface AppDelegate : NSObject <NSApplicationDelegate>
@property (nonatomic, strong) NSWindow *window;
@end

@implementation AppDelegate

-(BOOL)applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)sender
{
    return YES;
}

-(instancetype)init
{
    if (self = [super init])
    {
        NSViewController *rootViewController = [[AppViewController alloc] initWithNibName:nil bundle:nil];
        self.window = [[NSWindow alloc] initWithContentRect:NSZeroRect
                                                  styleMask:NSWindowStyleMaskTitled | NSWindowStyleMaskClosable | NSWindowStyleMaskResizable | NSWindowStyleMaskMiniaturizable
                                                    backing:NSBackingStoreBuffered
                                                      defer:NO];
        self.window.contentViewController = rootViewController;
        [self.window orderFront:self];
        [self.window center];
        [self.window becomeKeyWindow];
    }
    return self;
}

@end

#else

@interface AppDelegate : UIResponder <UIApplicationDelegate>
@property (strong, nonatomic) UIWindow *window;
@end

@implementation AppDelegate

-(BOOL)application:(UIApplication *)application
    didFinishLaunchingWithOptions:(NSDictionary<UIApplicationLaunchOptionsKey,id> *)launchOptions
{
    UIViewController *rootViewController = [[AppViewController alloc] init];
    self.window = [[UIWindow alloc] initWithFrame:UIScreen.mainScreen.bounds];
    self.window.rootViewController = rootViewController;
    [self.window makeKeyAndVisible];
    return YES;
}

@end

#endif

//-----------------------------------------------------------------------------------
// Application main() function
//-----------------------------------------------------------------------------------

#if TARGET_OS_OSX

int main(int argc, const char * argv[])
{
    return NSApplicationMain(argc, argv);
}

#else

int main(int argc, char * argv[])
{
    @autoreleasepool
    {
        return UIApplicationMain(argc, argv, nil, NSStringFromClass([AppDelegate class]));
    }
}

#endif

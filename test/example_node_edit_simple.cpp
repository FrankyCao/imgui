# include <imgui.h>
# define IMGUI_DEFINE_MATH_OPERATORS
# include <imgui_internal.h>
# include <imgui_node_editor.h>
# include <application.h>
# include "Config.h"

namespace ed = ax::NodeEditor;

static ed::EditorContext* g_Context = nullptr;
static std::string ini_file = std::string(DEFAULT_CONFIG_PATH) + "Simple.ini";
static std::string setting_file = std::string(DEFAULT_CONFIG_PATH) + "Simple.json";

const char* Application_GetName(void* handle)
{
    return "Simple";
}

void Application_Initialize(void** handle)
{
    ed::Config config;
    config.SettingsFile = setting_file.c_str();
    g_Context = ed::CreateEditor(&config);
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.IniFilename = ini_file.c_str();
}

void Application_Finalize(void** handle)
{
    ed::DestroyEditor(g_Context);
}

bool Application_Frame(void* handle)
{
    bool done = false;
    auto& io = ImGui::GetIO();

    ImGui::Text("FPS: %.2f (%.2gms)", io.Framerate, io.Framerate ? 1000.0f / io.Framerate : 0.0f);

    ImGui::Separator();

    ed::SetCurrentEditor(g_Context);
    ed::Begin("My Editor", ImVec2(0.0, 0.0f));
    int uniqueId = 1;
    // Start drawing nodes.
    ed::BeginNode(uniqueId++);
        ImGui::Text("Node A");
        ed::BeginPin(uniqueId++, ed::PinKind::Input);
            ImGui::Text("-> In");
        ed::EndPin();
        ImGui::SameLine();
        ed::BeginPin(uniqueId++, ed::PinKind::Output);
            ImGui::Text("Out ->");
        ed::EndPin();
    ed::EndNode();
    ed::End();
    ed::SetCurrentEditor(nullptr);
    return done;
}


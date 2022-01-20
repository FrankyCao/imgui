#pragma once
#include <imgui.h>
#include <string>

typedef struct ApplicationWindowProperty
{
    std::string name;
    int pos_x       {0};
    int pos_y       {0};
    int width       {1440};
    int height      {960};
    float scale     {1.0};
    float fps       {30.f};
    bool resizable  {true};
    bool docking    {true};
    bool viewport   {true};
    bool auto_merge {true};
    bool center     {true};
    bool power_save {true};
    void* handle    {nullptr};

} ApplicationWindowProperty;

void Application_GetWindowProperties(ApplicationWindowProperty& property);
void Application_SetupContext(ImGuiContext* ctx);
void Application_Initialize(void** handle = nullptr);
void Application_Finalize(void** handle = nullptr);
bool Application_Frame(void* handle = nullptr, bool app_will_quit = false);
void Application_FullScreen(bool on);

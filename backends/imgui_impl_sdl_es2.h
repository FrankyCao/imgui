#ifndef IMGUI_IMPL_SDL_ES2
#define IMGUI_IMPL_SDL_ES2

struct SDL_Window;
typedef union SDL_Event SDL_Event;

IMGUI_API bool        ImGui_ImplSdlGLES2_Init(SDL_Window* window);
IMGUI_API void        ImGui_ImplSdlGLES2_Shutdown();
IMGUI_API void        ImGui_ImplSdlGLES2_NewFrame(SDL_Window* window);
IMGUI_API void        ImGui_ImplSdlGLES2_RenderDrawData(ImDrawData* draw_data);
IMGUI_API bool        ImGui_ImplSdlGLES2_ProcessEvent(SDL_Event* event);

// Use if you want to reset your rendering device without losing ImGui state.
IMGUI_API void        ImGui_ImplSdlGLES2_InvalidateDeviceObjects();
IMGUI_API bool        ImGui_ImplSdlGLES2_CreateDeviceObjects();

#endif // IMGUI_IMPL_SDL_ES2
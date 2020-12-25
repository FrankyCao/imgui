#pragma once
#include <imgui.h>

ImTextureID Application_LoadTexture(const char* path);
ImTextureID Application_CreateTexture(const void* data, int width, int height);
void        Application_DestroyTexture(ImTextureID texture);
int         Application_GetTextureWidth(ImTextureID texture);
int         Application_GetTextureHeight(ImTextureID texture);

const char* Application_GetName(void* handle = nullptr);
void Application_Initialize(void** handle = nullptr);
void Application_Finalize(void** handle = nullptr);
void Application_Frame(void* handle = nullptr);

#pragma once
#include <imgui.h>

const char* Application_GetName(void* handle = nullptr);
void Application_Initialize(void** handle = nullptr);
void Application_Finalize(void** handle = nullptr);
void Application_Frame(void* handle = nullptr);

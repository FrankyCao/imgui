#pragma once
#include <imgui.h>

const char* Application_GetName(void* handle = nullptr);
void Application_Initialize(void** handle = nullptr);
void Application_Finalize(void** handle = nullptr);
bool Application_Frame(void* handle = nullptr);
void Application_lock();
bool Application_try_lock();
void Application_unlock();

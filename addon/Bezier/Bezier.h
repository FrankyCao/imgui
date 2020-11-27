#ifndef IMGUI_BEZIER_H
#define IMGUI_BEZIER_H
#include <imgui.h>

namespace ImGui {
    IMGUI_API int Bezier(const char *label, float P[5]);
    IMGUI_API float BezierValue(float dt01, float P[4]);
    IMGUI_API void ShowBezierDemo();
}

#endif /* IMGUI_BEZIER_H */
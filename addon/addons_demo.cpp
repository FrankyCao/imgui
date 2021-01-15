#include "addons_demo.h"

namespace ImGui
{
static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::BeginTooltip();
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void ShowAddonsDemoWindowWidgets()
{
    if (ImGui::TreeNode("Basic"))
    {
        static bool check = true;
        ImGui::Text("Toggle Button"); ImGui::SameLine();
        ImGui::ToggleButton("toggle", &check);
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Spin"))
    {
        static int int_v = 10;
        ImGui::SpinInt("##spin_int", &int_v, 1, 10);
        ImGui::SameLine(); HelpMarker("Hold key Ctrl to spin fast.");
        static float float_v = 10.0f;
        ImGui::SpinFloat("##spin_float", &float_v, 1.f, 10.f);
        ImGui::SameLine(); HelpMarker("Hold key Ctrl to spin fast.");
        static double double_v = 10.0;
        ImGui::SpinDouble("##spin_double", &double_v, 1., 10.);
        ImGui::SameLine(); HelpMarker("Hold key Ctrl to spin fast.");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Progress Indicators"))
    {
        ImGui::Text("Loading %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
        ImGui::Separator();

        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
        const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);

        ImGui::Spinner("##spinner", 15, 6, col);
        ImGui::SameLine(); HelpMarker("Spinner widget with radius/thickness/color.");
        ImGui::BufferingBar("##buffer_bar", 0.7f, ImVec2(400, 6), bg, col);
        ImGui::SameLine(); HelpMarker("BufferingBar widget with float value.");
        ImGui::Separator();

        const ImVec4 main_col = ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered);
        const ImVec4 bg_col = ImGui::GetStyleColorVec4(ImGuiCol_Button);
        ImGui::LoadingIndicatorCircle("##circle_indicator", 15, main_col, bg_col, 10, 5);
        ImGui::SameLine(); HelpMarker("Indicator Circle widget. Speed depend on circle_count");

        ImGui::ShowBezierDemo(); ImGui::SameLine(); HelpMarker("ImGui Bezier widget.");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("TimeLine Widgets"))
    {
        if (ImGui::BeginTimeline("MyTimeline",50.f,6,6))  // label, max_value, num_visible_rows, opt_exact_num_rows (for item culling)
        {
            static float events[12]={10.f,20.f,0.5f,30.f,40.f,50.f,20.f,40.f,15.f,22.5f,35.f,45.f};
            if (ImGui::TimelineEvent("Event1",&events[0])) {/*events[0] and/or events[1] modified*/}
            ImGui::TimelineEvent("Event2",&events[2]);
            ImGui::TimelineEvent("Event3",&events[4],true);    // Event3 can only be shifted
            ImGui::TimelineEvent("Event4",&events[6]);
            ImGui::TimelineEvent("Event5",&events[8]);
            ImGui::TimelineEvent("Event6",&events[10]);
        }
        const float elapsedTime = (float)(((unsigned)(ImGui::GetTime()*1000))%50000)/1000.f;    // So that it's always in [0,50]
        ImGui::EndTimeline(5,elapsedTime);  // num_vertical_grid_lines, current_time (optional), timeline_running_color (optional)
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Knob Widgets"))
    {
        static float freq = 0.5;
        static unsigned char intensity = 0;
        ImGui::Knob("Float.", &freq, 0.0f, 1.0f, ImVec2(40, 40), "float value knob");
        ImGui::SameLine();
        ImGui::KnobUchar("UCHar", &intensity, 0, 127, ImVec2(40, 40), "uchar value knob");
        int idb = freq * 80;
        ImGui::UvMeter("##uvr", ImVec2(10, 80), &idb, 0, 80); ImGui::ShowTooltipOnHover("Uv meters.");
        ImGui::SameLine();
        ImGui::Fader("##mastervol", ImVec2(20, 80), &idb, 0, 80, "%d", 1.0f); ImGui::ShowTooltipOnHover("Slide.");
        static bool instrument = true;
        static bool checked = true;
        ImGui::ToggleButton("Check", &checked, ImVec2(96, 20)); ImGui::ShowTooltipOnHover("Toggle Button.");
        ImGui::ToggleButtonWithCheckbox("Add", &instrument, &checked, ImVec2(96, 32)); ImGui::ShowTooltipOnHover("Toggle Button With Left Checkbox.");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Splitter windows"))
    {
        float h = 200;
        static float hsz1 = 300;
        static float hsz2 = 300;
        static float vsz1 = 100;
        static float vsz2 = 100;
        ImGui::Splitter(true, 8.0f, &hsz1, &hsz2, 8, 8, h);
        ImGui::BeginChild("1", ImVec2(hsz1, h), true);
            ImGui::Text("Window 1");
        ImGui::EndChild();
        ImGui::SameLine();

        ImGui::BeginChild("2", ImVec2(hsz2, h), true);
            ImGui::Splitter(false, 8.0f, &vsz1, &vsz2, 8, 8, hsz2);
            ImGui::BeginChild("3", ImVec2(hsz2, vsz1), false);
                ImGui::Text("Window 2");
            ImGui::EndChild();
            ImGui::BeginChild("4", ImVec2(hsz2, vsz2), false);
                ImGui::Text("Window 3");
            ImGui::EndChild();
        ImGui::EndChild();

        ImGui::TreePop();
    }
}
}
#include "ImGuiTabWindow.h"

namespace ImGui
{
void ShowAddonsTabWindow()
{
    ImGui::Spacing();
    ImGui::Text("TabLabels (based on the code by krys-spectralpixel):");
    static const char* tabNames[] = {"TabLabelStyle","Render","Layers","Scene","World","Object","Constraints","Modifiers","Data","Material","Texture","Particle"};
    static const int numTabs = sizeof(tabNames)/sizeof(tabNames[0]);
    static const char* tabTooltips[numTabs] = {"Edit the style of these labels","Render Tab Tooltip","This tab cannot be closed","Scene Tab Tooltip","","Object Tab Tooltip","","","","","Tired to add tooltips..."};
    static int tabItemOrdering[numTabs] = {0,1,2,3,4,5,6,7,8,9,10,11};
    static int selectedTab = 0;
    static int optionalHoveredTab = 0;
    static bool allowTabLabelDragAndDrop=true;static bool tabLabelWrapMode = true;static bool allowClosingTabs = false;
    int justClosedTabIndex=-1,justClosedTabIndexInsideTabItemOrdering = -1,oldSelectedTab = selectedTab;

    ImGui::Checkbox("Wrap Mode##TabLabelWrapMode",&tabLabelWrapMode);
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","WrapMode is only available\nin horizontal TabLabels");
    ImGui::SameLine();ImGui::Checkbox("Drag And Drop##TabLabelDragAndDrop",&allowTabLabelDragAndDrop);
    ImGui::SameLine();ImGui::Checkbox("Closable##TabLabelClosing",&allowClosingTabs);ImGui::SameLine();
    bool resetTabLabels = ImGui::SmallButton("Reset Tabs");if (resetTabLabels) {selectedTab=0;for (int i=0;i<numTabs;i++) tabItemOrdering[i] = i;}

    /*const bool tabSelectedChanged =*/ ImGui::TabLabels(numTabs,tabNames,selectedTab,tabTooltips,tabLabelWrapMode,false,&optionalHoveredTab,&tabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,&justClosedTabIndex,&justClosedTabIndexInsideTabItemOrdering);
    // Optional stuff
    if (justClosedTabIndex==1) {
        tabItemOrdering[justClosedTabIndexInsideTabItemOrdering] = justClosedTabIndex;   // Prevent the user from closing Tab "Layers"
        selectedTab = oldSelectedTab;   // This is safer, in case we had closed the selected tab
    }
    //if (optionalHoveredTab>=0) ImGui::Text("Mouse is hovering Tab Label: \"%s\".\n\n",tabNames[optionalHoveredTab]);

    // Draw tab page
    ImGui::BeginChild("MyTabLabelsChild",ImVec2(0,150),true);
    ImGui::Text("Tab Page For Tab: \"%s\" here.",selectedTab>=0?tabNames[selectedTab]:"None!");
    if (selectedTab==0) {
        static bool editTheme = false;
        ImGui::Spacing();
        ImGui::Checkbox("Edit tab label style",&editTheme);
        ImGui::Spacing();
        if (editTheme) ImGui::TabLabelStyle::Edit(ImGui::TabLabelStyle().Get());   // This is good if we want to edit the tab label style
        else {
            static int selectedIndex=0;
            ImGui::PushItemWidth(135);
            ImGui::SelectTabLabelStyleCombo("select tab label style",&selectedIndex); // Good for just selecting it
            ImGui::PopItemWidth();
        }
    }
    ImGui::EndChild();

    // ImGui::TabLabelsVertical() are similiar to ImGui::TabLabels(), but they do not support WrapMode.
    // ImGui::TabLabelsVertical() example usage
    static bool verticalTabLabelsAtLeft = true;ImGui::Checkbox("Vertical Tab Labels at the left side##VerticalTabLabelPosition",&verticalTabLabelsAtLeft);
    static const char* verticalTabNames[] = {"Layers","Scene","World"};
    static const int numVerticalTabs = sizeof(verticalTabNames)/sizeof(verticalTabNames[0]);
    static const char* verticalTabTooltips[numVerticalTabs] = {"Layers Tab Tooltip","Scene Tab Tooltip","World Tab Tooltip"};
    static int verticalTabItemOrdering[numVerticalTabs] = {0,1,2};
    static int selectedVerticalTab = 0;
    static int optionalHoveredVerticalTab = 0;
    if (resetTabLabels) {selectedVerticalTab=0;for (int i=0;i<numVerticalTabs;i++) verticalTabItemOrdering[i] = i;}

    const float verticalTabsWidth = ImGui::CalcVerticalTabLabelsWidth();
    if (verticalTabLabelsAtLeft)	{
        /*const bool verticalTabSelectedChanged =*/ ImGui::TabLabelsVertical(verticalTabLabelsAtLeft,numVerticalTabs,verticalTabNames,selectedVerticalTab,verticalTabTooltips,false,&optionalHoveredVerticalTab,&verticalTabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,NULL,NULL);
        //if (optionalHoveredVerticalTab>=0) ImGui::Text("Mouse is hovering Tab Label: \"%s\".\n\n",verticalTabNames[optionalHoveredVerticalTab]);
        ImGui::SameLine(0,0);
    }
    // Draw tab page
    ImGui::BeginChild("MyVerticalTabLabelsChild",ImVec2(ImGui::GetWindowWidth()-verticalTabsWidth-2.f*ImGui::GetStyle().WindowPadding.x-ImGui::GetStyle().ScrollbarSize,150),true);
    ImGui::Text("Tab Page For Tab: \"%s\" here.",selectedVerticalTab>=0?verticalTabNames[selectedVerticalTab]:"None!");
    ImGui::EndChild();
    if (!verticalTabLabelsAtLeft)	{
        ImGui::SameLine(0,0);
        /*const bool verticalTabSelectedChanged =*/ ImGui::TabLabelsVertical(verticalTabLabelsAtLeft,numVerticalTabs,verticalTabNames,selectedVerticalTab,verticalTabTooltips,false,&optionalHoveredVerticalTab,&verticalTabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,NULL,NULL);
        //if (optionalHoveredVerticalTab>=0) ImGui::Text("Mouse is hovering Tab Label: \"%s\".\n\n",verticalTabNames[optionalHoveredVerticalTab]);
    }
}
} // namespace ImGui
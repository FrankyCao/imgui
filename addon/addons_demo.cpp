#include "addons_demo.h"
#include "NumberTexture.h"
#include "ImGuiHelper.h"
#include "ImGuiVariousControls.h"
#include "ImGuiDateChooser.h"
#include "ProgressIndicators.h"
#include "Timeline.h"
#include "imgui_knob.h"
#include "Splitter.h"
#include "imgui_dock.h"
#include "ImGuiTabWindow.h"
#if IMGUI_VULKAN_SHADER
#include "ImVulkanShader.h"
#endif
#if defined(_WIN32) || defined(__EMSCRIPTEN__)
#include <time.h>
#endif

ImTextureID ImageTextureNumber = 0;

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
    if (ImageTextureNumber == 0)
    {
        ImageTextureNumber = ImCreateTexture(NumberTexture_pixels, NumberTexture_width, NumberTexture_height);
    }
    if (ImGui::TreeNode("Basic"))
    {
        static bool check = true;
        ImGui::Text("Toggle Button"); ImGui::SameLine();
        ImGui::ToggleButton("toggle", &check);
        ImGui::Separator();

        // Check Buttons
        ImGui::Text("Check Buttons:");ImGui::SameLine();
        if (ImGui::CheckButton("CheckButton",&check)) {/*checkButtonState1 changed*/}
        ImGui::SameLine();
        if (ImGui::SmallCheckButton("SmallCheckButton",&check)) {/*checkButtonState2 changed*/}
        ImGui::SameLine();
        ImGui::ToggleButton("StatusButton", &check, ImVec2(96, 20)); ImGui::ShowTooltipOnHover("Toggle Button.");
        ImGui::Separator();
        static bool checkStyled[2] = {false,true};
        ImGui::CheckboxStyled("Checkbox Styled 1 (default style)",&checkStyled[0]);
        // Custom look and size
        static const ImU32 optionalEightColors[8] = {
            IM_COL32(220,220,220,255),IM_COL32(255,255,255,255),    // on_circle (normal and hovered)
            IM_COL32(80,80,80,255),IM_COL32(100,100,100,255),       // off_circle (normal and hovered)
            IM_COL32(60,120,60,255),IM_COL32(80,155,80,255),        // on_bg (normal and hovered)
            IM_COL32(60,60,60,255),IM_COL32(80,80,80,255)           // off_bg (normal and hovered)
        };
        const ImVec2 checkBoxSizeScale(1.5f,2.f);   // (checkBoxSizeScale.y max is 2.f)
        const float checkBoxRounding = 6.f;         // -1 defaults to style.WindowRounding. Note that is scales with the font height)
        ImGui::CheckboxStyled("Checkbox Styled 2 (custom style)",&checkStyled[1],optionalEightColors,checkBoxSizeScale,checkBoxRounding);
        ImGui::Separator();
        // PopupMenuSimple
        static const char* recentFileList[] = {"filename01","filename02","filename03","filename04","filename05","filename06","filename07","filename08","filename09","filename10"};
        static ImGui::PopupMenuSimpleParams pmsParams;
        ImGui::Text("PopupMenuSimple:");ImGui::SameLine();
        ImGui::Button("Right-click me##PopupMenuSimpleTest");
        pmsParams.open|= ImGui::GetIO().MouseClicked[1] && ImGui::IsItemHovered();
        const int selectedEntry = ImGui::PopupMenuSimple(pmsParams,recentFileList,(int) sizeof(recentFileList)/sizeof(recentFileList[0]),5,true,"RECENT FILES", ICON_FA5_ARROW_UP, ICON_FA5_ARROW_DOWN);
        static int lastSelectedEntry = -1;
        if (selectedEntry>=0) {
            // Do something: clicked recentFileList[selectedEntry].
            // Good in most cases, but here we want to persist the last choice because this branch happens only one frame:
            lastSelectedEntry = selectedEntry;
        }
        if (lastSelectedEntry>=0) {ImGui::SameLine();ImGui::Text("Last selected: %s\n",recentFileList[lastSelectedEntry]);}

        // ColorComboTest
        ImGui::Separator();
        static ImVec4 chosenColor(1,1,1,1);
        if (ImGui::ColorCombo("ColorCombo",&chosenColor))
        {
            // choice OK here
        }

        ImGui::Separator();
        // Single column popup menu with icon support. It disappears when the mouse goes away. Never tested.
        // User is supposed to create a static instance of it, add entries once, and then call "render()".
        static ImGui::PopupMenu pm;
        if (pm.isEmpty())   {
            pm.addEntryTitle("Single Menu With Images");
            char tmp[1024];ImVec2 uv0(0,0),uv1(0,0);
            for (int i=0;i<9;i++) {
                strcpy(tmp,"Image Menu Entry ");
                sprintf(&tmp[strlen(tmp)],"%d",i+1);
                uv0 = ImVec2((float)(i%3)/3.f,(float)(i/3)/3.f);
                uv1 = ImVec2(uv0.x+1.f/3.f,uv0.y+1.f/3.f);

                pm.addEntry(tmp,reinterpret_cast<void*>(ImageTextureNumber),uv0,uv1);
            }
        }

        static bool trigger = false;
        trigger|=ImGui::Button("Press me for a menu with images##PopupMenuWithImagesTest");
        //const int selectedImageMenuEntry =
        pm.render(trigger);   // -1 = none
        // Buttons With Images
        ImGui::Spacing();ImGui::Separator();ImGui::Text("Buttons With Images:");ImGui::Separator();
        ImGui::ImageButtonWithText(ImageTextureNumber,"MyImageButtonWithText",ImVec2(16,16),ImVec2(0,0),ImVec2(0.33334f,0.33334f));

        // AnimatedImage
        ImGui::Separator();
        ImGui::Text("AnimatedImage:");
        static ImGui::AnimatedImage gif(ImageTextureNumber,64,64,9,3,3,30,true);
        gif.render();
        ImGui::SameLine();
        gif.renderAsButton("myButton123",ImVec2(-.5f,-.5f));    // Negative size multiplies the 'native' gif size

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("AutoCompletion Stuff (UP/DOWN/TAB keys)"))
    {
        // Bad implementation: users think they can click on the autocompletion-menu, instead of using TAB+ARROWS

        static const int bufferSize = 84;   // Mandatory [ImGui::InputText(...) needs it]
        // Customizable section (we will use ImVector<char[bufferSize]> here to store our autocompletion entries; it can be easily changed to std::vector<std::string> or similiar):
        typedef struct _TMP {
            // MANDATORY! The same signature as ImGui::Combo(...)
            static bool ItemGetter(void* data,int i,const char** txt)   {
                IM_ASSERT(data);
                const ImVector<char[bufferSize]>& v = *((const ImVector<char[bufferSize]>*)data);
                if (i>=0 && i<v.size()) {*txt=v[i];return true;}
                return false;
            }
            // This is optional in InputTextWithAutoCompletion(...) (all at the user side), but mandatory in InputComboWithAutoCompletion(...)
            static bool ItemInserter(void* data,int pos,const char* txt)  {
                IM_ASSERT(data && txt && strlen(txt)<bufferSize);
                ImVector<char[bufferSize]>& v = *((ImVector<char[bufferSize]>*)data);
                const int size = v.size();
                if (pos<0) pos=0;else if (pos>size) pos=size;

                v.resize(size+1);
                for (int i=size;i>pos;--i) strcpy(v[i],v[i-1]);
                strcpy(v[pos],txt);

                return true;
            }
            // This is optional in InputComboWithAutoCompletion(...) (not needed by InputTextWithAutoCompletion(...))
            static bool ItemDeleter(void* data,int pos)  {
                IM_ASSERT(data);
                ImVector<char[bufferSize]>& v = *((ImVector<char[bufferSize]>*)data);
                const int size = v.size();
                if (pos<0 || pos>size) return false;

                for (int i=pos,iSz=size-1;i<iSz;++i) strcpy(v[i],v[i+1]);
                v.resize(size-1);

                return true;
            }
            // This is optional in InputComboWithAutoCompletion(...) (not needed by InputTextWithAutoCompletion(...))
            // This method can be built out of ItemDeleter(...) and ItemInserter(...) [but it's still good to notify user that a rename has occurred]
            static bool ItemRenamer(void* data,int posOld,int posNew,const char* txtNew)  {
                // We must move item at posOld to position posNew, and change its text to txtNew
                IM_ASSERT(data && txtNew && strlen(txtNew)<bufferSize);
                ImVector<char[bufferSize]>& v = *((ImVector<char[bufferSize]>*)data);
                const int size = v.size();
                if (posOld<0 || posOld>size) return false;
                if (posNew<0 || posNew>size) return false;
                // worst implementation possible here:
                bool ok = ItemDeleter(data,posOld);
                if (ok) ok = ItemInserter(data,posNew,txtNew);
                return ok;
            }
        } TMP;

        ImGui::PushItemWidth(ImGui::GetWindowWidth()*0.275f);
        // InputTextWithAutoCompletion:
        {
            // Mandatory stuff
            static char buf[bufferSize];
            static ImGui::InputTextWithAutoCompletionData bufData(ImGuiInputTextFlags_CharsUppercase);  // Only ImGuiInputTextFlags_Chars... flags are allowed here
            static ImVector<char[bufferSize]> autocompletionEntries;    // The type here depends on our TMP struct
            // [Optional] Bad init, but very comfortable to start with something
            if (!bufData.isInited()) {
                const char* entries[] = {"WATERMELON","STRAWBERRY","APRICOT","APPLE","KIWI","CHERRY","PAPAYA",
                                         "LEMON","PEACH","PEAR","PINEAPPLE"};
                const int numEntries = (int) sizeof(entries)/sizeof(entries[0]);
                for (int i=0;i<numEntries;i++)  {
                    // This helper call should ensure sorting + duplicate elimination
                    ImGui::InputTextWithAutoCompletionData::HelperInsertItem(entries[i],TMP::ItemGetter,TMP::ItemInserter,autocompletionEntries.size(),&autocompletionEntries);
                }
                /* Otherwise we could have:
                // Important: entries must be sorted alphabetically
                TMP::ItemInserter(&autocompletionEntries,0,"APPLE");
                TMP::ItemInserter(&autocompletionEntries,1,"APRICOT");
                TMP::ItemInserter(&autocompletionEntries,2,"CHERRY");
                // ... and so on
                */
                // [Optional] user can handle bufData.currentAutocompletionItemIndex
                // bufData.currentAutocompletionItemIndex is owned by the user (for ImGui::InputTextWithAutoCompletion(...) only).
                // When !=-1, the specified item is displayed in a different way in the autocompletion menu.
                // bufData.currentAutocompletionItemIndex = 2;
            }

            if (ImGui::InputTextWithAutoCompletion("Fruits##AutoCompleteIT",buf,bufferSize,&bufData,TMP::ItemGetter,autocompletionEntries.size(),(void*)&autocompletionEntries) && buf[0]!='\0')    {
                // Return has been pressed and buf is valid
                if (bufData.getItemPositionOfReturnedText()>=0)  {
                    // The entered text must be inserted at that position
                    TMP::ItemInserter(&autocompletionEntries,bufData.getItemPositionOfReturnedText(),buf);
                    // bufData.currentAutocompletionItemIndex = bufData.getItemPositionOfReturnedText();
                }
                // else if (bufData.getItemIndexOfReturnedText()>=0)    {
                // User has entered an existing autocomplete item that can be retrieved at position: bufData.getItemIndexOfReturnedText()
                // bufData.currentAutocompletionItemIndex = getItemIndexOfReturnedText();
                //}
                buf[0]='\0';  // clear
                ImGui::SetKeyboardFocusHere(-1);    // So we keep typing
            }
        }
        ImGui::SameLine(0,ImGui::GetWindowWidth()*0.05f);
        // InputComboWithAutoCompletion:
        {
            // Mandatory stuff
            static int current_item=-1;
            static ImGui::InputComboWithAutoCompletionData bufData;
            static ImVector<char[bufferSize]> autocompletionEntries;    // The type here depends on our TMP struct
            // [Optional] Bad init, but very comfortable to start with something
            if (!bufData.isInited()) {
                const char* entries[] = {"black","blue","green","ivory","pink","red","white","yellow"};
                const int numEntries = (int) sizeof(entries)/sizeof(entries[0]);
                for (int i=0;i<numEntries;i++)  {
                    // This helper call should ensure sorting + duplicate elimination
                    ImGui::InputTextWithAutoCompletionData::HelperInsertItem(entries[i],TMP::ItemGetter,TMP::ItemInserter,autocompletionEntries.size(),&autocompletionEntries);
                }
                // We CAN'T handle bufData.currentAutocompletionItemIndex for Combos, because we have:
                current_item = 2;
            }

            if (ImGui::InputComboWithAutoCompletion("Colors##AutoCompleteIT",&current_item,bufferSize,&bufData,
                TMP::ItemGetter,TMP::ItemInserter,TMP::ItemDeleter,TMP::ItemRenamer,   // TMP::ItemDeleter and TMP::ItemRenemer can be NULL
                autocompletionEntries.size(),(void*)&autocompletionEntries))    {
                // something has changes (the Combo selected item, and/or an insert/delete operation
            }
            // For this particular widget ImGui::IsItemHovered() does not always work as expected. Workaround:
            //if (bufData.isItemHovered()) ImGui::SetTooltip("%s","InputComboWithAutoCompletion tooltip");
        }
        ImGui::PopItemWidth();
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Choose a date"))
    {
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Choose a date:");
        ImGui::SameLine();
        static tm myDate={}; 
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth()*0.2f);
        if (ImGui::DateChooser("Date Chooser ##MyDateChooser",myDate,"%d/%m/%Y")) {
            // A new date has been chosen
            //fprintf(stderr,"A new date has been chosen exacty now: \"%.2d-%.2d-%.4d\"\n",myDate.tm_mday,myDate.tm_mon+1,myDate.tm_year+1900);
        }
        ImGui::Text("Chosen date: \"%.2d-%.2d-%.4d\"",myDate.tm_mday,myDate.tm_mon+1,myDate.tm_year+1900);

        ImGui::Spacing();
        ImGui::AlignTextToFramePadding();
        ImGui::Text("Choose another date:");
        ImGui::SameLine();
        static tm myDate2={};       // IMPORTANT: must be static! (plenty of compiler warnings here if we write: static tm myDate={0}; Is there any difference?)
        ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth()*0.2f);
        if (ImGui::DateChooser("##MyDateChooser2",myDate2,"%d/%m/%Y")) {
            // A new date has been chosen
            //fprintf(stderr,"A new date has been chosen exacty now: \"%.2d-%.2d-%.4d\"\n",myDate2.tm_mday,myDate2.tm_mon+1,myDate2.tm_year+1900);
        }
        ImGui::Text("Chosen date2: \"%.2d-%.2d-%.4d\"",myDate2.tm_mday,myDate2.tm_mon+1,myDate2.tm_year+1900);

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Passwd"))
    {
        ImGui::Text("Password Drawer Widget:");
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Basically it's a widget you can\ndraw a password in, by mouse dragging.");
        ImGui::Separator();

        static char password[37] = "";
        static int gridComboSelection = 1;  // We start with option 1 -> 3x3 grid
        static int passwordSize = (2+gridComboSelection)*(2+gridComboSelection)+1;  // +1 -> trailing '\0'
        ImGui::PushItemWidth(ImGui::GetWindowWidth()*0.1f);
        static const char* gridComboItemNames[5]= {"2","3","4","5","6"};
        if (ImGui::Combo("Grid Size##MobileLockGridSize",&gridComboSelection,gridComboItemNames,5))   {
            passwordSize = 2+gridComboSelection;
            passwordSize*=passwordSize;
            passwordSize+=1;  // +1 -> trailing '\0'
            password[0]='\0';   // reset password
        }
        ImGui::PopItemWidth();
        static char passwordDisplayedBelow[37] = "";
        if (ImGui::PasswordDrawer(password, passwordSize, 0, 200))   {
            strcpy(passwordDisplayedBelow, password);
            password[0]='\0';   // reset password
        }
        if (strlen(passwordDisplayedBelow)>0) 
            ImGui::Text("Password: %s",passwordDisplayedBelow);
        else
            ImGui::Text("Password: ******");
        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Progress&Indicators"))
    {
        // Spin Test:
        static int int_v = 10;
        ImGui::SpinInt("##spin_int", &int_v, 1, 10);
        ImGui::SameLine(); HelpMarker("Hold key Ctrl to spin fast.");
        static float float_v = 10.0f;
        ImGui::SpinFloat("##spin_float", &float_v, 1.f, 10.f);
        ImGui::SameLine(); HelpMarker("Hold key Ctrl to spin fast.");
        static double double_v = 10.0;
        ImGui::SpinDouble("##spin_double", &double_v, 1., 10.);
        ImGui::SameLine(); HelpMarker("Hold key Ctrl to spin fast.");
        ImGui::Separator();

        // ProgressBar Test:
        ImGui::TestProgressBar();
        ImGui::Separator();

        ImGui::Text("Loading %c", "|/-\\"[(int)(ImGui::GetTime() / 0.05f) & 3]);
        ImGui::Separator();

        const ImU32 col = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
        const ImU32 bg = ImGui::GetColorU32(ImGuiCol_Button);

        ImGui::Spinner("##spinner", 15, 6, col);
        ImGui::SameLine(); HelpMarker("Spinner widget with radius/thickness/color.");
        ImGui::BufferingBar("##buffer_bar", 0.7f, ImVec2(400, 6), bg, col);
        ImGui::SameLine(); HelpMarker("BufferingBar widget with float value.");
        ImGui::Separator();

        // LoadingIndicatorCircle
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("LoadingIndicatorCircle(...) from https://github.com/ocornut/imgui/issues/1901");
        ImGui::Separator();
        ImGui::TextUnformatted("Test 1:");ImGui::SameLine();
        ImGui::LoadingIndicatorCircle("MyLIC1");ImGui::SameLine();
        ImGui::TextUnformatted("Test 2:");ImGui::SameLine();
        ImGui::LoadingIndicatorCircle("MyLIC2",1.f,&ImGui::GetStyle().Colors[ImGuiCol_Header],&ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered]);
        ImGui::AlignTextToFramePadding();ImGui::TextUnformatted("Test 3:");ImGui::SameLine();ImGui::LoadingIndicatorCircle("MyLIC3",2.0f);
        ImGui::AlignTextToFramePadding();ImGui::TextUnformatted("Test 4:");ImGui::SameLine();ImGui::LoadingIndicatorCircle("MyLIC4",4.0f,&ImGui::GetStyle().Colors[ImGuiCol_Header],&ImGui::GetStyle().Colors[ImGuiCol_HeaderHovered],12,2.f);
        ImGui::Separator();

        // LoadingIndicatorCircle2
        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("LoadingIndicatorCircle2(...) from https://github.com/ocornut/imgui/issues/1901");
        ImGui::Separator();
        ImGui::TextUnformatted("Test 1:");ImGui::SameLine();
        ImGui::LoadingIndicatorCircle2("MyLIC21");ImGui::SameLine();
        ImGui::TextUnformatted("Test 2:");ImGui::SameLine();
        ImGui::LoadingIndicatorCircle2("MyLIC22",1.f,1.5f,&ImGui::GetStyle().Colors[ImGuiCol_Header]);
        ImGui::AlignTextToFramePadding();ImGui::TextUnformatted("Test 3:");ImGui::SameLine();ImGui::LoadingIndicatorCircle2("MyLIC23",2.0f);
        ImGui::AlignTextToFramePadding();ImGui::TextUnformatted("Test 4:");ImGui::SameLine();ImGui::LoadingIndicatorCircle2("MyLIC24",4.0f,1.f,&ImGui::GetStyle().Colors[ImGuiCol_Header]);
        ImGui::Separator();

        ImGui::ShowBezierDemo(); ImGui::SameLine(); HelpMarker("ImGui Bezier widget.");
        ImGui::Separator();

        static ImVec2 val2d(0.f, 0.f);
        static ImVec4 val3d(0.f, 0.f, 0.f, 0.f);
        ImGui::BeginChild("##InputVec2", ImVec2(240, 340), true, ImGuiWindowFlags_NoMove);
        ImGui::InputVec2("Vec2D", &val2d, ImVec2(-1.f, -1.f), ImVec2(1.f, 1.f));
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##SliderScalar2D", ImVec2(240, 340), true, ImGuiWindowFlags_NoMove);
        ImGui::SliderScalar2D("Scalar2D ", &val2d.x, &val2d.y, -1.f, 1.f, -1.f, 1.f);
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##InputVec3D", ImVec2(240, 340), true, ImGuiWindowFlags_NoMove);
        ImGui::InputVec3("Vec3D", &val3d, ImVec4(-1.f, -1.f, -1.f, -1.f), ImVec4(1.f, 1.f, 1.f, 1.f));
        ImGui::EndChild();
        ImGui::SameLine();
        ImGui::BeginChild("##InputScalar3D", ImVec2(240, 340), true, ImGuiWindowFlags_NoMove);
        ImGui::SliderScalar3D("Scalar3D", &val3d.x, &val3d.y, &val3d.z, -1.f, 1.f, -1.f, 1.f, -1.f, 1.f);
        ImGui::EndChild();
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
        //const float elapsedTime = (float)(((unsigned)(ImGui::GetTime()*1000))%50000)/1000.f;    // So that it's always in [0,50]
        static float timeline_elapsed_time = 0.f;
        ImGui::EndTimeline(5,timeline_elapsed_time);  // num_vertical_grid_lines, current_time (optional), timeline_running_color (optional)
        
        // COMPLETELY Optional (And Manual): Timeline Start/Pause Buttons:-------
        static ImU32 timeline_state = 0;    // Manually handled by us (0 = stopped, 1 = playing, 2 = paused)
        static float timeline_begin_time = ImGui::GetTime();
        if (timeline_state==1) {
            // It's playing
            timeline_elapsed_time = ImGui::GetTime()-timeline_begin_time;
            if (timeline_elapsed_time>50.f) {
                // We reset the timer after 50.f seconds here
                timeline_state=0;   // Stopped
                timeline_elapsed_time=0.f;
            }
        }
        ImGui::SetCursorPosY(ImGui::GetCursorPosY()-ImGui::GetTextLineHeightWithSpacing()); // Go up one line
        if (ImGui::Button(timeline_state==1 ?  ICON_FA5_PAUSE"###timeline_play" : ICON_FA5_PLAY"###timeline_play")) {
            if (timeline_state==1) timeline_state=2;	// Paused
            else if (timeline_state==0)  {
                // Was stopped
                timeline_state=1;   // Now playing
                timeline_begin_time = ImGui::GetTime();	// From zero time
            }
            else {
                // Was paused
                timeline_state=1;   // Now playing
                const float pausedTime = ImGui::GetTime() - timeline_begin_time - timeline_elapsed_time;
                timeline_begin_time+= pausedTime;	// From last time
            }
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s",timeline_state==1 ? "Pause" : "Play");
        if (timeline_state!=0)	{
            ImGui::SameLine(0,0);
            if (ImGui::Button(ICON_FA5_STOP"###timeline_stop")) {
                timeline_state=0;   // Stopped
                timeline_elapsed_time=0.f;  // We reset the timer
            }
            if (ImGui::IsItemHovered()) ImGui::SetTooltip("%s","Stop");
        }
        // End COMPLETELY Optional (And Manual) Stuff-----------------------------

        ImGui::TreePop();
    }

    if (ImGui::TreeNode("Style Knob Widgets"))
    {
        static float val = 0.5, val_default = 0.5;
        float t = (float)ImGui::GetTime();
        float h = abs(sin(t * 0.2));
        float s = abs(sin(t * 0.1)) * 0.5 + 0.4;
        ImVec4 base_color = ImVec4(0.f, 0.f, 0.f, 1.f), active_color = ImVec4(0.f, 0.f, 0.f, 1.f), hovered_color = ImVec4(0.f, 0.f, 0.f, 1.f);
        ImGui::ColorConvertHSVtoRGB(h, s, 0.5f, base_color.x, base_color.y, base_color.z);
        ImGui::ColorConvertHSVtoRGB(h, s, 0.6f, active_color.x, active_color.y, active_color.z);
        ImGui::ColorConvertHSVtoRGB(h, s, 0.7f, hovered_color.x, hovered_color.y, hovered_color.z);
        ImVec4 highlight_base_color = ImVec4(0.f, 0.f, 0.f, 1.f), highlight_active_color = ImVec4(0.f, 0.f, 0.f, 1.f), highlight_hovered_color = ImVec4(0.f, 0.f, 0.f, 1.f);
        ImGui::ColorConvertHSVtoRGB(h, s, 0.75f, highlight_base_color.x, highlight_base_color.y, highlight_base_color.z);
        ImGui::ColorConvertHSVtoRGB(h, s, 0.95f, highlight_active_color.x, highlight_active_color.y, highlight_active_color.z);
        ImGui::ColorConvertHSVtoRGB(h, s, 1.0f, highlight_hovered_color.x, highlight_hovered_color.y, highlight_hovered_color.z);
        ImVec4 lowlight_base_color = ImVec4(0.f, 0.f, 0.f, 1.f), lowlight_active_color = ImVec4(0.f, 0.f, 0.f, 1.f), lowlight_hovered_color = ImVec4(0.f, 0.f, 0.f, 1.f);
        ImGui::ColorConvertHSVtoRGB(h, s, 0.2f, lowlight_base_color.x, lowlight_base_color.y, lowlight_base_color.z);
        ImGui::ColorConvertHSVtoRGB(h, s, 0.3f, lowlight_active_color.x, lowlight_active_color.y, lowlight_active_color.z);
        ImGui::ColorConvertHSVtoRGB(h, s, 0.4f, lowlight_hovered_color.x, lowlight_hovered_color.y, lowlight_hovered_color.z);
        ImVec4 tick_base_color = ImVec4(0.8f, 0.8f, 0.8f, 1.f), tick_active_color = ImVec4(1.f, 1.f, 1.f, 1.f), tick_hovered_color = ImVec4(1.f, 1.f, 1.f, 1.f);
        ColorSet circle_color = {base_color, active_color, hovered_color};
        ColorSet wiper_color = {highlight_base_color, highlight_active_color, highlight_hovered_color};
        ColorSet track_color = {lowlight_base_color, lowlight_active_color, lowlight_hovered_color};
        ColorSet tick_color = {tick_base_color, tick_active_color, tick_hovered_color};

        float knob_size = 80.f;
        ImGui::Knob("##Tick", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_TICK, "%.03fdB");
        ImGui::SameLine();
        ImGui::Knob("TickDot", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_TICK_DOT, "%.03fdB");
        ImGui::SameLine();
        ImGui::Knob("TickWiper", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_TICK_WIPER, "%.03fdB");
        ImGui::SameLine();
        ImGui::Knob("Wiper", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_WIPER, "%.03fdB");
        ImGui::SameLine();
        ImGui::Knob("WiperTick", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_WIPER_TICK, "%.03fdB");
        ImGui::SameLine();
        ImGui::Knob("WiperDot", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_WIPER_DOT, "%.03fdB");
        ImGui::SameLine();
        ImGui::Knob("WiperOnly", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_WIPER_ONLY, "%.03fdB");
        ImGui::SameLine();
        ImGui::Knob("SteppedTick", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_STEPPED_TICK, "%.03fdB", 10);
        ImGui::SameLine();
        ImGui::Knob("SteppedDot", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_STEPPED_DOT, "%.03fdB", 10);
        ImGui::SameLine();
        ImGui::Knob("Space", &val, 0.0f, 1.0f, val_default, knob_size, circle_color,  wiper_color, track_color, tick_color, ImGui::ImGuiKnobType::IMKNOB_SPACE, "%.03fdB");

        int idb = val * 80;
        ImGui::Fader("##mastervol", ImVec2(20, 80), &idb, 0, 80, "%d", 1.0f); ImGui::ShowTooltipOnHover("Slide.");
        ImGui::SameLine();
        static int stack = 0;
        static int count = 0;
        ImGui::UvMeter("##vuvr", ImVec2(10, 80), &idb, 0, 80, 20); ImGui::ShowTooltipOnHover("Vertical Uv meters.");
        ImGui::UvMeter("##huvr", ImVec2(80, 10), &idb, 0, 80, 20, &stack, &count); ImGui::ShowTooltipOnHover("Horizon Uv meters width stack effect.");

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

void ShowAddonsDuckWindow()
{
    static ImGui::DockContext* gDockContent = ImGui::CreateDockContext();
    ImGui::SetCurrentDockContext(gDockContent);
    ImGui::BeginDockspace();
    static char tmp[128];
    for (int i=0;i<10;i++)  {
        sprintf(tmp,"Dock %d",i);
        if (i==9) ImGui::SetNextDock(ImGuiDockSlot_Bottom);// optional
        if(ImGui::BeginDock(tmp))  {
            ImGui::Text("Content of dock window %d goes here",i);
        }
        ImGui::EndDock();
    }
    //=========== OPTIONAL STUFF ===================================================
    static bool draggingLookOpen = true;    // With this next dock has a close button (but its state is not serializable AFAIK)
    // We're also passing a 'default_size' as initial size of the window once undocked
    if (ImGui::BeginDock("Dragging Look",&draggingLookOpen,0,ImVec2(200,350)))    {
        ImGui::Checkbox("Textured##imguidockDraggingLook",&gImGuiDockReuseTabWindowTextureIfAvailable);
    }
    ImGui::EndDock();
    //===========END OPTIONAL STUFF =================================================
    //========== OPTIONAL STUFF =====================================================
#   if (!defined(NO_IMGUIHELPER) && !defined(NO_IMGUIHELPER_SERIALIZATION))
    if (ImGui::BeginDock("Load/Save"))  {
        static const char* saveName = "myDock.layout";
        const char* saveNamePersistent = "/persistent_folder/myDock.layout";
        const char* pSaveName = saveName;
#       ifndef NO_IMGUIHELPER_SERIALIZATION_SAVE
        if (ImGui::Button("Save")) {
            if (ImGui::SaveDock(pSaveName))   {
            }
        }
        ImGui::SameLine();
#       endif //NO_IMGUIHELPER_SERIALIZATION_SAVE
#       ifndef NO_IMGUIHELPER_SERIALIZATION_LOAD
        if (ImGui::Button("Load")) {
            ImGui::LoadDock(pSaveName);
        }
        ImGui::SameLine();
#       endif //NO_IMGUIHELPER_SERIALIZATION_LOAD
    }
    ImGui::EndDock();   //Load/Save
#   endif //NO_IMGUIHELPER_SERIALIZATION
    //=========== END OPTIONAL STUFF ================================================
    ImGui::EndDockspace();
}

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

    /*const bool tabSelectedChanged =*/ ImGui::TabLabels(numTabs,tabNames,selectedTab,tabTooltips,tabLabelWrapMode,&optionalHoveredTab,&tabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,&justClosedTabIndex,&justClosedTabIndexInsideTabItemOrdering);
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
        /*const bool verticalTabSelectedChanged =*/ ImGui::TabLabelsVertical(verticalTabLabelsAtLeft,numVerticalTabs,verticalTabNames,selectedVerticalTab,verticalTabTooltips,&optionalHoveredVerticalTab,&verticalTabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,NULL,NULL);
        //if (optionalHoveredVerticalTab>=0) ImGui::Text("Mouse is hovering Tab Label: \"%s\".\n\n",verticalTabNames[optionalHoveredVerticalTab]);
        ImGui::SameLine(0,0);
    }
    // Draw tab page
    ImGui::BeginChild("MyVerticalTabLabelsChild",ImVec2(ImGui::GetWindowWidth()-verticalTabsWidth-2.f*ImGui::GetStyle().WindowPadding.x-ImGui::GetStyle().ScrollbarSize,150),true);
    ImGui::Text("Tab Page For Tab: \"%s\" here.",selectedVerticalTab>=0?verticalTabNames[selectedVerticalTab]:"None!");
    ImGui::EndChild();
    if (!verticalTabLabelsAtLeft)	{
        ImGui::SameLine(0,0);
        /*const bool verticalTabSelectedChanged =*/ ImGui::TabLabelsVertical(verticalTabLabelsAtLeft,numVerticalTabs,verticalTabNames,selectedVerticalTab,verticalTabTooltips,&optionalHoveredVerticalTab,&verticalTabItemOrdering[0],allowTabLabelDragAndDrop,allowClosingTabs,NULL,NULL);
        //if (optionalHoveredVerticalTab>=0) ImGui::Text("Mouse is hovering Tab Label: \"%s\".\n\n",verticalTabNames[optionalHoveredVerticalTab]);
    }
}
void CleanupDemo()
{
    if (ImageTextureNumber) { ImDestroyTexture(ImageTextureNumber); ImageTextureNumber = 0; }
}

#if IMGUI_VULKAN_SHADER
int g_gpu_count = ImVulkan::get_gpu_count();

static const char glsl_p1_data[] = R"(
#version 450
#if ImVulkan_fp16_storage
#extension GL_EXT_shader_16bit_storage: require
#endif
#if ImVulkan_fp16_arithmetic
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require
#endif
layout (constant_id = 0) const int count = 0;
layout (constant_id = 1) const int loop = 1;
layout (binding = 0) readonly buffer a_blob { sfp a_blob_data[]; };
layout (binding = 1) readonly buffer b_blob { sfp b_blob_data[]; };
layout (binding = 2) writeonly buffer c_blob { sfp c_blob_data[]; };
void main()
{
    int gx = int(gl_GlobalInvocationID.x);
    int gy = int(gl_GlobalInvocationID.y);
    int gz = int(gl_GlobalInvocationID.z);
    if (gx >= count || gy >= 1 || gz >= 1)
        return;
    afp a = buffer_ld1(a_blob_data, gx);
    afp b = buffer_ld1(b_blob_data, gx);
    afp c = afp(1.f);
    for (int i = 0; i < loop; i++)
    {
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
    }
    buffer_st1(c_blob_data, gx, c);
}
)";

static const char glsl_p4_data[] = R"(
#version 450
#if ImVulkan_fp16_storage
#extension GL_EXT_shader_16bit_storage: require
#endif
#if ImVulkan_fp16_arithmetic
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require
#endif
layout (constant_id = 0) const int count = 0;
layout (constant_id = 1) const int loop = 1;
layout (binding = 0) readonly buffer a_blob { sfpvec4 a_blob_data[]; };
layout (binding = 1) readonly buffer b_blob { sfpvec4 b_blob_data[]; };
layout (binding = 2) writeonly buffer c_blob { sfpvec4 c_blob_data[]; };
void main()
{
    int gx = int(gl_GlobalInvocationID.x);
    int gy = int(gl_GlobalInvocationID.y);
    int gz = int(gl_GlobalInvocationID.z);
    if (gx >= count || gy >= 1 || gz >= 1)
        return;
    afpvec4 a = buffer_ld4(a_blob_data, gx);
    afpvec4 b = buffer_ld4(b_blob_data, gx);
    afpvec4 c = afpvec4(1.f);
    for (int i = 0; i < loop; i++)
    {
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
        c = a * c + b;
    }
    buffer_st4(c_blob_data, gx, c);
}
)";

static const char glsl_p8_data[] = R"(
#version 450
#if ImVulkan_fp16_storage
#extension GL_EXT_shader_16bit_storage: require
#endif
#if ImVulkan_fp16_arithmetic
#extension GL_EXT_shader_explicit_arithmetic_types_float16: require
#endif
layout (constant_id = 0) const int count = 0;
layout (constant_id = 1) const int loop = 1;
layout (binding = 0) readonly buffer a_blob { sfpvec8 a_blob_data[]; };
layout (binding = 1) readonly buffer b_blob { sfpvec8 b_blob_data[]; };
layout (binding = 2) writeonly buffer c_blob { sfpvec8 c_blob_data[]; };
void main()
{
    int gx = int(gl_GlobalInvocationID.x);
    int gy = int(gl_GlobalInvocationID.y);
    int gz = int(gl_GlobalInvocationID.z);
    if (gx >= count || gy >= 1 || gz >= 1)
        return;
    afpvec8 a = buffer_ld8(a_blob_data, gx);
    afpvec8 b = buffer_ld8(b_blob_data, gx);
    afpvec8 c = afpvec8(afpvec4(1.f), afpvec4(1.f));
    for (int i = 0; i < loop; i++)
    {
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
        c[0] = a[0] * c[0] + b[0];
        c[1] = a[1] * c[1] + b[1];
    }
    buffer_st8(c_blob_data, gx, c);
}
)";

static const char inverse_data[] = R"(
#version 450
layout (binding = 0) uniform unfp sampler3D bottom_blob_3d;
layout (binding = 1, imfmtc1) writeonly uniform unfp image3D top_blob_3d;
layout (push_constant) uniform parameter
{
    float fading;
} p;
void main()
{
    int gx = int(gl_GlobalInvocationID.x);
    int gy = int(gl_GlobalInvocationID.y);
    int gz = int(gl_GlobalInvocationID.z);
    afpvec4 v = image3d_ld4(bottom_blob_3d, ivec3(gx, gy, gz));
    afpvec4 v_out = afpvec4(v.r, v.g, v.b, p.fading);
    image3d_st4(top_blob_3d, ivec3(gx, gy, gz), v_out);
}
)";

static ImTextureID g_texture = nullptr;
static ImVulkan::VkImageMat test_vkimage;
static ImVulkan::VulkanDevice* g_vkdev = nullptr;
static ImVulkan::VkAllocator* g_blob_allocator = nullptr;
static ImVulkan::VkAllocator* g_staging_allocator = nullptr;
static ImVulkan::Option g_opt;
static ImVulkan::Pipeline * g_pipeline = nullptr;
static ImVulkan::VkCompute * g_cmd = nullptr;
static float fp32[8] = {0.f};
static float fp32v4[8] = {0.f};
static float fp32v8[8] = {0.f};
static float fp16pv4[8] = {0.f};
static float fp16pv8[8] = {0.f};
static float fp16s[8] = {0.f};
static float fp16sv4[8] = {0.f};
static float fp16sv8[8] = {0.f};
static int loop_count = 100;
static int block_count = 10;
static int cmd_count = 1;
#define TEST_WIDTH      256
#define TEST_HEIGHT     256
#define TEST_CHANNEL    4

static float vkpeak(ImVulkan::VulkanDevice* vkdev, int loop, int count_mb, int cmd_loop, int storage_type, int arithmetic_type, int packing_type)
{
    const int count = count_mb * 1024 * 1024;
    int elempack = packing_type == 0 ? 1 : packing_type == 1 ? 4 : 8;
    if (!vkdev->info.support_fp16_storage() && storage_type == 2)
    {
        return -233;
    }
    if (!vkdev->info.support_fp16_arithmetic() && arithmetic_type == 1)
    {
        return -233;
    }
    double max_gflops = 0;
    ImVulkan::Option opt;
    opt.use_fp16_packed = storage_type == 1;
    opt.use_fp16_storage = storage_type == 2;
    opt.use_fp16_arithmetic = arithmetic_type == 1;
    opt.use_shader_pack8 = packing_type == 2;

    // setup pipeline
    ImVulkan::Pipeline pipeline(vkdev);
    {
        int local_size_x = std::min(128, std::max(32, (int)vkdev->info.subgroup_size()));
        pipeline.set_local_size_xyz(local_size_x, 1, 1);
        std::vector<ImVulkan::vk_specialization_type> specializations(2);
        specializations[0].i = count;
        specializations[1].i = loop;
        // glsl to spirv
        // -1 for omit the tail '\0'
        std::vector<uint32_t> spirv;
        if (packing_type == 0)
        {
            ImVulkan::compile_spirv_module(glsl_p1_data, opt, spirv);
        }
        if (packing_type == 1)
        {
            ImVulkan::compile_spirv_module(glsl_p4_data, opt, spirv);
        }
        if (packing_type == 2)
        {
            ImVulkan::compile_spirv_module(glsl_p8_data, opt, spirv);
        }

        pipeline.create(spirv.data(), spirv.size() * 4, specializations);
    }

    ImVulkan::VkAllocator* allocator = vkdev->acquire_blob_allocator();

    // prepare storage
    {
        ImVulkan::VkImageBuffer a;
        ImVulkan::VkImageBuffer b;
        ImVulkan::VkImageBuffer c;
        {
            if (opt.use_fp16_packed || opt.use_fp16_storage)
            {
                a.create(count, (size_t)(2u * elempack), elempack, allocator);
                b.create(count, (size_t)(2u * elempack), elempack, allocator);
                c.create(count, (size_t)(2u * elempack), elempack, allocator);
            }
            else
            {
                a.create(count, (size_t)(4u * elempack), elempack, allocator);
                b.create(count, (size_t)(4u * elempack), elempack, allocator);
                c.create(count, (size_t)(4u * elempack), elempack, allocator);
            }
        }

        // encode command
        ImVulkan::VkCompute cmd(vkdev);
        for (int i = 0; i < cmd_loop; i++)
        {
            {
                std::vector<ImVulkan::VkImageBuffer> bindings(3);
                bindings[0] = a;
                bindings[1] = b;
                bindings[2] = c;
                std::vector<ImVulkan::vk_constant_type> constants(0);
                cmd.record_pipeline(&pipeline, bindings, constants, c);
            }

            // time this
            {
                double t0 = ImGui::get_current_time();

                int ret = cmd.submit_and_wait();
                if (ret != 0)
                {
                    vkdev->reclaim_blob_allocator(allocator);
                    return -1;
                }

                double time = ImGui::get_current_time() - t0;
                const double mac = (double)count * (double)loop * 8 * elempack * 2;
                double gflops = mac / time / 1000000000;
                max_gflops += gflops;
            }
            cmd.flash();
        }
    }
    vkdev->reclaim_blob_allocator(allocator);
    return max_gflops / cmd_loop;
}

static std::string print_result(float gflops)
{
    if (gflops == -1)
            return "  error";

    if (gflops == -233)
        return "  not supported";

    if (gflops == 0)
        return "  not tested";

    if (gflops > 1000)
        return "  " + std::to_string(gflops / 1000.0) + " TFLOPS";
    return "  " + std::to_string(gflops) + " GFLOPS";
}

void PrepareVulkanDemo()
{
    g_vkdev = ImVulkan::get_gpu_device(-1); // auto get gpu
    g_blob_allocator = g_vkdev->acquire_blob_allocator();
    g_staging_allocator = g_vkdev->acquire_staging_allocator();
    g_opt.blob_vkallocator = g_blob_allocator;
    g_opt.staging_vkallocator = g_staging_allocator;
    g_opt.use_image_storage = true;

    float * tempBitmap = new float[TEST_WIDTH * TEST_HEIGHT * TEST_CHANNEL];
    for (int y = 0; y < TEST_HEIGHT; y++)
    {
        for (int x = 0; x < TEST_WIDTH; x++)
        {
            float dx = x + .5f;
            float dy = y + .5f;
            float dv = sinf(x * 0.02f) + sinf(0.03f * (x + y)) + sinf(sqrtf(0.4f * (dx * dx + dy * dy) + 1.f));
            
            for (int c = 0; c < TEST_CHANNEL; c++)
            {
                if (c == 3)
                    tempBitmap[(TEST_WIDTH * TEST_HEIGHT) * c + y * TEST_WIDTH + x] = 1.f;
                else
                    tempBitmap[(TEST_WIDTH * TEST_HEIGHT) * c + y * TEST_WIDTH + x] = fabsf(sinf(dv * 3.141592f + (c + 1) * 3.141592f / 3.f));
            }
        }
    }
    ImVulkan::ImageBuffer test_image;
    test_image.create_type(TEST_WIDTH, TEST_HEIGHT, TEST_CHANNEL, tempBitmap, ImVulkan::FLOAT32);
    g_cmd = new ImVulkan::VkCompute(g_vkdev);
    g_cmd->record_upload(test_image, test_vkimage, g_opt);
    g_cmd->submit_and_wait();
    delete [] tempBitmap;
    g_texture = ImGui::ImCreateTexture(test_vkimage);

    static std::vector<uint32_t> spirv;
    ImVulkan::compile_spirv_module(inverse_data, g_opt, spirv);
    g_pipeline = new ImVulkan::Pipeline(g_vkdev);
    std::vector<ImVulkan::vk_specialization_type> specializations(0);
    g_pipeline->create(spirv.data(), spirv.size() * 4, specializations);
    g_cmd->flash();
}

void CleanVulkanDemo()
{
    if (!g_vkdev)
        return;
    if (g_blob_allocator) { g_vkdev->reclaim_blob_allocator(g_blob_allocator); g_blob_allocator = nullptr; }
    if (g_staging_allocator) { g_vkdev->reclaim_staging_allocator(g_staging_allocator); g_staging_allocator = nullptr; }
    if (g_texture) { ImDestroyTexture(&g_texture); g_texture = nullptr; }
    if (g_pipeline) { delete g_pipeline; g_pipeline = nullptr; }
}

void ShowAddonsVulkanShaderWindow()
{
    for (int i = 0; i < g_gpu_count; i++)
    {
        ImVulkan::VulkanDevice* vkdev = ImVulkan::get_gpu_device(i);
        uint32_t driver_version = vkdev->info.driver_version();
        uint32_t api_version = vkdev->info.api_version();
        std::string driver_ver = std::to_string(VK_VERSION_MAJOR(driver_version)) + "." + 
                                std::to_string(VK_VERSION_MINOR(driver_version)) + "." +
                                std::to_string(VK_VERSION_PATCH(driver_version));
        std::string api_ver =   std::to_string(VK_VERSION_MAJOR(api_version)) + "." + 
                                std::to_string(VK_VERSION_MINOR(api_version)) + "." +
                                std::to_string(VK_VERSION_PATCH(api_version));
        std::string device_name = vkdev->info.device_name();
        uint32_t gpu_memory_budget = vkdev->get_heap_budget();
        ImGui::Text("%uMB", gpu_memory_budget);
        ImGui::Text("Device[%d]", i);
        ImGui::Text("Driver:%s", driver_ver.c_str());
        ImGui::Text("   API:%s", api_ver.c_str());
        ImGui::Text("  Name:%s", device_name.c_str());
        ImGui::Text("Memory:%uMB", gpu_memory_budget);
        std::string buffon_label = "Perf Test##" + std::to_string(i);
        if (ImGui::Button(buffon_label.c_str(), ImVec2(120, 20)))
        {
            fp32[i] = vkpeak(vkdev, loop_count, block_count, cmd_count, 0, 0, 0);
            fp32v4[i] = vkpeak(vkdev, loop_count, block_count, cmd_count, 0, 0, 1);
            fp32v8[i] = vkpeak(vkdev, loop_count, block_count, cmd_count, 0, 0, 2);
            fp16pv4[i] = vkpeak(vkdev, loop_count, block_count, cmd_count, 1, 1, 1);
            fp16pv8[i] = vkpeak(vkdev, loop_count, block_count, cmd_count, 1, 1, 2);
            fp16s[i] = vkpeak(vkdev, loop_count, block_count, cmd_count, 2, 1, 0);
            fp16sv4[i] = vkpeak(vkdev, loop_count, block_count, cmd_count, 2, 1, 1);
            fp16sv8[i] = vkpeak(vkdev, loop_count, block_count, cmd_count, 2, 1, 2);
        }
        ImGui::Text(" FP32 Scalar :%s", print_result(fp32[i]).c_str());
        ImGui::Text("   FP32 Vec4 :%s", print_result(fp32v4[i]).c_str());
        ImGui::Text("   FP32 Vec8 :%s", print_result(fp32v8[i]).c_str());
        ImGui::Text("  FP16p Vec4 :%s", print_result(fp16pv4[i]).c_str());
        ImGui::Text("  FP16p Vec8 :%s", print_result(fp16pv8[i]).c_str());
        ImGui::Text("FP16s Scalar :%s", print_result(fp16s[i]).c_str());
        ImGui::Text("  FP16s Vec4 :%s", print_result(fp16sv4[i]).c_str());
        ImGui::Text("  FP16s Vec8 :%s", print_result(fp16sv8[i]).c_str());
        
        ImGui::Separator();
    }
    
    static float color_fading = 1.0f;
    static float fading_step = -0.01f;
    {
        std::vector<ImVulkan::VkImageMat> bindings(2);
        bindings[0] = test_vkimage;
        bindings[1] = test_vkimage;
        std::vector<ImVulkan::vk_constant_type> constants(1);
        constants[0].f = color_fading;
        g_cmd->record_pipeline(g_pipeline, bindings, constants, test_vkimage);
        g_cmd->submit_and_wait();
        g_cmd->flash();
        color_fading += fading_step;
        if (color_fading <= 0.f) { fading_step = 0.01f; }
        if (color_fading >= 1.f) { fading_step = -0.01f; }
    }
    ImGui::Image(g_texture, ImVec2(128, 128));
    ImGui::Separator();

}
#endif
} // namespace ImGui

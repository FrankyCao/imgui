#include <imgui.h>
#undef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>
//-----------------------------------------------------------------------------------------------------------------

#include "ImGuiVariousControls.h"
#include "NumberTexture.h"

ImTextureID ImageTextureNumber = 0;
namespace ImGui
{

void ShowVariousControlsDemoWindow()
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
    if (ImGui::TreeNode("Notify"))
    {
        if (ImGui::Button("success!"))
        {
            notify::insert({ toast_type::toast_type_success, 3000, "Hello World! This is a success! %s", "We can also format here:)" });
        }
        if (ImGui::Button("warning!"))
        {
            notify::insert({ toast_type::toast_type_warning, 3000, "Hello World! This is a warning!" });
        }
        if (ImGui::Button("error!"))
        {
            notify::insert({ toast_type::toast_type_error, 3000, "Hello World! This is an error!" });
        }
        if (ImGui::Button("info!"))
        {
            notify::insert({ toast_type::toast_type_info, 3000, "Hello World! This is an info!" });
        }
        if (ImGui::Button("info2!"))
        {
            notify::insert({ toast_type::toast_type_info, 3000, "Hello World! This is an info! Yes I also support multiline text! Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud exercitation" });
        }
        notify::render();
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

} // namespace ImGui

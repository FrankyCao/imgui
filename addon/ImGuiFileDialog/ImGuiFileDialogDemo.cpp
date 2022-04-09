#include <string>
#include <sstream>
#include <fstream>
#include "ImGuiFileDialog.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

static bool canValidateDialog = false;

inline void InfosPane(const char* vFilter, IGFDUserDatas vUserDatas, bool* vCantContinue) // if vCantContinue is false, the user cant validate the dialog
{
	ImGui::TextColored(ImVec4(0, 1, 1, 1), "Infos Pane");

	ImGui::Text("Selected Filter : %s", vFilter);

	const char* userDatas = (const char*)vUserDatas;
	if (userDatas)
		ImGui::Text("User Datas : %s", userDatas);

	ImGui::Checkbox("if not checked you cant validate the dialog", &canValidateDialog);

	if (vCantContinue)
		*vCantContinue = canValidateDialog;
}

inline bool RadioButtonLabeled(const char* label, const char* help, bool active, bool disabled)
{
	using namespace ImGui;

	ImGuiWindow* window = GetCurrentWindow();
	if (window->SkipItems)
		return false;

	ImGuiContext& g = *GImGui;
	const ImGuiStyle& style = g.Style;
	float w = CalcItemWidth();
	if (w == window->ItemWidthDefault)	w = 0.0f; // no push item width
	const ImGuiID id = window->GetID(label);
	const ImVec2 label_size = CalcTextSize(label, nullptr, true);
	ImVec2 bb_size = ImVec2(style.FramePadding.x * 2 - 1, style.FramePadding.y * 2 - 1) + label_size;
	bb_size.x = ImMax(w, bb_size.x);

	const ImRect check_bb(
		window->DC.CursorPos,
		window->DC.CursorPos + bb_size);
	ItemSize(check_bb, style.FramePadding.y);

	if (!ItemAdd(check_bb, id))
		return false;

	// check
	bool pressed = false;
	if (!disabled)
	{
		bool hovered, held;
		pressed = ButtonBehavior(check_bb, id, &hovered, &held);

		window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, GetColorU32((held && hovered) ? ImGuiCol_FrameBgActive : hovered ? ImGuiCol_FrameBgHovered : ImGuiCol_FrameBg), style.FrameRounding);
		if (active)
		{
			const ImU32 col = GetColorU32((hovered && held) ? ImGuiCol_ButtonActive : hovered ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
			window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, col, style.FrameRounding);
		}
	}

	// circle shadow + bg
	if (style.FrameBorderSize > 0.0f)
	{
		window->DrawList->AddRect(check_bb.Min + ImVec2(1, 1), check_bb.Max, GetColorU32(ImGuiCol_BorderShadow), style.FrameRounding);
		window->DrawList->AddRect(check_bb.Min, check_bb.Max, GetColorU32(ImGuiCol_Border), style.FrameRounding);
	}

	if (label_size.x > 0.0f)
	{
		RenderText(check_bb.GetCenter() - label_size * 0.5f, label);
	}

	if (help && ImGui::IsItemHovered())
		ImGui::SetTooltip("%s", help);

	return pressed;
}

template<typename T>
inline bool RadioButtonLabeled_BitWize(
	const char* vLabel, const char* vHelp, T* vContainer, T vFlag,
	bool vOneOrZeroAtTime = false, //only one selected at a time
	bool vAlwaysOne = true, // radio behavior, always one selected
	T vFlagsToTakeIntoAccount = (T)0,
	bool vDisableSelection = false,
	ImFont* vLabelFont = nullptr) // radio witl use only theses flags
{
	bool selected = (*vContainer) & vFlag;
	const bool res = RadioButtonLabeled(vLabel, vHelp, selected, vDisableSelection);
	if (res) {
		if (!selected) {
			if (vOneOrZeroAtTime) {
				if (vFlagsToTakeIntoAccount) {
					if (vFlag & vFlagsToTakeIntoAccount) {
						*vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
						*vContainer = (T)(*vContainer | vFlag); // add
					}
				}
				else *vContainer = vFlag; // set
			}
			else {
				if (vFlagsToTakeIntoAccount) {
					if (vFlag & vFlagsToTakeIntoAccount) {
						*vContainer = (T)(*vContainer & ~vFlagsToTakeIntoAccount); // remove these flags
						*vContainer = (T)(*vContainer | vFlag); // add
					}
				}
				else *vContainer = (T)(*vContainer | vFlag); // add
			}
		}
		else {
			if (vOneOrZeroAtTime) {
				if (!vAlwaysOne) *vContainer = (T)(0); // remove all
			}
			else *vContainer = (T)(*vContainer & ~vFlag); // remove one
		}
	}
	return res;
}

void prepare_file_dialog_demo_window(ImGuiFileDialog * dlg, const char * bookmark_path)
{
#ifdef USE_BOOKMARK
	// load bookmarks
	std::ifstream docFile(bookmark_path, std::ios::in);
	if (docFile.is_open())
	{
		std::stringstream strStream;
		strStream << docFile.rdbuf();//read the file
		dlg->DeserializeBookmarks(strStream.str());
		docFile.close();
	}
#endif
}

void end_file_dialog_demo_window(ImGuiFileDialog * dlg, const char * bookmark_path)
{
#ifdef USE_BOOKMARK
	// save bookmarks
	std::ofstream configFileWriter(bookmark_path, std::ios::out);
	if (!configFileWriter.bad())
	{
		configFileWriter << dlg->SerializeBookmarks();
		configFileWriter.close();
	}
#endif
}

void show_file_dialog_demo_window(ImGuiFileDialog * dlg, bool * open)
{
	static std::string filePathName = "";
	static std::string filePath = "";
	static std::string filter = "";
	static std::string userDatas = "";
	static std::vector<std::pair<std::string, std::string>> selection = {};

	ImGui::Begin("imGuiFileDialog Demo", open); 
	ImGui::Text("imGuiFileDialog Version : %s", IMGUIFILEDIALOG_VERSION);
	ImGui::Indent();
	{
#ifdef USE_EXPLORATION_BY_KEYS
		static float flashingAttenuationInSeconds = 1.0f;
		if (ImGui::Button("R##resetflashlifetime"))
		{
			flashingAttenuationInSeconds = 1.0f;
			dlg->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
		}
		ImGui::SameLine();
		ImGui::PushItemWidth(200);
		if (ImGui::SliderFloat("Flash lifetime (s)", &flashingAttenuationInSeconds, 0.01f, 5.0f))
		{
			dlg->SetFlashingAttenuationInSeconds(flashingAttenuationInSeconds);
		}
		ImGui::PopItemWidth();
#endif
		static bool _UseWindowContraints = true;
		ImGui::Separator();
		ImGui::Checkbox("Use file dialog constraint", &_UseWindowContraints);
		ImGui::Text("Constraints is used here for define min/max file dialog size");
		ImGui::Separator();
		static bool standardDialogMode = false;
		ImGui::Text("Open Mode : ");
		ImGui::SameLine();
		if (RadioButtonLabeled("Standard", "Open dialog in standard mode", standardDialogMode, false)) standardDialogMode = true;
		ImGui::SameLine();
		if (RadioButtonLabeled("Modal", "Open dialog in modal mode", !standardDialogMode, false)) standardDialogMode = false;

		static ImGuiFileDialogFlags flags = ImGuiFileDialogFlags_ConfirmOverwrite;
		ImGui::Text("ImGuiFileDialog Flags : ");
		ImGui::Indent();
		{
			ImGui::Text("Commons :");
			RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Overwrite", "Overwrite verifcation before dialog closing", &flags, ImGuiFileDialogFlags_ConfirmOverwrite);
			ImGui::SameLine();
			RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Hidden Files", "Hide Hidden Files", &flags, ImGuiFileDialogFlags_DontShowHiddenFiles);
			ImGui::SameLine();
			RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Disable Directory Creation", "Disable Directory Creation button in dialog", &flags, ImGuiFileDialogFlags_DisableCreateDirectoryButton);

			ImGui::Text("Hide Column by default : (saved in imgui.ini, \n\tso defined when the inmgui.ini is not existing)");
			RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Column Type", "Hide Column file type by default", &flags, ImGuiFileDialogFlags_HideColumnType);
			ImGui::SameLine();
			RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Column Size", "Hide Column file Size by default", &flags, ImGuiFileDialogFlags_HideColumnSize);
			ImGui::SameLine();
			RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Hide Column Date", "Hide Column file Date by default", &flags, ImGuiFileDialogFlags_HideColumnDate);

			RadioButtonLabeled_BitWize<ImGuiFileDialogFlags>("Case Insensitive Extentions", "will not take into account the case of file extentions",
				&flags, ImGuiFileDialogFlags_CaseInsensitiveExtention);
		}
		ImGui::Unindent();

		ImGui::Text("Singleton acces :");
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog"))
		{
			const char* filters = ".*,.cpp,.h,.hpp";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 1, nullptr, flags);
			else
				dlg->OpenModal("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 1, nullptr, flags);
		}
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with collections of filters"))
		{
			const char* filters = "Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 1, nullptr, flags);
			else
				dlg->OpenModal("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 1, nullptr, flags);
		}
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with selection of 5 items"))
		{
			const char* filters = ".*,.cpp,.h,.hpp";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 5, nullptr, flags);
			else
				dlg->OpenModal("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 5, nullptr, flags);
		}
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with infinite selection"))
		{
			const char* filters = ".*,.cpp,.h,.hpp";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 0, nullptr, flags);
			else
				dlg->OpenModal("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 0, nullptr, flags);
		}
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with last file path name"))
		{
			const char* filters = ".*,.cpp,.h,.hpp";
			if (standardDialogMode)
				ImGuiFileDialog::Instance()->OpenDialog("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, filePathName, 1, nullptr, flags);
			else
				ImGuiFileDialog::Instance()->OpenModal("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, filePathName, 1, nullptr, flags);
		}
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open All file types with filter .*"))
		{
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", ".*", ".", "", 1, nullptr, flags);
			else
				dlg->OpenModal("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", ".*", ".", "", 1, nullptr, flags);
		}
		if (ImGui::Button(ICON_IGFD_SAVE " Save File Dialog with a custom pane"))
		{
			const char* filters = "C++ File (*.cpp){.cpp}";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",
					ICON_IGFD_SAVE " Choose a File", filters,
					".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2,
						std::placeholders::_3), 350, 1, IGFDUserDatas("SaveFile"), flags);
			else
				dlg->OpenModal("ChooseFileDlgKey",
					ICON_IGFD_SAVE " Choose a File", filters,
					".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2,
						std::placeholders::_3), 350, 1, IGFDUserDatas("SaveFile"), flags);
		}
		if (ImGui::Button(ICON_IGFD_SAVE " Save File Dialog with Confirm Dialog For Overwrite File if exist"))
		{
			const char* filters = "C/C++ File (*.c *.cpp){.c,.cpp}, Header File (*.h){.h}";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey", ICON_IGFD_SAVE " Choose a File", filters, ".", "", 1, IGFDUserDatas("SaveFile"), ImGuiFileDialogFlags_ConfirmOverwrite);
			else
				dlg->OpenModal("ChooseFileDlgKey",	ICON_IGFD_SAVE " Choose a File", filters, ".", "", 1, IGFDUserDatas("SaveFile"), ImGuiFileDialogFlags_ConfirmOverwrite);
		}

		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog"))
		{
			// let filters be null for open directory chooser
			if (standardDialogMode)
				dlg->OpenDialog("ChooseDirDlgKey",
					ICON_IGFD_FOLDER_OPEN " Choose a Directory", nullptr, ".", 1, nullptr, flags);
			else
				dlg->OpenModal("ChooseDirDlgKey",
					ICON_IGFD_FOLDER_OPEN " Choose a Directory", nullptr, ".", 1, nullptr, flags);
		}
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open Directory Dialog with selection of 5 items"))
		{
			// set filters be null for open directory chooser
			if (standardDialogMode)
				dlg->OpenDialog("ChooseDirDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a Directory", nullptr, ".", "", 5, nullptr, flags);
			else
				dlg->OpenModal("ChooseDirDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a Directory", nullptr, ".", "", 5, nullptr, flags);
		}

        if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Embedded Dialog demo"))
		{
            dlg->OpenDialog("embedded", "Select File", ".*", "", -1, nullptr, 
					ImGuiFileDialogFlags_NoDialog | 
					ImGuiFileDialogFlags_DisableBookmarkMode | 
					ImGuiFileDialogFlags_DisableCreateDirectoryButton | 
					ImGuiFileDialogFlags_ReadOnlyFileNameField);
        }

		ImGui::Separator();

		ImVec2 minSize = ImVec2(0, 0);
		ImVec2 maxSize = ImVec2(FLT_MAX, FLT_MAX);

		if (_UseWindowContraints)
		{
			ImGuiIO& io = ImGui::GetIO(); (void)io;
			maxSize = ImVec2((float)io.DisplaySize.x, (float)io.DisplaySize.y);
			minSize = maxSize * 0.25f;
		}

		// you can define your flags and min/max window size (theses three settings ae defined by default :
		// flags => ImGuiWindowFlags_NoCollapse
		// minSize => 0,0
		// maxSize => FLT_MAX, FLT_MAX (defined is float.h)

        if (dlg->Display("embedded", ImGuiWindowFlags_NoCollapse, ImVec2(0,0), ImVec2(0,350)))
		{
			if (dlg->IsOk())
			{
				filePathName = dlg->GetFilePathName();
				filePath = dlg->GetCurrentPath();
				filter = dlg->GetCurrentFilter();
				// here convert from string because a string was passed as a userDatas, but it can be what you want
				if (dlg->GetUserDatas())
					userDatas = std::string((const char*)dlg->GetUserDatas());
				auto sel = dlg->GetSelection(); // multiselection
				selection.clear();
				for (auto s : sel)
				{
					selection.emplace_back(s.first, s.second);
				}
				// action
			}
			dlg->Close();
		}

		if (dlg->Display("ChooseFileDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
		{
			if (dlg->IsOk())
			{
				filePathName = dlg->GetFilePathName();
				filePath = dlg->GetCurrentPath();
				filter = dlg->GetCurrentFilter();
				// here convert from string because a string was passed as a userDatas, but it can be what you want
				if (dlg->GetUserDatas())
					userDatas = std::string((const char*)dlg->GetUserDatas());
				auto sel = dlg->GetSelection(); // multiselection
				selection.clear();
				for (auto s : sel)
				{
					selection.emplace_back(s.first, s.second);
				}
				// action
			}
			dlg->Close();
		}
        if (dlg->Display("ChooseDirDlgKey", ImGuiWindowFlags_NoCollapse, minSize, maxSize))
		{
			if (dlg->IsOk())
			{
				filePathName = dlg->GetFilePathName();
				filePath = dlg->GetCurrentPath();
				filter = dlg->GetCurrentFilter();
				// here convert from string because a string was passed as a userDatas, but it can be what you want
				if (dlg->GetUserDatas())
					userDatas = std::string((const char*)dlg->GetUserDatas());
				auto sel = dlg->GetSelection(); // multiselection
				selection.clear();
				for (auto s : sel)
				{
					selection.emplace_back(s.first, s.second);
				}
				// action
			}
			dlg->Close();
		}

		ImGui::Separator();

		ImGui::Text("ImGuiFileDialog Return's :\n");
		ImGui::Indent();
		{
			ImGui::Text("GetFilePathName() : %s", filePathName.c_str());
			ImGui::Text("GetFilePath() : %s", filePath.c_str());
			ImGui::Text("GetCurrentFilter() : %s", filter.c_str());
			ImGui::Text("GetUserDatas() (was a std::string in this sample) : %s", userDatas.c_str());
			ImGui::Text("GetSelection() : ");
			ImGui::Indent();
			{
				static int selected = false;
				if (ImGui::BeginTable("##GetSelection", 2,
					ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_RowBg |
					ImGuiTableFlags_ScrollY))
				{
					ImGui::TableSetupScrollFreeze(0, 1); // Make top row always visible
					ImGui::TableSetupColumn("File Name", ImGuiTableColumnFlags_WidthStretch, -1, 0);
					ImGui::TableSetupColumn("File Path name", ImGuiTableColumnFlags_WidthFixed, -1, 1);
					ImGui::TableHeadersRow();

					ImGuiListClipper clipper;
					clipper.Begin((int)selection.size(), ImGui::GetTextLineHeightWithSpacing());
					while (clipper.Step())
					{
						for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
						{
							const auto& sel = selection[i];
							ImGui::TableNextRow();
							if (ImGui::TableSetColumnIndex(0)) // first column
							{
								ImGuiSelectableFlags selectableFlags = ImGuiSelectableFlags_AllowDoubleClick;
								selectableFlags |= ImGuiSelectableFlags_SpanAllColumns | ImGuiSelectableFlags_AllowItemOverlap;
								if (ImGui::Selectable(sel.first.c_str(), i == selected, selectableFlags)) selected = i;
							}
							if (ImGui::TableSetColumnIndex(1)) // second column
							{
								ImGui::Text("%s", sel.second.c_str());
							}
						}
					}
					clipper.End();

					ImGui::EndTable();
				}
			}
			ImGui::Unindent();
		}
		ImGui::Unindent();
	}
	ImGui::Unindent();
	ImGui::End();
}

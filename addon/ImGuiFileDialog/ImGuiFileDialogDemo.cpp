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

inline bool RadioButtonLabeled(const char* label, bool active, bool disabled)
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

	return pressed;
}

void prepare_file_dialog_demo_window(ImGuiFileDialog * dlg, const char * bookmark_path)
{
#ifdef IMGUI_INTERNAL_ICONS
	// set type color and icons
	dlg->SetTypeInfos(std::to_string('f'), ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FK_FILE_O);
	dlg->SetTypeInfos(std::to_string('d'), ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FK_FOLDER);
	dlg->SetTypeInfos(std::to_string('l'), ImVec4(0.5f, 0.5f, 1.0f, 0.9f), ICON_FK_EXTERNAL_LINK);
	// set format color and icons
	dlg->SetExtentionInfos(".txt", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA4_FILE_TEXT_O);
	dlg->SetExtentionInfos(".cpp", ImVec4(1.0f, 1.0f, 0.0f, 0.9f), ICON_FA5_FILE_CODE);
	dlg->SetExtentionInfos(".h", ImVec4(0.0f, 1.0f, 0.0f, 0.9f), ICON_FA5_FILE_CODE);
	dlg->SetExtentionInfos(".hpp", ImVec4(0.0f, 0.0f, 1.0f, 0.9f), ICON_FA5_FILE_CODE);
	dlg->SetExtentionInfos(".md", ImVec4(1.0f, 0.0f, 1.0f, 0.9f), ICON_FK_MARKDOWN);
	dlg->SetExtentionInfos(".png", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC); // add an icon for the filter type
	dlg->SetExtentionInfos(".bmp", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC); // add an icon for the filter type
	dlg->SetExtentionInfos(".jpg", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC); // add an icon for the filter type
	dlg->SetExtentionInfos(".jpeg", ImVec4(0.0f, 1.0f, 1.0f, 0.9f), ICON_IGFD_FILE_PIC); // add an icon for the filter type
	dlg->SetExtentionInfos(".mp4", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA4_FILE_VIDEO_O);
	dlg->SetExtentionInfos(".MP4", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA4_FILE_VIDEO_O);
	dlg->SetExtentionInfos(".mkv", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA4_FILE_VIDEO_O);
	dlg->SetExtentionInfos(".MKV", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA4_FILE_VIDEO_O);
	dlg->SetExtentionInfos(".mov", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA4_FILE_VIDEO_O);
	dlg->SetExtentionInfos(".MOV", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA4_FILE_VIDEO_O);
	dlg->SetExtentionInfos(".webm", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA4_FILE_VIDEO_O);
	dlg->SetExtentionInfos(".ttf", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FK_FONT);
	dlg->SetExtentionInfos(".TTF", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FK_FONT);
	dlg->SetExtentionInfos(".doc", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA5_FILE_WORD);
	dlg->SetExtentionInfos(".docx", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA5_FILE_WORD);
	dlg->SetExtentionInfos(".ppt", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA5_FILE_POWERPOINT);
	dlg->SetExtentionInfos(".pptx", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA5_FILE_POWERPOINT);
	dlg->SetExtentionInfos(".xls", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA5_FILE_EXCEL);
	dlg->SetExtentionInfos(".xlsx", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA5_FILE_EXCEL);
	dlg->SetExtentionInfos(".pdf", ImVec4(1.0f, 1.0f, 1.0f, 0.9f), ICON_FA5_FILE_PDF);
#endif
	dlg->SetExtentionInfos(".gif", ImVec4(0.0f, 1.0f, 0.5f, 0.9f), "[GIF]"); // add an text for a filter type

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
		if (RadioButtonLabeled("Standard", standardDialogMode, false)) standardDialogMode = true;
		ImGui::SameLine();
		if (RadioButtonLabeled("Modal", !standardDialogMode, false)) standardDialogMode = false;

		ImGui::Text("Singleton acces :");
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog"))
		{
			const char* filters = ".*,.cpp,.h,.hpp";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "");
			else
				dlg->OpenModal("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "");
		}
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with collections of filters"))
		{
			const char* filters = "Source files (*.cpp *.h *.hpp){.cpp,.h,.hpp},Image files (*.png *.gif *.jpg *.jpeg){.png,.gif,.jpg,.jpeg},.md";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "");
			else
				dlg->OpenModal("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "");
		}
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with selection of 5 items"))
		{
			const char* filters = ".*,.cpp,.h,.hpp";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 5);
			else
				dlg->OpenModal("ChooseFileDlgKey", ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 5);
		}
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open File Dialog with infinite selection"))
		{
			const char* filters = ".*,.cpp,.h,.hpp";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 0);
			else
				dlg->OpenModal("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", filters, ".", "", 0);
		}
		if (ImGui::Button(ICON_IGFD_FOLDER_OPEN " Open All file types with filter .*"))
		{
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", ".*", ".", "");
			else
				dlg->OpenModal("ChooseFileDlgKey",	ICON_IGFD_FOLDER_OPEN " Choose a File", ".*", ".", "");
		}
		if (ImGui::Button(ICON_IGFD_SAVE " Save File Dialog with a custom pane"))
		{
			const char* filters = "C++ File (*.cpp){.cpp}";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey",
					ICON_IGFD_SAVE " Choose a File", filters,
					".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2,
						std::placeholders::_3), 350, 1, IGFDUserDatas("SaveFile"), ImGuiFileDialogFlags_ConfirmOverwrite);
			else
				dlg->OpenModal("ChooseFileDlgKey",
					ICON_IGFD_SAVE " Choose a File", filters,
					".", "", std::bind(&InfosPane, std::placeholders::_1, std::placeholders::_2,
						std::placeholders::_3), 350, 1, IGFDUserDatas("SaveFile"), ImGuiFileDialogFlags_ConfirmOverwrite);
		}
		if (ImGui::Button(ICON_IGFD_SAVE " Save File Dialog with Confirm Dialog For Overwrite File if exist"))
		{
			const char* filters = "C/C++ File (*.c *.cpp){.c,.cpp}, Header File (*.h){.h}";
			if (standardDialogMode)
				dlg->OpenDialog("ChooseFileDlgKey", ICON_IGFD_SAVE " Choose a File", filters, ".", "", 1, IGFDUserDatas("SaveFile"), ImGuiFileDialogFlags_ConfirmOverwrite);
			else
				dlg->OpenModal("ChooseFileDlgKey",	ICON_IGFD_SAVE " Choose a File", filters, ".", "", 1, IGFDUserDatas("SaveFile"), ImGuiFileDialogFlags_ConfirmOverwrite);
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

		static std::string filePathName = "";
		static std::string filePath = "";
		static std::string filter = "";
		static std::string userDatas = "";
		static std::vector<std::pair<std::string, std::string>> selection = {};

		if (dlg->Display("ChooseFileDlgKey",
			ImGuiWindowFlags_NoCollapse, minSize, maxSize))
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

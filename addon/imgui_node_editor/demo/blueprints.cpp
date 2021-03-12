# include <application.h>
# define IMGUI_DEFINE_MATH_OPERATORS
# include <imgui_internal.h>
# include <tinyfiledialogs.h>
# include <inttypes.h>

# include "crude_logger.h"
# include "crude_blueprint.h"
# include "crude_blueprint_library.h"
# include "blueprint_editor_document.h"
# include "blueprint_editor_utilities.h"
# include "blueprint_editor_icons.h"
# include "crude_layout.h"
# include "imgui_extras.h"
# include "imgui_node_editor.h"
# include "imgui_node_editor_internal.h"
# include "crude_json.h"

#include "Config.h"

// TODO:
//  - figure out how to present possible action (ex. Alt + click on link)
//  -

//#include <float.h>
//unsigned int fp_control_state = _controlfp(_EM_INEXACT, _MCW_EM);

# include <functional>
# include <type_traits>
# include <map>

namespace ed = ax::NodeEditor;

using namespace crude_blueprint;
using namespace blueprint_editor;
using namespace blueprint_editor_utilities;


static ImEx::MostRecentlyUsedList Application_GetMostRecentlyOpenFileList()
{
    return ImEx::MostRecentlyUsedList("MostRecentlyOpenList");
}

static EntryPointNode* FindEntryPointNode(Blueprint& blueprint)
{
    for (auto& node : blueprint.GetNodes())
    {
        if (node->GetTypeInfo().m_Id == EntryPointNode::GetStaticTypeInfo().m_Id)
        {
            return static_cast<EntryPointNode*>(node);
        }
    }

    return nullptr;
}

using std::function;

enum class EventHandle : uint64_t { Invalid };

template <typename... Args>
struct Event
{
    using Delegate = function<void(Args...)>;

    EventHandle Add(Delegate delegate)
    {
        auto eventHandle = static_cast<EventHandle>(++m_LastHandleId);
        m_Delegates[eventHandle] = std::move(delegate);
        return eventHandle;
    }

    bool Remove(EventHandle eventHandle)
    {
        return m_Delegates.erase(eventHandle) > 0;
    }

    void Clear()
    {
        m_Delegates.clear();
    }

    template <typename... CallArgs>
    void Invoke(CallArgs&&... args)
    {
        vector<Delegate> delegates;
        delegates.reserve(m_Delegates.size());
        for (auto& entry : m_Delegates)
            delegates.push_back(entry.second);

        for (auto& delegate : delegates)
            delegate(args...);
    }

    EventHandle operator += (Delegate delegate)       { return Add(std::move(delegate)); }
    bool        operator -= (EventHandle eventHandle) { return Remove(eventHandle);      }
    template <typename... CallArgs>
    void        operator () (CallArgs&&... args)      { Invoke(std::forward<CallArgs>(args)...); }

private:
    using EventHandleType = std::underlying_type_t<EventHandle>;

    map<EventHandle, Delegate> m_Delegates;
    EventHandleType            m_LastHandleId = 0;
};

static ed::EditorContext* m_Editor = nullptr;

# pragma region Action

struct Action
{
    using OnChangeEvent     = Event<Action*>;
    using OnTriggeredEvent  = Event<>;

    Action() = default;
    Action(string_view name, OnTriggeredEvent::Delegate delegate = {});

    void SetName(string_view name);
    const string& GetName() const;

    void SetEnabled(bool set);
    bool IsEnabled() const;

    void Execute();

    OnChangeEvent       OnChange;
    OnTriggeredEvent    OnTriggered;

private:
    string  m_Name;
    bool    m_IsEnabled = true;
};

Action::Action(string_view name, OnTriggeredEvent::Delegate delegate)
    : m_Name(name.to_string())
{
    if (delegate)
        OnTriggered += std::move(delegate);
}

void Action::SetName(string_view name)
{
    if (m_Name == name)
        return;

    m_Name = name.to_string();

    OnChange(this);
}

const crude_blueprint::string& Action::GetName() const
{
    return m_Name;
}

void Action::SetEnabled(bool set)
{
    if (m_IsEnabled == set)
        return;

    m_IsEnabled = set;

    OnChange(this);
}

bool Action::IsEnabled() const
{
    return m_IsEnabled;
}

void Action::Execute()
{
    LOGV("Action: %s", m_Name.c_str());
    OnTriggered();
}
# pragma endregion

struct BlueprintEditorExample
{
    void InstallDocumentCallbacks(ed::Config& config)
    {
        config.UserPointer = this;
        //config.BeginSaveSession = [](void* userPointer)
        //{
        //    auto self = reinterpret_cast<BlueprintEditorExample*>(userPointer);

        //    if (self->m_Document)
        //        self->m_Document->OnSaveBegin();
        //};
        //config.EndSaveSession = [](void* userPointer)
        //{
        //    auto self = reinterpret_cast<BlueprintEditorExample*>(userPointer);

        //    if (self->m_Document)
        //        self->m_Document->OnSaveEnd();
        //};
        //config.SaveSettingsJson = [](const crude_json::value& state, ed::SaveReasonFlags reason, void* userPointer) -> bool
        //{
        //    auto self = reinterpret_cast<BlueprintEditorExample*>(userPointer);

        //    if (self->m_Document)
        //        return self->m_Document->OnSaveState(state, reason);
        //    else
        //        return false;
        //};
        //config.LoadSettingsJson = [](void* userPointer) -> crude_json::value
        //{
        //    auto self = reinterpret_cast<BlueprintEditorExample*>(userPointer);

        //    if (self->m_Document)
        //        return self->m_Document->OnLoadState();
        //    else
        //        return {};
        //};
        //config.SaveNodeSettingsJson = [](ed::NodeId nodeId, const crude_json::value& value, ed::SaveReasonFlags reason, void* userPointer) -> bool
        //{
        //    auto self = reinterpret_cast<BlueprintEditorExample*>(userPointer);

        //    if (self->m_Document)
        //        return self->m_Document->OnSaveNodeState(static_cast<uint32_t>(nodeId.Get()), value, reason);
        //    else
        //        return false;
        //};
        //config.LoadNodeSettingsJson = [](ed::NodeId nodeId, void* userPointer) -> crude_json::value
        //{
        //    auto self = reinterpret_cast<BlueprintEditorExample*>(userPointer);

        //    if (self->m_Document)
        //        return self->m_Document->OnLoadNodeState(static_cast<uint32_t>(nodeId.Get()));
        //    else
        //        return {};
        //};

        struct TransactionWrapper final: ed::ITransaction
        {
            static const char* ActionToString(ed::TransactionAction action)
            {
                switch (action)
                {
                    case ed::TransactionAction::Unknown:    return "Unknown";
                    case ed::TransactionAction::Navigation: return "Navigation";
                    case ed::TransactionAction::Drag:       return "Drag";
                    default : return "";
                }

                return "";
            }

            TransactionWrapper(shared_ptr<Document::UndoTransaction> transaction)
                : m_Transaction(std::move(transaction))
            {
            }

            void AddAction(ed::TransactionAction action, const char* name) override
            {
                ++m_ActionCount;

                m_Transaction->AddAction("%s", name);
            }

            void Commit() override
            {
                ImGuiTextBuffer name;
                if (m_ActionCount == m_DragActionCount && m_DragActionCount > 1)
                    name.appendf("Drag %d nodes", static_cast<int>(m_DragActionCount));

                m_Transaction->Commit(name.c_str());
            }

            void Discard() override
            {
                m_Transaction->Discard();
            }

            void AddAction(ed::TransactionAction action, ed::NodeId nodeId, const char* name) override
            {
                ++m_ActionCount;

                m_NodeIds.push_back(nodeId);

                if (action == ed::TransactionAction::Drag)
                {
                    ++m_DragActionCount;

                    auto& blueprint = m_Transaction->GetDocument()->GetBlueprint();
                    auto  node      = blueprint.FindNode(static_cast<uint32_t>(nodeId.Get()));

                    m_Transaction->AddAction("Drag %" PRI_node, FMT_node(node));
                }
                else
                    m_Transaction->AddAction("%s", name);
            }

            shared_ptr<Document::UndoTransaction> m_Transaction;
            size_t m_ActionCount = 0;
            size_t m_DragActionCount = 0;
            vector<ed::NodeId> m_NodeIds;
        };

        config.TransactionInterface.Constructor = [](const char* name, void* userPointer) -> ed::ITransaction*
        {
            auto self = reinterpret_cast<BlueprintEditorExample*>(userPointer);
            return new TransactionWrapper(self->m_Document->BeginUndoTransaction(name));
        };

        config.TransactionInterface.Destructor = [](ed::ITransaction* transaction, void* userPointer)
        {
            delete transaction;
        };

        config.TransactionInterface.UserPointer = this;
    }

    void CreateExampleDocument()
    {
        using namespace crude_blueprint;

        m_Document = make_unique<Document>();
        m_Blueprint = &m_Document->m_Blueprint;

        auto printNode2Node = m_Blueprint->CreateNode<PrintNode>();         ed::SetNodePosition(printNode2Node->m_Id, ImVec2(828, 111));
        auto entryPointNode = m_Blueprint->CreateNode<EntryPointNode>();    ed::SetNodePosition(entryPointNode->m_Id, ImVec2(-20, 95));
        auto printNode1Node = m_Blueprint->CreateNode<PrintNode>();         ed::SetNodePosition(printNode1Node->m_Id, ImVec2(828, -1));
        auto flipFlopNode   = m_Blueprint->CreateNode<FlipFlopNode>();      ed::SetNodePosition(flipFlopNode->m_Id, ImVec2(408, -1));
        auto toStringNode   = m_Blueprint->CreateNode<ToStringNode>();      ed::SetNodePosition(toStringNode->m_Id, ImVec2(617, 111));
        auto doNNode        = m_Blueprint->CreateNode<DoNNode>();           ed::SetNodePosition(doNNode->m_Id, ImVec2(140, 88));
        auto addNode        = m_Blueprint->CreateNode<AddNode>();           ed::SetNodePosition(addNode->m_Id, ImVec2(360, 150));
        auto mulNode        = m_Blueprint->CreateNode<MulNode>();           ed::SetNodePosition(mulNode->m_Id, ImVec2(360, 248));
        auto subNode        = m_Blueprint->CreateNode<SubNode>();           ed::SetNodePosition(subNode->m_Id, ImVec2(360, 344));
        auto divNode        = m_Blueprint->CreateNode<DivNode>();           ed::SetNodePosition(divNode->m_Id, ImVec2(360, 440));
        auto constInt32Node = m_Blueprint->CreateNode<ConstInt32Node>();    ed::SetNodePosition(constInt32Node->m_Id, ImVec2(124, 320));
        auto commentNode    = m_Blueprint->CreateNode<CommentNode>();       ed::SetNodePosition(commentNode->m_Id, ImVec2(-48, 0));
                                                                            //ed::SetNodeSize(commentNode->m_Id, ImVec2(383, 276));
                                                                            ed::SetGroupSize(commentNode->m_Id, ImVec2(367, 240));
        entryPointNode->m_Exit.LinkTo(doNNode->m_Enter);

        constInt32Node->m_Int32.SetValue(10);
        doNNode->m_N.SetValue(50);
        doNNode->m_Exit.LinkTo(flipFlopNode->m_Enter);

        //toStringNode->m_Value.LinkTo(addNode->m_Result);
        toStringNode->m_Value.LinkTo(divNode->m_Result);

        addNode->m_A.LinkTo(doNNode->m_Counter);
        addNode->m_B.SetValue(3);

        mulNode->m_A.LinkTo(addNode->m_Result);
        mulNode->m_B.LinkTo(constInt32Node->m_Int32);

        subNode->m_A.LinkTo(mulNode->m_Result);
        subNode->m_B.LinkTo(constInt32Node->m_Int32);

        divNode->m_A.LinkTo(subNode->m_Result);
        divNode->m_B.LinkTo(constInt32Node->m_Int32);

        toStringNode->m_Exit.LinkTo(printNode2Node->m_Enter);

        printNode1Node->m_String.SetValue("FlipFlop slot A!");
        printNode2Node->m_String.SetValue("FlipFlop slot B!");
        printNode2Node->m_String.LinkTo(toStringNode->m_String);

        flipFlopNode->m_A.LinkTo(printNode1Node->m_Enter);
        flipFlopNode->m_B.LinkTo(toStringNode->m_Enter);
    }

    void Initialize(const char * setting_file, const char * ini_file, const char * bp_file)
    {
        crude_logger::OverlayLogger::SetCurrent(&m_OverlayLogger);

        m_OverlayLogger.AddKeyword("Node");
        m_OverlayLogger.AddKeyword("Pin");
        m_OverlayLogger.AddKeyword("Link");
        m_OverlayLogger.AddKeyword("CreateNodeDialog");
        m_OverlayLogger.AddKeyword("NodeContextMenu");
        m_OverlayLogger.AddKeyword("PinContextMenu");
        m_OverlayLogger.AddKeyword("LinkContextMenu");

        ImEx::MostRecentlyUsedList::Install(ImGui::GetCurrentContext());

        PrintNode::s_PrintFunction = [](const PrintNode& node, string_view message)
        {
            LOGI("PrintNode(%" PRIu32 "): \"%" PRI_sv "\"", node.m_Id, FMT_sv(message));
        };

        ed::Config config;
        config.SettingsFile = nullptr;
        InstallDocumentCallbacks(config);
        m_Editor = ed::CreateEditor(&config);
        ed::SetCurrentEditor(m_Editor);

        if (!File_Open(bp_file))
        {
            CreateExampleDocument();
        }

        for (auto nodeTypeInfo : m_Blueprint->GetNodeRegistry()->GetTypes())
            m_OverlayLogger.AddKeyword(nodeTypeInfo->m_Name);

        ed::SetCurrentEditor(nullptr);
        auto& io = ImGui::GetIO();
        io.IniFilename = ini_file;
    }

    void Finalize()
    {
        File_Close();
        ed::DestroyEditor(m_Editor);
    }

    bool Frame()
    {
        bool done = false;
        auto& io = ImGui::GetIO();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(io.DisplaySize);
        ImGui::Begin("Content", nullptr,
                    ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                    ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoSavedSettings |
                    ImGuiWindowFlags_NoBringToFrontOnFocus);

        ed::SetCurrentEditor(m_Editor);

        UpdateActions();

        ShowMainMenu();
        ShowToolbar();

        if (m_showStyleEditor)
            ShowStyleEditor(&m_showStyleEditor);

        ImGui::Separator();

        if (m_Blueprint)
        {
            DebugOverlay debugOverlay(*m_Blueprint);

            ed::Begin("###main_editor");

            CommitBlueprintNodes(*m_Blueprint, debugOverlay);

            HandleCreateAction(*m_Document);
            HandleDestroyAction(*m_Document);
            HandleContextMenuAction(*m_Blueprint);

            ShowDialogs(*m_Document);

            ShowInfoTooltip(*m_Blueprint);

            ed::End();
        }
        else
        {
            ImGui::Dummy(ImGui::GetContentRegionAvail());
        }

        m_OverlayLogger.Update(ImGui::GetIO().DeltaTime);
        m_OverlayLogger.Draw(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());

        ed::SetCurrentEditor(nullptr);
        ImGui::End();
        return done;
        //ImGui::ShowMetricsWindow();
    }
/*
    ImGuiWindowFlags GetWindowFlags() const override
    {
        auto flags = Application::GetWindowFlags();
        flags |= ImGuiWindowFlags_MenuBar;
        flags &= ~ImGuiWindowFlags_NoSavedSettings;
        return flags;
    }

    bool CanClose() override
    {
        return File_Close();
    }
*/
private:
    void UpdateTitle()
    {
        string title = std::string(Application_GetName(nullptr));

        if (File_IsOpen())
        {
            title += " - ";
            title += m_Document->m_Name.c_str();

            if (File_IsModified())
                title += "*";
        }

        //SetTitle(title.c_str()); // TODO::Dicky need add Application_SetName
    }

    void UpdateActions()
    {
        auto hasDocument = File_IsOpen();
        auto hasUndo     = hasDocument && !m_Document->m_Undo.empty();
        auto hasRedo     = hasDocument && !m_Document->m_Redo.empty();
        auto isModified  = hasDocument && File_IsModified();
        auto entryNode = m_Blueprint ? FindEntryPointNode(*m_Blueprint) : nullptr;
        bool hasBlueprint  = (m_Blueprint != nullptr);
        bool hasEntryPoint = (entryNode != nullptr);
        bool isExecuting   = hasBlueprint && (m_Blueprint->CurrentNode() != nullptr);

        m_File_Close.SetEnabled(hasDocument);
        m_File_SaveAs.SetEnabled(hasDocument);
        m_File_Save.SetEnabled(hasDocument);

        m_Edit_Undo.SetEnabled(hasUndo);
        m_Edit_Redo.SetEnabled(hasRedo);
        m_Edit_Cut.SetEnabled(hasDocument && false);
        m_Edit_Copy.SetEnabled(hasDocument && false);
        m_Edit_Paste.SetEnabled(hasDocument && false);
        m_Edit_Duplicate.SetEnabled(hasDocument && false);
        m_Edit_Delete.SetEnabled(hasDocument && false);
        m_Edit_SelectAll.SetEnabled(hasDocument && false);
        m_Edit_Style.SetEnabled(hasBlueprint);

        m_View_NavigateBackward.SetEnabled(hasDocument && false);
        m_View_NavigateForward.SetEnabled(hasDocument && false);
        m_View_ShowFlow.SetEnabled(hasDocument);
        m_View_ZoomToContent.SetEnabled(hasBlueprint);

        m_Blueprint_Start.SetEnabled(hasBlueprint && hasEntryPoint);
        m_Blueprint_Step.SetEnabled(hasBlueprint && isExecuting);
        m_Blueprint_Stop.SetEnabled(hasBlueprint && isExecuting);
        m_Blueprint_Run.SetEnabled(hasBlueprint && hasEntryPoint);
    }

    void ShowMainMenu()
    {
        auto menuAction = [](Action& action)
        {
            if (ImGui::MenuItem(action.GetName().c_str(), nullptr, nullptr, action.IsEnabled()))
            {
                action.Execute();
            }
        };

        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                auto mostRecentlyOpenFiles = Application_GetMostRecentlyOpenFileList();

                menuAction(m_File_New);
                ImGui::Separator();
                menuAction(m_File_Open);
                if (ImGui::BeginMenu("Open Recent", !mostRecentlyOpenFiles.GetList().empty()))
                {
                    for (auto& entry : mostRecentlyOpenFiles.GetList())
                        if (ImGui::MenuItem(entry.c_str()))
                            File_Open(entry.c_str());
                    ImGui::Separator();
                    if (ImGui::MenuItem("Clear Recently Opened"))
                        mostRecentlyOpenFiles.Clear();
                    ImGui::EndMenu();
                }
                ImGui::Separator();
                menuAction(m_File_SaveAs);
                menuAction(m_File_Save);
                ImGui::Separator();
                menuAction(m_File_Close);
                ImGui::Separator();
                menuAction(m_File_Exit);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Edit"))
            {
                menuAction(m_Edit_Undo);
                menuAction(m_Edit_Redo);
                ImGui::Separator();
                menuAction(m_Edit_Cut);
                menuAction(m_Edit_Copy);
                menuAction(m_Edit_Paste);
                menuAction(m_Edit_Duplicate);
                menuAction(m_Edit_Delete);
                ImGui::Separator();
                menuAction(m_Edit_SelectAll);
                ImGui::Separator();
                menuAction(m_Edit_Style);

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                menuAction(m_View_NavigateBackward);
                menuAction(m_View_NavigateForward);
                menuAction(m_View_ShowFlow);
                menuAction(m_View_ZoomToContent);
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Blueprint"))
            {
                menuAction(m_Blueprint_Start);
                menuAction(m_Blueprint_Step);
                menuAction(m_Blueprint_Stop);
                ImGui::Separator();
                menuAction(m_Blueprint_Run);

                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }
    }

    void ShowToolbar()
    {
        auto toolbarAction = [](Action& action)
        {
            ImEx::ScopedDisableItem disableAction(!action.IsEnabled());

            if (ImGui::Button(action.GetName().c_str()))
            {
                action.Execute();
            }
        };

        toolbarAction(m_File_Open);
        ImGui::SameLine();
        toolbarAction(m_File_Save);
        ImGui::SameLine();
        toolbarAction(m_File_SaveAs);
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        toolbarAction(m_Edit_Undo);
        ImGui::SameLine();
        toolbarAction(m_Edit_Redo);
        ImGui::SameLine();
        toolbarAction(m_Edit_Style);
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        toolbarAction(m_View_ShowFlow);
        ImGui::SameLine();
        toolbarAction(m_View_ZoomToContent);
        ImGui::SameLine();
        ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
        ImGui::SameLine();
        toolbarAction(m_Blueprint_Start);
        ImGui::SameLine();
        toolbarAction(m_Blueprint_Step);
        ImGui::SameLine();
        toolbarAction(m_Blueprint_Stop);
        ImGui::SameLine();
        toolbarAction(m_Blueprint_Run);

        ImGui::SameLine();
        ImGui::Text("Status: %s", m_Blueprint ? StepResultToString(m_Blueprint->LastStepResult()) : "-");

        if (auto currentNode = m_Blueprint ? m_Blueprint->CurrentNode() : nullptr)
        {
            ImGui::SameLine(); ImGui::Spacing();
            ImGui::SameLine(); ImGui::Text("Current: %" PRI_node, FMT_node(currentNode));

            auto nextNode = m_Blueprint->NextNode();
            ImGui::SameLine(); ImGui::Spacing();
            ImGui::SameLine();
            if (nextNode)
                ImGui::Text("Next: %" PRI_node, FMT_node(nextNode));
            else
                ImGui::Text("Next: -");
        }
    }

    // Iterate over blueprint nodes and commit them to node editor.
    void CommitBlueprintNodes(Blueprint& blueprint, DebugOverlay& debugOverlay)
    {
        const auto iconSize = ImVec2(ImGui::GetTextLineHeight(), ImGui::GetTextLineHeight());

        debugOverlay.Begin();

        // Commit all nodes to editor
        // Habdling Tree Node
        for (auto& node : blueprint.GetNodes())
        {
            if (node->GetType() != NodeType::Tree)
                continue;

            const float rounding = 5.0f;
            const float padding  = 12.0f;

            const auto pinBackground = ed::GetStyle().Colors[ed::StyleColor_NodeBg];

            ed::PushStyleColor(ed::StyleColor_NodeBg,        ImColor(128, 128, 128, 200));
            ed::PushStyleColor(ed::StyleColor_NodeBorder,    ImColor( 32,  32,  32, 200));
            ed::PushStyleColor(ed::StyleColor_PinRect,       ImColor( 60, 180, 255, 150));
            ed::PushStyleColor(ed::StyleColor_PinRectBorder, ImColor( 60, 180, 255, 150));

            ed::PushStyleVar(ed::StyleVar_NodePadding,  ImVec4(0, 0, 0, 0));
            ed::PushStyleVar(ed::StyleVar_NodeRounding, rounding);
            ed::PushStyleVar(ed::StyleVar_SourceDirection, ImVec2(0.0f,  1.0f));
            ed::PushStyleVar(ed::StyleVar_TargetDirection, ImVec2(0.0f, -1.0f));
            ed::PushStyleVar(ed::StyleVar_LinkStrength, 0.0f);
            ed::PushStyleVar(ed::StyleVar_PinBorderWidth, 1.0f);
            ed::PushStyleVar(ed::StyleVar_PinRadius, 5.0f);
            ed::BeginNode(node->m_Id);

            ImGui::BeginVertical(node->m_Id);
            ImGui::BeginHorizontal("inputs");
            ImGui::Spring(0, padding * 2);

            ImRect inputsRect;
            int inputAlpha = 200;
            if (node->GetInputPins().size() > 0)
            {
                    auto& pin = node->GetInputPins()[0];
                    ImGui::Dummy(ImVec2(0, padding));
                    ImGui::Spring(1, 0);
                    inputsRect = ax::NodeEditor::Detail::ImGui_GetItemRect();

                    ed::PushStyleVar(ed::StyleVar_PinArrowSize, 10.0f);
                    ed::PushStyleVar(ed::StyleVar_PinArrowWidth, 10.0f);
                    ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_NoRoundCornerT);
                    ed::BeginPin(pin->m_Id, ed::PinKind::Input);
                    ed::PinPivotRect(inputsRect.GetTL(), inputsRect.GetBR());
                    ed::PinRect(inputsRect.GetTL(), inputsRect.GetBR());
                    ed::EndPin();
                    ed::PopStyleVar(3);
                    
                    //if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                    //    inputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));

                    debugOverlay.DrawInputPin(*pin);
            }
            else
                ImGui::Dummy(ImVec2(0, padding));

            ImGui::Spring(0, padding * 2);
            ImGui::EndHorizontal();

            ImGui::BeginHorizontal("content_frame");
            ImGui::Spring(1, padding);

            ImGui::BeginVertical("content", ImVec2(0.0f, 0.0f));
            ImGui::Dummy(ImVec2(160, 0));
            ImGui::Spring(1);
            ImGui::TextUnformatted(node->GetName().data());
            ImGui::Spring(1);
            ImGui::EndVertical();
            auto contentRect = ax::NodeEditor::Detail::ImGui_GetItemRect();

            ImGui::Spring(1, padding);
            ImGui::EndHorizontal();

            ImGui::BeginHorizontal("outputs");
            ImGui::Spring(0, padding * 2);

            ImRect outputsRect;
            int outputAlpha = 200;
            if (node->GetOutputPins().size() > 0)
            {
                auto& pin = node->GetOutputPins()[0];
                ImGui::Dummy(ImVec2(0, padding));
                ImGui::Spring(1, 0);
                outputsRect = ax::NodeEditor::Detail::ImGui_GetItemRect();

                ed::PushStyleVar(ed::StyleVar_PinCorners, ImDrawFlags_NoRoundCornerB);
                ed::BeginPin(pin->m_Id, ed::PinKind::Output);
                ed::PinPivotRect(outputsRect.GetTL(), outputsRect.GetBR());
                ed::PinRect(outputsRect.GetTL(), outputsRect.GetBR());
                ed::EndPin();
                ed::PopStyleVar();
                
                //if (newLinkPin && !CanCreateLink(newLinkPin, &pin) && &pin != newLinkPin)
                //    outputAlpha = (int)(255 * ImGui::GetStyle().Alpha * (48.0f / 255.0f));

                debugOverlay.DrawOutputPin(*pin);
            }
            else
                ImGui::Dummy(ImVec2(0, padding));

            ImGui::Spring(0, padding * 2);
            ImGui::EndHorizontal();

            ImGui::EndVertical();

            ed::EndNode();
            ed::PopStyleVar(7);
            ed::PopStyleColor(4);

            auto drawList = ed::GetNodeBackgroundDrawList(node->m_Id);

            //const auto fringeScale = ImGui::GetStyle().AntiAliasFringeScale;
            //const auto unitSize    = 1.0f / fringeScale;

            //const auto ImDrawList_AddRect = [](ImDrawList* drawList, const ImVec2& a, const ImVec2& b, ImU32 col, float rounding, int rounding_corners, float thickness)
            //{
            //    if ((col >> 24) == 0)
            //        return;
            //    drawList->PathRect(a, b, rounding, rounding_corners);
            //    drawList->PathStroke(col, true, thickness);
            //};

            drawList->AddRectFilled(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, ImDrawFlags_NoRoundCornerT);
            //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
            drawList->AddRect(inputsRect.GetTL() + ImVec2(0, 1), inputsRect.GetBR(),
                IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), inputAlpha), 4.0f, ImDrawFlags_NoRoundCornerT);
            //ImGui::PopStyleVar();
            drawList->AddRectFilled(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, ImDrawFlags_NoRoundCornerB);
            //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
            drawList->AddRect(outputsRect.GetTL(), outputsRect.GetBR() - ImVec2(0, 1),
                IM_COL32((int)(255 * pinBackground.x), (int)(255 * pinBackground.y), (int)(255 * pinBackground.z), outputAlpha), 4.0f, ImDrawFlags_NoRoundCornerB);
            //ImGui::PopStyleVar();
            drawList->AddRectFilled(contentRect.GetTL(), contentRect.GetBR(), IM_COL32(24, 64, 128, 200), 0.0f);
            //ImGui::PushStyleVar(ImGuiStyleVar_AntiAliasFringeScale, 1.0f);
            drawList->AddRect(
                contentRect.GetTL(),
                contentRect.GetBR(),
                IM_COL32(48, 128, 255, 100), 0.0f);
            //ImGui::PopStyleVar();
        }
        // Handling Comment Node
        for (auto& node : blueprint.GetNodes())
        {
            if (node->GetType() != NodeType::Comment)
                continue;
            const float commentAlpha = 0.75f;

            ImGui::PushStyleVar(ImGuiStyleVar_Alpha, commentAlpha);
            ed::PushStyleColor(ed::StyleColor_NodeBg, ImColor(255, 255, 255, 64));
            ed::PushStyleColor(ed::StyleColor_NodeBorder, ImColor(255, 255, 255, 64));
            ed::BeginNode(node->m_Id);
            ImGui::BeginVertical("content");
            // Show title if node has one.
            auto nodeName = node->GetName();
            if (!nodeName.empty())
            {
                ImGui::BeginHorizontal("horizontal");
                ImGui::Spring(1);
                ImGui::TextUnformatted(nodeName.data());
                ImGui::Spring(1);
                ImGui::EndHorizontal();
            }
            auto nodeSize  = ed::GetNodeSize(node->m_Id);
            ed::Group(nodeSize);
            ImGui::EndVertical();
            ed::EndNode();
            ed::PopStyleColor(2);
            ImGui::PopStyleVar();
            if (ed::BeginGroupHint(node->m_Id))
            {
                auto bgAlpha = static_cast<int>(ImGui::GetStyle().Alpha * 255);
                auto min = ed::GetGroupMin();

                ImGui::SetCursorScreenPos(min - ImVec2(-8, ImGui::GetTextLineHeightWithSpacing() + 4));
                ImGui::BeginGroup();
                ImGui::TextUnformatted(nodeName.data());
                ImGui::EndGroup();

                auto drawList = ed::GetHintBackgroundDrawList();

                auto hintBounds      = ax::NodeEditor::Detail::ImGui_GetItemRect();
                auto hintFrameBounds = ax::NodeEditor::Detail::ImRect_Expanded(hintBounds, 8, 4);

                drawList->AddRectFilled(
                    hintFrameBounds.GetTL(),
                    hintFrameBounds.GetBR(),
                    IM_COL32(255, 255, 255, 64 * bgAlpha / 255), 4.0f);

                drawList->AddRect(
                    hintFrameBounds.GetTL(),
                    hintFrameBounds.GetBR(),
                    IM_COL32(255, 255, 255, 128 * bgAlpha / 255), 4.0f);
            }
            ed::EndGroupHint();

            debugOverlay.DrawNode(*node);
        }

        // Handling Blueprint Node
        for (auto& node : blueprint.GetNodes())
        {
            if (node->GetType() != NodeType::Blueprint && node->GetType() != NodeType::Simple)
                continue;
            const auto isSimple = node->GetType() == NodeType::Simple;
            ed::BeginNode(node->m_Id);

            // General node layout:
            //
            // +-----------------------------------+
            // | Title                             |
            // | +-----------[ Dummy ]-----------+ |
            // | +---------------+   +-----------+ |
            // | | o Pin         |   |   Out B o | |
            // | | o Pin <Value> |   |   Out A o | |
            // | | o Pin         |   |           | |
            // | +---------------+   +-----------+ |
            // +-----------------------------------+

            // Show title if node has one.
            auto nodeName = node->GetName();
            if (!nodeName.empty() && !isSimple)
            {
                auto headerBackgroundRenderer = ImEx::ItemBackgroundRenderer([node](ImDrawList* drawList)
                {
                    auto border   = ed::GetStyle().NodeBorderWidth;
                    auto rounding = ed::GetStyle().NodeRounding;

                    auto itemMin = ImGui::GetItemRectMin();
                    auto itemMax = ImGui::GetItemRectMax();

                    auto nodeStart = ed::GetNodePosition(node->m_Id);
                    auto nodeSize  = ed::GetNodeSize(node->m_Id);

                    itemMin   = nodeStart;
                    itemMin.x = itemMin.x + border - 0.5f;
                    itemMin.y = itemMin.y + border - 0.5f;
                    itemMax.x = nodeStart.x + nodeSize.x - border + 0.5f;
                    itemMax.y = itemMax.y + ImGui::GetStyle().ItemSpacing.y + 0.5f;

                    drawList->AddRectFilled(itemMin, itemMax, IM_COL32(255, 255, 255, 64), rounding, ImDrawCornerFlags_Top);

                    //drawList->AddRectFilledMultiColor(itemMin, itemMax, IM_COL32(255, 0, 0, 64), rounding, ImDrawCornerFlags_Top);
                });

                //ImGui::PushFont(HeaderFont());
                ImGui::TextUnformatted(nodeName.data(), nodeName.data() + nodeName.size());
                //ImGui::PopFont();

                //ImEx::Debug_DrawItemRect();
                ImGui::Spacing();
            }

            ImGui::Dummy(ImVec2(100.0f, 0.0f)); // For minimum node width

            crude_layout::Grid layout;
            layout.Begin(node->m_Id, isSimple ? 3 : 2, 100.0f);
            layout.SetColumnAlignment(0.0f);

            // Draw column with input pins.
            for (auto& pin : node->GetInputPins())
            {
                // Add a bit of spacing to separate pins and make value not cramped
                ImGui::Spacing();

                // Input pin layout:
                //
                //     +-[1]---+-[2]------+-[3]----------+
                //     |       |          |              |
                //    [X] Icon | Pin Name | Value/Editor |
                //     |       |          |              |
                //     +-------+----------+--------------+

                ed::BeginPin(pin->m_Id, ed::PinKind::Input);
                // [X] - Tell editor to put pivot point in the middle of
                //       the left side of the pin. This is the point
                //       where link will be hooked to.
                //
                //       By default pivot is in pin center point which
                //       does not look good for blueprint nodes.
                ed::PinPivotAlignment(ImVec2(0.0f, 0.5f));

                // [1] - Icon
                Icon(iconSize,
                    PinTypeToIconType(pin->GetType()),
                    blueprint.HasPinAnyLink(*pin),
                    PinTypeToColor(pin->GetValueType()));

                // [2] - Show pin name if it has one
                if (!pin->m_Name.empty() && !isSimple)
                {
                    ImGui::SameLine();
                    ImGui::TextUnformatted(pin->m_Name.data(), pin->m_Name.data() + pin->m_Name.size());
                }

                // [3] - Show value/editor when pin is not linked to anything
                if (!blueprint.HasPinAnyLink(*pin) && !isSimple)
                {
                    ImGui::SameLine();
                    DrawPinValueWithEditor(*pin);
                }

                ed::EndPin();

                // [Debug Overlay] Show value of the pin if node is currently executed
                debugOverlay.DrawInputPin(*pin);

                layout.NextRow();
            }
            
            if (isSimple)
            {
                layout.SetColumnAlignment(0.5f);
                layout.NextColumn();

                ImGui::TextUnformatted(nodeName.data());
            }

            layout.SetColumnAlignment(1.0f);
            layout.NextColumn();

            // Draw column with output pins.
            for (auto& pin : node->GetOutputPins())
            {
                // Add a bit of spacing to separate pins and make value not cramped
                ImGui::Spacing();

                // Output pin layout:
                //
                //    +-[1]------+-[2]---+
                //    |          |       |
                //    | Pin Name | Icon [X]
                //    |          |       |
                //    +----------+-------+

                ed::BeginPin(pin->m_Id, ed::PinKind::Output);

                // [X] - Tell editor to put pivot point in the middle of
                //       the right side of the pin. This is the point
                //       where link will be hooked to.
                //
                //       By default pivot is in pin center point which
                //       does not look good for blueprint nodes.
                ed::PinPivotAlignment(ImVec2(1.0f, 0.5f));

                // [1] - Show pin name if it has one
                if (!pin->m_Name.empty() && !isSimple)
                {
                    ImGui::TextUnformatted(pin->m_Name.data(), pin->m_Name.data() + pin->m_Name.size());
                    ImGui::SameLine();
                }

                // [2] - Show icon
                Icon(iconSize,
                    PinTypeToIconType(pin->GetType()),
                    blueprint.HasPinAnyLink(*pin),
                    PinTypeToColor(pin->GetValueType()));

                ed::EndPin();

                // [Debug Overlay] Show value of the pin if node is currently executed
                debugOverlay.DrawOutputPin(*pin);

                layout.NextRow();
            }

            layout.End();

            //ImEx::Debug_DrawItemRect();

            ed::EndNode();

            //ImEx::Debug_DrawItemRect();

            // [Debug Overlay] Show cursor over node
            debugOverlay.DrawNode(*node);
        }

        // Commit all links to editor
        for (auto& pin : blueprint.GetPins())
        {
            if (!pin->m_Link)
                continue;

            // To keep things simple, link id is same as pin id.
            ed::Link(pin->m_Id, pin->m_Id, pin->m_Link->m_Id, PinTypeToColor(pin->GetValueType()));
        }

        debugOverlay.End();
    }

    void HandleCreateAction(Document& document)
    {
        Blueprint& blueprint = document.m_Blueprint;

        ItemBuilder itemBuilder;

        if (!itemBuilder)
            return;

        if (auto linkBuilder = itemBuilder.QueryNewLink())
        {
            auto startPin = blueprint.FindPin(static_cast<uint32_t>(linkBuilder->m_StartPinId.Get()));
            auto endPin   = blueprint.FindPin(static_cast<uint32_t>(linkBuilder->m_EndPinId.Get()));

            // Editor return pins in order draw by the user. It is up to the
            // user to determine if it is valid. In blueprints we accept only links
            // from receivers to providers. Other graph types may allow bi-directional
            // links between nodes and this ordering make this feature possible.
            if (endPin->IsReceiver() && startPin->IsProvider())
                ImSwap(startPin, endPin);

            if (auto canLinkResult = startPin->CanLinkTo(*endPin))
            {
                ed::Suspend();
                ImGui::BeginTooltip();
                ImGui::Text("Valid Link%s%s",
                    canLinkResult.Reason().empty() ? "" : ": ",
                    canLinkResult.Reason().empty() ? "" : canLinkResult.Reason().c_str());
                ImGui::Separator();
                ImGui::TextUnformatted("From:");
                ImGui::Bullet(); ImGui::Text("%" PRI_pin, FMT_pin(startPin));
                ImGui::Bullet(); ImGui::Text("%" PRI_node, FMT_node(startPin->m_Node));
                ImGui::TextUnformatted("To:");
                ImGui::Bullet(); ImGui::Text("%" PRI_pin, FMT_pin(endPin));
                ImGui::Bullet(); ImGui::Text("%" PRI_node, FMT_node(endPin->m_Node));
                ImGui::EndTooltip();
                ed::Resume();

                if (linkBuilder->Accept())
                {
                    auto transaction = document.BeginUndoTransaction("Create Link");

                    if (startPin->LinkTo(*endPin))
                        LOGV("[HandleCreateAction] %" PRI_pin " linked with %" PRI_pin, FMT_pin(startPin), FMT_pin(endPin));
                    else
                        transaction->Discard();
                }
            }
            else
            {
                ed::Suspend();
                ImGui::SetTooltip(
                    "Invalid Link: %s",
                    canLinkResult.Reason().c_str()
                );
                ed::Resume();

                linkBuilder->Reject();
            }
        }
        else if (auto nodeBuilder = itemBuilder.QueryNewNode())
        {
            // Arguably creation of node is simpler than a link.
            ed::Suspend();
            ImGui::SetTooltip("Create Node...");
            ed::Resume();

            // Node builder accept return true when user release mouse button.
            // When this happen we request CreateNodeDialog to open.
            if (nodeBuilder->Accept())
            {
                // Get node from which link was pulled (if any). After creating
                // node we will try to make link with first matching pin of the node.
                auto pin = blueprint.FindPin(static_cast<uint32_t>(nodeBuilder->m_PinId.Get()));

                ed::Suspend();
                LOGV("[HandleCreateAction] Open CreateNodeDialog");
                m_CreateNodeDailog.Open(pin);
                ed::Resume();
            }
        }
    }

    void HandleDestroyAction(Document& document)
    {
        Blueprint& blueprint = document.m_Blueprint;

        ItemDeleter itemDeleter;

        if (!itemDeleter)
            return;

        auto deferredTransaction = document.GetDeferredUndoTransaction("Destroy Action");

        vector<Node*> nodesToDelete;
        uint32_t brokenLinkCount = 0;

        // Process all nodes marked for deletion
        while (auto nodeDeleter = itemDeleter.QueryDeletedNode())
        {
            deferredTransaction->Begin("Delete Item");

            // Remove node, pass 'true' so links attached to node will also be queued for deletion.
            if (nodeDeleter->Accept(true))
            {
                auto node = blueprint.FindNode(static_cast<uint32_t>(nodeDeleter->m_NodeId.Get()));
                if (node != nullptr)
                    // Queue nodes for deletion. We need to serve links first to avoid crash.
                    nodesToDelete.push_back(node);
            }
        }

        // Process all links marked for deletion
        while (auto linkDeleter = itemDeleter.QueryDeleteLink())
        {
            deferredTransaction->Begin("Delete Item");

            if (linkDeleter->Accept())
            {
                auto startPin = blueprint.FindPin(static_cast<uint32_t>(linkDeleter->m_StartPinId.Get()));
                if (startPin != nullptr && startPin->IsLinked())
                {
                    LOGV("[HandleDestroyAction] %" PRI_pin " unlinked from %" PRI_pin, FMT_pin(startPin), FMT_pin(startPin->GetLink()));
                    startPin->Unlink();
                    ++brokenLinkCount;
                }
            }
        }

        // After links was removed, now it is safe to delete nodes.
        for (auto node : nodesToDelete)
        {
            LOGV("[HandleDestroyAction] %" PRI_node, FMT_node(node));
            blueprint.DeleteNode(node);
        }

        if (!nodesToDelete.empty() || brokenLinkCount)
        {
            LOGV("[HandleDestroyAction] %" PRIu32 " node%s deleted, %" PRIu32 " link%s broken",
                static_cast<uint32_t>(nodesToDelete.size()), nodesToDelete.size() != 1 ? "s" : "",
                brokenLinkCount, brokenLinkCount != 1 ? "s" : "");
        }
    }

    void HandleContextMenuAction(Blueprint& blueprint)
    {
        if (ed::ShowBackgroundContextMenu())
        {
            ed::Suspend();
            LOGV("[HandleContextMenuAction] Open CreateNodeDialog");
            m_CreateNodeDailog.Open();
            ed::Resume();
        }

        ed::NodeId contextNodeId;
        if (ed::ShowNodeContextMenu(&contextNodeId))
        {
            auto node = blueprint.FindNode(static_cast<uint32_t>(contextNodeId.Get()));

            ed::Suspend();
            LOGV("[HandleContextMenuAction] Open NodeContextMenu for %" PRI_node, FMT_node(node));
            m_NodeContextMenu.Open(node);
            ed::Resume();
        }

        ed::PinId contextPinId;
        if (ed::ShowPinContextMenu(&contextPinId))
        {
            auto pin = blueprint.FindPin(static_cast<uint32_t>(contextPinId.Get()));

            ed::Suspend();
            LOGV("[HandleContextMenuAction] Open PinContextMenu for %" PRI_pin, FMT_pin(pin));
            m_PinContextMenu.Open(pin);
            ed::Resume();
        }

        ed::LinkId contextLinkId;
        if (ed::ShowLinkContextMenu(&contextLinkId))
        {
            auto pin = blueprint.FindPin(static_cast<uint32_t>(contextLinkId.Get()));

            ed::Suspend();
            LOGV("[HandleContextMenuAction] Open LinkContextMenu for %" PRI_pin, FMT_pin(pin));
            m_LinkContextMenu.Open(pin);
            ed::Resume();
        }
    }

    void ShowDialogs(Document& document)
    {
        Blueprint& blueprint = document.m_Blueprint;

        ed::Suspend();

        m_CreateNodeDailog.Show(document);
        m_NodeContextMenu.Show(blueprint);
        m_PinContextMenu.Show(blueprint);
        m_LinkContextMenu.Show(blueprint);

        ed::Resume();
    }

    void ShowStyleEditor(bool* show = nullptr)
    {
        if (!ImGui::Begin("Style", show))
        {
            ImGui::End();
            return;
        }

        //auto paneWidth = ImGui::GetContentRegionAvailWidth();
        float paneWidth = 600;

        auto& editorStyle = ed::GetStyle();
        ImGui::BeginHorizontal("Style buttons", ImVec2(paneWidth, 0), 1.0f);
        ImGui::TextUnformatted("Values");
        ImGui::Spring();
        if (ImGui::Button("Reset to defaults"))
            editorStyle = ed::Style();
        ImGui::EndHorizontal();
        ImGui::Spacing();
        ImGui::DragFloat4("Node Padding", &editorStyle.NodePadding.x, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Node Rounding", &editorStyle.NodeRounding, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Node Border Width", &editorStyle.NodeBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Hovered Node Border Width", &editorStyle.HoveredNodeBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Selected Node Border Width", &editorStyle.SelectedNodeBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Pin Rounding", &editorStyle.PinRounding, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Pin Border Width", &editorStyle.PinBorderWidth, 0.1f, 0.0f, 15.0f);
        ImGui::DragFloat("Link Strength", &editorStyle.LinkStrength, 1.0f, 0.0f, 500.0f);
        //ImVec2  SourceDirection;
        //ImVec2  TargetDirection;
        ImGui::DragFloat("Scroll Duration", &editorStyle.ScrollDuration, 0.001f, 0.0f, 2.0f);
        ImGui::DragFloat("Flow Marker Distance", &editorStyle.FlowMarkerDistance, 1.0f, 1.0f, 200.0f);
        ImGui::DragFloat("Flow Speed", &editorStyle.FlowSpeed, 1.0f, 1.0f, 2000.0f);
        ImGui::DragFloat("Flow Duration", &editorStyle.FlowDuration, 0.001f, 0.0f, 5.0f);
        //ImVec2  PivotAlignment;
        //ImVec2  PivotSize;
        //ImVec2  PivotScale;
        //float   PinCorners;
        //float   PinRadius;
        //float   PinArrowSize;
        //float   PinArrowWidth;
        ImGui::DragFloat("Group Rounding", &editorStyle.GroupRounding, 0.1f, 0.0f, 40.0f);
        ImGui::DragFloat("Group Border Width", &editorStyle.GroupBorderWidth, 0.1f, 0.0f, 15.0f);

        ImGui::Separator();

        static ImGuiColorEditFlags edit_mode = ImGuiColorEditFlags_RGB;
        ImGui::BeginHorizontal("Color Mode", ImVec2(paneWidth, 0), 1.0f);
        ImGui::TextUnformatted("Filter Colors");
        ImGui::Spring();
        ImGui::RadioButton("RGB", &edit_mode, ImGuiColorEditFlags_RGB);
        ImGui::Spring(0);
        ImGui::RadioButton("HSV", &edit_mode, ImGuiColorEditFlags_HSV);
        ImGui::Spring(0);
        ImGui::RadioButton("HEX", &edit_mode, ImGuiColorEditFlags_HEX);
        ImGui::EndHorizontal();

        static ImGuiTextFilter filter;
        filter.Draw("", paneWidth);

        ImGui::Spacing();

        for (int i = 0; i < ed::StyleColor_Count; ++i)
        {
            auto name = ed::GetStyleColorName((ed::StyleColor)i);
            if (!filter.PassFilter(name))
                continue;

            ImGui::ColorEdit4(name, &editorStyle.Colors[i].x, edit_mode);
        }

        ImGui::End();
    }

    void ShowInfoTooltip(Blueprint& blueprint)
    {
        if (!ed::IsActive())
            return;

        auto hoveredNode = blueprint.FindNode(static_cast<uint32_t>(ed::GetHoveredNode().Get()));
        auto hoveredPin  = blueprint.FindPin(static_cast<uint32_t>(ed::GetHoveredPin().Get()));
        if (!hoveredNode && hoveredPin)
            hoveredNode = hoveredPin->m_Node;

        auto pinTooltip = [](const char* label, const Pin& pin, bool showNode)
        {
            ImGui::TextUnformatted(label);
            ImGui::Bullet(); ImGui::Text("ID: %" PRIu32, pin.m_Id);
            if (!pin.m_Name.empty())
            {
                ImGui::Bullet(); ImGui::Text("Name: %" PRI_sv, FMT_sv(pin.m_Name));
            }
            if (showNode && pin.m_Node)
            {
                auto nodeName = pin.m_Node->GetName();
                ImGui::Bullet(); ImGui::Text("Node: %" PRI_sv, FMT_sv(nodeName));
            }
            ImGui::Bullet(); ImGui::Text("Type: %s", PinTypeToString(pin.GetType()));
            ImGui::Bullet(); ImGui::Text("Value Type: %s", PinTypeToString(pin.GetValueType()));

            string flags;
            if (pin.IsLinked())
                flags += "linked, ";
            if (pin.IsInput())
                flags += "input, ";
            if (pin.IsOutput())
                flags += "output, ";
            if (pin.IsProvider())
                flags += "provider, ";
            if (pin.IsReceiver())
                flags += "receiver, ";
            if (!flags.empty())
                flags = flags.substr(0, flags.size() - 2);
            ImGui::Bullet(); ImGui::Text("Flags: %s", flags.c_str());
        };

        if (hoveredNode)
        {
            auto nodeTypeInfo = hoveredNode->GetTypeInfo();
            auto nodeName = hoveredNode->GetName();
            auto nodeType = hoveredNode->GetType();

            ed::Suspend();
            ImGui::BeginTooltip();

            ImGui::Text("Node ID: %" PRIu32, hoveredNode->m_Id);
            ImGui::Text("Name: %" PRI_sv, FMT_sv(nodeName));
            ImGui::Separator();
            ImGui::TextUnformatted("Type:");
            ImGui::Bullet(); ImGui::Text("     ID: 0x%08" PRIX32, nodeTypeInfo.m_Id);
            ImGui::Bullet(); ImGui::Text("   Type: %s", NodeTypeToString(nodeType));
            ImGui::Bullet(); ImGui::Text("SubType: %" PRI_sv, FMT_sv(nodeTypeInfo.m_Name));

            if (hoveredPin)
            {
                ImGui::Separator();
                pinTooltip("Pin", *hoveredPin, false);
            }

            ImGui::EndTooltip();
            ed::Resume();
        }
        else if (auto hoveredLinkId = ed::GetHoveredLink())
        {
            ed::PinId startPinId, endPinId;
            ed::GetLinkPins(hoveredLinkId, &startPinId, &endPinId);

            auto startPin = blueprint.FindPin(static_cast<uint32_t>(startPinId.Get()));
            auto endPin = blueprint.FindPin(static_cast<uint32_t>(endPinId.Get()));

            ed::Suspend();
            ImGui::BeginTooltip();

            ImGui::Text("Link ID: %" PRIu32, startPin->m_Id);
            ImGui::Text("Type: %s", PinTypeToString(startPin->GetValueType()));
            ImGui::Separator();
            pinTooltip("Start Pin:", *startPin, true);
            ImGui::Separator();
            pinTooltip("End Pin:", *endPin, true);

            ImGui::EndTooltip();
            ed::Resume();
        }
    }

    bool File_IsOpen()
    {
        return m_Document != nullptr;
    }

    bool File_IsModified()
    {
        return m_Document->m_IsModified;
    }

    void File_MarkModified()
    {
        if (m_Document->m_IsModified)
            return;

        m_Document->m_IsModified = true;

        UpdateTitle();
    }

    void File_New()
    {
        if (!File_Close())
            return;

        LOGI("[File] New");

        m_Document = make_unique<Document>();
        m_Blueprint = &m_Document->m_Blueprint;
    }

    bool File_Open(string_view path, string* error = nullptr)
    {
        auto document = make_unique<Document>();
        if (!document->Load(path))
        {
            if (error)
            {
                ImGuiTextBuffer buffer;
                buffer.appendf("Failed to load blueprint from file \"%" PRI_sv "\".", FMT_sv(path));
                *error = buffer.c_str();
            }
            else
                LOGE("Failed to load blueprint from file \"%" PRI_sv "\".", FMT_sv(path));
            return false;
        }

        if (!File_Close())
            return false;

        LOGI("[File] Open \"%" PRI_sv "\"", FMT_sv(path));

        auto mostRecentlyOpenFiles = Application_GetMostRecentlyOpenFileList();
        mostRecentlyOpenFiles.Add(path.to_string());

        document->SetPath(path);

        m_Document = std::move(document);
        m_Blueprint = &m_Document->m_Blueprint;

        UpdateTitle();

        m_Document->OnMakeCurrent();

        return true;
    }

    bool File_Open()
    {
        const char* filterPatterns[1] = { "*.json" };
        auto path = tinyfd_openFileDialog(
            "Open Blueprint...",
            m_Document ? m_Document->m_Path.c_str() : nullptr,
            1, filterPatterns,
            "Blueprint Files (*.json)", 0);
        if (!path)
            return false;

        string error;
        if (!File_Open(path, &error) && !error.empty())
        {
            tinyfd_messageBox(
                (string(Application_GetName(nullptr)) + " - Open Blueprint...").c_str(),
                error.c_str(),
                "ok", "error", 1);

            LOGE("%s", error.c_str());

            return false;
        }

        return true;
    }

    bool File_Close()
    {
        if (!File_IsOpen())
            return true;

        if (File_IsModified())
        {
            auto dialogResult = tinyfd_messageBox(
                Application_GetName(nullptr),
                "Do you want to save changes to this blueprint before closing?",
                "yesnocancel", "question", 1);
            if (dialogResult == 1) // yes
            {
                if (!File_Save())
                    return false;
            }
            else if (dialogResult == 0) // cancel
            {
                return false;
            }
        }

        LOGI("[File] Close");

        m_Blueprint = nullptr;
        m_Document = nullptr;

        UpdateTitle();

        return true;
    }

    bool File_SaveAsEx(string_view path)
    {
        if (!File_IsOpen())
            return true;

        if (!m_Document->Save(path))
        {
            LOGE("Failed to save blueprint to file \"%" PRI_sv "\".", FMT_sv(path));
            return false;
        }

        m_Document->m_IsModified = false;

        LOGI("[File] Save \"%" PRI_sv "\".", FMT_sv(path));

        return true;
    }

    bool File_SaveAs()
    {
        const char* filterPatterns[1] = { "*.json" };
        auto path = tinyfd_saveFileDialog(
            "Save Blueprint...",
            m_Document->m_Path.c_str(),
            1, filterPatterns,
            "Blueprint Files (*.json)");
        if (!path)
            return false;

        if (!File_SaveAsEx(path))
            return false;

        m_Document->SetPath(path);

        UpdateTitle();

        return true;
    }

    bool File_Save()
    {
        if (!m_Document->m_Path.empty())
            return File_SaveAsEx(m_Document->m_Path);
        else
            return File_SaveAs();
    }

    void File_Exit()
    {
        //if (!Close())
        //    return;
        //LOGI("[File] Quit");
        //Quit();
    }

    void Edit_Undo()
    {
        m_Document->Undo();
    }

    void Edit_Redo()
    {
        m_Document->Redo();
    }

    void Edit_Cut()
    {
    }

    void Edit_Copy()
    {
    }

    void Edit_Paste()
    {
    }

    void Edit_Duplicate()
    {
    }

    void Edit_Delete()
    {
    }

    void Edit_SelectAll()
    {
    }

    void Edit_Style()
    {
        m_showStyleEditor = true;
    }

    void View_NavigateBackward()
    {
    }

    void View_NavigateForward()
    {
    }

    void View_ZoomToContent()
    {
        ed::NavigateToContent();
    }

    void View_ShowFlow()
    {
        if (m_Blueprint)
        {
            for (auto& pin : m_Blueprint->GetPins())
            {
                if (!pin->m_Link)
                    continue;
                ed::Flow(pin->m_Id, pin->GetType() == PinType::Flow ? ed::FlowDirection::Forward : ed::FlowDirection::Backward);
            }
        }
    }

    void Blueprint_Start()
    {
        auto entryNode = FindEntryPointNode(*m_Blueprint);

        LOGI("Execution: Start");
        m_Blueprint->Start(*entryNode);
    }

    void Blueprint_Step()
    {
        LOGI("Execution: Step %" PRIu32, m_Blueprint->StepCount());
        auto result = m_Blueprint->Step();
        if (result == StepResult::Done)
            LOGI("Execution: Done (%" PRIu32 " steps)", m_Blueprint->StepCount());
        else if (result == StepResult::Error)
            LOGI("Execution: Failed at step %" PRIu32, m_Blueprint->StepCount());
    }

    void Blueprint_Stop()
    {
        LOGI("Execution: Stop");
        m_Blueprint->Stop();
    }

    void Blueprint_Run()
    {
        auto entryNode = FindEntryPointNode(*m_Blueprint);

        LOGI("Execution: Run");
        auto result = m_Blueprint->Execute(*entryNode);
        if (result == StepResult::Done)
            LOGI("Execution: Done (%" PRIu32 " steps)", m_Blueprint->StepCount());
        else if (result == StepResult::Error)
            LOGI("Execution: Failed at step %" PRIu32, m_Blueprint->StepCount());
    }

    crude_logger::OverlayLogger m_OverlayLogger;

    unique_ptr<Document> m_Document;
    Blueprint*           m_Blueprint = nullptr;
    bool                 m_showStyleEditor = false;

    CreateNodeDialog m_CreateNodeDailog;
    NodeContextMenu  m_NodeContextMenu;
    PinContextMenu   m_PinContextMenu;
    LinkContextMenu  m_LinkContextMenu;

    Action m_File_New        = { "New",          [this] { File_New();    } };
    Action m_File_Open       = { "Open...",      [this] { File_Open();   } };
    Action m_File_SaveAs     = { "Save As...",   [this] { File_SaveAs(); } };
    Action m_File_Save       = { "Save",         [this] { File_Save();   } };
    Action m_File_Close      = { "Close",        [this] { File_Close();  } };
    Action m_File_Exit       = { "Exit",         [this] { File_Exit();   } };

    Action m_Edit_Undo       = { "Undo",         [this] { Edit_Undo();      } };
    Action m_Edit_Redo       = { "Redo",         [this] { Edit_Redo();      } };
    Action m_Edit_Cut        = { "Cut",          [this] { Edit_Cut();       } };
    Action m_Edit_Copy       = { "Copy",         [this] { Edit_Copy();      } };
    Action m_Edit_Paste      = { "Paste",        [this] { Edit_Paste();     } };
    Action m_Edit_Duplicate  = { "Duplicate",    [this] { Edit_Duplicate(); } };
    Action m_Edit_Delete     = { "Delete",       [this] { Edit_Delete();    } };
    Action m_Edit_SelectAll  = { "Select All",   [this] { Edit_SelectAll(); } };
    Action m_Edit_Style      = { "Edit Style",   [this] { Edit_Style(); } };

    Action m_View_ShowFlow         = { "Show Flow",         [this] { View_ShowFlow();  } };
    Action m_View_ZoomToContent    = { "Zoom To Content",   [this] { View_ZoomToContent(); } };
    Action m_View_NavigateBackward = { "Navigate Backward", [this] { View_NavigateBackward(); } };
    Action m_View_NavigateForward  = { "Navigate Forward",  [this] { View_NavigateForward();  } };

    Action m_Blueprint_Start = { "Start",        [this] { Blueprint_Start(); } };
    Action m_Blueprint_Step  = { "Step",         [this] { Blueprint_Step();  } };
    Action m_Blueprint_Stop  = { "Stop",         [this] { Blueprint_Stop();  } };
    Action m_Blueprint_Run   = { "Run",          [this] { Blueprint_Run();   } };
};

const char* Application_GetName(void * handle)
{
    return "Blueprints";
}

void Application_Initialize(void** handle)
{
    static std::string setting_file = std::string(DEFAULT_CONFIG_PATH) + "Blueprints.json";
    static std::string bluepoint_file = std::string(DEFAULT_CONFIG_PATH) + "Blueprints_bp.json";
    static std::string ini_file = std::string(DEFAULT_CONFIG_PATH) + "Blueprints.ini";
    BlueprintEditorExample * example = new BlueprintEditorExample();
    example->Initialize(setting_file.c_str(), ini_file.c_str(), bluepoint_file.c_str());
    *handle = example;
}

void Application_Finalize(void** handle)
{
    BlueprintEditorExample * example = (BlueprintEditorExample *)*handle;
    if (!example)
        return;
    example->Finalize();
    delete example;
}

bool Application_Frame(void * handle)
{
    BlueprintEditorExample * example = (BlueprintEditorExample *)handle;
    if (!example)
        return true;
    return example->Frame();
}
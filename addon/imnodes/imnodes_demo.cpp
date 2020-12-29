#include "imnodes.h"
#include <imgui.h>
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui_internal.h>

#ifdef IMGUI_SDL2
#include <SDL_keycode.h>
#include <SDL_timer.h>
#elif !defined(__WIN32__) && !defined(_WIN32)
#include <sys/time.h>
inline int64_t GetTickCount(void) {
	int ret = 0;
	struct timeval tv;
	ret = gettimeofday(&tv, NULL);
	if (0 != ret) {
		return -1;
	}
	return (int64_t)tv.tv_usec + (int64_t)tv.tv_sec * 1000000;
}
#endif
#if defined(__WIN32__) || defined(_WIN32)
#include <windows.h>
#endif
#include <algorithm>
#include <stack>
#include <cassert>
#include <chrono>
#include <cmath>
#include <vector>
#include <fstream>


// for example
namespace imnodes_sample
{
template<typename ElementType>
struct Span
{
    using iterator = ElementType*;

    template<typename Container>
    Span(Container& c) : begin_(c.data()), end_(begin_ + c.size())
    {
    }

    iterator begin() const { return begin_; }
    iterator end() const { return end_; }

private:
    iterator begin_;
    iterator end_;
};

template<typename ElementType>
class IdMap
{
public:
    using iterator = typename std::vector<ElementType>::iterator;
    using const_iterator = typename std::vector<ElementType>::const_iterator;

    // Iterators

    const_iterator begin() const { return elements_.begin(); }
    const_iterator end() const { return elements_.end(); }

    // Element access

    Span<const ElementType> elements() const { return elements_; }

    // Capacity

    bool empty() const { return sorted_ids_.empty(); }
    size_t size() const { return sorted_ids_.size(); }

    // Modifiers

    std::pair<iterator, bool> insert(int id, const ElementType& element);
    std::pair<iterator, bool> insert(int id, ElementType&& element);
    size_t erase(int id);
    void clear();

    // Lookup

    iterator find(int id);
    const_iterator find(int id) const;
    bool contains(int id) const;

    // Load and Save
    void load(std::fstream& fin);
    void save(std::fstream& fout);

    std::vector<ElementType> elements_;
    std::vector<int> sorted_ids_;
};

template<typename ElementType>
std::pair<typename IdMap<ElementType>::iterator, bool> IdMap<ElementType>::insert(
    const int id,
    const ElementType& element)
{
    auto lower_bound = std::lower_bound(sorted_ids_.begin(), sorted_ids_.end(), id);

    if (lower_bound != sorted_ids_.end() && id == *lower_bound)
    {
        return std::make_pair(
            std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound)), false);
    }

    auto insert_element_at =
        std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound));

    sorted_ids_.insert(lower_bound, id);
    return std::make_pair(elements_.insert(insert_element_at, element), true);
}

template<typename ElementType>
std::pair<typename IdMap<ElementType>::iterator, bool> IdMap<ElementType>::insert(
    const int id,
    ElementType&& element)
{
    auto lower_bound = std::lower_bound(sorted_ids_.begin(), sorted_ids_.end(), id);

    if (lower_bound != sorted_ids_.end() && id == *lower_bound)
    {
        return std::make_pair(
            std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound)), false);
    }

    auto insert_element_at =
        std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound));

    sorted_ids_.insert(lower_bound, id);
    return std::make_pair(elements_.insert(insert_element_at, std::move(element)), true);
}

template<typename ElementType>
size_t IdMap<ElementType>::erase(const int id)
{
    auto lower_bound = std::lower_bound(sorted_ids_.begin(), sorted_ids_.end(), id);

    if (lower_bound == sorted_ids_.end() || id != *lower_bound)
    {
        return 0ull;
    }

    auto erase_element_at =
        std::next(elements_.begin(), std::distance(sorted_ids_.begin(), lower_bound));

    sorted_ids_.erase(lower_bound);
    elements_.erase(erase_element_at);

    return 1ull;
}

template<typename ElementType>
void IdMap<ElementType>::clear()
{
    elements_.clear();
    sorted_ids_.clear();
}

template<typename ElementType>
typename IdMap<ElementType>::iterator IdMap<ElementType>::find(const int id)
{
    const auto lower_bound = std::lower_bound(sorted_ids_.cbegin(), sorted_ids_.cend(), id);
    return (lower_bound == sorted_ids_.cend() || *lower_bound != id)
               ? elements_.end()
               : std::next(elements_.begin(), std::distance(sorted_ids_.cbegin(), lower_bound));
}

template<typename ElementType>
typename IdMap<ElementType>::const_iterator IdMap<ElementType>::find(const int id) const
{
    const auto lower_bound = std::lower_bound(sorted_ids_.cbegin(), sorted_ids_.cend(), id);
    return (lower_bound == sorted_ids_.cend() || *lower_bound != id)
               ? elements_.cend()
               : std::next(elements_.cbegin(), std::distance(sorted_ids_.cbegin(), lower_bound));
}

template<typename ElementType>
bool IdMap<ElementType>::contains(const int id) const
{
    const auto lower_bound = std::lower_bound(sorted_ids_.cbegin(), sorted_ids_.cend(), id);

    if (lower_bound == sorted_ids_.cend())
    {
        return false;
    }

    return *lower_bound == id;
}

template<typename ElementType>
void IdMap<ElementType>::load(std::fstream& fin)
{
    // copy elements into memory
    size_t num_elements;
    fin.read(reinterpret_cast<char*>(&num_elements), static_cast<std::streamsize>(sizeof(size_t)));
    elements_.resize(num_elements);
    size_t element_size = sizeof(ElementType);
    fin.read(
            reinterpret_cast<char*>(elements_.data()),
            static_cast<std::streamsize>(element_size * num_elements));
    // copy the sorted_ids_ into memory
    size_t num_ids;
    fin.read(reinterpret_cast<char*>(&num_ids), static_cast<std::streamsize>(sizeof(size_t)));
    sorted_ids_.resize(num_ids);
    fin.read(
            reinterpret_cast<char*>(sorted_ids_.data()),
            static_cast<std::streamsize>(sizeof(int) * num_ids));
}

template<typename ElementType>
void IdMap<ElementType>::save(std::fstream& fout)
{
    // copy the elements vector to file
    const size_t num_elements = elements_.size();
    const size_t element_size = sizeof(ElementType);
    fout.write(
            reinterpret_cast<const char*>(&num_elements),
            static_cast<std::streamsize>(sizeof(size_t)));
    fout.write(
            reinterpret_cast<const char*>(elements_.data()),
            static_cast<std::streamsize>(element_size * num_elements));
    // copy the sorted_ids_ vector to file
    const size_t num_sorted_ids = sorted_ids_.size();
    fout.write(
            reinterpret_cast<const char*>(&num_sorted_ids),
            static_cast<std::streamsize>(sizeof(size_t)));
    fout.write(
            reinterpret_cast<const char*>(sorted_ids_.data()),
            static_cast<std::streamsize>(sizeof(int) * num_sorted_ids));
}

// a very simple directional graph
template<typename NodeType>
class Graph
{
public:
    Graph() : current_id_(0), nodes_(), edges_from_node_(), node_neighbors_(), edges_() {}

    struct Edge
    {
        int id;
        int from, to;

        Edge() = default;
        Edge(const int id, const int f, const int t) : id(id), from(f), to(t) {}

        inline int opposite(const int n) const { return n == from ? to : from; }
        inline bool contains(const int n) const { return n == from || n == to; }
    };

    // Element access

    NodeType& node(int node_id);
    const NodeType& node(int node_id) const;
    Span<const int> neighbors(int node_id) const;
    Span<const Edge> edges() const;

    // Capacity

    size_t num_edges_from_node(int node_id) const;

    // Modifiers

    int insert_node(const NodeType& node);
    void erase_node(int node_id);

    int insert_edge(int from, int to);
    void erase_edge(int edge_id);

    // Load and Save
    void load(std::fstream & fin);
    void save(std::fstream &fout);

private:
    int current_id_;
    // These contains map to the node id
    IdMap<NodeType> nodes_;
    IdMap<int> edges_from_node_;
    IdMap<std::vector<int>> node_neighbors_;

    // This container maps to the edge id
    IdMap<Edge> edges_;
};

template<typename NodeType>
NodeType& Graph<NodeType>::node(const int id)
{
    return const_cast<NodeType&>(static_cast<const Graph*>(this)->node(id));
}

template<typename NodeType>
const NodeType& Graph<NodeType>::node(const int id) const
{
    const auto iter = nodes_.find(id);
    assert(iter != nodes_.end());
    return *iter;
}

template<typename NodeType>
Span<const int> Graph<NodeType>::neighbors(int node_id) const
{
    const auto iter = node_neighbors_.find(node_id);
    assert(iter != node_neighbors_.end());
    return *iter;
}

template<typename NodeType>
Span<const typename Graph<NodeType>::Edge> Graph<NodeType>::edges() const
{
    return edges_.elements();
}

template<typename NodeType>
size_t Graph<NodeType>::num_edges_from_node(const int id) const
{
    auto iter = edges_from_node_.find(id);
    assert(iter != edges_from_node_.end());
    return *iter;
}

template<typename NodeType>
int Graph<NodeType>::insert_node(const NodeType& node)
{
    const int id = current_id_++;
    assert(!nodes_.contains(id));
    nodes_.insert(id, node);
    edges_from_node_.insert(id, 0);
    node_neighbors_.insert(id, std::vector<int>());
    return id;
}

template<typename NodeType>
void Graph<NodeType>::erase_node(const int id)
{

    // first, remove any potential dangling edges
    {
        static std::vector<int> edges_to_erase;

        for (const Edge& edge : edges_.elements())
        {
            if (edge.contains(id))
            {
                edges_to_erase.push_back(edge.id);
            }
        }

        for (const int edge_id : edges_to_erase)
        {
            erase_edge(edge_id);
        }

        edges_to_erase.clear();
    }

    nodes_.erase(id);
    edges_from_node_.erase(id);
    node_neighbors_.erase(id);
}

template<typename NodeType>
int Graph<NodeType>::insert_edge(const int from, const int to)
{
    const int id = current_id_++;
    assert(!edges_.contains(id));
    assert(nodes_.contains(from));
    assert(nodes_.contains(to));
    edges_.insert(id, Edge(id, from, to));

    // update neighbor count
    assert(edges_from_node_.contains(from));
    *edges_from_node_.find(from) += 1;
    // update neighbor list
    assert(node_neighbors_.contains(from));
    node_neighbors_.find(from)->push_back(to);

    return id;
}

template<typename NodeType>
void Graph<NodeType>::erase_edge(const int edge_id)
{
    // This is a bit lazy, we find the pointer here, but we refind it when we erase the edge based
    // on id key.
    assert(edges_.contains(edge_id));
    const Edge& edge = *edges_.find(edge_id);

    // update neighbor count
    assert(edges_from_node_.contains(edge.from));
    int& edge_count = *edges_from_node_.find(edge.from);
    assert(edge_count > 0);
    edge_count -= 1;

    // update neighbor list
    {
        assert(node_neighbors_.contains(edge.from));
        auto neighbors = node_neighbors_.find(edge.from);
        auto iter = std::find(neighbors->begin(), neighbors->end(), edge.to);
        assert(iter != neighbors->end());
        neighbors->erase(iter);
    }

    edges_.erase(edge_id);
}

template<typename NodeType>
void Graph<NodeType>::load(std::fstream & fin)
{
    // copy the nodes into memory
    nodes_.load(fin);
    // copy the edges_from_node into memory
    edges_from_node_.load(fin);
    // copy the node_neighbors into memory
    size_t elements_size;
    fin.read(reinterpret_cast<char*>(&elements_size), static_cast<std::streamsize>(sizeof(size_t)));
    node_neighbors_.elements_.resize(elements_size);
    for (int i = 0; i < elements_size; i++)
    {
        size_t num_elements;
        fin.read(reinterpret_cast<char*>(&num_elements), static_cast<std::streamsize>(sizeof(size_t)));
        node_neighbors_.elements_[i].resize(num_elements);
        fin.read(
            reinterpret_cast<char*>(node_neighbors_.elements_[i].data()),
            static_cast<std::streamsize>(sizeof(int) * num_elements));
    }
    size_t num_sorted_ids;
    fin.read(reinterpret_cast<char*>(&num_sorted_ids), static_cast<std::streamsize>(sizeof(size_t)));
    node_neighbors_.sorted_ids_.resize(num_sorted_ids);
    fin.read(
            reinterpret_cast<char*>(node_neighbors_.sorted_ids_.data()),
            static_cast<std::streamsize>(sizeof(int) * num_sorted_ids));

    // copy the edges into memory
    edges_.load(fin);
    // copy current_id into memory
    fin.read(reinterpret_cast<char*>(&current_id_), static_cast<std::streamsize>(sizeof(int)));
}

template<typename NodeType>
void Graph<NodeType>::save(std::fstream &fout)
{
    // copy the nodes to file
    nodes_.save(fout);
    // copy the edges_from_node to file
    edges_from_node_.save(fout);
    // copy the node_neighbors to file
    const size_t elements_size = node_neighbors_.elements_.size();
    fout.write(
            reinterpret_cast<const char*>(&elements_size),
            static_cast<std::streamsize>(sizeof(size_t)));
    for (int i = 0; i < node_neighbors_.elements_.size(); i++)
    {
        const size_t num_elements = node_neighbors_.elements_[i].size();
        fout.write(
                reinterpret_cast<const char*>(&num_elements),
                static_cast<std::streamsize>(sizeof(size_t)));
        fout.write(
                reinterpret_cast<const char*>(node_neighbors_.elements_[i].data()),
                static_cast<std::streamsize>(sizeof(int) * num_elements));
    }
    const size_t num_sorted_ids = node_neighbors_.sorted_ids_.size();
    fout.write(
            reinterpret_cast<const char*>(&num_sorted_ids),
            static_cast<std::streamsize>(sizeof(size_t)));
    fout.write(
            reinterpret_cast<const char*>(node_neighbors_.sorted_ids_.data()),
            static_cast<std::streamsize>(sizeof(int) * num_sorted_ids));

    // copy the edges to file
    edges_.save(fout);
    // copy the current_id to file
    fout.write(reinterpret_cast<const char*>(&current_id_), static_cast<std::streamsize>(sizeof(int)));
}

template<typename NodeType, typename Visitor>
void dfs_traverse(const Graph<NodeType>& graph, const int start_node, Visitor visitor)
{
    std::stack<int> stack;

    stack.push(start_node);

    while (!stack.empty())
    {
        const int current_node = stack.top();
        stack.pop();

        visitor(current_node);

        for (const int neighbor : graph.neighbors(current_node))
        {
            stack.push(neighbor);
        }
    }
}
namespace
{
enum class NodeType
{
    add,
    sub,
    multiply,
    division,
    sine,
    cosine,
    output,
    time,
    value
};

struct Node
{
    NodeType type;
    float value;

    explicit Node(const NodeType t = NodeType::add) : type(t), value(0.f) {}

    Node(const NodeType t, const float v) : type(t), value(v) {}
};

template<class T>
T clamp(T x, T a, T b)
{
    return std::min(b, std::max(x, a));
}

static float current_time_seconds = 0.f;

ImU32 evaluate(const Graph<Node>& graph, const int root_node)
{
    std::stack<int> postorder;
    dfs_traverse(
        graph, root_node, [&postorder](const int node_id) -> void { postorder.push(node_id); });

    std::stack<float> value_stack;
    while (!postorder.empty())
    {
        const int id = postorder.top();
        postorder.pop();
        const Node node = graph.node(id);

        switch (node.type)
        {
        case NodeType::add:
        {
            const float rhs = value_stack.top();
            value_stack.pop();
            const float lhs = value_stack.top();
            value_stack.pop();
            value_stack.push(lhs + rhs);
        }
        break;
        case NodeType::sub:
        {
            const float rhs = value_stack.top();
            value_stack.pop();
            const float lhs = value_stack.top();
            value_stack.pop();
            value_stack.push(lhs - rhs);
        }
        break;
        case NodeType::multiply:
        {
            const float rhs = value_stack.top();
            value_stack.pop();
            const float lhs = value_stack.top();
            value_stack.pop();
            value_stack.push(rhs * lhs);
        }
        break;
        case NodeType::division:
        {
            const float rhs = value_stack.top();
            value_stack.pop();
            const float lhs = value_stack.top();
            value_stack.pop();
            value_stack.push(lhs != 0 ? rhs / lhs : 0);
        }
        break;
        case NodeType::sine:
        {
            const float x = value_stack.top();
            value_stack.pop();
            const float res = std::abs(std::sin(x));
            value_stack.push(res);
        }
        break;
        case NodeType::cosine:
        {
            const float x = value_stack.top();
            value_stack.pop();
            const float res = std::abs(std::cos(x));
            value_stack.push(res);
        }
        break;
        case NodeType::time:
        {
            value_stack.push(current_time_seconds);
        }
        break;
        case NodeType::value:
        {
            // If the edge does not have an edge connecting to another node, then just use the value
            // at this node. It means the node's input pin has not been connected to anything and
            // the value comes from the node's UI.
            if (graph.num_edges_from_node(id) == 0ull)
            {
                value_stack.push(node.value);
            }
        }
        break;
        default:
            break;
        }
    }

    // The final output node isn't evaluated in the loop -- instead we just pop
    // the three values which should be in the stack.
    assert(value_stack.size() == 3ull);
    const int b = static_cast<int>(255.f * clamp(value_stack.top(), 0.f, 1.f) + 0.5f);
    value_stack.pop();
    const int g = static_cast<int>(255.f * clamp(value_stack.top(), 0.f, 1.f) + 0.5f);
    value_stack.pop();
    const int r = static_cast<int>(255.f * clamp(value_stack.top(), 0.f, 1.f) + 0.5f);
    value_stack.pop();

    return IM_COL32(r, g, b, 255);
}

class ColorNodeEditor
{
public:
    ColorNodeEditor() : graph_(), nodes_(), root_node_id_(-1) {}

    void show()
    {
        // Update timer context
#if defined(__WIN32__) || defined(_WIN32) || !defined(IMGUI_SDL2)
#define SDL_SCANCODE_A 'A'
#define SDL_SCANCODE_X 'X'
        current_time_seconds = 0.001f * GetTickCount();
#else
        current_time_seconds = 0.001f * SDL_GetTicks();
#endif
        // The node editor window
        ImGui::Begin("color node editor");
        ImGui::TextUnformatted("Edit the color of the output color window using nodes.");
        ImGui::Columns(2);
        ImGui::TextUnformatted("A -- add node");
        ImGui::TextUnformatted("X -- delete selected node or link");
        ImGui::NextColumn();
        ImGui::Checkbox(
            "emulate three button mouse", &imnodes::GetIO().emulate_three_button_mouse.enabled);
        ImGui::Columns(1);

        imnodes::BeginNodeEditor();

        // Handle new nodes
        // These are driven by the user, so we place this code before rendering the nodes
        {
            const bool open_popup = ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows) &&
                                    imnodes::IsEditorHovered() &&
                                    ImGui::IsKeyReleased(SDL_SCANCODE_A);

            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(8.f, 8.f));
            if (!ImGui::IsAnyItemHovered() && open_popup)
            {
                ImGui::OpenPopup("add node");
            }

            if (ImGui::BeginPopup("add node"))
            {
                const ImVec2 click_pos = ImGui::GetMousePosOnOpeningCurrentPopup();

                if (ImGui::MenuItem("add"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::add);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::add;
                    ui_node.add.lhs = graph_.insert_node(value);
                    ui_node.add.rhs = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.add.lhs);
                    graph_.insert_edge(ui_node.id, ui_node.add.rhs);

                    nodes_.push_back(ui_node);
                    imnodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("sub"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::sub);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::sub;
                    ui_node.sub.lhs = graph_.insert_node(value);
                    ui_node.sub.rhs = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.sub.lhs);
                    graph_.insert_edge(ui_node.id, ui_node.sub.rhs);

                    nodes_.push_back(ui_node);
                    imnodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("multiply"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::multiply);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::multiply;
                    ui_node.multiply.lhs = graph_.insert_node(value);
                    ui_node.multiply.rhs = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.multiply.lhs);
                    graph_.insert_edge(ui_node.id, ui_node.multiply.rhs);

                    nodes_.push_back(ui_node);
                    imnodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("division"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::division);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::division;
                    ui_node.division.lhs = graph_.insert_node(value);
                    ui_node.division.rhs = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.division.lhs);
                    graph_.insert_edge(ui_node.id, ui_node.division.rhs);

                    nodes_.push_back(ui_node);
                    imnodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("sine"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::sine);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::sine;
                    ui_node.sine.input = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.sine.input);

                    nodes_.push_back(ui_node);
                    imnodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("cosine"))
                {
                    const Node value(NodeType::value, 0.f);
                    const Node op(NodeType::cosine);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::cosine;
                    ui_node.cosine.input = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(op);

                    graph_.insert_edge(ui_node.id, ui_node.cosine.input);

                    nodes_.push_back(ui_node);
                    imnodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("time"))
                {
                    UiNode ui_node;
                    ui_node.type = UiNodeType::time;
                    ui_node.id = graph_.insert_node(Node(NodeType::time));

                    nodes_.push_back(ui_node);
                    imnodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                }

                if (ImGui::MenuItem("output") && root_node_id_ == -1)
                {
                    const Node value(NodeType::value, 0.f);
                    const Node out(NodeType::output);

                    UiNode ui_node;
                    ui_node.type = UiNodeType::output;
                    ui_node.output.r = graph_.insert_node(value);
                    ui_node.output.g = graph_.insert_node(value);
                    ui_node.output.b = graph_.insert_node(value);
                    ui_node.id = graph_.insert_node(out);

                    graph_.insert_edge(ui_node.id, ui_node.output.r);
                    graph_.insert_edge(ui_node.id, ui_node.output.g);
                    graph_.insert_edge(ui_node.id, ui_node.output.b);

                    nodes_.push_back(ui_node);
                    imnodes::SetNodeScreenSpacePos(ui_node.id, click_pos);
                    root_node_id_ = ui_node.id;
                }

                ImGui::EndPopup();
            }
            ImGui::PopStyleVar();
        }

        for (const UiNode& node : nodes_)
        {
            switch (node.type)
            {
            case UiNodeType::add:
            {
                const float node_width = 100.f;
                imnodes::BeginNode(node.id);

                imnodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("add");
                imnodes::EndNodeTitleBar();
                {
                    imnodes::BeginInputAttribute(node.add.lhs);
                    const float label_width = ImGui::CalcTextSize("left").x;
                    ImGui::TextUnformatted("left");
                    if (graph_.num_edges_from_node(node.add.lhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat("##hidelabel", &graph_.node(node.add.lhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                {
                    imnodes::BeginInputAttribute(node.add.rhs);
                    const float label_width = ImGui::CalcTextSize("right").x;
                    ImGui::TextUnformatted("right");
                    if (graph_.num_edges_from_node(node.add.rhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat("##hidelabel", &graph_.node(node.add.rhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    imnodes::BeginOutputAttribute(node.id);
                    const float label_width = ImGui::CalcTextSize("result").x;
                    ImGui::Indent(node_width - label_width);
                    ImGui::TextUnformatted("result");
                    imnodes::EndOutputAttribute();
                }

                imnodes::EndNode();
            }
            break;
            case UiNodeType::sub:
            {
                const float node_width = 100.f;
                imnodes::BeginNode(node.id);

                imnodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("sub");
                imnodes::EndNodeTitleBar();
                {
                    imnodes::BeginInputAttribute(node.sub.lhs);
                    const float label_width = ImGui::CalcTextSize("left").x;
                    ImGui::TextUnformatted("left");
                    if (graph_.num_edges_from_node(node.sub.lhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat("##hidelabel", &graph_.node(node.sub.lhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                {
                    imnodes::BeginInputAttribute(node.sub.rhs);
                    const float label_width = ImGui::CalcTextSize("right").x;
                    ImGui::TextUnformatted("right");
                    if (graph_.num_edges_from_node(node.sub.rhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat("##hidelabel", &graph_.node(node.sub.rhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    imnodes::BeginOutputAttribute(node.id);
                    const float label_width = ImGui::CalcTextSize("result").x;
                    ImGui::Indent(node_width - label_width);
                    ImGui::TextUnformatted("result");
                    imnodes::EndOutputAttribute();
                }

                imnodes::EndNode();
            }
            break;
            case UiNodeType::multiply:
            {
                const float node_width = 100.0f;
                imnodes::BeginNode(node.id);

                imnodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("multiply");
                imnodes::EndNodeTitleBar();

                {
                    imnodes::BeginInputAttribute(node.multiply.lhs);
                    const float label_width = ImGui::CalcTextSize("left").x;
                    ImGui::TextUnformatted("left");
                    if (graph_.num_edges_from_node(node.multiply.lhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.multiply.lhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                {
                    imnodes::BeginInputAttribute(node.multiply.rhs);
                    const float label_width = ImGui::CalcTextSize("right").x;
                    ImGui::TextUnformatted("right");
                    if (graph_.num_edges_from_node(node.multiply.rhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.multiply.rhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    imnodes::BeginOutputAttribute(node.id);
                    const float label_width = ImGui::CalcTextSize("result").x;
                    ImGui::Indent(node_width - label_width);
                    ImGui::TextUnformatted("result");
                    imnodes::EndOutputAttribute();
                }

                imnodes::EndNode();
            }
            break;
            case UiNodeType::division:
            {
                const float node_width = 100.0f;
                imnodes::BeginNode(node.id);

                imnodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("division");
                imnodes::EndNodeTitleBar();

                {
                    imnodes::BeginInputAttribute(node.division.lhs);
                    const float label_width = ImGui::CalcTextSize("left").x;
                    ImGui::TextUnformatted("left");
                    if (graph_.num_edges_from_node(node.division.lhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.division.lhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                {
                    imnodes::BeginInputAttribute(node.division.rhs);
                    const float label_width = ImGui::CalcTextSize("right").x;
                    ImGui::TextUnformatted("right");
                    if (graph_.num_edges_from_node(node.division.rhs) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.division.rhs).value, 0.01f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    imnodes::BeginOutputAttribute(node.id);
                    const float label_width = ImGui::CalcTextSize("result").x;
                    ImGui::Indent(node_width - label_width);
                    ImGui::TextUnformatted("result");
                    imnodes::EndOutputAttribute();
                }

                imnodes::EndNode();
            }
            break;
            case UiNodeType::sine:
            {
                const float node_width = 100.0f;
                imnodes::BeginNode(node.id);

                imnodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("sine");
                imnodes::EndNodeTitleBar();

                {
                    imnodes::BeginInputAttribute(node.sine.input);
                    const float label_width = ImGui::CalcTextSize("number").x;
                    ImGui::TextUnformatted("number");
                    if (graph_.num_edges_from_node(node.sine.input) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.sine.input).value, 0.01f, 0.f, 1.0f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    imnodes::BeginOutputAttribute(node.id);
                    const float label_width = ImGui::CalcTextSize("output").x;
                    ImGui::Indent(node_width - label_width);
                    ImGui::TextUnformatted("output");
                    imnodes::EndInputAttribute();
                }

                imnodes::EndNode();
            }
            break;
            case UiNodeType::cosine:
            {
                const float node_width = 100.0f;
                imnodes::BeginNode(node.id);

                imnodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("cosine");
                imnodes::EndNodeTitleBar();

                {
                    imnodes::BeginInputAttribute(node.cosine.input);
                    const float label_width = ImGui::CalcTextSize("number").x;
                    ImGui::TextUnformatted("number");
                    if (graph_.num_edges_from_node(node.cosine.input) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.cosine.input).value, 0.01f, 0.f, 1.0f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    imnodes::BeginOutputAttribute(node.id);
                    const float label_width = ImGui::CalcTextSize("output").x;
                    ImGui::Indent(node_width - label_width);
                    ImGui::TextUnformatted("output");
                    imnodes::EndInputAttribute();
                }

                imnodes::EndNode();
            }
            break;
            case UiNodeType::time:
            {
                imnodes::BeginNode(node.id);

                imnodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("time");
                imnodes::EndNodeTitleBar();

                imnodes::BeginOutputAttribute(node.id);
                ImGui::Text("output");
                imnodes::EndOutputAttribute();

                imnodes::EndNode();
            }
            break;
            case UiNodeType::output:
            {
                const float node_width = 200.0f;
                imnodes::PushColorStyle(imnodes::ColorStyle_TitleBar, IM_COL32(11, 109, 191, 255));
                imnodes::PushColorStyle(
                    imnodes::ColorStyle_TitleBarHovered, IM_COL32(45, 126, 194, 255));
                imnodes::PushColorStyle(
                    imnodes::ColorStyle_TitleBarSelected, IM_COL32(81, 148, 204, 255));
                imnodes::BeginNode(node.id);

                imnodes::BeginNodeTitleBar();
                ImGui::TextUnformatted("output");
                imnodes::EndNodeTitleBar();

                ImGui::Dummy(ImVec2(node_width, 0.f));
                {
                    imnodes::BeginInputAttribute(node.output.r);
                    const float label_width = ImGui::CalcTextSize("r").x;
                    ImGui::TextUnformatted("r");
                    if (graph_.num_edges_from_node(node.output.r) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width / 3 - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.output.r).value, 0.01f, 0.f, 1.0f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    imnodes::BeginInputAttribute(node.output.g);
                    const float label_width = ImGui::CalcTextSize("g").x;
                    ImGui::TextUnformatted("g");
                    if (graph_.num_edges_from_node(node.output.g) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width / 3 - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.output.g).value, 0.01f, 0.f, 1.f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                ImGui::Spacing();

                {
                    imnodes::BeginInputAttribute(node.output.b);
                    const float label_width = ImGui::CalcTextSize("b").x;
                    ImGui::TextUnformatted("b");
                    if (graph_.num_edges_from_node(node.output.b) == 0ull)
                    {
                        ImGui::SameLine();
                        ImGui::PushItemWidth(node_width / 3 - label_width);
                        ImGui::DragFloat(
                            "##hidelabel", &graph_.node(node.output.b).value, 0.01f, 0.f, 1.0f);
                        ImGui::PopItemWidth();
                    }
                    imnodes::EndInputAttribute();
                }

                {
                    const ImU32 color = root_node_id_ != -1 ? evaluate(graph_, root_node_id_) : IM_COL32(255, 20, 147, 255);
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    ImVec2 p = ImGui::GetCursorScreenPos();
                    draw_list->AddRectFilled(ImVec2(p.x + 120, p.y - 80), ImVec2(p.x + 200, p.y), color);   
                }

                imnodes::EndNode();
                imnodes::PopColorStyle();
                imnodes::PopColorStyle();
                imnodes::PopColorStyle();
            }
            break;
            }
        }

        for (const auto& edge : graph_.edges())
        {
            // If edge doesn't start at value, then it's an internal edge, i.e.
            // an edge which links a node's operation to its input. We don't
            // want to render node internals with visible links.
            if (graph_.node(edge.from).type != NodeType::value)
                continue;

            imnodes::Link(edge.id, edge.from, edge.to);
        }

        imnodes::EndNodeEditor();

        // Handle new links
        // These are driven by Imnodes, so we place the code after EndNodeEditor().

        {
            int start_attr, end_attr;
            if (imnodes::IsLinkCreated(&start_attr, &end_attr))
            {
                const NodeType start_type = graph_.node(start_attr).type;
                const NodeType end_type = graph_.node(end_attr).type;

                const bool valid_link = start_type != end_type;
                if (valid_link)
                {
                    // Ensure the edge is always directed from the value to
                    // whatever produces the value
                    if (start_type != NodeType::value)
                    {
                        std::swap(start_attr, end_attr);
                    }
                    graph_.insert_edge(start_attr, end_attr);
                }
            }
        }

        // Handle deleted links

        {
            int link_id;
            if (imnodes::IsLinkDestroyed(&link_id))
            {
                graph_.erase_edge(link_id);
            }
        }

        {
            const int num_selected = imnodes::NumSelectedLinks();
            if (num_selected > 0 && ImGui::IsKeyReleased(SDL_SCANCODE_X))
            {
                static std::vector<int> selected_links;
                selected_links.resize(static_cast<size_t>(num_selected));
                imnodes::GetSelectedLinks(selected_links.data());
                for (const int edge_id : selected_links)
                {
                    graph_.erase_edge(edge_id);
                }
            }
        }

        {
            const int num_selected = imnodes::NumSelectedNodes();
            if (num_selected > 0 && ImGui::IsKeyReleased(SDL_SCANCODE_X))
            {
                static std::vector<int> selected_nodes;
                selected_nodes.resize(static_cast<size_t>(num_selected));
                imnodes::GetSelectedNodes(selected_nodes.data());
                for (const int node_id : selected_nodes)
                {
                    graph_.erase_node(node_id);
                    auto iter = std::find_if(
                        nodes_.begin(), nodes_.end(), [node_id](const UiNode& node) -> bool {
                            return node.id == node_id;
                        });
                    // Erase any additional internal nodes
                    switch (iter->type)
                    {
                    case UiNodeType::add:
                        graph_.erase_node(iter->add.lhs);
                        graph_.erase_node(iter->add.rhs);
                        break;
                    case UiNodeType::sub:
                        graph_.erase_node(iter->sub.lhs);
                        graph_.erase_node(iter->sub.rhs);
                        break;
                    case UiNodeType::multiply:
                        graph_.erase_node(iter->multiply.lhs);
                        graph_.erase_node(iter->multiply.rhs);
                        break;
                    case UiNodeType::division:
                        graph_.erase_node(iter->division.lhs);
                        graph_.erase_node(iter->division.rhs);
                        break;
                    case UiNodeType::sine:
                        graph_.erase_node(iter->sine.input);
                        break;
                    case UiNodeType::cosine:
                        graph_.erase_node(iter->cosine.input);
                        break;
                    case UiNodeType::output:
                        graph_.erase_node(iter->output.r);
                        graph_.erase_node(iter->output.g);
                        graph_.erase_node(iter->output.b);
                        root_node_id_ = -1;
                        break;
                    default:
                        break;
                    }
                    nodes_.erase(iter);
                }
            }
        }

        ImGui::End();
    }

    void save()
    {
        // Save the internal imnodes state
        imnodes::SaveCurrentEditorStateToIniFile("nodes_save_load.ini");
        // Dump our editor state as bytes into a file
        std::fstream fout(
            "nodes_save_load.node", std::ios_base::out | std::ios_base::binary | std::ios_base::trunc);
        // copy the node vector to file
        const size_t num_nodes = nodes_.size();
        fout.write(
            reinterpret_cast<const char*>(&num_nodes),
            static_cast<std::streamsize>(sizeof(size_t)));
        fout.write(
            reinterpret_cast<const char*>(nodes_.data()),
            static_cast<std::streamsize>(sizeof(UiNode) * num_nodes));
        // Save graph_ to File
        graph_.save(fout);
        // copy the current_id to file
        fout.write(
            reinterpret_cast<const char*>(&root_node_id_), static_cast<std::streamsize>(sizeof(int)));
    }

    void load()
    {
        // Load the internal imnodes state
        imnodes::LoadCurrentEditorStateFromIniFile("nodes_save_load.ini");
        // Load our editor state into memory
        std::fstream fin("nodes_save_load.node", std::ios_base::in | std::ios_base::binary);
        if (!fin.is_open())
        {
            return;
        }
        // copy nodes into memory
        size_t num_nodes;
        fin.read(reinterpret_cast<char*>(&num_nodes), static_cast<std::streamsize>(sizeof(size_t)));
        nodes_.resize(num_nodes);
        fin.read(
            reinterpret_cast<char*>(nodes_.data()),
            static_cast<std::streamsize>(sizeof(UiNode) * num_nodes));
        // Load graph_ into Memory
        graph_.load(fin);
        // copy current_id into memory
        fin.read(reinterpret_cast<char*>(&root_node_id_), static_cast<std::streamsize>(sizeof(int)));
    }
private:
    enum class UiNodeType
    {
        add,
        sub,
        multiply,
        division,
        sine,
        cosine,
        time,
        output,
    };

    struct UiNode
    {
        UiNodeType type;
        // The identifying id of the ui node. For add, multiply, sine, and time
        // this is the "operation" node id. The additional input nodes are
        // stored in the structs.
        int id;

        union
        {
            struct
            {
                int lhs, rhs;
            } add;

            struct
            {
                int lhs, rhs;
            } sub;
            struct
            {
                int lhs, rhs;
            } multiply;
            struct
            {
                int lhs, rhs;
            } division;
            struct
            {
                int input;
            } sine;
            struct
            {
                int input;
            } cosine;
            struct
            {
                int r, g, b;
            } output;
        };
    };

    Graph<Node> graph_;
    std::vector<UiNode> nodes_;
    int root_node_id_;
};

static ColorNodeEditor color_editor;
} // namespace

void NodeEditorInitialize()
{
    imnodes::IO& io = imnodes::GetIO();
    io.link_detach_with_modifier_click.modifier = &ImGui::GetIO().KeyCtrl;
    color_editor.load();
}

void NodeEditorShow() { color_editor.show(); }

void NodeEditorShutdown() {
    color_editor.save();
}
} // namespace imnodes_sample

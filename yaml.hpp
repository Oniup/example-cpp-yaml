#ifndef __YAML_HPP__
#define __YAML_HPP__

#include <cassert>
#include <cstdint>
#include <string>
#include <typeinfo>
#include <vector>

namespace yaml {

template<typename _T>
struct TypeInfo {
    std::size_t get_hash() { return typeid(_T).hash_code(); }
    std::string get_name() { return typeid(_T).name(); }
};

class Node {
  public:
    static const std::size_t null_index;
    static Node open(const std::string& filename);
    inline static constexpr std::size_t max_line_size() { return 2048; }
    inline static constexpr std::size_t max_name_size() { return 100; }
    inline static constexpr std::size_t max_value_size() { return 1948; }

  public:
    Node() = default;
    Node(const std::string& field_name);
    Node(const std::string& field_name, const std::string& value);
    Node(const Node& other);
    Node(Node&& other);
    ~Node() = default;

    Node& operator=(const Node& other);
    Node& operator=(Node&& other);
    Node& operator<<(const Node& other);

    inline Node& operator[](const std::string& field_name) { return get_child(field_name); }
    inline Node& operator[](const std::string& field_name) const { return get_child(field_name); }
    inline Node& operator[](std::size_t index) { return get_child(index); }
    inline Node& operator[](std::size_t index) const { return get_child(index); }

    inline bool operator==(const Node& other) { return compare(other); }
    inline bool operator!=(const Node& other) { return !(*this == other); }

    inline const std::string& get_name() const { return m_name; }
    inline const std::string& get_value() const { return m_value; }
    inline const std::vector<Node>& get_children() const { return m_children; }
    inline const Node* get_parent() const { return m_parent; }
    inline std::size_t get_type_hash() const { return m_hash; }

    Node& get_child(const std::string& field_name);
    Node& get_child(std::size_t index);
    inline Node& get_child(const std::string& field_name) const { return get_child(field_name); }
    inline Node& get_child(std::size_t index) const { return get_child(index); }

    std::string get_as_string() const;

    inline void set_parent(Node* parent) { m_parent = parent; }
    inline void set_hash(std::size_t type_hash) { m_hash = type_hash; }

    inline bool empty() const { return m_name.size() > 0; }
    inline void push_back(const Node& node) { m_children.push_back(node); }
    inline void push_back(Node&& node) { m_children.push_back(std::move(node)); }
    inline void pop_back(std::size_t count = 1) { m_children.resize(m_children.size() - count); }
    bool compare(const Node& other) const;
    std::size_t exists(const std::string& field_name) const;

    bool write_file(std::FILE* file) const;
    bool write_file(const std::string& filename) const;
    bool write_if_file_exists(const std::string& filename) const;

  private:
    static void _construct_string(std::string& str, const Node& node, std::size_t indent);
    static bool _write_node(std::FILE* file, const Node& node, std::size_t indent);
    static void _read_node(std::FILE* file, Node* last, std::size_t last_indent_size);
    static bool _read_line(
        std::FILE* file, char* name, char* value, std::size_t& name_size, std::size_t& value_size,
        std::size_t& indent_size
    );

  private:
    std::string m_name = {};
    std::string m_value = {};
    std::vector<Node> m_children = {};
    Node* m_parent = nullptr;
    std::size_t m_hash = 0;
};

template<typename _T>
struct Convert {
    std::string value_to_str(const _T& value);
    _T value(const Node& node);
};

template<typename _T>
struct Convert<std::vector<_T>> {
    std::string value_to_str(const std::vector<_T>& value) { return ""; }
    std::vector<_T> value(const Node& node) { return {}; }
};

template<>
struct Convert<bool> {
    std::string value_to_str(const bool& value) { return std::to_string(value); }

    bool value(const Node& node) {
        if (node.get_value() == "true") {
            return true;
        } else {
            return false;
        }
    }
};

template<>
struct Convert<std::int16_t> {
    std::int16_t value(const Node& node) { return std::stoi(node.get_value()); }
    std::string value_to_str(const std::int16_t& value) { return std::to_string(value); }
};

template<>
struct Convert<std::int32_t> {
    std::int32_t value(const Node& node) { return std::stoi(node.get_value()); }
    std::string value_to_str(const std::int32_t& value) { return std::to_string(value); }
};

template<>
struct Convert<std::int64_t> {
    std::int64_t value(const Node& node) { return std::stoll(node.get_value()); }
    std::string value_to_str(const std::int64_t& value) { return std::to_string(value); }
};

template<>
struct Convert<std::uint16_t> {
    std::uint16_t value(const Node& node) { return std::stoul(node.get_value()); }
    std::string value_to_str(const std::uint16_t& value) { return std::to_string(value); }
};

template<>
struct Convert<std::uint32_t> {
    std::uint32_t value(const Node& node) { return std::stoul(node.get_value()); }
    std::string value_to_str(const std::uint32_t& value) { return std::to_string(value); }
};

template<>
struct Convert<std::size_t> {
    std::size_t value(const Node& node) { return std::stoull(node.get_value()); }
    std::string value_to_str(const std::size_t& value) { return std::to_string(value); }
};

template<>
struct Convert<float> {
    float value(const Node& node) { return std::stof(node.get_value()); }
    std::string value_to_str(const float& value) { return std::to_string(value); }
};

template<>
struct Convert<double> {
    double value(const Node& node) { return std::stod(node.get_value()); }
    std::string value_to_str(const double& value) { return std::to_string(value); }
};

template<>
struct Convert<long double> {
    long double value(const Node& node) { return std::stold(node.get_value()); }
    std::string value_to_str(const long double& value) { return std::to_string(value); }
};

template<>
struct Convert<char*> {
    char* value(const Node& node) { return const_cast<char*>(node.get_value().c_str()); }
    std::string value_to_str(const char*& value) { return std::string(value); }
};

template<>
struct Convert<std::string> {
    std::string value(const Node& node) { return node.get_value(); }
    std::string value_to_str(const std::string& value) { return std::string(value); }
};

template<typename _T>
Node node(const std::string& field_name, const _T& value) {
    Node node = Node(field_name, Convert<_T>().value_to_str(value));

    assert(
        node.get_name().size() < Node::max_name_size() &&
        "YAML ASSERT: node name cannot exceed the max name size"
    );
    assert(
        node.get_value().size() < Node::max_value_size() &&
        "YAML ASSERT: node variable formatted into string cannot exceed max size"
    );

    node.set_hash(TypeInfo<_T>().get_hash());
    return node;
}

inline Node node(const std::string& field_name) {
    assert(
        field_name.size() < Node::max_name_size() &&
        "YAML ASSERT: node name cannot exceed the max name size"
    );
    return Node(field_name);
}

} // namespace yaml

#endif

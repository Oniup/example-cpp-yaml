#ifndef __YAML_HPP__
#define __YAML_HPP__

#include <cassert>
#include <cstdint>
#include <string>
#include <typeinfo>
#include <vector>

namespace yaml {

template<typename _T>
struct Convert {
    std::string value_to_str(const _T& value);
    _T value(const std::string& str);

    _T value(const class Node& node);
};

class Node {
  public:
    static const std::size_t null_index;
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

    template<typename _T>
    Node& operator=(const _T& value) {
        m_value = Convert<_T>().value_to_str(value);
        return *this;
    }

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

    inline std::vector<Node>& get_children() { return m_children; }
    inline Node* get_parent() { return m_parent; }

    Node& get_child(const std::string& field_name);
    Node& get_child(std::size_t index);
    inline Node& get_child(const std::string& field_name) const { return get_child(field_name); }
    inline Node& get_child(std::size_t index) const { return get_child(index); }

    template<typename _T>
    _T as() {
        return Convert<_T>().value(m_value);
    }

    std::string get_as_string() const;

    inline void set_parent(Node* parent) { m_parent = parent; }
    inline void set_hash(std::size_t type_hash) { m_hash = type_hash; }

    bool open(const std::string& filename);
    inline bool empty() const { return m_children.size() == 0; }
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

Node& get_root_node(Node& node);
const Node& get_root_node(const Node& node);
Node open(const std::string& filename);

inline bool write(const Node& node, std::FILE* file) {
    return get_root_node(node).write_file(file);
}

inline bool write(const Node& node, const std::string& filename) {
    return get_root_node(node).write_file(filename);
}

inline bool write_if_exists(const Node& node, const std::string& filename) {
    return get_root_node(node).write_if_file_exists(filename);
}

inline std::string get_root_as_string(Node& node) { return get_root_node(node).get_as_string(); }

template<typename _T>
struct Convert<std::vector<_T>> {
    std::string value_to_str(const std::vector<_T>& value) {
        std::string str = "[";

        for (std::size_t i = 0; i < value.size(); i++) {
            str += Convert<_T>().value_to_str(value[i]);
            if (i < value.size() - 1) {
                str += ", ";
            }
        }

        str += "]";
        return str;
    }

    std::vector<_T> value(const std::string& str) {
        std::vector<_T> vec = {};

        bool fill_variable = false;
        bool is_filling_string = false;
        char value[Node::max_value_size()];
        std::size_t j = 0;

        for (std::size_t i = 0; i < str.size(); i++) {
            switch (str[i]) {
            case '\r':
                continue;
            case ' ':
                if (!fill_variable) {
                    continue;
                }
                break;

            case '[':
                continue;
            case ']':
                continue;
            case '\\':
                // NOTE: to still allow ',' when its supposed to be in the string returned
                if (str[i + 1] == '\"') {
                    is_filling_string = !is_filling_string;
                }
                break;
            case ',':
                if (!is_filling_string) {
                    value[j] = '\0';
                    vec.push_back(Convert<_T>().value(value));
                    j = 0;
                    continue;
                }
                break;
            }

            value[j] = str[i];
            j++;
        }

        if (j > 0) {
            value[j] = '\0';
            vec.push_back(Convert<_T>().value(value));
        }

        return vec;
    }

    std::vector<_T> value(const Node& node) { return value(node.get_value()); }
};

template<>
struct Convert<bool> {
    std::string value_to_str(const bool& value) { return std::to_string(value); }
    bool value(const Node& node) { return value(node.get_value()); }

    bool value(const std::string& str) {
        if (str == "true") {
            return true;
        } else {
            return false;
        }
    }
};

template<>
struct Convert<std::int16_t> {
    std::int16_t value(const std::string& str) { return std::stoi(str); }
    std::int16_t value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const std::int16_t& value) { return std::to_string(value); }
};

template<>
struct Convert<std::int32_t> {
    std::int32_t value(const std::string& str) { return std::stoi(str); }
    std::int32_t value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const std::int32_t& value) { return std::to_string(value); }
};

template<>
struct Convert<std::int64_t> {
    std::int64_t value(const std::string& str) { return std::stoll(str); }
    std::int64_t value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const std::int64_t& value) { return std::to_string(value); }
};

template<>
struct Convert<std::uint16_t> {
    std::uint16_t value(const std::string& str) { return std::stoul(str); }
    std::uint16_t value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const std::uint16_t& value) { return std::to_string(value); }
};

template<>
struct Convert<std::uint32_t> {
    std::uint32_t value(const std::string& str) { return std::stoul(str); }
    std::uint32_t value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const std::uint32_t& value) { return std::to_string(value); }
};

template<>
struct Convert<std::size_t> {
    std::size_t value(const std::string& str) { return std::stoull(str); }
    std::size_t value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const std::size_t& value) { return std::to_string(value); }
};

template<>
struct Convert<float> {
    float value(const std::string& str) { return std::stof(str); }
    float value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const float& value) { return std::to_string(value); }
};

template<>
struct Convert<double> {
    double value(const std::string& str) { return std::stod(str); }
    double value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const double& value) { return std::to_string(value); }
};

template<>
struct Convert<long double> {
    long double value(const std::string& str) { return std::stold(str); }
    long double value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const long double& value) { return std::to_string(value); }
};

template<>
struct Convert<char*> {
    char* value(const std::string& str) { return const_cast<char*>(str.c_str()); }
    char* value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const char*& value) { return std::string(value); }
};

template<>
struct Convert<char> {
    char value(const std::string& str) { return str[1]; }
    char value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const char*& value) { return std::string(value); }
};

template<>
struct Convert<std::string> {
    std::string value(const std::string& str) {
        return std::string(str.c_str() + 1, str.size() - 2);
    }

    std::string value(const Node& node) { return value(node.get_value()); }
    std::string value_to_str(const std::string& value) {
        if (value[0] == '\"' && value[value.size() - 1] == '\"') {
            return value;
        } else {
            return std::string("\"" + value + "\"");
        }
    }
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

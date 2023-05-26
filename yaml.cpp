#include "yaml.hpp"
#include <cassert>
#include <cinttypes>
#include <iostream>

namespace yaml {

const std::size_t Node::null_index = std::string::npos;

// TODO: finish basic writing useage first
Node Node::open(const std::string& filename) { return Node(); }

Node::Node(const std::string& field_name) : m_name(field_name) {}
Node::Node(const std::string& field_name, const std::string& value)
    : m_name(field_name), m_value(value) {}

Node::Node(const Node& other)
    : m_name(other.m_name),
      m_value(other.m_value),
      m_children(other.m_children),
      m_parent(other.m_parent),
      m_hash(other.m_hash) {}

Node::Node(Node&& other)
    : m_name(std::move(other.m_name)),
      m_value(std::move(other.m_value)),
      m_children(std::move(other.m_children)),
      m_parent(other.m_parent),
      m_hash(other.m_hash) {
    other.m_parent = nullptr;
    other.m_hash = 0;
}

Node& Node::operator=(const Node& other) {
    m_name = other.m_name;
    m_value = other.m_value;
    m_children = other.m_children;
    m_parent = other.m_parent;
    m_hash = other.m_hash;

    return *this;
}

Node& Node::operator=(Node&& other) {
    m_name = std::move(other.m_name);
    m_value = std::move(other.m_value);
    m_children = std::move(other.m_children);
    m_parent = other.m_parent;
    m_hash = other.m_hash;

    other.m_parent = nullptr;
    other.m_hash = 0;

    return *this;
}

Node& Node::get_child(const std::string& field_name) {
    for (std::size_t i = 0; i < m_children.size(); i++) {
        if (m_children[i].get_name() == field_name) {
            return m_children[i];
        }
    }

    assert(
        false && "YAML ASSERT: failed to find child with given field_name as it doesn't exist in "
                 "children vector"
    );
}

Node& Node::get_child(std::size_t index) {
    if (index < m_children.size()) {
        return m_children[index];
    }

    assert(
        false &&
        "YAML ASSERT: failed to find child with given index as it is greater than children vector"
    );
}

Node& Node::operator<<(const Node& other) {
    push_back(other);
    m_children.back().m_parent = m_parent;
    return *this;
}

std::string Node::get_as_string() const {
    std::string str = {};
    _construct_string(str, *this, 0);
    return str;
}

bool Node::compare(const Node& other) {
    if (m_name != other.m_name || m_value != other.m_value || m_parent != other.m_parent) {
        return false;
    }

    for (std::size_t i = 0; i < m_children.size(); i++) {
        if (!m_children[i].compare(other.m_children[i])) {
            return false;
        }
    }

    return true;
}

std::size_t Node::exists(const std::string& field_name) {
    for (std::size_t i = 0; i < m_children.size(); i++) {
        if (m_children[i].get_name() == field_name) {
            return i;
        }
    }

    return null_index;
}

bool Node::write_file(std::FILE* file) const {
    for (std::size_t i = 0; i < m_children.size(); i++) {
        bool result = _write_node(file, m_children[i], 0);
        if (!result) {
            return false;
        }
    }
    return true;
}

bool Node::write_file(const std::string& filename) const {
    std::FILE* file = std::fopen(filename.c_str(), "w");
    bool result = write_file(file);
    std::fclose(file);
    return result;
}

bool Node::write_if_file_exists(const std::string& filename) const {
    std::FILE* file = std::fopen(filename.c_str(), "r");
    if (file != nullptr) {
        std::fclose(file);
        return write_file(filename);
    }
    return false;
}

void Node::_construct_string(std::string& str, const Node& node, std::size_t indent) {
    char str_indent[1024] = {};
    for (std::size_t i = 0; i < indent; i++) {
        str_indent[i] = ' ';
    }
    str_indent[indent] = '\0';

    if (node.get_name().size() > 0) {
        str += str_indent + node.get_name() + ": ";
        if (node.m_value.size() > 0) {
            str += node.get_value() + "\n";
        } else {
            str += "\n";
            for (std::size_t i = 0; i < node.m_children.size(); i++) {
                _construct_string(str, node.m_children[i], indent + 2);
            }
        }
    } else {
        for (std::size_t i = 0; i < node.m_children.size(); i++) {
            _construct_string(str, node.m_children[i], indent);
        }
    }
}

bool Node::_write_node(std::FILE* file, const Node& node, std::size_t indent) {
    char str_indent[1024] = {};
    for (std::size_t i = 0; i < indent; i++) {
        str_indent[i] = ' ';
    }
    str_indent[indent] = '\0';

    // write node with value into file
    if (node.get_type_hash() != 0) {
        if (node.get_type_hash() == TypeInfo<std::string>().get_hash() ||
            node.get_type_hash() == TypeInfo<const char*>().get_hash()) {

            std::fputs(
                std::string(str_indent + node.get_name() + ": \"" + node.get_value() + "\"\n")
                    .c_str(),
                file
            );
        } else {
            std::fputs(
                std::string(str_indent + node.get_name() + ": " + node.get_value() + "\n").c_str(), file
            );
        }
    } else {
        std::fputs(std::string(str_indent + node.get_name() + ":\n").c_str(), file);
        for (std::size_t i = 0; i < node.m_children.size(); i++) {
            bool result = _write_node(file, node.m_children[i], indent + 2);
            if (!result) {
                return false;
            }
        }
    }

    return true;
}

} // namespace yaml

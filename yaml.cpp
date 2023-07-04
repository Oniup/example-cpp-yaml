/**
 * @file yaml.cpp
 * @copyright Copyright (c) 2023-present Ewan Robson.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include "yaml.hpp"

#include <cassert>
#include <cinttypes>
#include <iostream>
#include <string.h>

namespace yaml {

const std::size_t Node::null_index = std::string::npos;

Node::Node(const std::string& field_name) : m_name(field_name) {}
Node::Node(const std::string& field_name, const std::string& value)
    : m_name(field_name), m_value(value)
{
}

Node::Node(const Node& other)
    : m_name(other.m_name),
      m_value(other.m_value),
      m_children(other.m_children),
      m_parent(other.m_parent)
{
}

Node::Node(Node&& other)
    : m_name(std::move(other.m_name)),
      m_value(std::move(other.m_value)),
      m_children(std::move(other.m_children)),
      m_parent(other.m_parent)
{
    other.m_parent = nullptr;
}

Node& Node::operator=(const Node& other)
{
    m_name = other.m_name;
    m_value = other.m_value;
    m_children = other.m_children;
    m_parent = other.m_parent;

    return *this;
}

Node& Node::operator=(Node&& other)
{
    m_name = std::move(other.m_name);
    m_value = std::move(other.m_value);
    m_children = std::move(other.m_children);
    m_parent = other.m_parent;

    other.m_parent = nullptr;

    return *this;
}

Node& Node::get_child(const std::string& field_name)
{
    for (std::size_t i = 0; i < m_children.size(); i++)
    {
        if (m_children[i].get_name() == field_name)
            return m_children[i];
    }

    assert(
        false && "YAML ASSERT: failed to find child with given field_name as it doesn't exist in "
                 "children vector"
    );
}

Node& Node::get_child(std::size_t index)
{
    if (index < m_children.size())
        return m_children[index];

    assert(
        false &&
        "YAML ASSERT: failed to find child with given index as it is greater than children vector"
    );
}

Node& Node::operator<<(const Node& other)
{
    push_back(other);
    m_children.back().m_parent = m_parent;
    return *this;
}

std::string Node::get_as_string() const
{
    std::string str = {};
    _construct_string(str, *this, 0);
    return str;
}

bool Node::open(const std::string& filename)
{
    m_children.clear();

    std::FILE* file = std::fopen(filename.c_str(), "r");
    if (file != nullptr)
    {
        _read_node(file, this, 0);
        std::fclose(file);
        return true;
    }
    return false;
}

bool Node::compare(const Node& other) const
{
    if (m_name != other.m_name || m_value != other.m_value || m_parent != other.m_parent)
        return false;

    for (std::size_t i = 0; i < m_children.size(); i++)
    {
        if (!m_children[i].compare(other.m_children[i]))
            return false;
    }

    return true;
}

std::size_t Node::exists(const std::string& field_name) const
{
    for (std::size_t i = 0; i < m_children.size(); i++)
    {
        if (m_children[i].get_name() == field_name)
            return i;
    }

    return null_index;
}

bool Node::write_file(std::FILE* file) const
{
    for (std::size_t i = 0; i < m_children.size(); i++)
    {
        bool result = _write_node(file, m_children[i], 0);
        if (!result)
            return false;
    }
    return true;
}

bool Node::write_file(const std::string& filename) const
{
    std::FILE* file = std::fopen(filename.c_str(), "w");
    bool result = write_file(file);
    std::fclose(file);
    return result;
}

bool Node::write_if_file_exists(const std::string& filename) const
{
    std::FILE* file = std::fopen(filename.c_str(), "r");
    if (file != nullptr)
    {
        std::fclose(file);
        return write_file(filename);
    }
    return false;
}

void Node::_construct_string(std::string& str, const Node& node, std::size_t indent)
{
    char str_indent[1024];
    for (std::size_t i = 0; i < indent; i++)
        str_indent[i] = ' ';
    str_indent[indent] = '\0';

    if (node.get_name().size() > 0)
    {
        str += str_indent + node.get_name() + ": ";
        if (node.m_value.size() > 0)
            str += node.get_value() + "\n";
        else
        {
            str += "\n";
            for (std::size_t i = 0; i < node.m_children.size(); i++)
                _construct_string(str, node.m_children[i], indent + 2);
        }
    }
    else
    {
        for (std::size_t i = 0; i < node.m_children.size(); i++)
            _construct_string(str, node.m_children[i], indent);
    }
}

bool Node::_write_node(std::FILE* file, const Node& node, std::size_t indent)
{
    char str_indent[1024];
    for (std::size_t i = 0; i < indent; i++)
        str_indent[i] = ' ';
    str_indent[indent] = '\0';

    // write node with value into file
    if (node.get_value().size() > 0)
    {
        std::fputs(
            std::string(str_indent + node.get_name() + ": " + node.get_value() + "\n").c_str(), file
        );
    }
    else
    {
        std::fputs(std::string(str_indent + node.get_name() + ":\n").c_str(), file);
        for (std::size_t i = 0; i < node.m_children.size(); i++)
        {
            bool result = _write_node(file, node.m_children[i], indent + 2);
            if (!result)
                return false;
        }
    }

    return true;
}

void Node::_read_node(std::FILE* file, Node* last, std::size_t last_indent_size)
{
    char name[max_name_size()];
    char value[max_value_size()];
    std::size_t name_size = 0;
    std::size_t value_size = 0;
    std::size_t indent_size = 0;

    bool line_not_null = true;
    while (name_size == 0 && line_not_null)
        line_not_null = _read_line(file, name, value, name_size, value_size, indent_size);

    if (line_not_null)
    {
        Node* current = nullptr;
        if (indent_size == last_indent_size + 2)
        {
            last->push_back(Node(name, value));
            current = &last->m_children.back();
            current->m_parent = last;
        }
        else
        {
            std::size_t diff = last_indent_size - indent_size;
            for (std::size_t i = 0; i < diff; i += 2)
                last = last->m_parent;

            if (last->m_parent != nullptr)
            {
                last->m_parent->push_back(Node(name, value));
                current = &last->m_parent->m_children.back();
                current->m_parent = last->m_parent;
            }
            else
            {
                // at the root node
                last->push_back(Node(name, value));
                current = &last->m_children.back();
                current->m_parent = last;
            }
        }
        _read_node(file, current, indent_size);
    }
}

bool Node::_read_line(
    std::FILE* file, char* name, char* value, std::size_t& name_size, std::size_t& value_size,
    std::size_t& indent_size
)
{
    char line[max_line_size()];
    if (std::fgets(line, max_line_size(), file) != nullptr)
    {
        std::size_t length = static_cast<std::size_t>(strlen(line));

        bool finished_intent_count = false;
        bool finished_line = false;
        bool fill_value = false;
        std::size_t j = 0;

        for (std::size_t i = 0; i < length; i++)
        {
            switch (line[i])
            {
            case '\n':
                finished_line = true;
                break;
            case ' ':
                if (!finished_intent_count)
                    indent_size++;
                continue;
            case '#': // comments
                finished_line = true;
                if (fill_value)
                    value[j] = '\0';
                else
                {
                    name[j] = '\0';
                    fill_value = true;
                }
                break;

            case '\r':
                continue;

            default:
                finished_intent_count = true;

                if (line[i] == ':' && !fill_value)
                {
                    fill_value = true;
                    name[j] = '\0';
                    name_size = j;
                    j = 0;
                }
                else
                {
                    if (fill_value)
                        value[j] = line[i];
                    else
                        name[j] = line[i];
                    j++;
                }

                break;
            }

            if (finished_line)
            {
                assert(fill_value && "yaml syntax error, must have a ':' after field name");
                break;
            }
        }
        value[j] = '\0';
        value_size = j;

        return true;
    }
    return false;
}

Node& get_root_node(Node& node)
{
    while (node.get_parent() != nullptr)
        node = *node.get_parent();
    return node;
}

const Node& get_root_node(const Node& node)
{
    const Node* target = &node;
    while (node.get_parent() != nullptr)
        target = target->get_parent();
    return *target;
}

Node open(const std::string& filename)
{
    Node node = {};
    node.open(filename);
    return node;
}

} // namespace yaml

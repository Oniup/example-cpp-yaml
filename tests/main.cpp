/**
 * @file main.cpp
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

#include "../yaml.hpp"
#include <iostream>

struct Vector3
{
    float x = 0;
    float y = 0;
    float z = 0;

    float& operator[](std::size_t index)
    {
        switch (index)
        {
        case 0:
            return x;
            break;
        case 1:
            return y;
            break;
        case 2:
            return z;
            break;
        default:
            return x;
        }
    }
};

template<>
struct yaml::Convert<Vector3>
{
    std::string value_to_str(const Vector3& value)
    {
        return std::string(
            "[" + std::to_string(value.x) + ", " + std::to_string(value.y) + ", " +
            std::to_string(value.z) + "]"
        );
    }

    Vector3 value(const std::string& str)
    {
        Vector3 vec = {};

        char number[50];
        std::size_t j = 0;
        std::size_t index = 0;

        for (std::size_t i = 0; i < str.size(); i++)
        {
            if (str[i] == '[' || str[i] == ']' || str[i] == '\r' || str[i] == ' ')
            {
                continue;
            }
            else if (str[i] == ',')
            {
                number[j] = '\0';
                vec[index] = std::stof(number);
                j = 0;
                index++;
            }
            else
            {
                number[j] = str[i];
                j++;
            }
        }

        return vec;
    }
};

yaml::Node construct_yaml_example()
{
    std::cout << "Construct yaml file example:\n\n";

    yaml::Node root_node = {};

    std::vector<std::string> scene_names = {"MenuScene", "TestScene", "LastScene"};
    root_node << yaml::node("SceneNames", scene_names);
    for (const std::string& name : scene_names)
    {
        root_node << yaml::node(name);
        yaml::Node& scene_node = root_node[name];

        for (std::size_t i = 0; i < 3; i++)
        {
            std::string entity_name = "Entity" + std::to_string(i);
            scene_node << yaml::node(entity_name);

            yaml::Node& entity_node = scene_node[entity_name];
            entity_node << yaml::node("TransformComponent");

            yaml::Node& transform = entity_node[0];
            transform << yaml::node("translation", Vector3{1, 2, 3})
                      << yaml::node("rotation", Vector3{43, 23, 1})
                      << yaml::node("scale", Vector3{1, 1, 1});
        }
    }

    // NOTE: can also use yaml::get_root_as_string() and pass any of its nodes to print the entire
    // yaml file
    std::cout << root_node.get_as_string() << "\n\n";

    return root_node;
}

void write_to_file_example(const yaml::Node& node, const std::string& filename)
{
    if (!yaml::write(node, filename))
    {
        std::cout << "Write to file example:\n\n";
        std::cout << "failed to write to file\n";
    }
}

void read_file_example(const std::string& filename)
{
    std::cout << "Read file example:\n\n";
    yaml::Node root_node = yaml::open(filename);
    std::cout << root_node.get_as_string() << "\n";
}

int main(int argc, char** argv)
{
    yaml::Node node = construct_yaml_example();
    write_to_file_example(node, "scene_save.yaml");
    read_file_example("scene_save.yaml");
    return 0;
}

#include "../yaml.hpp"
#include <iostream>

struct Vector3 {
    float x = 0;
    float y = 0;
    float z = 0;

    float& operator[](std::size_t index) {
        switch (index) {
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
struct yaml::Convert<Vector3> {
    std::string value_to_str(const Vector3& value) {
        return std::string(
            "[" + std::to_string(value.x) + ", " + std::to_string(value.y) + ", " +
            std::to_string(value.z) + "]"
        );
    }

    Vector3 value(const Node& node) {
        Vector3 result = {};

        char number[50];
        std::size_t j = 0;
        std::size_t index = 0;

        const std::string& value = node.get_value();
        for (std::size_t i = 0; i < value.size(); i++) {
            if (value[i] == '[' || value[i] == ']' || value[i] == '\r' || value[i] == ' ') {
                continue;
            } else if (value[i] == ',') {
                number[j] = '\0';
                result[index] = std::stof(number);
                j = 0;
                index++;
            } else {
                number[j] = value[i];
                j++;
            }
        }

        return result;
    }
};

yaml::Node construct_yaml_example() {
    std::cout << "Construct yaml file example:\n\n";

    yaml::Node scene_node = {};

    scene_node << yaml::node("SceneName", (const char*)"TestScene");
    scene_node << yaml::node("Entity1");

    yaml::Node& entity_node = scene_node["Entity1"];
    entity_node << yaml::node("TransformComponent");

    yaml::Node& transform = entity_node[0];
    transform << yaml::node("translation", Vector3{1, 2, 3})
              << yaml::node("rotation", Vector3{43, 23, 1})
              << yaml::node("scale", Vector3{1, 1, 1});

    std::cout << scene_node.get_as_string() << "\n\n";

    return scene_node;
}

void write_to_file_example(const yaml::Node& node, const std::string& filename) {
    std::cout << "Write to file example:\n\n";
    if (!node.write_file(filename)) {
        std::cout << "failed to write to file\n";
    }
}

int main(int argc, char** argv) {
    yaml::Node node = construct_yaml_example();
    write_to_file_example(node, "scene_save.yaml");
    return 0;
}

# Yaml Parser

A simple single header and source file yaml parser/emitter library.

Note: currently doesn't support values to be multiple lines

# Docs

## Basics

```cpp
// Create the root node, this shouldnt have a name or value, only children nodes
yaml::Node scene_node = {};

// Attach children nodes
scene_node << yaml::node("Entity001");
           << yaml::node("Entity002");
           << yaml::node("Entity003");
           << yaml::node("Entity004");
```

***Result in yaml***

```yaml
Entity001:
Entity002:
Entity003:
Entity004:
```

### Add child nodes to "Entity001"

```cpp
// Get the node (can provide the index)
yaml::Node& entity_node = scene_node["Entity001"];

// Add some data to entity
entity_node << yaml::node("Type", (const char*)"Person")
            << yaml::node("FirstName", (const char*)"John")
            << yaml::node("LastName", (const char*)"Doe")
            << yaml::node("Age", 32)
```

***Result in yaml***

```yaml
Entity001:
  Type: "Person"
  FirstName: "John"
  LastName: "Doe"
  Age: 32
Entity002:
Entity003:
Entity004:
```

### Read/Write to file

```cpp
scene_node.read_file("scene_data.yaml");
scene_node.write_file("scene_data.yaml");
```

### Add custom data types

Example struct

```cpp
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
```

To allow the library to know how to parse and write you'll need to implement the 
following Convert struct

```cpp
template<>
struct yaml::Convert<Vector3> {
    // Format in the yaml file
    std::string value_to_str(const Vector3& value) {
        return std::string(
            "[" + std::to_string(value.x) + ", " + std::to_string(value.y) + ", " +
            std::to_string(value.z) + "]"
        );
    }

    // Converting format about back into original type
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
```

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
yaml::open("scene_data.yaml");
yaml::write(node, "scene_data.yaml");


// NOTE: This does not find the root node so make sure that the node calling this
// function is the root node. However, yaml::write() will find the root node for you
scene_node.write_file("scene_data.yaml");
```

## Custom Types

to allow the library to know how to parse the write, you'll need to implement a
```yaml::Convert<_T>``` struct and define two functions. first the
```std::string value_to_str(const _T& value)``` (serialization) and
```_T value(const std::string& str)``` (deserialization)

### Example object

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
### Example convert struct

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
    Vector3 value(const std::string& str) {
        Vector3 vec = {};

        char number[50];
        std::size_t j = 0;
        std::size_t index = 0;

        for (std::size_t i = 0; i < str.size(); i++) {
            if (str[i] == '[' || str[i] == ']' || str[i] == '\r' || str[i] == ' ') {
                continue;
            } else if (str[i] == ',') {
                number[j] = '\0';
                vec[index] = std::stof(number);
                j = 0;
                index++;
            } else {
                number[j] = str[i];
                j++;
            }
        }

        return vec;
    }
};
```

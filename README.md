# Yaml Parser

A simple single header and source file yaml reading/writing library written by a first year uni student

# Docs

## Basics

```cpp
// create the root node, this shouldnt have a name or value, only children nodes
yaml::Node scene_node = {};

// attach children nodes
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
// get the node (can provide the index)
yaml::Node& entity_node = scene_node["Entity001"];

// add some data to entity
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

### Now to write data to file

```cpp
scene_node.write_file("scene_data.yaml");
```

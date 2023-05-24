# Yaml Parser

### Creating

```cpp
yaml::Node root_node;
// Open yaml formatted file
if (!yaml::Node::open_file(&root_node, "path/to/file.yaml")) {
    // file doesn't exist? create one
    root_node = yaml::Node::create_file("path/to/file/location.yaml");
}
```

### Writing

```cpp
if (!root_node.node_exists("Entity")) {
    // Create node
    root_node << yaml::node<std::string>("Entity", "Name");
} else {
    // Override node
    root_node["Entity"] = yaml::value<std::string>("Rename");
}

// Delete node
root_node.remove("Entity");
```

### Reading

```cpp
if (root_node.node_exists("Entity")) {
    std::cout << yaml::convert<std::string>(root_node["Entity"]) << "\n";
}
```

### Save File

```cpp
root_node.write_file();
```

#ifndef __YAML_HPP__
#define __YAML_HPP__

#include <list>

namespace yaml {

struct Node {
    std::list<Node> nodes = {};
};

}

#endif

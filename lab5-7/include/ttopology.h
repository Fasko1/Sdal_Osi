#ifndef TTOPOLOGY_H
#define TTOPOLOGY_H

#include "common.h"

class TTopology {
private:
    std::map<int, TNodeInfo> nodes_;

public:
    bool AddNode(int id, int parent_id, const std::string& endpoint);
    bool RemoveNode(int id);
    TNodeInfo* GetNode(int id);
    bool NodeExists(int id);
    std::vector<int> GetChildren(int id);
    std::map<int, TNodeInfo>& GetAllNodes() { return nodes_; }
};

#endif

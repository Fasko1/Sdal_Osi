#include "ttopology.h"
#include <algorithm>

bool TTopology::AddNode(int id, int parent_id, const std::string& endpoint) {
    if (nodes_.find(id) != nodes_.end()) return false;
    TNodeInfo info;
    info.id = id;
    info.parent_id = parent_id;
    info.endpoint = endpoint;
    info.alive = true;
    info.last_heartbeat = std::chrono::steady_clock::now();
    nodes_[id] = info;
    return true;
}

bool TTopology::RemoveNode(int id) {
    return nodes_.erase(id) > 0;
}

TNodeInfo* TTopology::GetNode(int id) {
    auto it = nodes_.find(id);
    if (it == nodes_.end()) return nullptr;
    return &it->second;
}

bool TTopology::NodeExists(int id) {
    return nodes_.find(id) != nodes_.end();
}

std::vector<int> TTopology::GetChildren(int id) {
    std::vector<int> children;
    for (auto& [nid, info] : nodes_) {
        if (info.parent_id == id) children.push_back(nid);
    }
    return children;
}

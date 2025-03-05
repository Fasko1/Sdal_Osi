#ifndef TMESSAGING_H
#define TMESSAGING_H

#include <string>
#include <zmq.hpp>

class TMessaging {
private:
    zmq::context_t context_{1};
    zmq::socket_t socket_{context_, ZMQ_ROUTER};
    bool is_controller_ = false;
    int self_node_id_ = -1;

public:
    bool InitController(const std::string& endpoint);
    bool InitWorker(const std::string& controller_endpoint, int node_id);

    bool SendToWorker(int node_id, const std::string& message);
    bool SendToController(const std::string& message);

    bool RecvFromAnyWorker(std::string& node_id_str, std::string& message);
    bool RecvFromController(std::string& message);

    bool IsController() const { return is_controller_; }
};

#endif

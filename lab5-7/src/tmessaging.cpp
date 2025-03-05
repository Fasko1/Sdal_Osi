#include "tmessaging.h"
#include <iostream>

bool TMessaging::InitController(const std::string& endpoint) {
    is_controller_ = true;
    socket_ = zmq::socket_t(context_, ZMQ_ROUTER);
    try {
        socket_.bind(endpoint.c_str());
    } catch (const zmq::error_t& e) {
        std::cerr << "Messaging: Failed to bind: " << e.what() << "\n";
        return false;
    }
    return true;
}

bool TMessaging::InitWorker(const std::string& controller_endpoint, int node_id) {
    is_controller_ = false;
    self_node_id_ = node_id;
    socket_ = zmq::socket_t(context_, ZMQ_DEALER);
    std::string identity = std::to_string(node_id);
    socket_.setsockopt(ZMQ_IDENTITY, identity.c_str(), identity.size());
    try {
        socket_.connect(controller_endpoint.c_str());
    } catch (const zmq::error_t& e) {
        std::cerr << "Messaging: Failed to connect worker: " << e.what() << "\n";
        return false;
    }
    return true;
}

bool TMessaging::SendToWorker(int node_id, const std::string& message) {
    if (!is_controller_) return false;
    zmq::message_t id_msg(std::to_string(node_id).data(), std::to_string(node_id).size());
    // zmq::message_t empty_msg;
    zmq::message_t msg(message.data(), message.size());

    try {
        socket_.send(id_msg, ZMQ_SNDMORE);
        // socket_.send(empty_msg, ZMQ_SNDMORE);
        socket_.send(msg);
    } catch (const zmq::error_t& e) {
        std::cerr << "Messaging: SendToWorker error: " << e.what() << "\n";
        return false;
    }
    return true;
}

bool TMessaging::SendToController(const std::string& message) {
    if (is_controller_) return false;
    zmq::message_t msg(message.data(), message.size());
    try {
        socket_.send(msg);
    } catch (const zmq::error_t& e) {
        std::cerr << "Messaging: SendToController error: " << e.what() << "\n";
        return false;
    }
    return true;
}

bool TMessaging::RecvFromAnyWorker(std::string& node_id_str, std::string& message) {
    if (!is_controller_) return false;
    zmq::message_t id_msg;
    // zmq::message_t empty_msg;
    zmq::message_t msg;
    try {
        if (!socket_.recv(&id_msg)) return false;
        // if (!socket_.recv(&empty_msg)) return false;
        if (!socket_.recv(&msg)) return false;
    } catch (const zmq::error_t& e) {
        std::cerr << "Messaging: RecvFromAnyWorker error: " << e.what() << "\n";
        return false;
    }

    node_id_str = std::string((char*)id_msg.data(), id_msg.size());
    message = std::string((char*)msg.data(), msg.size());
    return true;
}

bool TMessaging::RecvFromController(std::string& message) {
    if (is_controller_) return false;
    zmq::message_t msg;
    try {
        if (!socket_.recv(&msg)) return false;
    } catch (const zmq::error_t& e) {
        std::cerr << "Messaging: RecvFromController error: " << e.what() << "\n";
        return false;
    }
    message = std::string((char*)msg.data(), msg.size());
    return true;
}

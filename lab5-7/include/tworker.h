#ifndef TWORKER_H
#define TWORKER_H

#include <string>
#include <atomic>
#include "tmessaging.h"

class TWorkerNode {
private:
    int id_;
    int parent_id_;
    std::string endpoint_;
    TMessaging messaging_;
    std::atomic<bool> running_{true};

    void HandleExec(const std::string& text, const std::string& pattern);
    void SendHeartbeat();
    static void* HeartbeatThreadStatic(void* arg);

public:
    TWorkerNode(int id, int parent_id, const std::string& endpoint);
    bool Init();
    void Run();
};

#endif

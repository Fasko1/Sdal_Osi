#ifndef TCONTROLLER_H
#define TCONTROLLER_H

#include "ttopology.h"
#include "tmessaging.h"
#include <string>
#include <chrono>
#include <atomic>

class TControllerNode {
private:
    void HandleCreate(int id, int parent);
    void HandleExec(int id);
    void HandleHeartbeat(int time_ms);
    static void* ReceiverThreadStatic(void* arg);
    void ReceiverThreadFunc();
    void Quit();

    bool IsNodeAvailable(int id);
    void CheckHeartbeats();

    TTopology topology_;
    TMessaging messaging_;
    int heartbeat_time_ = 0;
    std::chrono::steady_clock::time_point last_heartbeat_check_ = std::chrono::steady_clock::now();
    std::atomic<bool> running_{true};
    pthread_t recv_thread_;

public:
    TControllerNode();
    bool Init(const std::string& endpoint);
    void Run();
};

#endif

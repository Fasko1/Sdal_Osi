#include "tcontroller.h"
#include <iostream>
#include <sstream>
#include <pthread.h>
#include <chrono>
#include <unistd.h> // execl, fork

TControllerNode::TControllerNode() {}

bool TControllerNode::Init(const std::string& endpoint) {
    if (!messaging_.InitController(endpoint)) {
        std::cerr << "Controller: Failed to init messaging.\n";
        return false;
    }
    return true;
}

void TControllerNode::Run() {
    std::cout << "Available commands:\n";
    std::cout << "  create <id> [parent]\n";
    std::cout << "  exec <id>\n";
    std::cout << "  heartbeat <time_ms>\n";
    std::cout << "  quit\n";

    // Поток приёма сообщений
    pthread_create(&recv_thread_, NULL, &TControllerNode::ReceiverThreadStatic, this);
    pthread_detach(recv_thread_);

    while (running_) {
        std::cout << "> ";
        std::string line;
        if (!std::getline(std::cin, line)) break;
        if (line.empty()) continue;
        std::istringstream iss(line);
        std::string cmd; 
        iss >> cmd;

        if (cmd == "create") {
            std::string id_str, parent_str;
            id_str = ""; 
            parent_str = "-1";
            iss >> id_str; // id
            if (iss.good()) iss >> parent_str;

            int id, parent;
            try {
                if (id_str.empty()) {
                    std::cout << "Error: invalid arguments for create\n";
                    continue;
                }
                id = std::stoi(id_str);
                parent = std::stoi(parent_str);
            } catch (...) {
                std::cout << "Error: invalid arguments for create\n";
                continue;
            }
            HandleCreate(id, parent);
        } else if (cmd == "exec") {
            std::string id_str;
            iss >> id_str;
            if (id_str.empty()) {
                std::cout << "Error: invalid id for exec\n";
                continue;
            }
            int id;
            try {
                id = std::stoi(id_str);
            } catch (...) {
                std::cout << "Error: invalid id for exec\n";
                continue;
            }
            HandleExec(id);
        } else if (cmd == "heartbeat") {
            std::string t_str;
            iss >> t_str;
            if (t_str.empty()) {
                std::cout << "Error: invalid time for heartbeat\n";
                continue;
            }
            int t;
            try {
                t = std::stoi(t_str);
            } catch (...) {
                std::cout << "Error: invalid time for heartbeat\n";
                continue;
            }
            HandleHeartbeat(t);
        } else if (cmd == "quit") {
            Quit();
        } else {
            std::cout << "Unknown command\n";
        }

        CheckHeartbeats();
    }
    // Дадим время всем завершиться
    usleep(500*1000); // 0.5 секунды
}

void* TControllerNode::ReceiverThreadStatic(void* arg) {
    TControllerNode* self = static_cast<TControllerNode*>(arg);
    self->ReceiverThreadFunc();
    return NULL;
}

void TControllerNode::ReceiverThreadFunc() {
    while (running_) {
        std::string node_id_str;
        std::string msg;
        if (!messaging_.RecvFromAnyWorker(node_id_str, msg)) {
            // Нет сообщений, подождём
            usleep(100 * 1000); // задержка на 100 ms
            continue;
        }

        int nid;
        try {
            nid = std::stoi(node_id_str);
        } catch (...) {
            std::cerr << "Controller: Invalid node_id_str received: " << node_id_str << "\n";
            std::cerr << msg << "\n"; // Выведем полученное сообщение
            continue;
        }

        if (msg == "HB") {
            // Heartbeat signal
            TNodeInfo* node = topology_.GetNode(nid);
            if (node) {
                node->last_heartbeat = std::chrono::steady_clock::now();
                node->hb_received = true;
                if (!node->alive) {
                    // Узел снова послал HB, можно считать его живым
                    node->alive = true; 
                }
            } else {
                // Неизвестный узел
                std::cerr << "Controller: Received HB from unknown node " << nid << "\n";
            }
        } else if (msg == "Ok" || msg.rfind("Ok:",0)==0 || msg.rfind("Error:",0)==0) {
            // Ответ на команду exec или другую команду
            std::cout << msg << "\n";
        } else {
            // Неизвестное сообщение
            std::cout << "Unknown message from node " << nid << ": " << msg << "\n";
        }
    }
}

void TControllerNode::HandleCreate(int id, int parent) {
    if (topology_.NodeExists(id)) {
        std::cout << "Error: Already exists\n";
        return;
    }
    if (parent != -1 && !topology_.NodeExists(parent)) {
        std::cout << "Error: Parent not found\n";
        return;
    }
    if (parent != -1 && !IsNodeAvailable(parent)) {
        std::cout << "Error: Parent is unavailable\n";
        return;
    }

    const char* workerPath = std::getenv("WORKER_PATH");
    if (!workerPath) {
        std::cout << "Error: WORKER_PATH environment variable is not set\n";
        return;
    }

    std::string endpoint = "tcp://127.0.0.1:" + std::to_string(6000 + id);
    pid_t pid = fork();
    if (pid == 0) {
        execl(workerPath, "worker", std::to_string(id).c_str(), std::to_string(parent).c_str(), endpoint.c_str(), (char*)NULL);
        std::cerr << "Error: Failed to exec worker from WORKER_PATH\n";
        _exit(1);
    } else if (pid < 0) {
        std::cout << "Error: Failed to create process\n";
        return;
    }

    if (!topology_.AddNode(id, parent, endpoint)) {
        std::cout << "Error: Internal AddNode failed\n";
        return;
    }
    std::cout << "Ok: " << pid << "\n";
}

void TControllerNode::HandleExec(int id) {
    if (!topology_.NodeExists(id)) {
        std::cout << "Error:" << id << ": Not found\n";
        return;
    }
    if (!IsNodeAvailable(id)) {
        std::cout << "Error:" << id << ": Node is unavailable\n";
        return;
    }

    std::cout << "(text_string): ";
    std::string text; 
    if (!std::getline(std::cin, text) || text.empty()) std::getline(std::cin, text);
    std::cout << "(pattern_string): ";
    std::string pattern; 
    if (!std::getline(std::cin, pattern) || pattern.empty()) std::getline(std::cin, pattern);

    std::string msg = "EXEC\n" + text + "\n" + pattern;
    if(!messaging_.SendToWorker(id, msg)) {
        std::cout << "Error:" << id << ": Failed to send message\n";
    }
}

void TControllerNode::HandleHeartbeat(int time_ms) {
    heartbeat_time_ = time_ms;
    std::cout << "Ok\n";

    // Посылаем всем узлам "HB_START"
    for (auto& [nid, info] : topology_.GetAllNodes()) {
        messaging_.SendToWorker(nid, "HB_START");
    }
}

void TControllerNode::Quit() {
    running_ = false;
    std::cout << "Exiting...\n";
    // Посылаем QUIT всем узлам, чтобы они завершились
    for (auto& [nid, info] : topology_.GetAllNodes()) {
        messaging_.SendToWorker(nid, "QUIT");
    }
}

bool TControllerNode::IsNodeAvailable(int id) {
    TNodeInfo* node = topology_.GetNode(id);
    if (!node) return false;
    if (!node->alive) return false;
    if (heartbeat_time_ > 0) {
        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - node->last_heartbeat).count();
        if (diff > 4 * heartbeat_time_) {
            return false;
        }
    }
    return true;
}

void TControllerNode::CheckHeartbeats() {
    if (heartbeat_time_ <= 0) return;
    auto now = std::chrono::steady_clock::now();
    for (auto& [nid, info] : topology_.GetAllNodes()) {
        if (!info.hb_received) continue; // Для недавно активированных
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - info.last_heartbeat).count();
        if (diff > 4 * heartbeat_time_ && info.alive) {
            info.alive = false;
            std::cout << "Heartbeat: node " << nid << " is unavailable now\n";
        }
    }
}

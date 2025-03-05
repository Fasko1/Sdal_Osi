#include "tworker.h"
#include <iostream>

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cerr << "Usage: worker id parent endpoint\n";
        return 1;
    }
    int id = std::stoi(argv[1]);
    int parent = std::stoi(argv[2]);
    std::string endpoint = argv[3];

    TWorkerNode worker(id, parent, endpoint);
    if (!worker.Init()) {
        std::cerr << "Error: Failed to init worker\n";
        return 1;
    }
    worker.Run();
    return 0;
}

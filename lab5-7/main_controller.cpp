#include "tcontroller.h"
#include <iostream>

int main() {
    TControllerNode controller;
    if (!controller.Init("tcp://127.0.0.1:5555")) {
        std::cerr << "Error: Failed to init controller\n";
        return 1;
    }
    controller.Run();
    return 0;
}

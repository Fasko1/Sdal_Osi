#include "dag.h"

int main(int argc, char* argv[]) {
    std::string configFile = "config.ini";
    if (argc > 1) {
        configFile = argv[1];
    }

    TDagExecutor Executor;
    if (!Executor.ReadIni(configFile)) {
        return 1;
    }

    if (Executor.CheckCycle()) {
        SafePrint("ERROR: Graph has a cycle!");
        return 2;
    }
    if (!Executor.CheckStartFinish()) {
        SafePrint("ERROR: Graph has no start/finish jobs!");
        return 3;
    }

    if (!Executor.CheckSingleComponent()) {
        SafePrint("ERROR: Graph has multiple components!");
        return 4;
    }

    Executor.RunDAG();

    return 0;
}

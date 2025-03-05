#ifndef DAG_H
#define DAG_H

#include <pthread.h>
#include <atomic>
#include <map>
#include <queue>
#include <string>
#include <vector>
#include <set>
#include <iostream>
#include <fstream>

// ----------------------------------------------------------------------------
// Глобальный мьютекс для потокобезопасного вывода
// ----------------------------------------------------------------------------
extern pthread_mutex_t g_PrintMutex;

// ----------------------------------------------------------------------------
// Вспомогательная функция для «безопасного» вывода
// ----------------------------------------------------------------------------
void SafePrint(const std::string &message);

// ----------------------------------------------------------------------------
// Класс «джобы» (TJob) с пользовательским копированием,
// чтобы исправить ошибку "use of deleted function ..."
// ----------------------------------------------------------------------------
class TJob {
public:
    TJob() = default;

    TJob(const TJob &other) {
        JobId = other.JobId;
        BarrierName = other.BarrierName;
        Dependencies = other.Dependencies;
        Children = other.Children;
        RemainDeps.store(other.RemainDeps.load());
        IsFinished.store(other.IsFinished.load());
        IsSuccess.store(other.IsSuccess.load());
    }

    TJob& operator=(const TJob &other) {
        if (this != &other) {
            JobId = other.JobId;
            BarrierName = other.BarrierName;
            Dependencies = other.Dependencies;
            Children = other.Children;
            RemainDeps.store(other.RemainDeps.load());
            IsFinished.store(other.IsFinished.load());
            IsSuccess.store(other.IsSuccess.load());
        }
        return *this;
    }

    int JobId = 0;
    std::string BarrierName;
    std::vector<int> Dependencies;
    std::vector<int> Children;

    std::atomic<int> RemainDeps{0};
    std::atomic<bool> IsFinished{false};
    std::atomic<bool> IsSuccess{true};
};

// ----------------------------------------------------------------------------
// «Примитив-барьер» — хранит счётчик оставшихся джоб в группе
// ----------------------------------------------------------------------------
class TBarrierGroup {
public:
    TBarrierGroup() {
        pthread_mutex_init(&Mutex, nullptr);
        pthread_cond_init(&Cond, nullptr);
    }
    ~TBarrierGroup() {
        pthread_mutex_destroy(&Mutex);
        pthread_cond_destroy(&Cond);
    }

    // Общее число джоб в группе
    int TotalCount = 0;
    // Сколько джоб этой группы ещё не завершились
    int Remaining = 0;

    pthread_mutex_t Mutex;
    pthread_cond_t Cond;
};

// ----------------------------------------------------------------------------
// Исполнитель DAG (Directed Acyclic Graph)
// ----------------------------------------------------------------------------
class TDagExecutor {
public:
    TDagExecutor();
    ~TDagExecutor();

    // Читаем конфиг
    bool ReadIni(const std::string &filename);

    // Проверяем на наличие только одной компоненты связности
    bool CheckSingleComponent();

    // Проверяем, есть ли цикл
    bool CheckCycle();

    // Проверяем, что есть хотя бы одна стартовая и одна финишная джоба
    bool CheckStartFinish();

    // Запускаем весь DAG
    void RunDAG();

    const std::map<int, TJob>& GetJobs() const { return Jobs_; }
    int GetMaxParallel() const { return MaxParallel_; }

private:
    bool HasCycleUtil(int jobId,
                      std::map<int, bool> &visited,
                      std::map<int, bool> &recStack);

    static void* JobRunner(void* arg);

    void BarrierArrive(const std::string &barrierName);
    bool IsBarrierUnlocked(const std::string &barrierName);
    bool AreAllParentBarriersUnlocked(int &childId);

    // Основные структуры данных:
    std::map<int, TJob> Jobs_;
    std::map<std::string, TBarrierGroup> BarrierGroups_;
    std::queue<int> ReadyQueue_;

    // Макс. число одновременно исполняемых джоб
    int MaxParallel_ = 2;

    // Глобальный флаг остановки (если какая-то джоба упала)
    std::atomic<bool> StopExecution_{false};

    pthread_mutex_t QueueMutex_;
    pthread_cond_t QueueCond_;

    // Счётчик активных джоб (выполняющихся потоков)
    std::atomic<int> CurrentActiveThreads_{0};
};

#endif

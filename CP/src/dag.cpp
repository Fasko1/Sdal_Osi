#include "dag.h"
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>

pthread_mutex_t g_PrintMutex = PTHREAD_MUTEX_INITIALIZER;

void SafePrint(const std::string &message) {
    pthread_mutex_lock(&g_PrintMutex);
    std::cout << message << std::endl;
    pthread_mutex_unlock(&g_PrintMutex);
}

TDagExecutor::TDagExecutor() {
    srand(time(nullptr));
    pthread_mutex_init(&QueueMutex_, nullptr);
    pthread_cond_init(&QueueCond_, nullptr);
}

TDagExecutor::~TDagExecutor() {
    pthread_mutex_destroy(&QueueMutex_);
    pthread_cond_destroy(&QueueCond_);
}

// ----------------------------------------------------------------------------
// Парсер config.ini
// ----------------------------------------------------------------------------
bool TDagExecutor::ReadIni(const std::string &filename) {
    std::ifstream fin(filename);
    if (!fin.is_open()) {
        SafePrint("Cannot open config file: " + filename);
        return false;
    }

    // Сбрасываем состояния
    Jobs_.clear();
    BarrierGroups_.clear();
    while (!ReadyQueue_.empty()) {
        ReadyQueue_.pop();
    }
    StopExecution_.store(false);
    CurrentActiveThreads_.store(0);

    std::string line;
    enum class ESection {
        None,
        Jobs,
        Edges,
        Parallel
    };
    ESection currentSection = ESection::None;

    while (std::getline(fin, line)) {
        // Trim
        {
            size_t startPos = 0;
            while (startPos < line.size() && isspace((unsigned char)line[startPos])) {
                startPos++;
            }
            size_t endPos = line.size();
            while (endPos > startPos && isspace((unsigned char)line[endPos-1])) {
                endPos--;
            }
            line = line.substr(startPos, endPos - startPos);
        }

        if (line.empty() || line[0] == '#') {
            continue; // Пропускаем комментарии/пустые строки
        }
        if (line == "[Jobs]") {
            currentSection = ESection::Jobs;
            continue;
        } else if (line == "[Edges]") {
            currentSection = ESection::Edges;
            continue;
        } else if (line == "[Parallel]") {
            currentSection = ESection::Parallel;
            continue;
        }

        switch (currentSection) {
        case ESection::Jobs: {
            // формат: "job=1 barrier=B1" или "job=4"
            int jobId = 0;
            std::string barrier;

            std::istringstream iss(line);
            std::string token;
            while (iss >> token) {
                size_t pos = token.find('=');
                if (pos != std::string::npos) {
                    std::string key = token.substr(0, pos);
                    std::string val = token.substr(pos+1);
                    if (key == "job") {
                        jobId = std::stoi(val);
                    } else if (key == "barrier") {
                        barrier = val;
                    }
                }
            }

            if (jobId <= 0) {
                std::ostringstream oss;
                oss << "Bad job definition line: " << line;
                SafePrint(oss.str());
            } else {
                TJob job;
                job.JobId = jobId;
                job.BarrierName = barrier;
                Jobs_[jobId] = job;
            }
            break;
        }
        case ESection::Edges: {
            // формат "1->4"
            size_t pos = line.find("->");
            if (pos == std::string::npos) {
                std::ostringstream oss;
                oss << "Bad edge line: " << line;
                SafePrint(oss.str());
                break;
            }
            int from = std::stoi(line.substr(0, pos));
            int to   = std::stoi(line.substr(pos+2));
            if (Jobs_.find(from) == Jobs_.end() || Jobs_.find(to) == Jobs_.end()) {
                std::ostringstream oss;
                oss << "Edge references unknown job: " << line;
                SafePrint(oss.str());
            } else {
                Jobs_[from].Children.push_back(to);
                Jobs_[to].Dependencies.push_back(from);
            }
            break;
        }
        case ESection::Parallel: {
            MaxParallel_ = std::stoi(line);
            break;
        }
        default:
            // не в секции — пропускаем
            break;
        }
    }
    fin.close();

    // Инициализация полей
    for (auto &kv : Jobs_) {
        kv.second.RemainDeps.store(kv.second.Dependencies.size());
        kv.second.IsFinished.store(false);
        kv.second.IsSuccess.store(true);
    }

    // Инициализация барьеров
    for (auto &kv : Jobs_) {
        const std::string &bn = kv.second.BarrierName;
        if (!bn.empty()) {
            if (BarrierGroups_.find(bn) == BarrierGroups_.end()) {
                TBarrierGroup bg;
                BarrierGroups_[bn] = bg;
            }
            BarrierGroups_[bn].TotalCount++;
            BarrierGroups_[bn].Remaining++;
        }
    }

    return true;
}

// ----------------------------------------------------------------------------
// Проверяем граф на наличие только одной компоненты связности
// ----------------------------------------------------------------------------
bool TDagExecutor::CheckSingleComponent() {
    if (Jobs_.empty()) {
        return false;
    }

    // Построим неориентированные связи
    std::map<int, std::vector<int>> undirected;
    for (auto &kv : Jobs_) {
        int id = kv.first;
        undirected[id];
        for (int child : kv.second.Children) {
            undirected[id].push_back(child);
            undirected[child].push_back(id);
        }
    }

    // Возьмём первую попавшуюся джобу
    auto it = Jobs_.begin();
    int startId = it->first;

    // DFS
    std::vector<int> stack;
    stack.push_back(startId);
    std::set<int> visited;
    visited.insert(startId);

    while (!stack.empty()) {
        int curr = stack.back();
        stack.pop_back();
        for (int neigh : undirected[curr]) {
            if (visited.count(neigh) == 0) {
                visited.insert(neigh);
                stack.push_back(neigh);
            }
        }
    }

    // Если посетили все джобы, то одна компонента
    return (visited.size() == Jobs_.size());
}

// ----------------------------------------------------------------------------
// Проверяем граф на наличие цикла
// ----------------------------------------------------------------------------
bool TDagExecutor::HasCycleUtil(int jobId, std::map<int, bool> &visited, std::map<int, bool> &recStack) {
    if (!visited[jobId]) {
        visited[jobId] = true;
        recStack[jobId] = true;

        for (int childId : Jobs_[jobId].Children) {
            if (!visited[childId] && HasCycleUtil(childId, visited, recStack)) {
                return true;
            } else if (recStack[childId]) {
                return true;
            }
        }
    }
    recStack[jobId] = false;
    return false;
}

bool TDagExecutor::CheckCycle() {
    std::map<int, bool> visited;
    std::map<int, bool> recStack;

    for (auto &kv : Jobs_) {
        visited[kv.first] = false;
        recStack[kv.first] = false;
    }

    for (auto &kv : Jobs_) {
        if (!visited[kv.first]) {
            if (HasCycleUtil(kv.first, visited, recStack)) {
                return true;
            }
        }
    }
    return false;
}

// ----------------------------------------------------------------------------
// Проверка наличия хотя бы одной start-джобы (нет зависимостей)
// и хотя бы одной finish-джобы (нет детей)
// ----------------------------------------------------------------------------
bool TDagExecutor::CheckStartFinish() {
    bool hasStart = false;
    bool hasFinish = false;

    for (auto &kv : Jobs_) {
        if (kv.second.Dependencies.empty()) {
            hasStart = true;
        }
        if (kv.second.Children.empty()) {
            hasFinish = true;
        }
    }
    return (hasStart && hasFinish);
}

// ----------------------------------------------------------------------------
// Проверяем, «разблокирован» ли барьер
// ----------------------------------------------------------------------------
bool TDagExecutor::IsBarrierUnlocked(const std::string &barrierName) {
    if (barrierName.empty()) {
        return true;
    }

    auto it = BarrierGroups_.find(barrierName);
    if (it == BarrierGroups_.end()) {
        // нет такого барьера — считаем, что разблокирован
        return true;
    }

    TBarrierGroup &bg = it->second;
    pthread_mutex_lock(&bg.Mutex);
    bool unlocked = (bg.Remaining == 0);
    pthread_mutex_unlock(&bg.Mutex);

    return unlocked;
}

bool TDagExecutor::AreAllParentBarriersUnlocked(int& childId) { 
    for (int parentId : Jobs_[childId].Dependencies) {        
        const std::string &parentBarrier = Jobs_[parentId].BarrierName;
        if (!parentBarrier.empty() && !IsBarrierUnlocked(parentBarrier)) {
            return false;     
        }
    }    return true;
}

// ----------------------------------------------------------------------------
// Уменьшаем счётчик барьера после окончания джобы.
// Если дошли до 0 — барьер разблокирован, пробуждаем всех
// ----------------------------------------------------------------------------
void TDagExecutor::BarrierArrive(const std::string &barrierName) {
    if (barrierName.empty()) {
        return;
    }

    auto it = BarrierGroups_.find(barrierName);
    if (it == BarrierGroups_.end()) {
        return;
    }

    TBarrierGroup &bg = it->second;

    pthread_mutex_lock(&bg.Mutex);
    bg.Remaining--;
    if (bg.Remaining < 0) {
        bg.Remaining = 0;
    }
    if (bg.Remaining == 0) {
        pthread_cond_broadcast(&bg.Cond);
    }
    pthread_mutex_unlock(&bg.Mutex);
}

// ----------------------------------------------------------------------------
// Поток выполнения конкретной джобы
// ----------------------------------------------------------------------------
void* TDagExecutor::JobRunner(void* arg) {
    auto *pairPtr = reinterpret_cast<std::pair<TDagExecutor*, int>*>(arg);
    TDagExecutor *Executor = pairPtr->first;
    int jobId = pairPtr->second;
    delete pairPtr;

    TJob &job = Executor->Jobs_[jobId];

    {
        std::ostringstream oss;
        oss << "[Thread " << pthread_self() 
            << "] Starting job " << jobId;
        SafePrint(oss.str());
    }

    // Имитируем работу: sleep от 1 до 3 сек
    bool success = true;
    if (jobId % 2 == 1) {
        // Случайная имитация ошибки (1 из 5) для нечётных джоб
        int r = rand() % 5;
        if (r == 0) {
            success = false;
        }
    }
    ::sleep(1 + rand() % 3);

    if (!success) {
        {
            std::ostringstream oss;
            oss << "!!! Job " << jobId << " FAILED !!!";
            SafePrint(oss.str());
        }
        job.IsSuccess = false;
        Executor->StopExecution_.store(true);
    } else {
        job.IsSuccess = true;
        std::ostringstream oss;
        oss << "[Thread " << pthread_self() << "] Finished job " 
            << jobId << " SUCCESS";
        SafePrint(oss.str());
    }

    job.IsFinished = true;

    // Сообщаем барьеру
    Executor->BarrierArrive(job.BarrierName);

    // Если не было глобального стопа и сама джоба успех — «разблокируем» дочерние
    if (!Executor->StopExecution_.load() && job.IsSuccess.load()) {
        for (int childId : job.Children) {
            int depsLeft = --Executor->Jobs_[childId].RemainDeps;
            if (depsLeft == 0) {
                while (!Executor->StopExecution_.load()) {
                    if (Executor->AreAllParentBarriersUnlocked(childId)) {
                        pthread_mutex_lock(&Executor->QueueMutex_);
                        Executor->ReadyQueue_.push(childId);
                        pthread_cond_signal(&Executor->QueueCond_);
                        pthread_mutex_unlock(&Executor->QueueMutex_);
                        
                        break;
                    } else {
                        std::ostringstream oss;
                        oss << "[Thread " << pthread_self()
                            << "] Child job " << childId
                            << " is ready, but parent's barrier not unlocked yet. Waiting...";
                        SafePrint(oss.str());

                        ::sleep(0.3);
                    }
                }
            }
        }
    }

    // Освобождаем «слот»
    Executor->CurrentActiveThreads_.fetch_sub(1);

    pthread_mutex_lock(&Executor->QueueMutex_);
    pthread_cond_broadcast(&Executor->QueueCond_);
    pthread_mutex_unlock(&Executor->QueueMutex_);

    return nullptr;
}

// ----------------------------------------------------------------------------
// Основной диспетчер: запускает джобы, контролируя maxParallel
// ----------------------------------------------------------------------------
void TDagExecutor::RunDAG() {
    // Добавим в очередь «стартовые» джобы (remainDeps=0)
    pthread_mutex_lock(&QueueMutex_);
    for (auto &kv : Jobs_) {
        if (kv.second.RemainDeps.load() == 0) {
            ReadyQueue_.push(kv.first);
        }
    }
    pthread_mutex_unlock(&QueueMutex_);

    while (true) {
        pthread_mutex_lock(&QueueMutex_);

        while (ReadyQueue_.empty() &&
               CurrentActiveThreads_.load() > 0 &&
               !StopExecution_.load()) 
        {
            pthread_cond_wait(&QueueCond_, &QueueMutex_);
        }

        if (StopExecution_.load()) {
            pthread_mutex_unlock(&QueueMutex_);
            break;
        }

        // Если нет готовых и нет активных — все дела сделаны
        if (ReadyQueue_.empty() && CurrentActiveThreads_.load() == 0) {
            pthread_mutex_unlock(&QueueMutex_);
            break;
        }

        // Пока есть готовые и не превышен лимит — запускаем
        while (!ReadyQueue_.empty() &&
               CurrentActiveThreads_.load() < MaxParallel_ &&
               !StopExecution_.load())
        {
            int jobId = ReadyQueue_.front();
            ReadyQueue_.pop();

            CurrentActiveThreads_.fetch_add(1);

            // Создаём поток
            pthread_t threadId;
            auto *arg = new std::pair<TDagExecutor*, int>(this, jobId);
            pthread_create(&threadId, nullptr, &TDagExecutor::JobRunner, arg);
            pthread_detach(threadId);
        }

        pthread_mutex_unlock(&QueueMutex_);
    }

    if (StopExecution_.load()) {
        pthread_mutex_lock(&QueueMutex_);
        std::queue<int> emptyQ;
        std::swap(ReadyQueue_, emptyQ);
        pthread_mutex_unlock(&QueueMutex_);

        SafePrint("DAG execution interrupted due to job failure.");
    } else {
        SafePrint("DAG execution completed successfully.");
    }
}

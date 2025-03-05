#include <gtest/gtest.h>
#include <fstream>
#include "dag.h"

bool WriteTestConfig(const std::string &filename, const std::string &content) {
    std::ofstream fout(filename);
    if (!fout.is_open()) return false;
    fout << content;
    fout.close();
    return true;
}

// Проверяем, что парсится нормальный DAG без цикла
TEST(TDagExecutor, ParseWithoutCycle) {
    std::string tempFile = "test_no_cycle.ini";
    // Простой DAG из условия
    std::string content = R"INI(
[Jobs]
job=1 barrier=B1
job=2 barrier=B1
job=3 barrier=B2
job=4
job=5
job=6

[Edges]
1->4
2->4
3->5
4->6
5->6

[Parallel]
4
)INI";

    ASSERT_TRUE(WriteTestConfig(tempFile, content));

    TDagExecutor Executor;
    bool ok = Executor.ReadIni(tempFile);
    ASSERT_TRUE(ok);

    // Проверяем, что 6 джоб считались
    auto &jobs = Executor.GetJobs();
    EXPECT_EQ(jobs.size(), 6u);

    // Проверяем цикл
    bool cycle = Executor.CheckCycle();
    EXPECT_FALSE(cycle);

    // Проверяем старт/финиш
    bool startFinish = Executor.CheckStartFinish();
    EXPECT_TRUE(startFinish);

    // Проверяем параллель
    EXPECT_EQ(Executor.GetMaxParallel(), 4);
}

// Проверяем, что обнаруживается цикл
TEST(TDagExecutor, ParseWithCycle) {
    std::string tempFile = "test_cycle.ini";
    std::string content = R"INI(
[Jobs]
job=1
job=2
job=3

[Edges]
1->2
2->3
3->2
)INI";

    ASSERT_TRUE(WriteTestConfig(tempFile, content));

    TDagExecutor Executor;
    bool ok = Executor.ReadIni(tempFile);
    ASSERT_TRUE(ok);

    bool cycle = Executor.CheckCycle();
    EXPECT_TRUE(cycle);
}

// Пустой (или некорректный) файл
TEST(TDagExecutor, ParseEmptyConfig) {
    std::string tempFile = "test_empty.ini";
    std::string content = ""; // пустой
    ASSERT_TRUE(WriteTestConfig(tempFile, content));

    TDagExecutor Executor;
    bool ok = Executor.ReadIni(tempFile);

    ASSERT_TRUE(ok);

    EXPECT_FALSE(Executor.CheckStartFinish());
}

TEST(TDagExecutor, BarrierCheck) {
    std::string content = R"INI(
[Jobs]
job=1 barrier=B1
job=2 barrier=B1
job=3

[Edges]
1->3
2->3

[Parallel]
2
)INI";

    std::string tempFile = "test_barrier.ini";
    ASSERT_TRUE(WriteTestConfig(tempFile, content));

    TDagExecutor Executor;
    bool ok = Executor.ReadIni(tempFile);
    ASSERT_TRUE(ok);

    EXPECT_FALSE(Executor.CheckCycle());
    EXPECT_TRUE(Executor.CheckStartFinish());

    const auto &jobs = Executor.GetJobs();
    ASSERT_EQ(jobs.at(3).Dependencies.size(), 2u);
}

TEST(TDagExecutor, ParallelLimit) {
    std::string content = R"INI(
[Jobs]
job=1
job=2
job=3
job=4

[Edges]
)INI";

    // Тут нет зависимостей, все job "start" => 4 стартовые
    // [Parallel] = 2 => одновременно не более 2
    content += "\n[Parallel]\n2\n";
    std::string tempFile = "test_parallel.ini";
    ASSERT_TRUE(WriteTestConfig(tempFile, content));

    TDagExecutor Executor;
    bool ok = Executor.ReadIni(tempFile);
    ASSERT_TRUE(ok);

    EXPECT_FALSE(Executor.CheckCycle());
    EXPECT_TRUE(Executor.CheckStartFinish());
    EXPECT_EQ(Executor.GetMaxParallel(), 2);
}

// Проверяем поведение при нескольких компонентах связности
TEST(TDagExecutor, ParseMultipleComponents) {
    // job=1, job=2 связаны, а job=3 изолирована => 2 компоненты
    std::string content = R"INI(
[Jobs]
job=1
job=2
job=3

[Edges]
1->2
[Parallel]
2
)INI";

    std::string tempFile = "test_multicomp.ini";
    ASSERT_TRUE(WriteTestConfig(tempFile, content));

    TDagExecutor Executor;
    bool ok = Executor.ReadIni(tempFile);
    ASSERT_TRUE(ok);

    // Нет цикла
    bool cycle = Executor.CheckCycle();
    EXPECT_FALSE(cycle);

    // start/finish: job=1 (start) => job=2 (finish), job=3 (start & finish, но изолирован)
    EXPECT_TRUE(Executor.CheckStartFinish());

    bool singleComp = Executor.CheckSingleComponent();
    EXPECT_FALSE(singleComp); // ожидаем false, так как job=3 изолирована
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

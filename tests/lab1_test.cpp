#include <gtest/gtest.h>
#include <lab1.h>

TEST(ParentProcessTest, CheckSumCalculation) {
    const char* testFileName = "output.txt";
    
    {
        // Ввод для родительского процесса
        std::ofstream testInput("test_input.txt");
        testInput << testFileName << "\n7.6 5.5";
    }
    {
        std::ifstream testFile("test_input.txt");
        ASSERT_TRUE(testFile.is_open()) << "Не удалось открыть файл";

        // Запуск родительского процесса
        RunParentProcess(testFile);  // Передаем поток для тестирования

        // Проверяем, что файл был создан программой
        std::ifstream resultFile(testFileName);
        ASSERT_TRUE(resultFile.good()) << "Файл не был создан";

        // Читаем результат
        std::string line;
        std::getline(resultFile, line);
        EXPECT_EQ(line, "Сумма: 13.100000") << "Неверный результат суммы в файле";
    }

    // Смываем
    std::remove(testFileName);
    std::remove("test_input.txt");
}

TEST(ParentProcessTest, EmptyInput) {
    const char* testFileName = "empty_test_file.txt";

    {
        // Ввод для родительского процесса
        std::ofstream testInput("empty_input.txt");
        testInput << testFileName;
    }
    {
        std::ifstream testFile("empty_input.txt");
        ASSERT_TRUE(testFile.is_open()) << "Не удалось открыть файл";

        // Запуск родительского процесса
        RunParentProcess(testFile);  // Передаем поток для тестирования

        // Проверяем, что файл был создан программой
        std::ifstream resultFile(testFileName);
        ASSERT_TRUE(resultFile.good()) << "Файл не был создан";

        // Читаем результат
        std::string line;
        std::getline(resultFile, line);
        EXPECT_EQ(line, "Сумма: 0.000000") << "Неверный результат для пустого ввода";
    }

    // Удаляем тестовые файлы
    std::remove(testFileName);
    std::remove("empty_input.txt");
}

TEST(ParentProcessTest, LargeNumbersInput) {
    const char* testFileName = "large_test_file.txt";
    
    {
        // Ввод для родительского процесса
        std::ofstream testInput("large_input.txt");
        testInput << testFileName << "\n6.1234 5.230011 1.2 3 .1";
    }
    {
        std::ifstream testFile("large_input.txt");
        ASSERT_TRUE(testFile.is_open()) << "Не удалось открыть файл";

        // Запускаем родительский процесс
        RunParentProcess(testFile);  // Передаем поток для тестирования

        // Проверяем, что файл был создан программой
        std::ifstream resultFile(testFileName);
        ASSERT_TRUE(resultFile.good()) << "Файл не был создан";

        // Читаем результат
        std::string line;
        std::getline(resultFile, line);
        EXPECT_EQ(line, "Сумма: 15.653411") << "Неверный результат для больших чисел";
    }

    // Удаляем тестовый файл
    std::remove(testFileName);
    std::remove("large_input.txt");
}
int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
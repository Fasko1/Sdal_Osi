#include <gtest/gtest.h>
#include <lab3.h>

class ParentProcessTest : public ::testing::Test {
protected:
    std::string testName;
    std::string outputFileName;
    std::string inputFileName;
    
    void SetUp() override {
        testName = ::testing::UnitTest::GetInstance()->current_test_info()->name();

        // Генерируем имена файлов
        outputFileName = testName + "_output.txt";
        inputFileName = testName + "_input.txt";
    }

    void TearDown() override {
        // Удаляем временные файлы после каждого теста
        std::remove(outputFileName.c_str());
        std::remove(inputFileName.c_str());
    }

    // Метод для создания входного файла
    void CreateInputFile(const std::string& inputData) {
        std::ofstream testInput(inputFileName);
        testInput << outputFileName << "\n" << inputData;
    }

    // Метод для проверки выходного файла
    void CheckOutputFile(const std::string& expectedLine) {
        std::ifstream resultFile(outputFileName);
        ASSERT_TRUE(resultFile.is_open()) << "Файл не был создан";

        std::string line;
        std::getline(resultFile, line);
        EXPECT_EQ(line, expectedLine) << "Неверный результат в выходном файле";
    }
};


TEST_F(ParentProcessTest, CheckSumCalculation) {
    // Создаём входной файл
    CreateInputFile("7.6 5.5");

    std::ifstream testFile(inputFileName);
    ASSERT_TRUE(testFile.is_open()) << "Не удалось открыть файл ввода";

    // Запуск родительского процесса
    RunParentProcess(testFile);  // Передаем поток для тестирования

    // Проверяем выходной файл
    CheckOutputFile("Сумма: 13.100000");
}

TEST_F(ParentProcessTest, EmptyInput) {
    // Создаём входной файл (только имя выходного файла)
    CreateInputFile("");

    std::ifstream testFile(inputFileName);
    ASSERT_TRUE(testFile.is_open()) << "Не удалось открыть файл ввода";

    // Запуск родительского процесса
    RunParentProcess(testFile);  // Передаем поток для тестирования

    // Проверяем выходной файл
    CheckOutputFile("Сумма: 0.000000");
}

TEST_F(ParentProcessTest, LargeNumberOfInputs) {
    // Генерируем данные для входного файла
    std::ostringstream inputData;
    for (int i = 1; i <= 100; ++i) {
        inputData << i << " ";
    }

    // Создаём входной файл
    CreateInputFile(inputData.str());

    std::ifstream testFile(inputFileName);
    ASSERT_TRUE(testFile.is_open()) << "Не удалось открыть файл ввода";

    // Запуск родительского процесса
    RunParentProcess(testFile);  // Передаем поток для тестирования

    // Проверяем выходной файл
    CheckOutputFile("Сумма: 5050.000000");
}

TEST_F(ParentProcessTest, InvalidInput) {
    // Создаём входной файл с некорректными данными
    CreateInputFile("10 abc");

    std::ifstream testFile(inputFileName);
    ASSERT_TRUE(testFile.is_open()) << "Не удалось открыть файл ввода";

    // Запуск родительского процесса
    RunParentProcess(testFile);  // Передаем поток для тестирования

    // Проверяем выходной файл
    CheckOutputFile("Сумма: 10.000000");
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
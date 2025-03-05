#include <iostream>
#include "TDeterminantCalculator.h"
#include "TMatrix.h"


int main(int argc, char* argv[]) {
    // Обработка аргументов командной строки
    if (argc != 3 || std::string(argv[1]) != "-t") {
        std::cerr << "Использование: " << argv[0] << " -t <число_потоков>" << std::endl;
        return 1;
    }
    
    int numThreads;
    try {
        numThreads = std::stoi(argv[2]);
    } catch (const std::exception& e) {
        std::cerr << "Ошибка: неверное значение числа потоков." << std::endl;
        return 1;
    }

    if (numThreads <= 0) {
        std::cerr << "Ошибка: число потоков должно быть положительным." << std::endl;
        return 1;
    }

    // Ввод матрицы пользователем
    TMatrix Matrix = InputTMatrix();

    // Вычисляем определитель (разложение по первой строке)
    TDeterminantCalculator Calculator(Matrix, numThreads);
    double determinant = Calculator.Compute();

    std::cout << "Определитель: " << determinant << std::endl;

    return 0;
}

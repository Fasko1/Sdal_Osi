#include <iostream>
#include "TMatrix.h"

TMatrix InputTMatrix() {
    int n;
    std::cout << "Введите размер матрицы (n x n): ";
    std::cin >> n;

    if (n <= 0) {
        std::cerr << "Размер матрицы должен быть положительным." << std::endl;
        exit(1);
    }

    TMatrix Matrix(n, std::vector<double>(n));
    std::cout << "Введите элементы матрицы построчно:\n";
    for (int i = 0; i < n; ++i) {
        std::cout << "Строка " << i + 1 << ": ";
        for (int j = 0; j < n; ++j) {
            std::cin >> Matrix[i][j];
        }
    }

    return Matrix;
}
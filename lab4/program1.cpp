#include <iostream>
#include <vector>
#include "functions.h"

int main() {
    std::cout << "Программа №1 (статическая линковка)\n";

    while (true) {
        std::cout << "\nВведите команду:\n";
        std::cout << "1 A deltaX - вычислить производную cos(A) с приращением deltaX\n";
        std::cout << "2 N elem1 elem2 ... elemN - отсортировать массив из N элементов\n";
        std::cout << "3 - выйти\n";
        int command;
        std::cin >> command;

        if (command == 1) {
            float A, deltaX;
            std::cin >> A >> deltaX;
            float result = Derivative(A, deltaX);
            std::cout << "Производная cos(" << A << ") = " << result << "\n";
        } else if (command == 2) {
            int N;
            std::cin >> N;
            std::vector<int> vect(N);
            for (int i = 0; i < N; ++i) {
                std::cin >> vect[i];
            }
            Sort(vect.data(), N);
            std::cout << "Отсортированный массив: ";
            for (int i = 0; i < N; ++i) {
                std::cout << vect[i] << " ";
            }
            std::cout << "\n";
        } else if (command == 3) {
            break;
        } else {
            std::cout << "Неверная команда\n";
        }
    }

    return 0;
}

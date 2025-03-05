#include <iostream>
#include <dlfcn.h>
#include <cstring>
#include <vector>

using DerivativeFunc = float(*)(float, float);
using SortFunc = int*(*)(int*, int);

int main() {
    std::cout << "Программа №2 (динамическая загрузка библиотек)\n";

    const char* pathToLib1 = std::getenv("PATH_TO_LIB1");
    if (!pathToLib1) {
        std::cerr << "Переменная окружения PATH_TO_LIB1 не установлена" << std::endl;
        return 1;
    }

    const char* pathToLib2 = std::getenv("PATH_TO_LIB2");
    if (!pathToLib2) {
        std::cerr << "Переменная окружения PATH_TO_LIB2 не установлена" << std::endl;
        return 1;
    }

    const char* lib_paths[] = {pathToLib1, pathToLib2};
    int current_lib = 0;
    void* handle = dlopen(lib_paths[current_lib], RTLD_LAZY);

    if (!handle) {
        std::cerr << "Ошибка загрузки библиотеки: " << dlerror() << "\n";
        return 1;
    }

    DerivativeFunc Derivative = reinterpret_cast<DerivativeFunc>(dlsym(handle, "Derivative"));
    SortFunc Sort = reinterpret_cast<SortFunc>(dlsym(handle, "Sort"));

    char* error;
    if ((error = dlerror()) != nullptr) {
        std::cerr << "Ошибка получения символа: " << error << "\n";
        dlclose(handle);
        return 1;
    }

    while (true) {
        std::cout << "\nТекущая библиотека: " << lib_paths[current_lib] << "\n";
        std::cout << "Введите команду:\n";
        std::cout << "0 - переключить реализацию\n";
        std::cout << "1 A deltaX - вычислить производную cos(A) с приращением deltaX\n";
        std::cout << "2 N elem1 elem2 ... elemN - отсортировать массив из N элементов\n";
        std::cout << "3 - выйти\n";
        int command;
        std::cin >> command;

        if (command == 0) {
            // Переключаем библиотеку
            dlclose(handle);
            current_lib = 1 - current_lib;
            handle = dlopen(lib_paths[current_lib], RTLD_LAZY);
            if (!handle) {
                std::cerr << "Ошибка загрузки библиотеки: " << dlerror() << "\n";
                return 1;
            }
            Derivative = (DerivativeFunc)dlsym(handle, "Derivative");
            Sort = (SortFunc)dlsym(handle, "Sort");

            if ((error = dlerror()) != nullptr) {
                std::cerr << "Ошибка получения символа: " << error << "\n";
                dlclose(handle);
                return 1;
            }
        } else if (command == 1) {
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

    dlclose(handle);
    return 0;
}

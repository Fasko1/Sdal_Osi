#include "lab1.h"

int main() {
    // Чтение имени файла из pipe1 (STDIN)
    char fileName[50];
    read(0, fileName, 50);

    // Чтение чисел из pipe1 и подсчет суммы
    float num, sum = 0;
    while (read(0, &num, sizeof(num)) > 0) {
        sum += num;
    }

    {
        // Открытие файла для записи
        std::ofstream file(fileName);
        if (!file.is_open()) {
            std::cerr << "Ошибка при открытии файла" << std::endl;
            exit(1);
        }

        // Запись суммы в файл
        file << std::fixed << std::setprecision(6);
        file << "Сумма: " << sum << '\n';
    }

    // Передача результата родительскому процессу через pipe2 (STDOUT)
    std::cout << std::fixed << std::setprecision(6);
    std::cout << "Сумма: " << sum << '\n';

    return 0;
}

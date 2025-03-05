#include "lab1.h"

void RunParentProcess(std::istream& stream) {
    int pipe1[2], pipe2[2];

    if (pipe(pipe1) == -1 || pipe(pipe2) == -1) {
        std::cerr << "Ошибка создания pipe" << std::endl;
        exit(1);
    }

    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Ошибка fork" << std::endl;
        exit(1);
    }

    if (pid > 0) {
        // Родительский процесс

        // Закрывваем неиспользуемое родительским процессом
        close(pipe1[0]);
        close(pipe2[1]);
        
        char fileName[50];
        std::cout << "Введите имя файла:\n";
        stream >> fileName;

        // Отправляем имя файла дочернему процессу
        if (write(pipe1[1], &fileName, 50) == -1) {
            std::cerr << "Ошибка write" << std::endl;
            close(pipe1[1]);
            exit(1);
        }

        // Отправляем числа дочернему процессу
        float num;
        std::cout << "Введите числа (EOF для завершения):\n";
        while (stream >> num) {
            if (write(pipe1[1], &num, sizeof(num)) == -1) {
                std::cerr << "Ошибка write" << std::endl;
                close(pipe1[1]);
                exit(1);
            };
        }
        close(pipe1[1]);

        // Ждем завершения дочернего процесса
        wait(nullptr);

        // Получаем результат из pipe2
        char buffer[100];
        int bytesRead = read(pipe2[0], buffer, sizeof(buffer) - 1);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';  // Завершаем строку
        }
        close(pipe2[0]);

        std::cout << "\nРезультат от дочернего процесса: " << buffer;
    } else {
        // Дочерний процесс

        // Закрываем неиспользуемое
        close(pipe1[1]);
        close(pipe2[0]);

        // Готовим почву для последующего принятия fileName через pipe и передачи результата родителю
        if (dup2(pipe1[0], 0) == -1 or dup2(pipe2[1], 1) == -1) {
            std::cerr << "Ошибка dup2" << std::endl;
            exit(1);
        }
        close(pipe1[0]);  // Закрываем конец чтения pipe1, он больше не нужен, раз мы переопределили вход
        close(pipe2[1]);  // Закрываем конец записи pipe2

        // Дочерний процесс через exec
        const char* pathToChild = std::getenv("PATH_TO_CHILD");
        execlp(pathToChild, pathToChild, nullptr);

        std::cerr << "Ошибка exec" << std::endl;
        exit(1);
    }
}

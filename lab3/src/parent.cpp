#include "lab3.h"
#include <limits>


void RunParentProcess(std::istream& stream) {
    int fd = shm_open(SHARED_FILE, O_CREAT | O_RDWR, 0666);
    if (fd == -1) {
        std::cerr << "Ошибка открытия общего файла" << std::endl;
        exit(1);
    }

    // Устанавливаем размер файла
    if (ftruncate(fd, sizeof(SharedData)) == -1) {
        std::cerr << "Ошибка ftruncate" << std::endl;
        shm_unlink(SHARED_FILE);
        exit(1);
    }

    // Отображаем файл в память
    SharedData* shared = (SharedData*)mmap(nullptr, sizeof(SharedData),
                                           PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        std::cerr << "Ошибка mmap" << std::endl;
        close(fd);
        shm_unlink(SHARED_FILE);
        exit(1);
    }
    close(fd);

    sem_init(&shared->sem_parent, 1, 0);
    sem_init(&shared->sem_child, 1, 0);

    pid_t pid = fork();

    if (pid < 0) {
        std::cerr << "Ошибка fork" << std::endl;
        exit(1);
    }

    if (pid > 0) {
        // Родительский процесс

        // Ввод данных
        std::cout << "Введите имя файла:\n";
        stream.getline(shared->fileName, sizeof(shared->fileName));

        sem_post(&shared->sem_child);

        std::cout << "Введите числа (EOF для завершения):\n";
        float num;
        while (stream >> num) {
            shared->finished = false;
            shared->number = num;
            
            sem_post(&shared->sem_child);

            sem_wait(&shared->sem_parent);
        }
        shared->finished = true;
        sem_post(&shared->sem_child);

        // Очищаем поток ввода
        stream.clear();
        stream.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        sem_wait(&shared->sem_parent);

        // Читаем результат
        std::cout << "\nРезультат от дочернего процесса: Сумма = " << shared->sum << std::endl;

        // Уничтожаем семафоры
        sem_destroy(&shared->sem_parent);
        sem_destroy(&shared->sem_child);

        // Удаляем отображение памяти
        munmap(shared, sizeof(SharedData));

        // Удаляем файл
        shm_unlink(SHARED_FILE);

        // Ждем завершения дочернего процесса
        wait(nullptr);
    } else {
        // Дочерний процесс

        const char* pathToChild = std::getenv("PATH_TO_CHILD");
        if (!pathToChild) {
            std::cerr << "Переменная окружения PATH_TO_CHILD не установлена" << std::endl;
            exit(1);
        }
        execlp(pathToChild, pathToChild, nullptr);

        std::cerr << "Ошибка exec, проверьте корректность PATH_TO_CHILD" << std::endl;
        exit(1);
    }
}

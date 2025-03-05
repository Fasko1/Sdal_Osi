#include "lab3.h"

int main() {
    int fd = shm_open(SHARED_FILE, O_RDWR, 0666);
    if (fd == -1) {
        std::cerr << "Ошибка открытия общего файла в дочернем процессе" << std::endl;
        exit(1);
    }

    // Отображаем файл в память
    SharedData* shared = (SharedData*)mmap(nullptr, sizeof(SharedData),
                                           PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (shared == MAP_FAILED) {
        std::cerr << "Ошибка mmap в дочернем процессе" << std::endl;
        close(fd);
        shm_unlink(SHARED_FILE);
        exit(1);
    }
    close(fd);

    // Ждем сигнал от родительского процесса
    sem_wait(&shared->sem_child);

    // Читаем данные
    char fileName[256];
    strcpy(fileName, shared->fileName);

    float sum = 0;
    while (true) {
        // Ждем число
        sem_wait(&shared->sem_child);

        if (shared->finished) {
            break;
        }

        sum += shared->number;

        // Можно продолжать
        sem_post(&shared->sem_parent);
    }

    // Записываем сумму в файл
    {
        std::ofstream file(fileName);
        if (!file.is_open()) {
            std::cerr << "Ошибка при открытии файла" << std::endl;
            exit(1);
        }

        file << std::fixed << std::setprecision(6);
        file << "Сумма: " << sum << '\n';
    }

    // Записываем результат в общую память
    shared->sum = sum;

    // Сигнализируем родительскому процессу о завершении
    sem_post(&shared->sem_parent);

    // Удаляем отображение памяти
    munmap(shared, sizeof(SharedData));

    return 0;
}

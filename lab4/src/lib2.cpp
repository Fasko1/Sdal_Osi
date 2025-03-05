#include <cmath>
#include "functions.h"

extern "C" {
    float Derivative(float A, float deltaX) {
        return (cosf(A + deltaX) - cosf(A - deltaX)) / (2 * deltaX);
    }

    int Partition(int* array, int low, int high) {
        int pivot = array[high]; // Опорный элемент
        int i = low - 1;

        for (int j = low; j < high; ++j) {
            if (array[j] < pivot) {
                ++i;
                // Обмен элементов
                int temp = array[i];
                array[i] = array[j];
                array[j] = temp;
            }
        }
        // Обмен опорного элемента
        array[high] = array[i + 1];
        array[i + 1] = pivot;
        return i + 1;
    }

    void QuickSort(int* array, int low, int high) {
        if (low < high) {
            int pi = Partition(array, low, high);

            QuickSort(array, low, pi - 1);
            QuickSort(array, pi + 1, high);
        }
    }

    int* Sort(int* array, int size) {
        // Сортировка Хоара
        QuickSort(array, 0, size - 1);
        return array;
    }
}

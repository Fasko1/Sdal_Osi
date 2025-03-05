#include <cmath>
#include "functions.h"

extern "C" {
    float Derivative(float A, float deltaX) {
        return (cosf(A + deltaX) - cosf(A)) / deltaX;
    }

    int* Sort(int* array, int size) {
        // Пузырьковая сортировка
        for (int i = 0; i < size - 1; ++i) {
            for (int j = 0; j < size - i - 1; ++j) {
                if (array[j] > array[j + 1]) {
                    int temp = array[j];
                    array[j] = array[j + 1];
                    array[j + 1] = temp;
                }
            }
        }
        return array;
    }
}

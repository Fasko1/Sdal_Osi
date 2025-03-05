#ifndef FUNCTIONS_H
#define FUNCTIONS_H

#ifdef __cplusplus
extern "C" { // Совместимость для динамической загрузки
#endif

    float Derivative(float A, float deltaX);
    int* Sort(int* array, int size);

#ifdef __cplusplus
}
#endif

#endif

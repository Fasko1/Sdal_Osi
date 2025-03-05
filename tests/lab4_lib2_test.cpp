#include <gtest/gtest.h>
#include <cmath>
#include <functions.h>

TEST(DerivativeTest, Implementation2) {
    // Тестируем Derivative из lib2
    float A = 0.5f;
    float deltaX = 0.01f;

    float result = Derivative(A, deltaX);
    float expected = (cosf(A + deltaX) - cosf(A - deltaX)) / (2 * deltaX);

    EXPECT_NEAR(result, expected, 1e-5);
}

TEST(SortTest, Implementation2) {
    // Тестируем Sort из lib2
    int array[] = {5, 2, 3, 1, 4};
    int expected[] = {1, 2, 3, 4, 5};
    int size = sizeof(array) / sizeof(array[0]);

    int* result = Sort(array, size);

    for (int i = 0; i < size; ++i) {
        EXPECT_EQ(result[i], expected[i]);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
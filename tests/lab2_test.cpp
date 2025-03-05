#include <TDeterminantCalculator.h>
#include <TMatrix.h>
#include <TDeterminantCalculator.h>
#include <TMatrix.h>
#include <gtest/gtest.h>
#include <chrono>


// Проверка корректности однопоточного варианта
TEST(DeterminantCalculatorTest, SingleThreadCorrectness) {
    // 1x1 матрица
    TMatrix Mat1 = {{5}};
    TDeterminantCalculator Calc1(Mat1, 1);
    double det1 = Calc1.Compute();
    EXPECT_DOUBLE_EQ(det1, 5.0);

    // 2x2 матрица
    TMatrix Mat2 = {
        {1, 2},
        {3, 4}
    };
    TDeterminantCalculator Calc2(Mat2, 1);
    double det2 = Calc2.Compute();
    EXPECT_DOUBLE_EQ(det2, -2.0);

    // 3x3 матрица
    TMatrix Mat3 = {
        {6, 1, 1},
        {4, -2, 5},
        {2, 8, 7}
    };
    TDeterminantCalculator Calc3(Mat3, 1);
    double det3 = Calc3.Compute();
    EXPECT_DOUBLE_EQ(det3, -306.0);

    // 4x4 матрица
    TMatrix Mat4 = {
        {3, 2, 0, 1},
        {4, 0, 1, 2},
        {3, 0, 2, 1},
        {9, 2, 3, 1}
    };
    TDeterminantCalculator Calc4(Mat4, 1);
    double det4 = Calc4.Compute();
    EXPECT_DOUBLE_EQ(det4, 24.0);
}

// Проверка соответствия результатов однопоточного и многопоточного вариантов
TEST(DeterminantCalculatorTest, SingleVsMultiThreadConsistency) {
    // 3x3 матрица
    TMatrix Mat = {
        {6, 1, 1},
        {4, -2, 5},
        {2, 8, 7}
    };

    // Однопоточный вариант
    TDeterminantCalculator CalcSingle(Mat, 1);
    double detSingle = CalcSingle.Compute();

    // Многопоточный вариант
    for (int i = 2; i <= 8; i += 2) {
        TDeterminantCalculator CalcMulti(Mat, i);
        double detMulti = CalcMulti.Compute();
        EXPECT_DOUBLE_EQ(detSingle, detMulti);
    }
}

// Проверка производительности многопоточного варианта
TEST(DeterminantCalculatorTest, MultiThreadPerformance) {
    // 11x11 матрица
    TMatrix Mat(11, std::vector<double>(11, 2.0));

    // Измеряем время для однопоточного варианта
    TDeterminantCalculator CalcSingle(Mat, 1);
    auto startSingle = std::chrono::high_resolution_clock::now();
    double detSingle = CalcSingle.Compute();
    auto endSingle = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationSingle = endSingle - startSingle;

    // Измеряем время для многопоточного варианта (например, 8 потоков)
    TDeterminantCalculator CalcMulti(Mat, 8);
    auto startMulti = std::chrono::high_resolution_clock::now();
    double detMulti = CalcMulti.Compute();
    auto endMulti = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> durationMulti = endMulti - startMulti;

    // Проверяем, что результаты совпадают
    EXPECT_DOUBLE_EQ(detSingle, detMulti);

    // Проверяем, что многопоточный вариант быстрее
    std::cout << "Однопоточный вариант: " << durationSingle.count() << " секунд\n";
    std::cout << "Многопоточный вариант: " << durationMulti.count() << " секунд\n";
    
    EXPECT_LT(durationMulti.count(), durationSingle.count());
}

int main(int argc, char **argv) {
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

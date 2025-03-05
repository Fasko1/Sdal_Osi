add_executable(lab1_test lab1_test.cpp ../lab1/include/lab1.h ../lab1/src/parent.cpp)
add_executable(lab2_test lab2_test.cpp ../lab2/src/TDeterminantCalculator.cpp ../lab2/include/TDeterminantCalculator.h ../lab2/src/TMatrix.cpp ../lab2/include/TMatrix.h)
add_executable(lab3_test lab3_test.cpp ../lab3/src/parent.cpp ../lab3/include/lab3.h)
add_executable(lab4_lib1_test lab4_lib1_test.cpp)
add_executable(lab4_lib2_test lab4_lib2_test.cpp)
add_executable(lab5-7_test lab5-7_test.cpp ../lab5-7/src/tcontroller.cpp ../lab5-7/src/tmessaging.cpp ../lab5-7/src/tsearch.cpp ../lab5-7/src/ttopology.cpp ../lab5-7/src/tworker.cpp)
add_executable(CP_test CP_test.cpp ../CP/src/dag.cpp)

# Установка Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.15.2
  TLS_VERIFY false
)
FetchContent_MakeAvailable(googletest)
target_link_libraries(lab1_test PRIVATE gtest_main)
target_link_libraries(lab2_test PRIVATE gtest_main)
target_link_libraries(lab3_test PRIVATE gtest_main)
target_link_libraries(lab4_lib1_test PRIVATE gtest_main lib1)
target_link_libraries(lab4_lib2_test PRIVATE gtest_main lib2)
target_link_libraries(lab5-7_test PRIVATE gtest_main zmq)
target_link_libraries(CP_test PRIVATE gtest_main)

target_include_directories(lab1_test PRIVATE ../lab1/include)
target_include_directories(lab2_test PRIVATE ../lab2/include)
target_include_directories(lab3_test PRIVATE ../lab3/include)
target_include_directories(lab4_lib1_test PRIVATE ../lab4/include)
target_include_directories(lab4_lib2_test PRIVATE ../lab4/include)
target_include_directories(lab5-7_test PRIVATE ../lab5-7/include)
target_include_directories(CP_test PRIVATE ../CP/include)

add_test(lab1_test lab1_test)
add_test(lab2_test lab2_test)
add_test(lab3_test lab3_test)
add_test(lab4_lib1_test lab4_lib1_test)
add_test(lab4_lib2_test lab4_lib2_test)
add_test(lab5-7_test lab5-7_test)
add_test(CP_test CP_test)

include(GoogleTest)

gtest_discover_tests(lab1_test)
gtest_discover_tests(lab2_test)
gtest_discover_tests(lab3_test)
gtest_discover_tests(lab4_lib1_test)
gtest_discover_tests(lab4_lib2_test)
gtest_discover_tests(lab5-7_test)
gtest_discover_tests(CP_test)

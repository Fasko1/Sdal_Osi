add_executable(lab3 main.cpp src/parent.cpp include/lab3.h)
target_include_directories(lab3 PRIVATE include)

add_executable(lab3_child src/child.cpp include/lab3.h)
target_include_directories(lab3_child PRIVATE include)

add_executable(lab1 main.cpp src/parent.cpp include/lab1.h)
add_executable(child src/child.cpp include/lab1.h)

target_include_directories(lab1 PRIVATE include)
target_include_directories(child PRIVATE include)

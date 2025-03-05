add_library(lib1 SHARED src/lib1.cpp)
set_target_properties(lib1 PROPERTIES OUTPUT_NAME "lib1" PREFIX "")
target_include_directories(lib1 PRIVATE include)

add_library(lib2 SHARED src/lib2.cpp)
set_target_properties(lib2 PROPERTIES OUTPUT_NAME "lib2" PREFIX "")
target_include_directories(lib2 PRIVATE include)

add_executable(program1 program1.cpp)
add_executable(program2 program2.cpp)

target_include_directories(program1 PRIVATE include)

target_link_libraries(program1 PRIVATE lib1)

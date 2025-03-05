
find_package(PkgConfig REQUIRED)
pkg_check_modules(ZMQ REQUIRED libzmq)

add_executable(lab5-7_controller
    main_controller.cpp
    src/tcontroller.cpp
    src/tmessaging.cpp
    src/tsearch.cpp
    src/ttopology.cpp
)
target_link_libraries(lab5-7_controller PRIVATE zmq)
target_include_directories(lab5-7_controller PRIVATE include)

add_executable(lab5-7_worker
    main_worker.cpp
    src/tworker.cpp
    src/tmessaging.cpp
    src/tsearch.cpp
    src/ttopology.cpp
)
target_link_libraries(lab5-7_worker PRIVATE zmq)
target_include_directories(lab5-7_worker PRIVATE include)

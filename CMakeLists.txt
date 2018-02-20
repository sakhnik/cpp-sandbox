cmake_minimum_required(VERSION 3.10)

add_definitions(-std=c++17)
add_executable(run
    BitStream.cpp
    main.cpp
    )

enable_testing()
add_test("all" run)

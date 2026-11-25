
cmake_minimum_required(VERSION 3.20)

project(NetworkLab LANGUAGES C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(SFML 3 REQUIRED COMPONENTS Graphics)

add_executable(NetworkLab
    main.cpp
    network.c
    graph.c
    link.c
    dijkstra.c
)

target_link_libraries(NetworkLab PRIVATE SFML::Graphics)

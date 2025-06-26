cmake_minimum_required(VERSION 3.10)
project(BYkaKeyAgreement)

set(CMAKE_CXX_STANDARD 17)

include_directories(include)

add_executable(byka
    src/main.cpp
    src/Matrix.cpp
    src/KeyGenerator.cpp
    src/Node.cpp
    src/BYka.cpp
)

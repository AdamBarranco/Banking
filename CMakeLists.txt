cmake_minimum_required(VERSION 3.30)
project(Banking VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

# === Catch2 setup via FetchContent ===
include(FetchContent)
FetchContent_Declare(
        Catch2
        GIT_REPOSITORY https://github.com/catchorg/Catch2.git
        GIT_TAG v3.6.0  # use latest stable tag
)
FetchContent_MakeAvailable(Catch2)

# === Unit tests ===
enable_testing()

add_executable(bank_tests bank_tests.cpp)
target_link_libraries(bank_tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(bank_tests)


add_executable(Banking main.cpp)

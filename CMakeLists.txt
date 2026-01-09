cmake_minimum_required(VERSION 3.30)
project(Banking VERSION 1.0)

set(CMAKE_CXX_STANDARD 20)

# === Source files ===
set(BANK_SOURCES
    src/Bank.cpp
)

set(BANK_HEADERS
    src/Bank.h
    src/Constants.h
    src/Transaction.h
)

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

add_executable(bank_tests bank_tests.cpp ${BANK_SOURCES})
target_include_directories(bank_tests PRIVATE src)
target_link_libraries(bank_tests PRIVATE Catch2::Catch2WithMain)

include(Catch)
catch_discover_tests(bank_tests)


add_executable(Banking src/main.cpp ${BANK_SOURCES})
target_include_directories(Banking PRIVATE src)

# === Web Server ===
set(WEBSERVER_SOURCES
    src/WebServer.cpp
)

add_executable(BankingWeb src/web_main.cpp ${BANK_SOURCES} ${WEBSERVER_SOURCES})
target_include_directories(BankingWeb PRIVATE src)

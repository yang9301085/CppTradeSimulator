#[[
File: CMakeLists.txt
Purpose: Build configuration for the CppTradeSimulator project.
Notes:
- Define library targets for core simulator modules.
- Define executables for CLI and tests.
- Centralize compiler settings and include paths.
]]
cmake_minimum_required(VERSION 3.16)
project(CppTradeSimulator LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Library: trade_sim
add_library(trade_sim
    src/io/Storage.cpp
    src/order/OrderFactory.cpp
    src/core/OrderManager.cpp
    src/core/AccountManager.cpp
    src/core/HistoryManager.cpp
    src/core/MatchingEngine.cpp
    src/core/TradeExecutor.cpp
)

target_include_directories(trade_sim PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

# (Optional) warnings
if (MSVC)
    target_compile_options(trade_sim PRIVATE /W4 /permissive-)
else()
    target_compile_options(trade_sim PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Executable: CLI
add_executable(trade_sim_cli
    src/main.cpp
)
target_link_libraries(trade_sim_cli PRIVATE trade_sim)

# Executable: smoke_test
add_executable(smoke_test
    test/smoke_test.cpp
)
target_link_libraries(smoke_test PRIVATE trade_sim)

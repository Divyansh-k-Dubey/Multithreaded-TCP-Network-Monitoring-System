cmake_minimum_required(VERSION 3.10)
project(Multithreaded_TCP_Network_Monitoring_System VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add compiler warning flags
add_compile_options(-Wall -Wextra -Wpedantic)

# Find pthreads library
find_package(Threads REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)

# Shared implementation sources
set(COMMON_SOURCES
    src/Telemetry.cpp
    src/NetworkUtils.cpp
)

set(SERVER_SOURCES
    src/server.cpp
    src/Logger.cpp
    src/Device.cpp
    src/FaultDetector.cpp
    src/DeviceManager.cpp
    ${COMMON_SOURCES}
)

set(CLIENT_SOURCES
    src/client.cpp
    ${COMMON_SOURCES}
)

# Executable targets
add_executable(network_monitor_server ${SERVER_SOURCES})
target_link_libraries(network_monitor_server PRIVATE Threads::Threads)

add_executable(network_monitor_client ${CLIENT_SOURCES})
target_link_libraries(network_monitor_client PRIVATE Threads::Threads)

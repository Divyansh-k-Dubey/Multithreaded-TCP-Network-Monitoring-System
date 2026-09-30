# Makefile for Multithreaded TCP Network Monitoring System

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic -Iinclude
LDFLAGS = -pthread

BUILD_DIR = build
BIN_SERVER = network_monitor_server
BIN_CLIENT = network_monitor_client

SERVER_SRCS = src/server.cpp src/Logger.cpp src/Device.cpp src/FaultDetector.cpp src/DeviceManager.cpp src/Telemetry.cpp src/NetworkUtils.cpp
CLIENT_SRCS = src/client.cpp src/Telemetry.cpp src/NetworkUtils.cpp

SERVER_OBJS = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(SERVER_SRCS))
CLIENT_OBJS = $(patsubst src/%.cpp, $(BUILD_DIR)/%.o, $(CLIENT_SRCS))

.PHONY: all clean server client setup

all: setup $(BIN_SERVER) $(BIN_CLIENT)

setup:
	@mkdir -p $(BUILD_DIR)
	@mkdir -p logs

$(BIN_SERVER): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(BIN_CLIENT): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) $^ $(LDFLAGS) -o $@

$(BUILD_DIR)/%.o: src/%.cpp
	@mkdir -p $(dir $@)
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR) $(BIN_SERVER) $(BIN_CLIENT) logs/*.log

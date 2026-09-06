# DHARTI Pure C++ Makefile
CXX = g++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra -static -static-libgcc -static-libstdc++ -Iinclude -Isrc/native
SRCDIR = src
INCDIR = include
BINDIR = bin

SOURCES = \
	src/config/settings.cpp \
	src/utils/logger.cpp \
	src/services/pci_engine.cpp \
	src/native/pci_engine.cpp \
	src/native/contradiction_engine.cpp \
	src/native/sia_inclusion_engine.cpp \
	src/native/dharti_c_api.cpp \
	src/adapters/land_record_adapter.cpp \
	src/adapters/court_adapter.cpp \
	src/adapters/finance_adapter.cpp \
	src/core/event_store.cpp \
	src/services/payment_reconciler.cpp \
	src/services/workflow_coordinator.cpp

TEST_SOURCES = tests/cpp/test_main.cpp $(SOURCES)
CLI_SOURCES = src/cli/main.cpp $(SOURCES)

TARGET_LIB = src/native/dharti_core.dll
TEST_BIN = tests/cpp/test_dharti_core.exe
CLI_BIN = bin/dharti_cli.exe

.PHONY: all clean test test-nyayabot cli

all: $(TARGET_LIB) $(TEST_BIN) $(CLI_BIN)

$(TARGET_LIB): $(SOURCES)
	@echo [CXX] Building shared library $(TARGET_LIB)...
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SOURCES)

$(TEST_BIN): $(TEST_SOURCES)
	@echo [CXX] Building test suite $(TEST_BIN)...
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_SOURCES)

$(CLI_BIN): $(CLI_SOURCES)
	@if not exist bin mkdir bin
	@echo [CXX] Building generalized CLI $(CLI_BIN)...
	$(CXX) $(CXXFLAGS) -o $@ $(CLI_SOURCES)

test: $(TEST_BIN)
	@echo [TEST] Executing DHARTI C++ Test Suite...
	./$(TEST_BIN)

test-nyayabot:
	@echo [TEST] Executing NyayaBot JavaScript Test Suite...
	node tests/js/test_nyaya_bot.js

cli: $(CLI_BIN)
	@echo [RUN] Launching DHARTI CLI...
	./$(CLI_BIN)

clean:
	@echo [CLEAN] Removing binaries...
	del /Q $(TARGET_LIB) $(TEST_BIN) $(CLI_BIN) 2>NUL || rm -f $(TARGET_LIB) $(TEST_BIN) $(CLI_BIN)

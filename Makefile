# DHARTI Pure C++ Cross-Platform Makefile
CXX ?= g++
CXXFLAGS = -O3 -std=c++17 -Wall -Wextra -Iinclude -Isrc/native
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
	src/services/workflow_coordinator.cpp \
	src/adapters/parivesh_adapter.cpp \
	src/services/evidence_engine.cpp \
	src/storage/gdrive_client.cpp \
	src/storage/neon_client.cpp \
	src/scrapers/web_scraper.cpp \
	src/adapters/bhoomi_rashi_adapter.cpp \
	src/services/polling_daemon.cpp \
	src/services/explanatory_query_engine.cpp

TEST_SOURCES = tests/cpp/test_main.cpp $(SOURCES)
CLI_SOURCES = src/cli/main.cpp $(SOURCES)

# OS Detection
ifeq ($(OS),Windows_NT)
    CXXFLAGS += -static -static-libgcc -static-libstdc++
    TARGET_LIB = src/native/dharti_core.dll
    TEST_BIN = tests/cpp/test_dharti_core.exe
    CLI_BIN = bin/dharti_cli.exe
    MKDIR_BIN = if not exist bin mkdir bin
    CLEAN_CMD = del /Q $(TARGET_LIB) $(TEST_BIN) $(CLI_BIN) 2>NUL || rm -f $(TARGET_LIB) $(TEST_BIN) $(CLI_BIN)
else
    UNAME_S := $(shell uname -s)
    ifeq ($(UNAME_S),Darwin)
        TARGET_LIB = src/native/libdharti_core.dylib
    else
        TARGET_LIB = src/native/libdharti_core.so
    endif
    TEST_BIN = tests/cpp/test_dharti_core
    CLI_BIN = bin/dharti_cli
    MKDIR_BIN = mkdir -p bin
    CLEAN_CMD = rm -f $(TARGET_LIB) $(TEST_BIN) $(CLI_BIN)
endif

.PHONY: all clean test cli

all: $(TARGET_LIB) $(TEST_BIN) $(CLI_BIN)

$(TARGET_LIB): $(SOURCES)
	@echo [CXX] Building shared library $(TARGET_LIB)...
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SOURCES)

$(TEST_BIN): $(TEST_SOURCES)
	@echo [CXX] Building test suite $(TEST_BIN)...
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_SOURCES)

$(CLI_BIN): $(CLI_SOURCES)
	@$(MKDIR_BIN)
	@echo [CXX] Building generalized CLI $(CLI_BIN)...
	$(CXX) $(CXXFLAGS) -o $@ $(CLI_SOURCES)

test: $(TEST_BIN)
	@echo [TEST] Executing DHARTI C++ Test Suite...
	./$(TEST_BIN)

cli: $(CLI_BIN)
	@echo [RUN] Launching DHARTI CLI...
	./$(CLI_BIN)

clean:
	@echo [CLEAN] Removing binaries...
	$(CLEAN_CMD)


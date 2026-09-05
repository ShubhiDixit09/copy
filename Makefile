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
	src/native/dharti_c_api.cpp

TEST_SOURCES = tests/cpp/test_main.cpp $(SOURCES)

TARGET_LIB = src/native/dharti_core.dll
TEST_BIN = tests/cpp/test_dharti_core.exe

.PHONY: all clean test

all: $(TARGET_LIB) $(TEST_BIN)

$(TARGET_LIB): $(SOURCES)
	@echo [CXX] Building shared library $(TARGET_LIB)...
	$(CXX) $(CXXFLAGS) -shared -o $@ $(SOURCES)

$(TEST_BIN): $(TEST_SOURCES)
	@echo [CXX] Building test suite $(TEST_BIN)...
	$(CXX) $(CXXFLAGS) -o $@ $(TEST_SOURCES)

test: $(TEST_BIN)
	@echo [TEST] Executing DHARTI C++ Test Suite...
	./$(TEST_BIN)

clean:
	@echo [CLEAN] Removing binaries...
	del /Q $(TARGET_LIB) $(TEST_BIN) 2>NUL || rm -f $(TARGET_LIB) $(TEST_BIN)

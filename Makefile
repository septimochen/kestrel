BUILD_DIR ?= build
BUILD_TYPE ?= Debug
DEPTH ?= 3
CXX_STANDARD ?= 23
SANITIZERS ?= OFF
CPP_EXPERIMENTS ?= OFF
CLANG_FORMAT ?= clang-format
CPP_FILES := $(wildcard src/*.cpp include/kestrel/*.hpp tests/*.cpp examples/*.cpp)

.PHONY: all configure build check test run perft format

all: build

configure:
	cmake -S . -B "$(BUILD_DIR)" -DCMAKE_BUILD_TYPE="$(BUILD_TYPE)" -DKESTREL_CXX_STANDARD="$(CXX_STANDARD)" -DKESTREL_SANITIZERS="$(SANITIZERS)" -DKESTREL_CPP_EXPERIMENTS="$(CPP_EXPERIMENTS)"

build: configure
	cmake --build "$(BUILD_DIR)"

check: build
	ctest --test-dir "$(BUILD_DIR)" --output-on-failure

test: check

run: build
	"./$(BUILD_DIR)/kestrel"

perft: build
	"./$(BUILD_DIR)/kestrel" perft "$(DEPTH)"

format:
	$(CLANG_FORMAT) -i $(CPP_FILES)

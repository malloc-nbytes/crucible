CXX      ?= c++
CPPFLAGS ?= -Iinclude
CXXFLAGS ?= -std=gnu++20 -Wall -Wextra -Wpedantic
LDFLAGS  ?=
LDLIBS   ?=

PROGRAM   := crucible
SOURCE_DIR := .
BUILD_DIR  := build

SOURCES := $(sort $(wildcard $(SOURCE_DIR)/*.cpp))

RELEASE_DIR     := $(BUILD_DIR)/release
RELEASE_OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp,$(RELEASE_DIR)/%.o,$(SOURCES))
RELEASE_DEPS    := $(RELEASE_OBJECTS:.o=.d)
RELEASE_PROGRAM := $(RELEASE_DIR)/$(PROGRAM)

DEBUG_DIR     := $(BUILD_DIR)/debug
DEBUG_OBJECTS := $(patsubst $(SOURCE_DIR)/%.cpp,$(DEBUG_DIR)/%.o,$(SOURCES))
DEBUG_DEPS    := $(DEBUG_OBJECTS:.o=.d)
DEBUG_PROGRAM := $(DEBUG_DIR)/$(PROGRAM)

RELEASE_FLAGS := -O2 -DNDEBUG
DEBUG_FLAGS   := -O0 -g3

.PHONY: all release debug clean rebuild

all: release

release: $(RELEASE_PROGRAM)

debug: $(DEBUG_PROGRAM)

$(RELEASE_PROGRAM): $(RELEASE_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(DEBUG_PROGRAM): $(DEBUG_OBJECTS)
	@mkdir -p $(@D)
	$(CXX) $(LDFLAGS) -o $@ $^ $(LDLIBS)

$(RELEASE_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(RELEASE_FLAGS) -MMD -MP -c -o $@ $<

$(DEBUG_DIR)/%.o: $(SOURCE_DIR)/%.cpp
	@mkdir -p $(@D)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) $(DEBUG_FLAGS) -MMD -MP -c -o $@ $<

clean:
	rm -rf $(BUILD_DIR)

rebuild: clean
	$(MAKE) all

-include $(RELEASE_DEPS) $(DEBUG_DEPS)

TARGET_EXEC ?= cpp_solver
DTARGET_EXEC ?= cpp_solver_d

BUILD_DIR ?= ./build
SRC_DIRS ?= .

SRCS := $(shell find $(SRC_DIRS) -name '*.cpp' -or -name '*.c' -or -name '*.s')
OBJS := $(SRCS:%=$(BUILD_DIR)/%.o)
DOBJS := $(SRCS:%=$(BUILD_DIR)/%.do)
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c++14 -O3 -march=native -g -Wno-narrowing -DNDEBUG
LDFLAGS ?= -lstdc++ -lm -lpthread

DCPPFLAGS ?= $(INC_FLAGS) -MMD -MP -std=c++14 -O0 -march=native -g -Wno-narrowing

$(BUILD_DIR)/$(TARGET_EXEC): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)

$(BUILD_DIR)/$(DTARGET_EXEC): $(DOBJS)
	$(CC) $(DOBJS) -o $@ $(LDFLAGS)

# assembly
$(BUILD_DIR)/%.s.o: %.s
	$(MKDIR_P) $(dir $@)
	$(AS) $(ASFLAGS) -c $< -o $@

# c source
$(BUILD_DIR)/%.c.o: %.c
	$(MKDIR_P) $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

# c++ source
$(BUILD_DIR)/%.cpp.do: %.cpp
	$(MKDIR_P) $(dir $@)
	$(CXX) $(DCPPFLAGS) $(CXXFLAGS) -c $< -o $@

debug: $(BUILD_DIR)/$(DTARGET_EXEC)

.PHONY: clean

clean:
	$(RM) -r $(BUILD_DIR)

-include $(DEPS)

MKDIR_P ?= mkdir -p

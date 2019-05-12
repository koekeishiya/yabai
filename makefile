FRAMEWORK_PATH = -F/System/Library/PrivateFrameworks
FRAMEWORK      = -framework Carbon -framework Cocoa -framework CoreServices -framework SkyLight -framework ScriptingBridge
BUILD_FLAGS    = -std=c99 -Wall -g -O0
BUILD_PATH     = ./bin
YABAI_SRC      = ./src/main.m
BINS           = $(BUILD_PATH)/yabai

.PHONY: all clean install sign

all: clean $(BINS)

install: BUILD_FLAGS=-std=c99 -O2
install: clean $(BINS)

sign:
	codesign -fs "yabai-cert" $(BUILD_PATH)/yabai

clean:
	rm -rf $(BUILD_PATH)

$(BUILD_PATH)/yabai: $(YABAI_SRC)
	mkdir -p $(BUILD_PATH)
	clang $^ $(BUILD_FLAGS) $(FRAMEWORK_PATH) $(FRAMEWORK) -o $@

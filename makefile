FRAMEWORK_PATH = -F/System/Library/PrivateFrameworks
FRAMEWORK      = -framework Carbon -framework Cocoa -framework CoreServices -framework SkyLight -framework ScriptingBridge
BUILD_FLAGS    = -std=c99 -Wall -g -O0
BUILD_PATH     = ./bin
DOC_PATH       = ./doc
SMP_PATH       = ./examples
ARCH_PATH      = ./archive
YABAI_SRC      = ./src/main.m
BINS           = $(BUILD_PATH)/yabai

.PHONY: all clean install sign archive man

all: clean $(BINS)

install: BUILD_FLAGS=-std=c99 -O2
install: clean $(BINS)

man:
	asciidoctor -b manpage $(DOC_PATH)/yabai.asciidoc -o $(DOC_PATH)/yabai.1

archive: install sign man
	rm -rf $(ARCH_PATH)
	mkdir -p $(ARCH_PATH)
	cp -r $(BUILD_PATH) $(ARCH_PATH)/
	cp -r $(DOC_PATH) $(ARCH_PATH)/
	cp -r $(SMP_PATH) $(ARCH_PATH)/
	tar -cvzf $(BUILD_PATH)/$(shell $(BUILD_PATH)/yabai --version).tar.gz $(ARCH_PATH)
	rm -rf $(ARCH_PATH)

sign:
	codesign -fs "yabai-cert" $(BUILD_PATH)/yabai

clean:
	rm -rf $(BUILD_PATH)

$(BUILD_PATH)/yabai: $(YABAI_SRC)
	mkdir -p $(BUILD_PATH)
	clang $^ $(BUILD_FLAGS) $(FRAMEWORK_PATH) $(FRAMEWORK) -o $@

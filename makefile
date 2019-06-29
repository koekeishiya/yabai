FRAMEWORK_PATH = -F/System/Library/PrivateFrameworks
FRAMEWORK      = -framework Carbon -framework Cocoa -framework CoreServices -framework SkyLight -framework ScriptingBridge -framework IOKit
BUILD_FLAGS    = -std=c99 -Wall -g -O0 -fvisibility=hidden
BUILD_PATH     = ./bin
DOC_PATH       = ./doc
SMP_PATH       = ./examples
ARCH_PATH      = ./archive
YABAI_SRC      = ./src/manifest.m
OSAX_PATH      = ./src/osax
BINS           = $(BUILD_PATH)/yabai

.PHONY: all clean install sign archive man sa

all: clean $(BINS)

install: BUILD_FLAGS=-std=c99 -Wall -O2 -fvisibility=hidden
install: clean $(BINS)

sa:
	clang $(OSAX_PATH)/loader.m -shared -O2 -o $(OSAX_PATH)/loader -framework Cocoa
	clang $(OSAX_PATH)/payload.m -shared -fPIC -O2 -o $(OSAX_PATH)/payload -framework Cocoa -framework Carbon
	xxd -i -a $(OSAX_PATH)/loader $(OSAX_PATH)/sa_loader.c
	xxd -i -a $(OSAX_PATH)/payload $(OSAX_PATH)/sa_payload.c
	rm -f $(OSAX_PATH)/loader
	rm -f $(OSAX_PATH)/payload

man:
	asciidoctor -b manpage $(DOC_PATH)/yabai.asciidoc -o $(DOC_PATH)/yabai.1

archive: man sa install sign
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
	@if [ -d $(BUILD_PATH) ]; then echo "Removing build path $(BUILD_PATH) ..." && rm -rf $(BUILD_PATH); fi
	@if [ -L /tmp/yabai_$(USER).lock ]; then echo "Unlinking /tmp/yabai_$(USER).lock ..." && unlink /tmp/yabai_$(USER).lock; fi
	@if [ -L /tmp/yabai_$(USER).socket ]; then echo "Unlinking /tmp/yabai_$(USER).socket ..." && unlink /tmp/yabai_$(USER).socket; fi
	@if [ -L /tmp/yabai-sa_$(USER).socket ]; then echo "Unlinking /tmp/yabai-sa_$(USER).socket ..." && unlink /tmp/yabai-sa_$(USER).socket; fi
	@if [ -L /usr/local/share/man/man1/yabai.1 ]; then echo "Removing man page ..." && unlink /usr/local/share/man/man1/yabai.1; fi
	@if [ -L /usr/local/bin/yabai ]; then echo "Unlinking binary ..." && unlink /usr/local/bin/yabai; fi
	@if [ -f ~/.yabairc ]; then echo "Removing configuration file ..." && rm ~/.yabairc; fi


$(BUILD_PATH)/yabai: $(YABAI_SRC)
	@echo "Starting build in $(BUILD_PATH) ..."
	@mkdir -p $(BUILD_PATH)
	clang $^ $(BUILD_FLAGS) $(FRAMEWORK_PATH) $(FRAMEWORK) -o $@
	@echo "Creating binary /usr/local/bin/yabai ..."
	@ln -s ./bin/yabai /usr/local/bin/yabai
	@echo "Creating man page ..."
	@ln -s ./doc/yabai.1 /usr/local/share/man/man1/yabai.1
	@echo "Creating default config file ~/.yabairc ..."
	@cp ./examples/yabairc ~/.yabairc



FRAMEWORK_PATH = -F/System/Library/PrivateFrameworks
FRAMEWORK      = -framework Carbon -framework Cocoa -framework CoreServices -framework SkyLight -framework ScriptingBridge
BUILD_FLAGS    = -std=c99 -Wall -g -O0 -fvisibility=hidden -mmacosx-version-min=10.13
BUILD_PATH     = ./bin
DOC_PATH       = ./doc
SCRIPT_PATH    = ./scripts
ASSET_PATH     = ./assets
SMP_PATH       = ./examples
ARCH_PATH      = ./archive
OSAX_SRC       = ./src/osax/sa_loader.c ./src/osax/sa_payload.c ./src/osax/sa_mach_bootstrap.c
YABAI_SRC      = ./src/manifest.m $(OSAX_SRC)
OSAX_PATH      = ./src/osax
BINS           = $(BUILD_PATH)/yabai

.PHONY: all clean install sign archive man

all: clean-build $(BINS)

install: BUILD_FLAGS=-std=c99 -Wall -DNDEBUG -O2 -fvisibility=hidden -mmacosx-version-min=10.13
install: clean-build $(BINS)

$(OSAX_SRC): $(OSAX_PATH)/loader.m $(OSAX_PATH)/payload.m $(OSAX_PATH)/mach_bootstrap.c
	clang $(OSAX_PATH)/loader.m -shared -O2 -mmacosx-version-min=10.13 -o $(OSAX_PATH)/loader -framework Cocoa
	clang $(OSAX_PATH)/payload.m -DOBJC_OLD_DISPATCH_PROTOTYPES=1 -shared -fPIC -O2 -mmacosx-version-min=10.13 -o $(OSAX_PATH)/payload -framework Cocoa -framework Carbon
	clang $(OSAX_PATH)/mach_bootstrap.c -shared -fPIC -O2 -mmacosx-version-min=10.13 -o $(OSAX_PATH)/mach_bootstrap -framework Carbon -lpthread
	xxd -i -a $(OSAX_PATH)/loader $(OSAX_PATH)/sa_loader.c
	xxd -i -a $(OSAX_PATH)/payload $(OSAX_PATH)/sa_payload.c
	xxd -i -a $(OSAX_PATH)/mach_bootstrap $(OSAX_PATH)/sa_mach_bootstrap.c
	rm -f $(OSAX_PATH)/loader
	rm -f $(OSAX_PATH)/payload
	rm -f $(OSAX_PATH)/mach_bootstrap

man:
	asciidoctor -b manpage $(DOC_PATH)/yabai.asciidoc -o $(DOC_PATH)/yabai.1

icon:
	python $(SCRIPT_PATH)/seticon.py $(ASSET_PATH)/icon/2x/icon-512px@2x.png $(BUILD_PATH)/yabai

archive: man install sign icon
	rm -rf $(ARCH_PATH)
	mkdir -p $(ARCH_PATH)
	cp -r $(BUILD_PATH) $(ARCH_PATH)/
	cp -r $(DOC_PATH) $(ARCH_PATH)/
	cp -r $(SMP_PATH) $(ARCH_PATH)/
	tar -cvzf $(BUILD_PATH)/$(shell $(BUILD_PATH)/yabai --version).tar.gz $(ARCH_PATH)
	rm -rf $(ARCH_PATH)

sign:
	codesign -fs "yabai-cert" $(BUILD_PATH)/yabai

clean-build:
	rm -rf $(BUILD_PATH)

clean: clean-build
	rm -f $(OSAX_SRC)

$(BUILD_PATH)/yabai: $(YABAI_SRC)
	mkdir -p $(BUILD_PATH)
	clang $^ $(BUILD_FLAGS) $(FRAMEWORK_PATH) $(FRAMEWORK) -o $@

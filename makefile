FRAMEWORK_PATH = -F/System/Library/PrivateFrameworks
FRAMEWORK      = -framework Carbon -framework Cocoa -framework CoreServices -framework CoreVideo -framework SkyLight
CLI_FLAGS      =
BUILD_FLAGS    = -std=c11 -Wall -Wextra -g -O0 -fvisibility=hidden -mmacosx-version-min=11.0 -fno-objc-arc -arch x86_64 -arch arm64 -sectcreate __TEXT __info_plist $(INFO_PLIST)
BUILD_PATH     = ./bin
DOC_PATH       = ./doc
SCRIPT_PATH    = ./scripts
ASSET_PATH     = ./assets
SMP_PATH       = ./examples
ARCH_PATH      = ./archive
OSAX_SRC       = ./src/osax/payload_bin.c ./src/osax/loader_bin.c
NIMBUSWM_SRC   = ./src/manifest.m $(OSAX_SRC)
OSAX_PATH      = ./src/osax
INFO_PLIST     = $(ASSET_PATH)/Info.plist
BINS           = $(BUILD_PATH)/nimbuswm

.PHONY: all asan tsan install man icon archive publish sign clean-build clean

all: clean-build $(BINS)

asan: BUILD_FLAGS=-std=c11 -Wall -Wextra -g -O0 -fvisibility=hidden -fsanitize=address,undefined -mmacosx-version-min=11.0 -fno-objc-arc -arch x86_64 -arch arm64 -sectcreate __TEXT __info_plist $(INFO_PLIST)
asan: clean-build $(BINS)

tsan: BUILD_FLAGS=-std=c11 -Wall -Wextra -g -O0 -fvisibility=hidden -fsanitize=thread,undefined -mmacosx-version-min=11.0 -fno-objc-arc -arch x86_64 -arch arm64 -sectcreate __TEXT __info_plist $(INFO_PLIST)
tsan: clean-build $(BINS)

install: BUILD_FLAGS=-std=c11 -Wall -Wextra -DNDEBUG -O3 -fvisibility=hidden -mmacosx-version-min=11.0 -fno-objc-arc -arch x86_64 -arch arm64 -sectcreate __TEXT __info_plist $(INFO_PLIST)
install: clean-build $(BINS)

$(OSAX_SRC): $(OSAX_PATH)/loader.m $(OSAX_PATH)/payload.m
	xcrun clang $(OSAX_PATH)/payload.m -shared -fPIC -O3 -mmacosx-version-min=11.0 -arch x86_64 -arch arm64e -o $(OSAX_PATH)/payload $(FRAMEWORK_PATH) -framework SkyLight -framework Foundation -framework Carbon
	xcrun clang $(OSAX_PATH)/loader.m -O3 -mmacosx-version-min=11.0 -arch x86_64 -arch arm64e -o $(OSAX_PATH)/loader -framework Cocoa
	xxd -i -a $(OSAX_PATH)/payload $(OSAX_PATH)/payload_bin.c
	xxd -i -a $(OSAX_PATH)/loader $(OSAX_PATH)/loader_bin.c
	rm -f $(OSAX_PATH)/payload
	rm -f $(OSAX_PATH)/loader

man:
	asciidoctor -b manpage $(DOC_PATH)/nimbuswm.asciidoc -o $(DOC_PATH)/nimbuswm.1

icon:
	python3 $(SCRIPT_PATH)/seticon.py $(ASSET_PATH)/icon/2x/icon-512px@2x.png $(BUILD_PATH)/nimbuswm

publish:
	sed -i '' "60s/^VERSION=.*/VERSION=\"$(shell $(BUILD_PATH)/nimbuswm --version | cut -d "v" -f 2)\"/" $(SCRIPT_PATH)/install.sh
	sed -i '' "61s/^EXPECTED_HASH=.*/EXPECTED_HASH=\"$(shell shasum -a 256 $(BUILD_PATH)/$(shell $(BUILD_PATH)/nimbuswm --version).tar.gz | cut -d " " -f 1)\"/" $(SCRIPT_PATH)/install.sh

archive: man install sign icon
	rm -rf $(ARCH_PATH)
	mkdir -p $(ARCH_PATH)
	cp -r $(BUILD_PATH) $(ARCH_PATH)/
	cp -r $(DOC_PATH) $(ARCH_PATH)/
	cp -r $(SMP_PATH) $(ARCH_PATH)/
	tar -cvzf $(BUILD_PATH)/$(shell $(BUILD_PATH)/nimbuswm --version).tar.gz $(ARCH_PATH)
	rm -rf $(ARCH_PATH)

sign:
	codesign -fs "nimbuswm-cert" $(BUILD_PATH)/nimbuswm

clean-build:
	rm -rf $(BUILD_PATH)

clean: clean-build
	rm -f $(OSAX_SRC)

$(BUILD_PATH)/nimbuswm: $(NIMBUSWM_SRC)
	mkdir -p $(BUILD_PATH)
	xcrun clang $^ $(BUILD_FLAGS) $(CLI_FLAGS) $(FRAMEWORK_PATH) $(FRAMEWORK) -o $@

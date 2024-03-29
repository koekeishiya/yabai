all:
	rm -rf ./bin
	mkdir -p ./bin
	clang ./src/tests.m -o ./bin/tests -DTESTS -F/System/Library/PrivateFrameworks -framework Carbon -framework Cocoa -framework CoreServices -framework CoreVideo -framework SkyLight
	./bin/tests

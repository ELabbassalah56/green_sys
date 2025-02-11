.PHONY: all
all: format test build

.PHONY: format
format:
	clang-format src/* include/* -i

.PHONY: build
build:
	mkdir -p build
	cd build && \
	cmake -DGTEST_ROOT=/usr/local -Dgtest_DIR=/usr/local/lib/cmake/GTest -Dgtest_output=xml:report.xml .. && \
	make

.PHONY: test
test:
	cd build && \
	ctest --output-on-failure

.PHONY: debug
debug:
	mkdir -p build
	cd build && \
	cmake -DCMAKE_BUILD_TYPE=Debug -DGTEST_ROOT=/usr/local -Dgtest_DIR=/usr/local/lib/cmake/GTest .. && \
	make

.PHONY: clean
clean:
	rm -rf build

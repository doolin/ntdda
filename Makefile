# Top-level Makefile - wrapper for CMake build
# Legacy: also cleans Debug/Release/bin from old layouts

.PHONY: all clean test

all:
	@mkdir -p build && cd build && cmake .. && $(MAKE)

test: all
	cd build && ctest --output-on-failure

clean:
	rm -rf build Debug Release bin *~

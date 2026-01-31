all:
	cmake -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
	cmake --build build
	./test/test.py build/libtest.so
	./mktest.sh > reveng.hpp
	c++ -std=c++23 reveng.hpp -c
	@echo OK

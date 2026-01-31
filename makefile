all:
	cmake -B build -G Ninja -DCMAKE_EXPORT_COMPILE_COMMANDS=ON -DCMAKE_BUILD_TYPE=Debug
	ln -srf build/compile_commands.json .
	cmake --build build
	./test/test.py build/libtest.so
	./test/mkreveng.sh

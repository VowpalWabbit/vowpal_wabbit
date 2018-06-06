cmake -DCMAKE_TOOLCHAIN_FILE=C:\work\vcpkg\scripts\buildsystems\vcpkg.cmake .
cmake --build .
rem cmake --build . --target RunsTests
cmake --build . --target check
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" arm64
cmake -S. -Bwin_build -DNEAPU_BUILD_DEMO=ON -G "Ninja"
cmake --build win_build --target all -j6
pause
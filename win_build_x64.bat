call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
cmake -S. -Bwin_build -DNEAPU_BUILD_DEMO=ON -G "Ninja"
cmake --build win_build --target all -j6
pause
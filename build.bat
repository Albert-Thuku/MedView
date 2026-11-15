@echo off
setlocal
cd /d "%~dp0"
where g++ >nul 2>nul
if errorlevel 1 (
 echo C++ compiler not found. Install MinGW-w64 or use build_msvc.bat from a Visual Studio Developer Command Prompt.
 pause
 exit /b 1
)
if not exist build mkdir build
g++ -std=c++11 -O2 -Wall -Wextra -pedantic -Iinclude tests/test_graphics.cpp src/graphics.cpp -static -o build/test_graphics.exe
if errorlevel 1 goto fail
build\test_graphics.exe
if errorlevel 1 goto fail
g++ -std=c++11 -O2 -Wall -Wextra -pedantic -Iinclude tests/test_explorer.cpp src/explorer.cpp src/anatomy.cpp src/learning.cpp src/graphics.cpp src/font.cpp -static -o build/test_explorer.exe
if errorlevel 1 goto fail
build\test_explorer.exe
if errorlevel 1 goto fail
g++ -std=c++11 -O2 -Wall -Wextra -pedantic -Iinclude src/win32_main.cpp src/explorer.cpp src/anatomy.cpp src/learning.cpp src/graphics.cpp src/font.cpp -mwindows -static -lgdi32 -luser32 -o build/MediView.exe
if errorlevel 1 goto fail
echo Build and tests successful. Run build\MediView.exe.
pause
exit /b 0
:fail
echo Build or tests failed. See the error above. No application has been launched.
pause
exit /b 1

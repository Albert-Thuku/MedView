@echo off
setlocal
cd /d "%~dp0"
where cl >nul 2>nul
if errorlevel 1 (
 echo Open the x64 Native Tools Command Prompt for Visual Studio first.
 exit /b 1
)
if not exist build mkdir build
pushd build
cl /nologo /EHsc /std:c++14 /W4 /O2 /MT /I..\include ..\tests\test_graphics.cpp ..\src\graphics.cpp /Fe:test_graphics.exe
if errorlevel 1 goto fail
test_graphics.exe
if errorlevel 1 goto fail
cl /nologo /EHsc /std:c++14 /W4 /O2 /MT /I..\include ..\tests\test_explorer.cpp ..\src\explorer.cpp ..\src\anatomy.cpp ..\src\learning.cpp ..\src\graphics.cpp ..\src\font.cpp /Fe:test_explorer.exe
if errorlevel 1 goto fail
test_explorer.exe
if errorlevel 1 goto fail
cl /nologo /EHsc /std:c++14 /W4 /O2 /MT /I..\include ..\src\win32_main.cpp ..\src\explorer.cpp ..\src\anatomy.cpp ..\src\learning.cpp ..\src\graphics.cpp ..\src\font.cpp /Fe:MediView.exe /link /SUBSYSTEM:WINDOWS user32.lib gdi32.lib
if errorlevel 1 goto fail
popd
exit /b 0
:fail
popd
exit /b 1

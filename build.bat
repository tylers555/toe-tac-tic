@echo off
setlocal

if not exist build mkdir build

set COMPILE_OPTIONS= /FC /W4 /WX /nologo /Zi /std:c++latest /GR- /Oi /EHa- /MT /wd4312 /wd4201 /wd4200 /wd4100 /wd4456 /wd4505 /wd4189
set LINK_OPTIONS= -SUBSYSTEM:WINDOWS -incremental:no
set INCLUDE_PATHS= /I "P:\Libs\SDL2-2.0.10-VC\include" /I "P:\Libs\glew\include"
set LIBRARY_PATHS=  
set LIBRARIES= User32.lib Gdi32.lib

pushd build
cl %COMPILE_OPTIONS% %INCLUDE_PATHS% /Fe:SnailJumpy.exe ..\src\win32_snail_jumpy.cpp /link  %LINK_OPTIONS% %LIBRARY_PATHS% %LIBRARIES%
popd

endlocal
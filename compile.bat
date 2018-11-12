@echo off

SET ARG1=%1

call console.bat 

SET OUTPUT=%~dp0bin\main.exe

SET SOURCE=%~dp0src\main.cpp %~dp0src\source\*
SET SDLINC=%~dp0lib\SDL2-2.0.7\include\
SET SDLLIB=%~dp0lib\SDL2-2.0.7\lib\x86\
REM SET SDLLIB=%~dp0lib\SDL2-2.0.7\lib\x64\
SET SDL_TTFINC=%~dp0lib\SDL2_ttf-2.0.14\include\
SET SDL_TTFLIB=%~dp0lib\SDL2_ttf-2.0.14\lib\x86\
SET SDL_IMGINC=%~dp0lib\SDL2_image-2.0.3\include\
SET SDL_IMGLIB=%~dp0lib\SDL2_image-2.0.3\lib\x86\
SET AUDIO_INC=%~dp0lib\SDL2_mixer-2.0.4\include\
SET AUDIO_LIB=%~dp0lib\SDL2_mixer-2.0.4\lib\x86\

REM cl /EHsc .\src\main.cpp /link /OUT:C:\temp\sdl\bin\main /SUBSYSTEM:CONSOLE

REM cl /EHsc .\src\main.cpp /link /out:%OUTPUT% /SUBSYSTEM:CONSOLE
echo.
IF "%ARG1%"=="release" (
    echo ---- RELEASE BUILD, no optimizations or completed config ---- 

    cl /EHsc %SOURCE% %SOURCEEXTERN% /I %SDLINC% /I %SDL_TTFINC% /I %~dp0src\extern\ /I %~dp0src\headers\ /link /LIBPATH:%SDLLIB% /LIBPATH:%SDL_TTFLIB% SDL2main.lib SDL2.lib SDL2_ttf.lib opengl32.lib /out:%OUTPUT% /SUBSYSTEM:CONSOLE

    echo ---- COMPLETED RELEASE BUILD ---- 
) ELSE (
    echo ---- DEBUG BUILD ---- 

    REM ----- COMPILE ALL SOURCES to make lib/dll -----
	REM cl /c /MP /MTd /DEBUG /EHsc %SOURCEEXTERN% /I %SDLINC% /I %SDL_TTFINC%
    
    REM -- MAKE DLL: (dynamic library)
    REM link /DLL /OUT:bin\extern.dll *.obj /LIBPATH:%SDLLIB% /LIBPATH:%SDL_TTFLIB% SDL2main.lib SDL2.lib SDL2_ttf.lib opengl32.lib
	
    REM -- MAKE lib: (static library)
    REM lib /OUT:extern.lib *.obj	
    
    REM --- BUILD WITH DLL OR LIB ---
    REM cl /MP /MTd /DEBUG /Zi /EHsc %SOURCE% /I %SDLINC% /I %SDL_TTFINC% /I %~dp0src\headers\ /link /LIBPATH:%SDLLIB% /LIBPATH:%SDL_TTFLIB% /LIBPATH:.\ SDL2main.lib SDL2.lib SDL2_ttf.lib opengl32.lib extern.lib /out:%OUTPUT% /SUBSYSTEM:CONSOLE

	REM --- ORIGINAL BUILD ALL ---
	cl /nologo /EHsc /W4 /MP /MTd /wd4996 /wd4100 /DEBUG /Zi %SOURCE% /I %SDLINC% /I %SDL_TTFINC% /I %SDL_IMGINC% /I %AUDIO_INC% /I %~dp0src\headers\ /link /LIBPATH:%SDLLIB% /LIBPATH:%SDL_TTFLIB% /LIBPATH:%SDL_IMGLIB% /LIBPATH:%AUDIO_LIB% SDL2main.lib SDL2.lib SDL2_ttf.lib SDL2_image.lib SDL2_mixer.lib opengl32.lib /out:%OUTPUT% /SUBSYSTEM:CONSOLE
	
    echo ---- COMPLETED DEBUG BUILD ---- 
)
echo.

echo -- REMOVING *.obj
del *.obj
echo.
echo -- COPYING DATA FILES (IF NEWER)
call rescp.bat
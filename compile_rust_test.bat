@echo off

SET ARG1=%1

call console.bat 

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
	cl %SWITCHES% %SOURCE% /I %SDLINC% /I %SDL_TTFINC% /I %SDL_IMGINC% /I %AUDIO_INC% /I %HEADERS% /link /LIBPATH:%SDLLIB% /LIBPATH:%SDL_TTFLIB% /LIBPATH:%SDL_IMGLIB% /LIBPATH:%AUDIO_LIB% SDL2main.lib SDL2.lib SDL2_ttf.lib SDL2_image.lib SDL2_mixer.lib opengl32.lib /out:%OUTPUT% /SUBSYSTEM:CONSOLE
	
    echo ---- COMPLETED DEBUG BUILD ---- 
)
echo.

echo -- REMOVING *.obj
del *.obj
echo.
echo -- COPYING DATA FILES (IF NEWER)
call rescp.bat
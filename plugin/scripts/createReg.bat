@echo off

set DOSBOXDIR=
call :setdosboxdir %0

set DOSBOX=%DOSBOXDIR%\\dosbox-x86-noconsole.exe

set DOSCONFIG=%DOSBOXDIR%\\dosbox-dos.conf
set WIN16CONFIG=%DOSBOXDIR%\\dosbox-win16.conf

set REG_DOS=install-dos.reg
set REG_WIN16=install-win16.reg
set REG_ALL=install-all.reg
set REG_REMOVE=uninstall.reg

rem goto fr
goto en


:en
set LAUNCH_AS=Launch as...
set APP_DOS_WINDOWED=MS-DOS application (windowed)
set APP_DOS_FULLSCREEN=MS-DOS application (fullscreen)
set APP_WIN16_WINDOWED=Windows 16 bits application (windowed)
set APP_WIN16_FULLSCREEN=Windows 16 bits application (fullscreen)
goto generate

:fr
set LAUNCH_AS=Exécuter comme...
set APP_DOS_WINDOWED=Application MS-DOS (fenêtré)
set APP_DOS_FULLSCREEN=Application MS-DOS (plein écran)
set APP_WIN16_WINDOWED=Application Windows 16 bits (fenêtré)
set APP_WIN16_FULLSCREEN=Application Windows 16 bits (plein écran)
goto generate


:generate
echo.
echo Create registeries files...
echo.

rem DOS only
set REG=%REG_DOS%
call :create_reg "Launcher.dos;Launcher.dos.fullscreen"
call :write_dos

rem Win16 olny
set REG=%REG_WIN16%
call :create_reg "Launcher.win16;Launcher.win16.fullscreen"
call :write_win16 

rem DOS + Win16
set REG=%REG_ALL%
call :create_reg "Launcher.dos;Launcher.dos.fullscreen;Launcher.win16;Launcher.win16.fullscreen"
call :write_dos
call :write_win16 

rem Remove
if exist "%REG_REMOVE%" del /q "%REG_REMOVE%" > nul

echo Windows Registry Editor Version 5.00 >> %REG_REMOVE%
echo. >> %REG_REMOVE%
echo [-HKEY_CLASSES_ROOT\exefile\shell\launchas]>> %REG_REMOVE%
echo [-HKEY_CLASSES_ROOT\comfile\shell\launchas]>> %REG_REMOVE%
echo [-HKEY_CLASSES_ROOT\batfile\shell\launchas]>> %REG_REMOVE%
echo [-HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.dos]>> %REG_REMOVE%
echo [-HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.dos.fullscreen]>> %REG_REMOVE%
echo [-HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.win16]>> %REG_REMOVE%
echo [-HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.win16.fullscreen]>> %REG_REMOVE%
goto :EOF


:create_reg
if exist "%REG%" del /q "%REG%" > nul
echo Windows Registry Editor Version 5.00 >> %REG%
echo. >> %REG%
echo [HKEY_CLASSES_ROOT\exefile\shell\launchas]>> %REG%
echo "MUIVerb"="%LAUNCH_AS%">> %REG%
echo "SubCommands"=%1>> %REG%
echo. >> %REG%
echo [HKEY_CLASSES_ROOT\comfile\shell\launchas]>> %REG%
echo "MUIVerb"="%LAUNCH_AS%">> %REG%
echo "SubCommands"=%1>> %REG%
echo. >> %REG%
echo [HKEY_CLASSES_ROOT\batfile\shell\launchas]>> %REG%
echo "MUIVerb"="%LAUNCH_AS%">> %REG%
echo "SubCommands"=%1>> %REG%
echo. >> %REG%
goto :EOF

:write_dos
echo [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.dos]>> %REG%
echo @="%APP_DOS_WINDOWED%">> %REG%
echo "icon"="%DOSBOXDIR%\\dos.ico">> %REG%
echo. >> %REG%
echo [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.dos\command]>> %REG%
echo @="\"%DOSBOX%\" -conf \"%DOSCONFIG%\" -x-application \"%%1\"">> %REG%
echo. >> %REG%
echo [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.dos.fullscreen]>> %REG%
echo @="%APP_DOS_FULLSCREEN%">> %REG%
echo "icon"="%DOSBOXDIR%\\dos.ico">> %REG%
echo. >> %REG%
echo [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.dos.fullscreen\command]>> %REG%
echo @="\"%DOSBOX%\" -fullscreen -conf \"%DOSCONFIG%\" -x-application \"%%1\"">> %REG%
echo.>> %REG%
goto :EOF

:write_win16
echo [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.win16]>> %REG%
echo @="%APP_WIN16_WINDOWED%">> %REG%
echo "icon"="%DOSBOXDIR%\\win16.ico">> %REG%
echo.>> %REG%
echo [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.win16\command]>> %REG%
echo @="\"%DOSBOX%\" -conf \"%WIN16CONFIG%\" -x-application \"%%1\"">> %REG%
echo.>> %REG%
echo [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.win16.fullscreen]>> %REG%
echo @="%APP_WIN16_FULLSCREEN%">> %REG%
echo "icon"="%DOSBOXDIR%\\win16.ico">> %REG%
echo.>> %REG%
echo [HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Windows\CurrentVersion\Explorer\CommandStore\shell\Launcher.win16.fullscreen\command]>> %REG%
echo @="\"%DOSBOX%\" -fullscreen -conf \"%WIN16CONFIG%\" -x-application \"%%1\"">> %REG%
goto :EOF


:setdosboxdir
set filename=%~nx1
set folder=%~dp1
if "%filename%" == "" call :setdosboxdir2 %~d0 %DOSBOXDIR%
if "%filename%" == "" goto :EOF
if "%2" == "1" set DOSBOXDIR=\\%filename%%DOSBOXDIR%
call :setdosboxdir "%folder%\." 1
goto :EOF
:setdosboxdir2
set DOSBOXDIR=%~d1%2
goto :EOF

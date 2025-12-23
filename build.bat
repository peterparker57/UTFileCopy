@echo off
setlocal enabledelayedexpansion

REM Silent File Copy DLL Build Script
REM Finds MSBuild and builds 32-bit DLL

echo Building UTFileCopy.dll (32-bit)...
echo.

REM Find MSBuild from Visual Studio 2022 or 2019
set "MSBUILD="
for %%v in (2022 2019) do (
    for %%e in (Community Professional Enterprise) do (
        set "TESTPATH=C:\Program Files\Microsoft Visual Studio\%%v\%%e\MSBuild\Current\Bin\MSBuild.exe"
        if exist "!TESTPATH!" (
            set "MSBUILD=!TESTPATH!"
            echo Found MSBuild: !MSBUILD!
            goto :build
        )
    )
)

if not defined MSBUILD (
    echo ERROR: MSBuild not found. Please install Visual Studio 2019 or 2022.
    exit /b 1
)

:build
echo.
echo Building Release configuration for Win32...
"%MSBUILD%" "%~dp0UTFileCopy.vcxproj" /p:Configuration=Release /p:Platform=Win32 /v:minimal /nologo

if %ERRORLEVEL% EQU 0 (
    echo.
    echo Build successful!
    echo Output: %~dp0bin\Release\UTFileCopy.dll
) else (
    echo.
    echo Build failed with error code %ERRORLEVEL%
)

endlocal

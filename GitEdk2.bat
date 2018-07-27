@echo off

set OriginDir=%cd%
set Branch=master

:CmdLineParse
if /I "%1" == "" (
  goto Continue
) else if /I "%1" == "-?" (
  goto Help
) else if /I "%1" == "-h" (
  goto Help
) else if /I "%1" == "--help" (
  goto Help
) else if /I "%1" == "-w" (
  @REM Create WORKSPACE directory.
  if "%2" NEQ "" (
    if not exist "%2" (
      mkdir "%2"
    )
    cd/D "%2"
  ) else (
    echo.
    echo Invalid parameters.
    echo.
    goto Help
  )
) else if /I "%1" == "-b" (
  @REM Set Branch environment.
  if /I "%2" == "master" (
    set Branch=master
  ) else if /I "%2" == "trunk" (
    set Branch=master
  ) else if /I "%2" == "udk2015" (
    set Branch=UDK2015
  ) else if /I "%2" == "bp13" (
    set Branch=UDK2015
  ) else (
    echo.
    echo Invalid parameters.
    echo.
    cd/D %OriginDir%
    goto Help
  )
) else (
  echo.
  echo Invalid parameters.
  echo.
  cd/D %OriginDir%
  goto Help
)
shift
shift
goto CmdLineParse

:Continue
@REM Set WORKSPACE environment.
set WORKSPACE=%cd%
echo.
echo Set WORKSPACE as: %WORKSPACE%
echo.

@REM Check whether Git has been installed and been added to system path.
git --help >nul 2>nul
if %ERRORLEVEL% NEQ 0 (
  echo.
  echo The 'git' command is not recognized.
  echo Please make sure that Git is installed and has been added to system path.
  echo.
  goto :EOF
)

cd %WORKSPACE%
@REM Create the Conf directory under WORKSPACE
if not exist %WORKSPACE%\Conf (
  mkdir Conf
)

@REM Set other environments.
set PACKAGES_PATH=%WORKSPACE%\Core;%WORKSPACE%\Device;%WORKSPACE%

@if not defined PYTHON_HOME (
  @if exist C:\Python27 (
    set PYTHON_HOME=C:\Python27
  )
)

set EDK_SETUP_OPTION=
@rem if python is installed, disable the binary base tools.
if defined PYTHON_HOME (
  set EDK_TOOLS_BIN=
  set EDK_SETUP_OPTION=--nt32
)
pushd %WORKSPACE%\edk2
call edksetup.bat %EDK_SETUP_OPTION%
popd
pushd %WORKSPACE%
@rem if python is installed, nmake BaseTools source and enable BaseTools source build
@if defined PYTHON_HOME (
  nmake -f %BASE_TOOLS_PATH%\Makefile
)
popd

set openssl_path=%WORKSPACE%

goto :EOF

:Help
echo.
echo Usage:
echo GitEdk2.bat [-w Workspace_Directory] (optional) [-b Branch_Name] (optional)
echo.
echo -w    A absolute/relative path to be the workspace.
echo       Default value is the current directory.
echo.
echo -b    The branch name of the repository. Currently, only master, udk2015,
echo       trunk (same as master) and bp13 (same as udk2015) are supported.
echo       Default value is master.
echo.

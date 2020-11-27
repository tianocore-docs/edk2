@REM
@REM Copyright(c) 2013 Intel Corporation. All rights reserved.
@REM
@REM Redistribution and use in source and binary forms, with or without
@REM modification, are permitted provided that the following conditions
@REM are met:
@REM
@REM * Redistributions of source code must retain the above copyright
@REM notice, this list of conditions and the following disclaimer.
@REM * Redistributions in binary form must reproduce the above copyright
@REM notice, this list of conditions and the following disclaimer in
@REM the documentation and/or other materials provided with the
@REM distribution.
@REM * Neither the name of Intel Corporation nor the names of its
@REM contributors may be used to endorse or promote products derived
@REM from this software without specific prior written permission.
@REM
@REM THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
@REM "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
@REM LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
@REM A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
@REM OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
@REM SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
@REM LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
@REM DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
@REM THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
@REM (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
@REM OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
@REM

@echo off

@REM Make sure you fully understand (the lack of) delayedexpansion
@REM in batch files before you try to use error reporting
set MY_ERROR_LVL=0


@if /I "%1"=="" goto Usage
@if /I "%1"=="-h" goto Usage
@if /I "%1"=="-clean" goto Clean
@if /I "%2"=="" goto Usage
@if /I "%3"=="" goto Usage

@REM
@REM Windows build environment traditionally has problems with long path names.
@REM most notably the MAX_PATH limit.
@REM Substitute the source code root to avoid these problems.
@REM
set CURRENTDIR=%~dp0
set CURRENTDIR=%CURRENTDIR:~0,-1%
set CURRENTDRIVE=%~d1
set SUBSTDRIVE=%2:

@REM subst /d %SUBSTDRIVE%
@REM subst %SUBSTDRIVE% %CURRENTDIR%
@REM %SUBSTDRIVE%

@if not defined WORKSPACE (
  call %SUBSTDRIVE%\edksetup.bat
)

@echo off

if /I "%1"=="-r32" (
  set TARGET=RELEASE
  set DEBUG_PRINT_ERROR_LEVEL=-DDEBUG_PRINT_ERROR_LEVEL=0x80000000
  set DEBUG_PROPERTY_MASK=-DDEBUG_PROPERTY_MASK=0x23
  goto CheckParameter3
)

if /I "%1"=="-d32" (
  set TARGET=DEBUG
  set DEBUG_PRINT_ERROR_LEVEL=-DDEBUG_PRINT_ERROR_LEVEL=0x80000042
  set DEBUG_PROPERTY_MASK=-DDEBUG_PROPERTY_MASK=0x27
  goto CheckParameter3
) else (
  goto Usage
)

:CheckParameter3
  set PLATFORM=%3
  goto BuildAll

:Clean
echo Removing Build/Conf directories ... 
if exist Build rmdir Build /s /q
if exist Conf  rmdir Conf  /s /q
if exist *.log  del *.log /q /f
set WORKSPACE=
set EDK_TOOLS_PATH=
goto End

:BuildAll
@echo off

@REM Add all arguments after the 3rd to the EDK_PARAMS variable
@REM tokens=1-3 puts the 1st, 2nd and 3rd arguments in %%a, %%b and %%c
@REM All remaining arguments are in %%d


for /f "tokens=1-3*" %%a in ("%*") do (
    set EDK_PARAMS=%%d
)

if NOT exist %WORKSPACE%\BaseTools (
  echo Error: SVN directories missing - please run svn update to download
  goto End
)

if exist %WORKSPACE%\QuarkSocPkg\QuarkNorthCluster\Binary\QuarkMicrocode\RMU.bin  goto GotCMC
echo Trying to fetch Chipset Microcode...
%WORKSPACE%\fetchCMCBinary.py
if %ERRORLEVEL% NEQ 0 (
    set MY_ERROR_LVL=%ERRORLEVEL%
    echo Error: Chipset Microcode is missing
    goto End
)

:GotCMC

@REM ###############################################################################
@REM ########################       PreBuild-processing       ######################
@REM ###############################################################################
if NOT exist %WORKSPACE%\Conf (
  echo Error: Missing folder %WORKSPACE%\Conf\
  goto End
) else (
  if exist %WORKSPACE%\QuarkPlatformPkg\Override\BaseTools\Conf\tools_def.template (
    echo copying ... tools_def.template to %WORKSPACE%\Conf\tools_def.txt
    copy /Y %WORKSPACE%\QuarkPlatformPkg\Override\BaseTools\Conf\tools_def.template %WORKSPACE%\Conf\tools_def.txt > nul
  )
  if exist %WORKSPACE%\QuarkPlatformPkg\Override\BaseTools\Conf\build_rule.template (
    echo copying ... build_rule.template to %WORKSPACE%\Conf\build_rule.txt
    copy /Y %WORKSPACE%\QuarkPlatformPkg\Override\BaseTools\Conf\build_rule.template %WORKSPACE%\Conf\build_rule.txt > nul
  )  
)


@if not exist Build\%PLATFORM%\%TARGET%_%VS_VERSION%%VS_X86%\IA32 (
  mkdir Build\%PLATFORM%\%TARGET%_%VS_VERSION%%VS_X86%\IA32
)

@REM ####################################################################################################################
@REM ######                                Perform the actual build                                         #############
@REM ######   Warning: parameters here are supposed to override any corresponding value in Conf/target.txt  #############
@REM ####################################################################################################################
build  -p %PLATFORM%Pkg\%PLATFORM%Pkg.dsc -b %TARGET% -a IA32 -n 4 -t %VS_VERSION%%VS_X86% -y Report.log %EDK_PARAMS% %DEBUG_PRINT_ERROR_LEVEL% %DEBUG_PROPERTY_MASK%
if %ERRORLEVEL% NEQ 0   ( set MY_ERROR_LVL=%ERRORLEVEL% & Goto End )

@REM ###############################################################################
@REM ########################       PostBuild-processing       #####################
@REM ###############################################################################

@echo on

@REM   ###############################################################################
@REM   ########################     2) Rebase BFV to SRAM         ####################
@REM   ###############################################################################
copy %OutputModulesDir%\EDKII_BOOT_STAGE1_IMAGE1.fv %OutputModulesDir%\EDKII_BOOT_STAGE1_IMAGE1.org.fv
.\TinyBootPkg\Tools\PeiRebase\PeiRebase.exe -I %OutputModulesDir%\EDKII_BOOT_STAGE1_IMAGE1.org.fv -O %OutputModulesDir%\EDKII_BOOT_STAGE1_IMAGE1.fv -B 0x80000000

@REM   ###############################################################################
@REM   ########################     3) Gen Fds again as final     ####################
@REM   ###############################################################################

@echo off

build fds -p %PLATFORM%Pkg\%PLATFORM%Pkg.dsc -b %TARGET% -a IA32 -n 4 -t %VS_VERSION%%VS_X86% -y Report.log %EDK_PARAMS% %DEBUG_PRINT_ERROR_LEVEL% %DEBUG_PROPERTY_MASK%
if %ERRORLEVEL% NEQ 0   ( set MY_ERROR_LVL=%ERRORLEVEL% & Goto End )

@REM ###############################################################################
@REM ########################     Image signing stage           ####################
@REM ########################       (dummy signing)             ####################
@REM ###############################################################################
set OutputModulesDir=%WORKSPACE%\Build\%PLATFORM%\%TARGET%_%VS_VERSION%%VS_X86%\FV

@REM ###############################################################################
@REM ####################         Capsule creation stage            ################
@REM ####################     (Recovery and Update capsules)        ################
@REM ###############################################################################
set CapsuleConfigFile=%WORKSPACE%\%PLATFORM%Pkg\Tools\CapsuleCreate\%PLATFORM%PkgCapsuleComponents.inf
set CapsuleOutputFileNoReset=%OutputModulesDir%\%PLATFORM%PkgNoReset.Cap
set CapsuleOutputFileReset=%OutputModulesDir%\%PLATFORM%PkgReset.Cap
set CapsuleFlagsNoReset=0x00000000
set CapsuleFlagsReset=0x00050000

@REM ###############################################################################
@REM ################       Create useful output directories        ################
@REM ###############################################################################
@if not exist %OutputModulesDir%\RemediationModules (
  mkdir %OutputModulesDir%\RemediationModules
)

@if not exist %OutputModulesDir%\Tools (
  mkdir %OutputModulesDir%\Tools
)

@if not exist %OutputModulesDir%\Applications (
  mkdir %OutputModulesDir%\Applications
)

copy Report.log Build\%PLATFORM%\%TARGET%_%VS_VERSION%%VS_X86%\ > nul
if ERRORLEVEL 1 exit /b 1
goto End

:Usage
  echo.
  echo  Usage: "%0 [-h | -r32 | -d32 | -clean] [subst drive letter] [PlatformName] [-DSECURE_LD(optional)] [-DSECURE_BOOT(optional)]"
  echo.

:End
%CURRENTDRIVE%
@REM subst /d %SUBSTDRIVE%
echo.
exit /b %MY_ERROR_LVL%



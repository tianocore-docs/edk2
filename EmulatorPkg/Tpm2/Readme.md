# This package is to demonstrate how to enable TPM2 emulator on EDKII Emulator platform.

## Feature
- TPM2

A user can enable TPM2 simulator support on EmulatorPkg.

The purpose of EmulatorPkg TPM2 support is to debug or test the hardware independent TPM2 feature, such as Tcg2Pei, Tcg2Dxe.

EmulatorPkg TPM2 cannot be used to validate TPM2 hardware dependent modules such as Tpm2DeviceLibDTpm.
EmulatorPkg TPM2 uses the simulator version in EmulatorPkg\Tpm2 directory.

## build

  0) Download EDKII branch from github https://github.com/jyao1/edk2/tree/feature_tpm_emulator

  1) Build EmulatorPkg as normal way.

## Run

Step: (Windows)

1) Download the TPM2 simulator from https://www.microsoft.com/en-us/download/details.aspx?id=52507

2) Run "TSS.MSR v2.0 TPM2 Simulator\Simulator.exe" in the windows.

3) Now the user can run "WinHost.exe" built from the EmulatorPkg

Step: (Linux)

1) Download the TPM2 simulator from https://github.com/microsoft/ms-tpm-20-ref, then follow the readme.md to build.

2) Go to "ms-tpm-20-ref/TPMCmd/Simulator/src" and run "./tpm2-simulator" in Linux.

3) Now the user can run "./Host" built from the EmulatorPkg

The EmulatorPkg emulator can communicate with TPM2.0 simulator. In UEFI shell, the user can use the Tcg2DumpLog at https://github.com/jyao1/EdkiiShellTool/tree/master/EdkiiShellToolPkg/Tcg2DumpLog.

## Known limitation:
This package is only the sample code to show the concept.
It does not have a full validation and does not meet the production quality yet.

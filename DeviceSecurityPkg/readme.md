# This is a sample implementation for UEFI SPDM requester

## Feature

1) A UEFI SPDM requester as DeviceSecurity driver.

2) A UEFI test stub as SPDM responder.

   Both Code can run in UEFI emulation env.

## Build

This repo uses below submodules:

  DeviceSecurityPkg/Library/SpdmLib/openspdm

Build:
  Follow standard EDKII build process for DeviceSecurityPkg.

Run :
  Copy all *.efi in Build\DeviceSecurityPkg\<TARGET>_<TOOLCHAIN>\<ARCH>\*.efi to Build\EmulationPkg\<TARGET>_<TOOLCHAIN>\<ARCH>\.

  Boot to UEFI shell and run below command:

  ```
    load Tcg2Stub.efi
    DeployCert.efi
    load DeviceSecurityPolicyStub.efi
    load PciIoStub.efi
    load SpdmStub.efi
    load SpdmDeviceSecurityDxe.efi
    TestSpdm.efi
  ```

  To test PCI DOE, boot to UEFI shell and run below command:

  ```
    load Tcg2Stub.efi
    DeployCert.efi
    load DeviceSecurityPolicyStub.efi
    #load PciIoStub.efi
    load PciIoPciDoeStub.efi
    #load SpdmStub.efi
    load SpdmPciDoeStub.efi
    load SpdmDeviceSecurityDxe.efi
    TestSpdm.efi
  ```

  In EmulationPkg, the PEI SPDM module can only be launched in second boot, after DeployCert.efi in UEFI shell.

## TCG SPDM Event Log

  We can use [Tcg2DumpLog](https://github.com/jyao1/edk2/tree/DeviceSecurity/DeviceSecurityPkg/Test/Tcg2DumpLog) to dump the SPDM Event Log defined in TCG [PFP Specification](https://trustedcomputinggroup.org/resource/pc-client-specific-platform-firmware-profile-specification/).

  Sample TCG Event Log can be found at [Example](https://github.com/jyao1/edk2/tree/DeviceSecurity/DeviceSecurityPkg/Example).

## TCG RIM

  Sample RIM(CoSWID) or CoRIM(CoMID) can be found at [Example](https://github.com/jyao1/edk2/tree/DeviceSecurity/DeviceSecurityPkg/Example).

## Known limitation
This package is only the sample code to show the concept.
It does not have a full validation such as robustness functional test and fuzzing test. It does not meet the production quality yet.
Any codes including the API definition, the libary and the drivers are subject to change.


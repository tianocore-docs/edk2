## @file
# UEFI 2.4 Network Module Package for All Architectures
#
# (C) Copyright 2014 Hewlett-Packard Development Company, L.P.<BR>
# Copyright (c) 2009 - 2018, Intel Corporation. All rights reserved.<BR>
#
#    This program and the accompanying materials
#    are licensed and made available under the terms and conditions of the BSD License
#    which accompanies this distribution. The full text of the license may be found at
#    http://opensource.org/licenses/bsd-license.php
#
#    THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
#    WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

[Defines]
  PLATFORM_NAME                  = TcgTpmPkg
  PLATFORM_GUID                  = 1729B106-A2A4-4E51-8125-5560ACC4DD07
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/TcgTpmPkg
  SUPPORTED_ARCHITECTURES        = IA32|IPF|X64|EBC|ARM|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PeimEntryPoint|MdePkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  SmmServicesTableLib|MdePkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  TimerLib|MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  SafeIntLib|MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf  
  FileHandleLib|MdePkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf

  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf  
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  SmmMemLib|MdePkg/Library/SmmMemLib/SmmMemLib.inf
  ResetSystemLib|ResetPkg/Library/BaseResetSystemLibNull/BaseResetSystemLibNull.inf
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  IntrinsicLib|CryptoPkg/Library/IntrinsicLib/IntrinsicLib.inf
  OpensslLib|CryptoPkg/Library/OpensslLib/OpensslLib.inf

  TpmMeasurementLib|TcgTpmPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf
  TcgPhysicalPresenceLib|TcgTpmPkg/Tpm12/Library/TcgPhysicalPresenceLibNull/TcgPhysicalPresenceLibNull.inf
  Tpm12CommandLib|TcgTpmPkg/Tpm12/Library/Tpm12CommandLib/Tpm12CommandLib.inf
  Tpm2CommandLib|TcgTpmPkg/Tpm2/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  Tcg2PhysicalPresenceLib|TcgTpmPkg/Tpm2/Library/Tcg2PhysicalPresenceLibNull/Tcg2PhysicalPresenceLibNull.inf
  TcgPpVendorLib|TcgTpmPkg/Tpm12/Library/TcgPpVendorLibNull/TcgPpVendorLibNull.inf
  Tcg2PpVendorLib|TcgTpmPkg/Tpm2/Library/Tcg2PpVendorLibNull/Tcg2PpVendorLibNull.inf

[LibraryClasses.common.PEIM]
  HobLib|MdePkg/Library/PeiHobLib/PeiHobLib.inf
  MemoryAllocationLib|MdePkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/PeiCryptLib.inf

  HashLib|TcgTpmPkg/Tpm2/Library/HashLibBaseCryptoRouter/HashLibBaseCryptoRouterPei.inf
  Tpm12DeviceLib|TcgTpmPkg/Tpm12/Library/Tpm12DeviceLibDTpm/Tpm12DeviceLibDTpm.inf
  Tpm2DeviceLib|TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf

[LibraryClasses.common.DXE_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/BaseCryptLib.inf

  HashLib|TcgTpmPkg/Tpm2/Library/HashLibBaseCryptoRouter/HashLibBaseCryptoRouterDxe.inf
  Tpm12DeviceLib|TcgTpmPkg/Tpm12/Library/Tpm12DeviceLibTcg/Tpm12DeviceLibTcg.inf
  Tpm2DeviceLib|TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|MdePkg/Library/SmmMemoryAllocationLib/SmmMemoryAllocationLib.inf
  BaseCryptLib|CryptoPkg/Library/BaseCryptLib/SmmCryptLib.inf

  Tpm12DeviceLib|TcgTpmPkg/Tpm12/Library/Tpm12DeviceLibTcg/Tpm12DeviceLibTcg.inf
  Tpm2DeviceLib|TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf

[PcdsDynamicDefault.common.DEFAULT]
  gEfiTcgTpmPkgTokenSpaceGuid.PcdTpmInstanceGuid|{0xb6, 0xe5, 0x01, 0x8b, 0x19, 0x4f, 0xe8, 0x46, 0xab, 0x93, 0x1c, 0x53, 0x67, 0x1b, 0x90, 0xcc}
  gEfiTcgTpmPkgTokenSpaceGuid.PcdTpm2InitializationPolicy|1
  gEfiTcgTpmPkgTokenSpaceGuid.PcdTpm2SelfTestPolicy|1
  gEfiTcgTpmPkgTokenSpaceGuid.PcdTpm2ScrtmPolicy|1
  gEfiTcgTpmPkgTokenSpaceGuid.PcdTpmInitializationPolicy|1
  gEfiTcgTpmPkgTokenSpaceGuid.PcdTpmScrtmPolicy|1
  gEfiTcgTpmPkgTokenSpaceGuid.PcdTpm2HashMask|3
  gEfiTcgTpmPkgTokenSpaceGuid.PcdTcg2HashAlgorithmBitmap|3

[PcdsDynamicHii.common.DEFAULT]
  gEfiTcgTpmPkgTokenSpaceGuid.PcdTcgPhysicalPresenceInterfaceVer|L"TCG2_VERSION"|gTcg2ConfigFormSetGuid|0x0|"1.3"|NV,BS
  gEfiTcgTpmPkgTokenSpaceGuid.PcdTpm2AcpiTableRev|L"TCG2_VERSION"|gTcg2ConfigFormSetGuid|0x8|3|NV,BS

###################################################################################################
#
# Components Section - list of the modules and components that will be processed by compilation
#                      tools and the EDK II tools to generate PE32/PE32+/Coff image files.
#
# Note: The EDK II DSC file is not used to specify how compiled binary images get placed
#       into firmware volume images. This section is just a list of modules to compile from
#       source into UEFI-compliant binaries.
#       It is the FDF file that contains information on combining binary files into firmware
#       volume images, whose concept is beyond UEFI and is described in PI specification.
#       Binary modules do not need to be listed in this section, as they should be
#       specified in the FDF file. For example: Shell binary (Shell_Full.efi), FAT binary (Fat.efi),
#       Logo (Logo.bmp), and etc.
#       There may also be modules listed in this section that are not required in the FDF file,
#       When a module listed here is excluded from FDF file, then UEFI-compliant binary will be
#       generated for it, but the binary will not be put into any firmware volume.
#
###################################################################################################

[Components]

  #
  # TPM
  #
  TcgTpmPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf

  #
  # TPM12
  #
  TcgTpmPkg/Tpm12/Library/DxeTpmMeasureBootLib/DxeTpmMeasureBootLib.inf
  TcgTpmPkg/Tpm12/Library/Tpm12CommandLib/Tpm12CommandLib.inf
  TcgTpmPkg/Tpm12/Library/Tpm12DeviceLibDTpm/Tpm12DeviceLibDTpm.inf
  TcgTpmPkg/Tpm12/Library/Tpm12DeviceLibTcg/Tpm12DeviceLibTcg.inf

  TcgTpmPkg/Tpm12/TcgPei/TcgPei.inf
  TcgTpmPkg/Tpm12/TcgDxe/TcgDxe.inf
  TcgTpmPkg/Tpm12/TcgConfigDxe/TcgConfigDxe.inf {
    <LibraryClasses>
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }

  #
  # TPM2
  #
  TcgTpmPkg/Tpm2/Library/DxeTpm2MeasureBootLib/DxeTpm2MeasureBootLib.inf

  TcgTpmPkg/Tpm2/Library/HashLibBaseCryptoRouter/HashLibBaseCryptoRouterDxe.inf
  TcgTpmPkg/Tpm2/Library/HashLibBaseCryptoRouter/HashLibBaseCryptoRouterPei.inf

  TcgTpmPkg/Tpm2/Library/Tpm2CommandLib/Tpm2CommandLib.inf
  TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
  TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf
  TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
  TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
  TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterPei.inf

  TcgTpmPkg/Tpm2/Library/HashLibTpm2/HashLibTpm2.inf

  TcgTpmPkg/Tpm2/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
  TcgTpmPkg/Tpm2/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf

  TcgTpmPkg/Tpm2/Tcg2Config/Tcg2ConfigPei.inf {
    <LibraryClasses>
      Tpm12DeviceLib|TcgTpmPkg/Tpm12/Library/Tpm12DeviceLibDTpm/Tpm12DeviceLibDTpm.inf
      Tpm2DeviceLib|TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibDTpm/Tpm2DeviceLibDTpm.inf
  }
  TcgTpmPkg/Tpm2/Tcg2Pei/Tcg2Pei.inf {
    <LibraryClasses>
      Tpm2DeviceLib|TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterPei.inf
      NULL|TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
      NULL|TcgTpmPkg/Tpm2/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
      NULL|TcgTpmPkg/Tpm2/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
  }

  TcgTpmPkg/Tpm2/Tcg2Dxe/Tcg2Dxe.inf {
    <LibraryClasses>
      Tpm2DeviceLib|TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibRouter/Tpm2DeviceLibRouterDxe.inf
      NULL|TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibDTpm/Tpm2InstanceLibDTpm.inf
      NULL|TcgTpmPkg/Tpm2/Library/HashInstanceLibSha1/HashInstanceLibSha1.inf
      NULL|TcgTpmPkg/Tpm2/Library/HashInstanceLibSha256/HashInstanceLibSha256.inf
      PcdLib|MdePkg/Library/DxePcdLib/DxePcdLib.inf
  }
  TcgTpmPkg/Tpm2/Tcg2Config/Tcg2ConfigDxe.inf {
    <LibraryClasses>
      Tpm2DeviceLib|TcgTpmPkg/Tpm2/Library/Tpm2DeviceLibTcg2/Tpm2DeviceLibTcg2.inf
  }

[Components.IA32, Components.X64]
  TcgTpmPkg/Tpm12/TcgSmm/TcgSmm.inf
  TcgTpmPkg/Tpm2/Tcg2Smm/Tcg2Smm.inf

[BuildOptions]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES

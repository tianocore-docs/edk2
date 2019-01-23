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
  PLATFORM_NAME                  = PiSmmFoundationPkg
  PLATFORM_GUID                  = 74176DF9-88F9-4E83-875F-06F11BE5675B
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/PiSmmFoundationPkg
  SUPPORTED_ARCHITECTURES        = IA32|IPF|X64|EBC|ARM|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[LibraryClasses]
  DebugLib|BasePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  BaseLib|BasePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|BasePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  DevicePathLib|UefiPkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  MemoryAllocationLib|UefiPkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  PcdLib|BasePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PrintLib|BasePkg/Library/BasePrintLib/BasePrintLib.inf
  PeimEntryPoint|PiPeiPkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  UefiDriverEntryPoint|UefiPkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  UefiApplicationEntryPoint|UefiPkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  UefiBootServicesTableLib|UefiPkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  SmmServicesTableLib|PiMmPkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf
  UefiLib|UefiPkg/Library/UefiLib/UefiLib.inf
  UefiRuntimeServicesTableLib|UefiPkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  PeiServicesTablePointerLib|PiPeiPkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  PeiServicesLib|PiPeiPkg/Library/PeiServicesLib/PeiServicesLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  TimerLib|BasePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  PerformanceLib|BasePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PeCoffGetEntryPointLib|BasePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  DxeServicesLib|PiDxePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DxeServicesTableLib|PiDxePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf
  SafeIntLib|BasePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf
  CacheMaintenanceLib|BasePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  PeCoffLib|BasePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffGetEntryPointLib|BasePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  PeCoffExtraActionLib|BasePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf

  DebugPrintErrorLevelLib|BasePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf  
  FileHandleLib|UefiPkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  SortLib|MdeModulePkg/Library/UefiSortLib/UefiSortLib.inf

  ReportStatusCodeLib|PiPeiPkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf  
  UefiRuntimeLib|UefiPkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
  CpuExceptionHandlerLib|MdeModulePkg/Library/CpuExceptionHandlerLibNull/CpuExceptionHandlerLibNull.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf
  SmmMemLib|PiMmPkg/Library/SmmMemLib/SmmMemLib.inf
  IoLib|BasePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  SynchronizationLib|BasePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  SerialPortLib|BasePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  UefiDecompressLib|BasePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf

[LibraryClasses.common.DXE_DRIVER,LibraryClasses.common.UEFI_DRIVER]
  HobLib|PiDxePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|UefiPkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

[LibraryClasses.common.DXE_RUNTIME_DRIVER]
  HobLib|PiDxePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|UefiPkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  HobLib|PiDxePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|PiMmPkg/Library/SmmMemoryAllocationLib/SmmMemoryAllocationLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  HobLib|PiDxePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|UefiPkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf

[LibraryClasses.common.SMM_CORE]
  HobLib|PiDxePkg/Library/DxeHobLib/DxeHobLib.inf
  MemoryAllocationLib|PiSmmFoundationPkg/Library/PiSmmCoreMemoryAllocationLib/PiSmmCoreMemoryAllocationLib.inf
  SmmServicesTableLib|PiSmmFoundationPkg/Library/PiSmmCoreSmmServicesTableLib/PiSmmCoreSmmServicesTableLib.inf
  SmmCorePlatformHookLib|PiSmmFoundationPkg/Library/SmmCorePlatformHookLibNull/SmmCorePlatformHookLibNull.inf  
  SmmMemLib|PiMmPkg/Library/SmmMemLib/SmmMemLib.inf

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

[Components.IA32, Components.X64]
  PiSmmFoundationPkg/Library/PiSmmCoreMemoryAllocationLib/PiSmmCoreMemoryAllocationProfileLib.inf
  PiSmmFoundationPkg/Library/PiSmmCoreMemoryAllocationLib/PiSmmCoreMemoryAllocationLib.inf
  PiSmmFoundationPkg/Library/SmmCorePerformanceLib/SmmCorePerformanceLib.inf
  PiSmmFoundationPkg/Library/SmmCorePlatformHookLibNull/SmmCorePlatformHookLibNull.inf
  PiSmmFoundationPkg/Universal/SmmCommunicationBufferDxe/SmmCommunicationBufferDxe.inf
  PiSmmFoundationPkg/Library/PiSmmCoreSmmServicesTableLib/PiSmmCoreSmmServicesTableLib.inf

  PiSmmFoundationPkg/PiSmmCore/PiSmmIpl.inf
  PiSmmFoundationPkg/PiSmmCore/PiSmmCore.inf

[BuildOptions]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES

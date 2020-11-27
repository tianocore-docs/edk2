## @file ContikiPkg.dsc
# 
# Copyright (c) 2014, Intel Corporation. All rights reserved.
# This program and the accompanying materials
# are licensed and made available under the terms and conditions of the BSD License
# which accompanies this distribution.  The full text of the license may be found at
# http://opensource.org/licenses/bsd-license.php
# 
# THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
# WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.
#
##

[Defines]
  PLATFORM_NAME                  = ContikiPkg
  PLATFORM_GUID                  = 4BD52605-0AE3-4b4e-8BE2-F135995BDB2D
  PLATFORM_VERSION               = 0.1
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/ContikiPkgNt32
  SUPPORTED_ARCHITECTURES        = IA32|X64
  BUILD_TARGETS                  = DEBUG|RELEASE
  SKUID_IDENTIFIER               = DEFAULT

  DEFINE IPV6_ENABLE             = TRUE

[LibraryClasses]
  BaseLib|MdePkg/Library/BaseLib/BaseLib.inf
  BaseMemoryLib|MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
#  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  CacheMaintenanceLib|MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf
  DebugLib|Nt32Pkg/Library/DxeDebugLibWinNt/DxeDebugLibWinNt.inf
  ReportStatusCodeLib|MdeModulePkg/Library/DxeReportStatusCodeLib/DxeReportStatusCodeLib.inf
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf
!if $(IPV6_ENABLE) == TRUE
  ContikiLib|ContikiPkg/Contiki/ContikiLibIpv6.inf
!else
  ContikiLib|ContikiPkg/Contiki/ContikiLibIpv4.inf
!endif
  WinNtLib|Nt32Pkg/Library/DxeWinNtLib/DxeWinNtLib.inf
  PlatformStubLib|ContikiPkg/Library/Nt32StubLib/Nt32Stub.inf
  SynchronizationLib|ContikiPkg/Library/SimpleSynchronizationLib/SimpleSynchronizationLib.inf

[LibraryClasses.common.UEFI_APPLICATION]
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf
  ShellLib|ShellPkg/Library/UefiShellLib/UefiShellLib.inf
#  FileHandleLib|ShellPkg/Library/BaseFileHandleLib/BaseFileHandleLib.inf
  FileHandleLib|ShellPkg/Library/UefiFileHandleLib/UefiFileHandleLib.inf
  ShellCEntryLib|ShellPkg/Library/UefiShellCEntryLib/UefiShellCEntryLib.inf
  UefiHiiServicesLib|MdeModulePkg/Library/UefiHiiServicesLib/UefiHiiServicesLib.inf
  HiiLib|MdeModulePkg/Library/UefiHiiLib/UefiHiiLib.inf
  SortLib|ShellPkg/Library/UefiSortLib/UefiSortLib.inf

[LibraryClasses.common.DXE_SMM_DRIVER]
  SmmServicesTableLib|MdePkg/Library/SmmServicesTableLib/SmmServicesTableLib.inf
  MemoryAllocationLib|MdePkg/Library/SmmMemoryAllocationLib/SmmMemoryAllocationLib.inf

[Components]
  ContikiPkg/Contiki/UefiSupport/examples/hello-world/ContikiHelloWorld.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = /X
  }
  ContikiPkg/Contiki/UefiSupport/examples/hello-world/ContikiBlinkHello.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = /X
  }
  ContikiPkg/Contiki/UefiSupport/examples/hello-world/ContikiTimerTest.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = /X
  }

!if $(IPV6_ENABLE) == TRUE
  ContikiPkg/Contiki/UefiSupport/examples/udp-ipv6/ContikiUdpIp6Client.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = /X
  }
  ContikiPkg/Contiki/UefiSupport/examples/udp-ipv6/ContikiUdpIp6Server.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = /X
  }
!else
  ContikiPkg/Contiki/UefiSupport/examples/udp-ipv4/ContikiUdpIp4Client.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = /X
  }
  ContikiPkg/Contiki/UefiSupport/examples/udp-ipv4/ContikiUdpIp4Server.inf {
    <BuildOptions>
      *_*_*_CC_FLAGS = /X
  }
!endif

[PcdsFixedAtBuild.common]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x1f
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80080046
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x07

[BuildOptions]
  DEBUG_*_*_DLINK_FLAGS = /EXPORT:InitializeDriver=$(IMAGE_ENTRY_POINT) /BASE:0x10000 /ALIGN:4096 /FILEALIGN:4096 /SUBSYSTEM:CONSOLE
  RELEASE_*_*_DLINK_FLAGS = /ALIGN:4096 /FILEALIGN:4096
  *_*_*_CC_FLAGS = /Od /GL- /DEFI_NT_EMULATOR=1

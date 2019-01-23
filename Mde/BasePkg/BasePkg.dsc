## @file
# EFI/PI BasePkg Package
#
# Copyright (c) 2007 - 2018, Intel Corporation. All rights reserved.<BR>
# Portions copyright (c) 2008 - 2009, Apple Inc. All rights reserved.<BR>
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
  PLATFORM_NAME                  = Mde
  PLATFORM_GUID                  = 082F8BFC-0455-4859-AE3C-ECD64FB81642
  PLATFORM_VERSION               = 1.08
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/Mde
  SUPPORTED_ARCHITECTURES        = IA32|IPF|X64|EBC|ARM|AARCH64
  BUILD_TARGETS                  = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER               = DEFAULT

[PcdsFixedAtBuild]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x0f
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000000
  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0xE0000000

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
  BasePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  BasePkg/Library/BaseCpuLib/BaseCpuLib.inf
  BasePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  BasePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  BasePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  BasePkg/Library/BaseLib/BaseLib.inf
  BasePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  BasePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  BasePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  BasePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  BasePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  BasePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  BasePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  BasePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf
  BasePkg/Library/BasePciSegmentInfoLibNull/BasePciSegmentInfoLibNull.inf
  BasePkg/Library/BasePciSegmentLibSegmentInfo/BasePciSegmentLibSegmentInfo.inf
  BasePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  BasePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  BasePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  BasePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  BasePkg/Library/BasePostCodeLibDebug/BasePostCodeLibDebug.inf
  BasePkg/Library/BasePostCodeLibPort80/BasePostCodeLibPort80.inf
  BasePkg/Library/BasePrintLib/BasePrintLib.inf
  BasePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  BasePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  BasePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  BasePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  BasePkg/Library/BaseSmbusLibNull/BaseSmbusLibNull.inf
  BasePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

[Components.IA32, Components.X64]
  BasePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  BasePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicSev.inf
  BasePkg/Library/BaseMemoryLibMmx/BaseMemoryLibMmx.inf
  BasePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf
  BasePkg/Library/BaseMemoryLibOptPei/BaseMemoryLibOptPei.inf
  BasePkg/Library/BaseMemoryLibRepStr/BaseMemoryLibRepStr.inf
  BasePkg/Library/BaseMemoryLibSse2/BaseMemoryLibSse2.inf
  BasePkg/Library/BaseRngLib/BaseRngLib.inf

[Components.IPF]
  BasePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  
[Components.EBC]
  BasePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf

[Components.ARM, Components.AARCH64]
  BasePkg/Library/BaseStackCheckLib/BaseStackCheckLib.inf

[BuildOptions]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES

## @file
# EFI/PI MdePkg Package
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

[PcdsFeatureFlag]
  gEfiMdePkgTokenSpaceGuid.PcdUgaConsumeSupport|TRUE

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
  MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf
  MdePkg/Library/BaseCpuLib/BaseCpuLib.inf
  MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  MdePkg/Library/BaseDebugLibSerialPort/BaseDebugLibSerialPort.inf
  MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  MdePkg/Library/BaseLib/BaseLib.inf
  MdePkg/Library/BaseMemoryLib/BaseMemoryLib.inf
  MdePkg/Library/BaseOrderedCollectionRedBlackTreeLib/BaseOrderedCollectionRedBlackTreeLib.inf
  MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf
  MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf
  MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf
  MdePkg/Library/BasePciLibPciExpress/BasePciLibPciExpress.inf
  MdePkg/Library/BasePciSegmentLibPci/BasePciSegmentLibPci.inf
  MdePkg/Library/BasePciSegmentInfoLibNull/BasePciSegmentInfoLibNull.inf
  MdePkg/Library/BasePciSegmentLibSegmentInfo/BasePciSegmentLibSegmentInfo.inf
  MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf
  MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf
  MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf
  MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  MdePkg/Library/BasePostCodeLibDebug/BasePostCodeLibDebug.inf
  MdePkg/Library/BasePostCodeLibPort80/BasePostCodeLibPort80.inf
  MdePkg/Library/BasePrintLib/BasePrintLib.inf
  MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  MdePkg/Library/BaseSerialPortLibNull/BaseSerialPortLibNull.inf
  MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf
  MdePkg/Library/BaseTimerLibNullTemplate/BaseTimerLibNullTemplate.inf
  MdePkg/Library/BaseUefiDecompressLib/BaseUefiDecompressLib.inf
  MdePkg/Library/BaseSmbusLibNull/BaseSmbusLibNull.inf
  MdePkg/Library/BaseSafeIntLib/BaseSafeIntLib.inf

[Components.IA32, Components.X64]
  MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsicSev.inf
  MdePkg/Library/BaseMemoryLibMmx/BaseMemoryLibMmx.inf
  MdePkg/Library/BaseMemoryLibOptDxe/BaseMemoryLibOptDxe.inf
  MdePkg/Library/BaseMemoryLibOptPei/BaseMemoryLibOptPei.inf
  MdePkg/Library/BaseMemoryLibRepStr/BaseMemoryLibRepStr.inf
  MdePkg/Library/BaseMemoryLibSse2/BaseMemoryLibSse2.inf
  MdePkg/Library/BaseRngLib/BaseRngLib.inf

[Components.IPF]
  MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf
  
[Components.EBC]
  MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf

[Components.ARM, Components.AARCH64]
  MdePkg/Library/BaseStackCheckLib/BaseStackCheckLib.inf

[BuildOptions]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES

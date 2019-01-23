## @file
# EFI/PI PiPeiPkg Package
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
  PLATFORM_NAME                  = PiPeiPkg
  PLATFORM_GUID                  = FA724C06-3080-4B53-8B8E-C3382ADB5214
  PLATFORM_VERSION               = 1.08
  DSC_SPECIFICATION              = 0x00010005
  OUTPUT_DIRECTORY               = Build/PiPeiPkg
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

  PiPeiPkg/Library/PeiDxePostCodeLibReportStatusCode/PeiDxePostCodeLibReportStatusCode.inf
  PiPeiPkg/Library/PeiExtractGuidedSectionLib/PeiExtractGuidedSectionLib.inf
  PiPeiPkg/Library/PeiHobLib/PeiHobLib.inf
  PiPeiPkg/Library/PeiIoLibCpuIo/PeiIoLibCpuIo.inf
  PiPeiPkg/Library/PeiMemoryAllocationLib/PeiMemoryAllocationLib.inf
  PiPeiPkg/Library/PeiMemoryLib/PeiMemoryLib.inf
  PiPeiPkg/Library/PeimEntryPoint/PeimEntryPoint.inf
  PiPeiPkg/Library/PeiPcdLib/PeiPcdLib.inf
  PiPeiPkg/Library/PeiResourcePublicationLib/PeiResourcePublicationLib.inf
  PiPeiPkg/Library/PeiServicesLib/PeiServicesLib.inf
  PiPeiPkg/Library/PeiServicesTablePointerLib/PeiServicesTablePointerLib.inf
  PiPeiPkg/Library/PeiSmbusLibSmbus2Ppi/PeiSmbusLibSmbus2Ppi.inf
  PiPeiPkg/Library/PeiPciLibPciCfg2/PeiPciLibPciCfg2.inf
  PiPeiPkg/Library/PeiPciSegmentLibPciCfg2/PeiPciSegmentLibPciCfg2.inf
  PiPeiPkg/Library/BaseExtractGuidedSectionLib/BaseExtractGuidedSectionLib.inf
  PiPeiPkg/Library/BaseS3PciSegmentLib/BaseS3PciSegmentLib.inf
  PiPeiPkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf

[Components.IA32, Components.X64]
  PiPeiPkg/Library/PeiServicesTablePointerLibIdt/PeiServicesTablePointerLibIdt.inf
  PiPeiPkg/Library/SecPeiDxeTimerLibCpu/SecPeiDxeTimerLibCpu.inf
  PiPeiPkg/Library/BaseS3BootScriptLibNull/BaseS3BootScriptLibNull.inf
  PiPeiPkg/Library/BaseS3IoLib/BaseS3IoLib.inf
  PiPeiPkg/Library/BaseS3PciLib/BaseS3PciLib.inf
  PiPeiPkg/Library/BaseS3SmbusLib/BaseS3SmbusLib.inf
  PiPeiPkg/Library/BaseS3StallLib/BaseS3StallLib.inf

[BuildOptions]
  *_*_*_CC_FLAGS = -D DISABLE_NEW_DEPRECATED_INTERFACES

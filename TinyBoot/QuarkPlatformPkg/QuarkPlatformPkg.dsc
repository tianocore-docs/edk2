## @file       
# Clanton Peak CRB platform with 32-bit DXE for 4MB/8MB flash devices.       
#       
# This package provides Clanton Peak CRB platform specific modules.       
# Copyright(c) 2013 Intel Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
# * Neither the name of Intel Corporation nor the names of its
# contributors may be used to endorse or promote products derived
# from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#       
##       
 
################################################################################       
#       
# Defines Section - statements that will be processed to create a Makefile.       
#       
################################################################################       
[Defines]       
  PLATFORM_NAME                  = QuarkPlatformPkg       
  PLATFORM_GUID                  = 1BEDB57A-7904-406e-8486-C89FC7FB39EE       
  VPD_TOOL_GUID                  = 8C3D856A-9BE6-468E-850A-24F7A8D38E08        
  PLATFORM_VERSION               = 0.1       
  DSC_SPECIFICATION              = 0x00010005       
  OUTPUT_DIRECTORY               = Build/QuarkPlatform       
  SUPPORTED_ARCHITECTURES        = IA32       
  BUILD_TARGETS                  = DEBUG|RELEASE       
  SKUID_IDENTIFIER               = DEFAULT       
  
  #
  # Set the global variables
  #
  EDK_GLOBAL PLATFORM_PKG             = QuarkPlatformPkg
    
  #
  # Platform On/Off features are defined here
  #
  !include $(PLATFORM_PKG)/QuarkPlatformPkgConfig.dsc  
  
  FLASH_DEFINITION                    = $(PLATFORM_PKG)/QuarkPlatformPkg.fdf
  
################################################################################       
#       
# SKU Identification section - list of all SKU IDs supported by this       
#                              Platform.       
#       
################################################################################       
[SkuIds]       
  0|DEFAULT              # The entry: 0|DEFAULT is reserved and always required.       
 
################################################################################       
#       
# Library Class section - list of all Library Classes needed by this Platform.       
#       
################################################################################       
[LibraryClasses]       
  #       
  # Entry point       
  #       
  DxeCoreEntryPoint|MdePkg/Library/DxeCoreEntryPoint/DxeCoreEntryPoint.inf       
  UefiDriverEntryPoint|MdePkg/Library/UefiDriverEntryPoint/UefiDriverEntryPoint.inf       
  UefiApplicationEntryPoint|MdePkg/Library/UefiApplicationEntryPoint/UefiApplicationEntryPoint.inf       
  #       
  # Basic       
  #       
  BaseLib|QuarkSocPkg/Override/MdePkg/Library/BaseLib/BaseLib.inf       
  BaseMemoryLib|QuarkSocPkg/Override/MdePkg/Library/BaseMemoryLibRepStr/BaseMemoryLibRepStr.inf
  PrintLib|MdePkg/Library/BasePrintLib/BasePrintLib.inf       
  CpuLib|MdePkg/Library/BaseCpuLib/BaseCpuLib.inf       
  IoLib|MdePkg/Library/BaseIoLibIntrinsic/BaseIoLibIntrinsic.inf       
  PciLib|MdePkg/Library/BasePciLibCf8/BasePciLibCf8.inf       
  PciCf8Lib|MdePkg/Library/BasePciCf8Lib/BasePciCf8Lib.inf       
  PciExpressLib|MdePkg/Library/BasePciExpressLib/BasePciExpressLib.inf       
  CacheMaintenanceLib|MdePkg/Library/BaseCacheMaintenanceLib/BaseCacheMaintenanceLib.inf       
  #PeCoffLib|MdePkg/Library/BasePeCoffLib/BasePeCoffLib.inf       
  PeCoffLib|TinyBootPkg/Library/BasePeCoffLibPe32/BasePeCoffLib.inf
  PeCoffExtraActionLib|MdePkg/Library/BasePeCoffExtraActionLibNull/BasePeCoffExtraActionLibNull.inf       
  PeCoffGetEntryPointLib|MdePkg/Library/BasePeCoffGetEntryPointLib/BasePeCoffGetEntryPointLib.inf       
  #       
  # UEFI & PI       
  #       
  UefiBootServicesTableLib|MdePkg/Library/UefiBootServicesTableLib/UefiBootServicesTableLib.inf       
  UefiRuntimeServicesTableLib|MdePkg/Library/UefiRuntimeServicesTableLib/UefiRuntimeServicesTableLib.inf       
  UefiRuntimeLib|MdePkg/Library/UefiRuntimeLib/UefiRuntimeLib.inf       
  UefiLib|MdePkg/Library/UefiLib/UefiLib.inf       
  DevicePathLib|MdePkg/Library/UefiDevicePathLib/UefiDevicePathLib.inf       
  UefiDecompressLib|TinyBootPkg/Library/DecompressLibNull/DecompressLibNull.inf
  PeiServicesTablePointerLib|MdePkg/Library/PeiServicesTablePointerLibIdt/PeiServicesTablePointerLibIdt.inf       
  PeiServicesLib|MdePkg/Library/PeiServicesLib/PeiServicesLib.inf       
  DxeServicesLib|MdePkg/Library/DxeServicesLib/DxeServicesLib.inf       
  DxeServicesTableLib|MdePkg/Library/DxeServicesTableLib/DxeServicesTableLib.inf       
  UefiCpuLib|UefiCpuPkg/Library/BaseUefiCpuLib/BaseUefiCpuLib.inf       
  #       
  # Framework       
  #       
  S3BootScriptLib|MdePkg/Library/BaseS3BootScriptLibNull/BaseS3BootScriptLibNull.inf
  S3IoLib|MdePkg/Library/BaseS3IoLib/BaseS3IoLib.inf
  S3PciLib|MdePkg/Library/BaseS3PciLib/BaseS3PciLib.inf
  #       
  # Generic Modules       
  #       
  UefiUsbLib|MdePkg/Library/UefiUsbLib/UefiUsbLib.inf       
  UefiScsiLib|MdePkg/Library/UefiScsiLib/UefiScsiLib.inf       
  OemHookStatusCodeLib|MdeModulePkg/Library/OemHookStatusCodeLibNull/OemHookStatusCodeLibNull.inf          
  SynchronizationLib|MdePkg/Library/BaseSynchronizationLib/BaseSynchronizationLib.inf       
  SecurityManagementLib|MdeModulePkg/Library/DxeSecurityManagementLib/DxeSecurityManagementLib.inf
  SmmCorePlatformHookLib|MdeModulePkg/Library/SmmCorePlatformHookLibNull/SmmCorePlatformHookLibNull.inf
  #       
  # CPU       
  #       
  MtrrLib|QuarkSocPkg/Override/UefiCpuPkg/Library/MtrrLib/MtrrLib.inf
  LocalApicLib|UefiCpuPkg/Library/BaseXApicLib/BaseXApicLib.inf
  CpuConfigLib|IA32FamilyCpuBasePkg/Library/CpuConfigLib/CpuConfigLib.inf       
  CpuOnlyResetLib|IA32FamilyCpuBasePkg/Library/CpuOnlyResetLibNull/CpuOnlyResetLibNull.inf       
  #       
  # Quark North Cluster
  #       
  SmmLib|QuarkSocPkg/QuarkNorthCluster/Library/QNCSmmLib/QNCSmmLib.inf       
  SmbusLib|QuarkSocPkg/QuarkNorthCluster/Library/SmbusLib/SmbusLib.inf       
  TimerLib|QuarkSocPkg/QuarkNorthCluster/Library/IntelQNCAcpiTimerLib/IntelQNCAcpiTimerLib.inf        
  ResetSystemLib|QuarkSocPkg/QuarkNorthCluster/Library/ResetSystemLib/SimpleResetSystemLib.inf       
  IntelQNCLib|QuarkSocPkg/QuarkNorthCluster/Library/IntelQNCLib/IntelQNCLib.inf       
  QNCAccessLib|QuarkSocPkg/QuarkNorthCluster/Library/QNCAccessLib/QNCAccessLib.inf          
  RedirectPeiServicesLib|QuarkSocPkg/QuarkNorthCluster/Library/RedirectPeiServicesLib/RedirectPeiServicesLib.inf
  MrcLib|QuarkSocPkg/QuarkNorthCluster/MemoryInit/Pei/MrcLib.inf
  #       
  # Quark South Cluster
  #       
  IohLib|QuarkSocPkg/QuarkSouthCluster/Library/IohLib/IohLib.inf       
  SerialPortLib|QuarkSocPkg/QuarkSouthCluster/Library/IohSerialPortLib/IohSerialPortLib.inf
  #       
  # Platform       
  #       
  SmmCpuPlatformHookLib|QuarkPlatformPkg/Library/SmmCpuPlatformHookLib/SmmCpuPlatformHookLib.inf
  PlatformSecureLib|QuarkPlatformPkg/Library/PlatformSecureLib/PlatformSecureLib.inf
  SerialDebugLib|TinyBootPkg/Library/SerialDebugLib/SerialDebugLib.inf
  #       
  # Misc       
  #       
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf
  DebugPrintErrorLevelLib|MdePkg/Library/BaseDebugPrintErrorLevelLib/BaseDebugPrintErrorLevelLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf
  DebugAgentLib|MdeModulePkg/Library/DebugAgentLibNull/DebugAgentLibNull.inf       
  
  CpuExceptionHandlerLib|MdeModulePkg/Library/CpuExceptionHandlerLibNull/CpuExceptionHandlerLibNull.inf       
  TpmMeasurementLib|SecurityPkg/Library/DxeTpmMeasurementLib/DxeTpmMeasurementLib.inf

  ReportStatusCodeLib|MdePkg/Library/BaseReportStatusCodeLibNull/BaseReportStatusCodeLibNull.inf
  DebugLib|MdePkg/Library/BaseDebugLibNull/BaseDebugLibNull.inf       
  PcdLib|MdePkg/Library/BasePcdLibNull/BasePcdLibNull.inf       
  LockBoxLib|MdeModulePkg/Library/LockBoxNullLib/LockBoxNullLib.inf
  PerformanceLib|MdePkg/Library/BasePerformanceLibNull/BasePerformanceLibNull.inf

  ExtractGuidedSectionLib|TinyBootPkg/Library/ExtractGuidedSectionLibNull/ExtractGuidedSectionLibNull.inf
  PeRelocationLib|TinyBootPkg/Library/PeRelocationLib/PeRelocationLib.inf
  FvLib|TinyBootPkg/Library/FvLib/FvLib.inf

[LibraryClasses.IA32.SEC]       
  #       
  # SEC specific phase       
  #       

[LibraryClasses.IA32]       
  #       
  # DXE phase common       
  #       
  HobLib|MdePkg/Library/DxeHobLib/DxeHobLib.inf       
  MemoryAllocationLib|MdePkg/Library/UefiMemoryAllocationLib/UefiMemoryAllocationLib.inf       
  
  SmmPlatformHookLib|QuarkPlatformPkg/Library/SmmPlatformHookLib/SmmPlatformHookLib.inf
  AcpiTableLib|TinyBootPkg/Library/AcpiTableLib/AcpiTableLib.inf

  #
  # Platform DXE phase common.
  #
  PlatformHelperLib|QuarkPlatformPkg/Library/PlatformHelperLib/DxePlatformHelperLib.inf

[LibraryClasses.IA32.DXE_CORE]       
  HobLib|MdePkg/Library/DxeCoreHobLib/DxeCoreHobLib.inf       
  MemoryAllocationLib|MdeModulePkg/Library/DxeCoreMemoryAllocationLib/DxeCoreMemoryAllocationLib.inf       
 
[LibraryClasses.IA32.DXE_RUNTIME_DRIVER]       
  QNCAccessLib|QuarkSocPkg/QuarkNorthCluster/Library/QNCAccessLib/RuntimeQNCAccessLib.inf
  #SerialPortLib|QuarkSocPkg/QuarkSouthCluster/Library/IohSerialPortLib/IohSerialPortLibRuntime.inf
  #NULL|QuarkSocPkg/QuarkSouthCluster/Library/IohSerialPortLib/IohSerialPortLibRuntimeInit.inf

################################################################################
#
# Pcd Section - list of all EDK II PCD Entries defined by this Platform
#
################################################################################
[PcdsFeatureFlag]       
  gEfiMdeModulePkgTokenSpaceGuid.PcdSupportUpdateCapsuleReset|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdDxeIplSwitchToLongMode|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdPciBusHotplugDeviceSupport|FALSE
  gEfiMdeModulePkgTokenSpaceGuid.PcdInstallAcpiSdtProtocol|FALSE
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdPlatformCsmSupport|FALSE
  gEfiCpuTokenSpaceGuid.PcdCpuMaxCpuIDValueLimitFlag|FALSE
  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdIsaBusSerialUseHalfHandshake|FALSE
  !ifdef SECURE_LD
    gQuarkPlatformTokenSpaceGuid.PcdEnableSecureLock|TRUE
    gEfiQuarkNcSocIdTokenSpaceGuid.PcdRmuDmaLock|TRUE
  !else
    gQuarkPlatformTokenSpaceGuid.PcdEnableSecureLock|FALSE
    gEfiQuarkNcSocIdTokenSpaceGuid.PcdRmuDmaLock|FALSE
  !endif

  !if $(TARGET) == "RELEASE"
    gQuarkPlatformTokenSpaceGuid.WaitIfResetDueToError|FALSE
  !else
    gQuarkPlatformTokenSpaceGuid.WaitIfResetDueToError|TRUE
  !endif

  gEfiMdePkgTokenSpaceGuid.PcdComponentNameDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnosticsDisable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdComponentName2Disable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdDriverDiagnostics2Disable|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdUefiVariableDefaultLangDeprecate|TRUE
  gEfiMdePkgTokenSpaceGuid.PcdUgaConsumeSupport|FALSE

[PcdsFixedAtBuild]

  gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x00
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareRevision| 0x00010000
  gEfiMdeModulePkgTokenSpaceGuid.PcdFirmwareVendor|"EDK II"
  gEfiMdePkgTokenSpaceGuid.PcdReportStatusCodePropertyMask|0x06       
  gEfiMdePkgTokenSpaceGuid.PcdPostCodePropertyMask|0x18       
  gEfiMdePkgTokenSpaceGuid.PcdPerformanceLibraryPropertyMask|0x00
  gEfiMdePkgTokenSpaceGuid.PcdPciExpressBaseAddress|0xE0000000       
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultBaudRate|115200
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultDataBits|8
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultParity|1
  gEfiMdePkgTokenSpaceGuid.PcdUartDefaultStopBits|1
  gEfiMdePkgTokenSpaceGuid.PcdDefaultTerminalType|0

  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxPeiPerformanceLogEntries|40  
  gEfiMdeModulePkgTokenSpaceGuid.PcdLoadModuleAtFixAddressEnable|0       
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxVariableSize|0x2000       
  gEfiMdeModulePkgTokenSpaceGuid.PcdHwErrStorageSize|0x00002000       
  gEfiMdeModulePkgTokenSpaceGuid.PcdMaxHardwareErrorVariableSize|0x1000       
  ## RTC Update Timeout Value, need to increase timeout since also
  # waiting for RTC to be busy.
  gEfiMdeModulePkgTokenSpaceGuid.PcdRealTimeClockUpdateTimeout|500000

  gEfiCpuTokenSpaceGuid.PcdTemporaryRamSize|0x4000       
  gEfiCpuTokenSpaceGuid.PcdPlatformType|1         
  gEfiCpuTokenSpaceGuid.PcdPlatformCpuMaxCoreFrequency|3800       
  gEfiCpuTokenSpaceGuid.PcdPlatformCpuMaxFsbFrequency|1066       

  gPcAtChipsetPkgTokenSpaceGuid.Pcd8259LegacyModeEdgeLevel|0x0000
  gPcAtChipsetPkgTokenSpaceGuid.Pcd8259LegacyModeMask|0xFFFF

  gEfiIntelFrameworkModulePkgTokenSpaceGuid.PcdLegacyBiosCacheLegacyRegion|FALSE       

  gQuarkPlatformTokenSpaceGuid.PcdMemorySize|0x10000000

  gEfiCpuTokenSpaceGuid.PcdCpuMaxLogicalProcessorNumber|1

[PcdsPatchableInModule]
  gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x00000000

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
 
[Components.IA32]       
  #       
  # SEC Core       
  #       
  QuarkPlatformPkg/Cpu/SecCore/SecCore.inf {
    <PcdsFixedAtBuild>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
    <PcdsPatchableInModule>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
    <LibraryClasses>
!if $(MRC_COMPRESSION) == FALSE
      PlatformSecLib|QuarkPlatformPkg/Library/QuarkSecLib/QuarkSecLibDecompress.inf       
      ExtractGuidedSectionLib|TinyBootPkg/Library/SimpleExtractGuidedSectionLib/SimpleExtractGuidedSectionLib.inf
      NULL|IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LzmaArchCustomDecompressLib.inf
!else
      PlatformSecLib|QuarkPlatformPkg/Library/QuarkSecLib/QuarkSecLib.inf       
!endif
      PeRelocationLib|TinyBootPkg/Library/PeRelocationLib/PeRelocationLibPe.inf
      #DebugLib|TinyBootPkg/Library/SimpleDebugLibSerialPort/SimpleDebugLibSerialPort.inf
  }
  QuarkPlatformPkg/Cpu/Sec/ResetVector/QuarkResetVector.inf {
    <PcdsFixedAtBuild>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
    <PcdsPatchableInModule>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
    <LibraryClasses>
      PlatformSecServicesLib|QuarkPlatformPkg/Library/PlatformSecServicesLib/PlatformSecServicesLib.inf
      PeRelocationLib|TinyBootPkg/Library/PeRelocationLib/PeRelocationLibTe.inf
  }
!if $(MRC_COMPRESSION) == TRUE
  QuarkPlatformPkg/Cpu/SecTrampoline/SecTrampoline.inf {
    <PcdsFixedAtBuild>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
    <PcdsPatchableInModule>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
    <LibraryClasses>
      ExtractGuidedSectionLib|TinyBootPkg/Library/SimpleExtractGuidedSectionLib/SimpleExtractGuidedSectionLib.inf
      NULL|IntelFrameworkModulePkg/Library/LzmaCustomDecompressLib/LzmaArchCustomDecompressLib.inf
      PeRelocationLib|TinyBootPkg/Library/PeRelocationLib/PeRelocationLibTe.inf
      #DebugLib|TinyBootPkg/Library/SimpleDebugLibSerialPort/SimpleDebugLibSerialPort.inf
  }
!endif

[Components.IA32]       
  #
  # DXE Core       
  #       
!if $(SMALL_DXE_CORE) == TRUE
  TinyBootPkg/Core/SimpleDxeCore/DxeMain.inf {
      <PcdsFixedAtBuild>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
    <PcdsPatchableInModule>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
    <LibraryClasses>       
      #DebugLib|TinyBootPkg/Library/SimpleDebugLibSerialPort/SimpleDebugLibSerialPort.inf
  }       
!else
  MdeModulePkg/Core/Dxe/DxeMain.inf {
      <PcdsFixedAtBuild>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
    <PcdsPatchableInModule>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
    <LibraryClasses>       
      #DebugLib|TinyBootPkg/Library/SimpleDebugLibSerialPort/SimpleDebugLibSerialPort.inf
  }       
!endif
  QuarkPlatformPkg/Platform/Dxe/PlatformInit/PlatformInitDxe.inf {
    <LibraryClasses>       
      NULL|QuarkPlatformPkg/Platform/Dxe/PlatformEarlyInit/PlatformEarlyInitLib.inf
  }

  #     
  # Components that produce the architectural protocols     
  #       
  IA32FamilyCpuBasePkg/SimpleCpuArchDxe/CpuArchDxe.inf
  MdeModulePkg/Universal/Metronome/Metronome.inf       
  MdeModulePkg/Core/RuntimeDxe/RuntimeDxe.inf
  TinyBootPkg/Universal/Variable/NullVariableRuntimeDxe/NullVariableRuntimeDxe.inf {
    <LibraryClasses>       
      NULL|TinyBootPkg/Library/NullArchProtocolsLib/ArchProtocols.inf
  }

  MdeModulePkg/Universal/ResetSystemRuntimeDxe/ResetSystemRuntimeDxe.inf
  TinyBootPkg/PcAtChipset/SimplePcatRealTimeClockRuntimeDxe/PcatRealTimeClockRuntimeDxe.inf

  QuarkPlatformPkg/Bds/TinyBootLoader/TinyBds.inf {
      <PcdsFixedAtBuild>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPropertyMask|0x27
    <PcdsPatchableInModule>
      #gEfiMdePkgTokenSpaceGuid.PcdDebugPrintErrorLevel|0x80000042
    <LibraryClasses>       
      #DebugLib|TinyBootPkg/Library/SimpleDebugLibSerialPort/SimpleDebugLibSerialPort.inf
  }       
 
  QuarkPlatformPkg/Pci/PlatformFixedPciResource/PlatformFixedPciResource.inf
 
  #       
  # Platform  
  #   
  QuarkSocPkg/QuarkNorthCluster/QNCInit/Dxe/QNCInitDxe.inf

  #       
  # ACPI       
  #       
  QuarkPlatformPkg/Acpi/AcpiTables/AcpiTables.inf       
  QuarkPlatformPkg/Acpi/Dxe/AcpiPlatform/AcpiPlatform.inf       
 
  #       
  # SMM       
  #       
  IA32FamilyCpuBasePkg/SimplePiSmmCpuDxeSmm/SmmIpl.inf
  IA32FamilyCpuBasePkg/SimplePiSmmCpuDxeSmm/PiSmmCpuDxeSmm.inf
 
  #       
  # PCI       
  #       
  QuarkSocPkg/QuarkSouthCluster/IohInit/Dxe/IohInitDxe.inf        
 
  #       
  # SDIO       
  #       
  #QuarkSocPkg/QuarkSouthCluster/Sdio/Dxe/SDControllerDxe/SDControllerDxe.inf       
  #QuarkSocPkg/QuarkSouthCluster/Sdio/Dxe/SDMediaDeviceDxe/SDMediaDeviceDxe.inf       
 
  #       
  # IDE/SCSI       
  #       
  #MdeModulePkg/Universal/Disk/DiskIoDxe/DiskIoDxe.inf       
  #MdeModulePkg/Universal/Disk/PartitionDxe/PartitionDxe.inf       
 
  #       
  # Console       
  #
  TinyBootPkg/Universal/Console/SimpleCombinedTerminalDxe/TerminalDxe.inf

  #       
  # Legacy Modules       
  #        
  PcAtChipsetPkg/8259InterruptControllerDxe/8259.inf       
 
  #       
  # File System Modules
  #        
  #MdeModulePkg/Universal/Disk/UnicodeCollation/EnglishDxe/EnglishDxe.inf       
  #FatPkg/EnhancedFatDxe/Fat.inf

###################################################################################################
#
# BuildOptions Section - Define the module specific tool chain flags that should be used as
#                        the default flags for a module. These flags are appended to any 
#                        standard flags that are defined by the build process. They can be 
#                        applied for any modules or only those modules with the specific 
#                        module style (EDK or EDKII) specified in [Components] section.
#
###################################################################################################
#BugID31643 - Start
[BuildOptions.Common.EDKII]
GCC:*_*_IA32_CC_FLAGS    = -include $(WORKSPACE)/QuarkPlatformPkg/Override/MdePkg/Include/Library/DebugLib.h
#BugID31643 - End

[BuildOptions]
# *_*_*_CC_FLAGS = /Zi
# *_*_*_DLINK_FLAGS = /DEBUG


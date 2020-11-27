/** @file
  DXE Core Main Entry Point

  Copyright(c) 2014 Intel Corporation. All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions
  are met:

  * Redistributions of source code must retain the above copyright
  notice, this list of conditions and the following disclaimer.
  * Redistributions in binary form must reproduce the above copyright
  notice, this list of conditions and the following disclaimer in
  the documentation and/or other materials provided with the
  distribution.
  * Neither the name of Intel Corporation nor the names of its
  contributors may be used to endorse or promote products derived
  from this software without specific prior written permission.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

**/

#include "DxeMain.h"

//
// DXE Core Global Variables for Protocols from PEI
//

//
// DXE Core globals for Architecture Protocols
//
EFI_SECURITY_ARCH_PROTOCOL        *gSecurity      = NULL;
EFI_SECURITY2_ARCH_PROTOCOL       *gSecurity2     = NULL;
EFI_CPU_ARCH_PROTOCOL             *gCpu           = NULL;
EFI_METRONOME_ARCH_PROTOCOL       *gMetronome     = NULL;
EFI_TIMER_ARCH_PROTOCOL           *gTimer         = NULL;
EFI_BDS_ARCH_PROTOCOL             *gBds           = NULL;
EFI_WATCHDOG_TIMER_ARCH_PROTOCOL  *gWatchdogTimer = NULL;

//
// DXE Core globals for optional protocol dependencies
//
EFI_SMM_BASE2_PROTOCOL            *gSmmBase2      = NULL;

//
// DXE Core Global used to update core loaded image protocol handle
//
EFI_GUID                           *gDxeCoreFileName;
EFI_LOADED_IMAGE_PROTOCOL          *gDxeCoreLoadedImage;

//
// DXE Core Module Variables
//
EFI_BOOT_SERVICES mBootServices = {
  {
    EFI_BOOT_SERVICES_SIGNATURE,                                                          // Signature
    EFI_BOOT_SERVICES_REVISION,                                                           // Revision
    sizeof (EFI_BOOT_SERVICES),                                                           // HeaderSize
    0,                                                                                    // CRC32
    0                                                                                     // Reserved
  },
  (EFI_RAISE_TPL)                               CoreRaiseTpl,                             // RaiseTPL
  (EFI_RESTORE_TPL)                             CoreRestoreTpl,                           // RestoreTPL
  (EFI_ALLOCATE_PAGES)                          CoreAllocatePages,                        // AllocatePages
  (EFI_FREE_PAGES)                              CoreFreePages,                            // FreePages
  (EFI_GET_MEMORY_MAP)                          CoreGetMemoryMap,                         // GetMemoryMap
  (EFI_ALLOCATE_POOL)                           CoreAllocatePool,                         // AllocatePool
  (EFI_FREE_POOL)                               CoreFreePool,                             // FreePool
  (EFI_CREATE_EVENT)                            CoreCreateEvent,                          // CreateEvent
  (EFI_SET_TIMER)                               CoreSetTimer,                             // SetTimer
  (EFI_WAIT_FOR_EVENT)                          CoreWaitForEvent,                         // WaitForEvent
  (EFI_SIGNAL_EVENT)                            CoreSignalEvent,                          // SignalEvent
  (EFI_CLOSE_EVENT)                             CoreCloseEvent,                           // CloseEvent
  (EFI_CHECK_EVENT)                             CoreCheckEvent,                           // CheckEvent
  (EFI_INSTALL_PROTOCOL_INTERFACE)              CoreInstallProtocolInterface,             // InstallProtocolInterface
  (EFI_REINSTALL_PROTOCOL_INTERFACE)            CoreReinstallProtocolInterface,           // ReinstallProtocolInterface
  (EFI_UNINSTALL_PROTOCOL_INTERFACE)            CoreUninstallProtocolInterface,           // UninstallProtocolInterface
  (EFI_HANDLE_PROTOCOL)                         CoreHandleProtocol,                       // HandleProtocol
  (VOID *)                                      NULL,                                     // Reserved
  (EFI_REGISTER_PROTOCOL_NOTIFY)                CoreRegisterProtocolNotify,               // RegisterProtocolNotify
  (EFI_LOCATE_HANDLE)                           CoreLocateHandle,                         // LocateHandle
  (EFI_LOCATE_DEVICE_PATH)                      CoreLocateDevicePath,                     // LocateDevicePath
  (EFI_INSTALL_CONFIGURATION_TABLE)             CoreInstallConfigurationTable,            // InstallConfigurationTable
  (EFI_IMAGE_LOAD)                              CoreLoadImage,                            // LoadImage
  (EFI_IMAGE_START)                             CoreStartImage,                           // StartImage
  (EFI_EXIT)                                    CoreExit,                                 // Exit
  (EFI_IMAGE_UNLOAD)                            CoreUnloadImage,                          // UnloadImage
  (EFI_EXIT_BOOT_SERVICES)                      CoreExitBootServices,                     // ExitBootServices
  (EFI_GET_NEXT_MONOTONIC_COUNT)                CoreEfiNotAvailableYetArg1,               // GetNextMonotonicCount
  (EFI_STALL)                                   CoreStall,                                // Stall
  (EFI_SET_WATCHDOG_TIMER)                      CoreSetWatchdogTimer,                     // SetWatchdogTimer
  (EFI_CONNECT_CONTROLLER)                      CoreConnectController,                    // ConnectController
  (EFI_DISCONNECT_CONTROLLER)                   CoreDisconnectController,                 // DisconnectController
  (EFI_OPEN_PROTOCOL)                           CoreOpenProtocol,                         // OpenProtocol
  (EFI_CLOSE_PROTOCOL)                          CoreCloseProtocol,                        // CloseProtocol
  (EFI_OPEN_PROTOCOL_INFORMATION)               CoreOpenProtocolInformation,              // OpenProtocolInformation
  (EFI_PROTOCOLS_PER_HANDLE)                    CoreProtocolsPerHandle,                   // ProtocolsPerHandle
  (EFI_LOCATE_HANDLE_BUFFER)                    CoreLocateHandleBuffer,                   // LocateHandleBuffer
  (EFI_LOCATE_PROTOCOL)                         CoreLocateProtocol,                       // LocateProtocol
  (EFI_INSTALL_MULTIPLE_PROTOCOL_INTERFACES)    CoreInstallMultipleProtocolInterfaces,    // InstallMultipleProtocolInterfaces
  (EFI_UNINSTALL_MULTIPLE_PROTOCOL_INTERFACES)  CoreUninstallMultipleProtocolInterfaces,  // UninstallMultipleProtocolInterfaces
  (EFI_CALCULATE_CRC32)                         CoreEfiNotAvailableYetArg3,               // CalculateCrc32
  (EFI_COPY_MEM)                                CopyMem,                                  // CopyMem
  (EFI_SET_MEM)                                 SetMem,                                   // SetMem
  (EFI_CREATE_EVENT_EX)                         CoreCreateEventEx                         // CreateEventEx
};

EFI_DXE_SERVICES mDxeServices = {
  {
    DXE_SERVICES_SIGNATURE,                                           // Signature
    DXE_SERVICES_REVISION,                                            // Revision
    sizeof (DXE_SERVICES),                                            // HeaderSize
    0,                                                                    // CRC32
    0                                                                     // Reserved
  },
  (EFI_ADD_MEMORY_SPACE)             CoreAddMemorySpace,                  // AddMemorySpace
  (EFI_ALLOCATE_MEMORY_SPACE)        CoreAllocateMemorySpace,             // AllocateMemorySpace
  (EFI_FREE_MEMORY_SPACE)            CoreFreeMemorySpace,                 // FreeMemorySpace
  (EFI_REMOVE_MEMORY_SPACE)          CoreRemoveMemorySpace,               // RemoveMemorySpace
  (EFI_GET_MEMORY_SPACE_DESCRIPTOR)  CoreGetMemorySpaceDescriptor,        // GetMemorySpaceDescriptor
  (EFI_SET_MEMORY_SPACE_ATTRIBUTES)  CoreSetMemorySpaceAttributes,        // SetMemorySpaceAttributes
  (EFI_GET_MEMORY_SPACE_MAP)         CoreGetMemorySpaceMap,               // GetMemorySpaceMap
  (EFI_ADD_IO_SPACE)                 CoreAddIoSpace,                      // AddIoSpace
  (EFI_ALLOCATE_IO_SPACE)            CoreAllocateIoSpace,                 // AllocateIoSpace
  (EFI_FREE_IO_SPACE)                CoreFreeIoSpace,                     // FreeIoSpace
  (EFI_REMOVE_IO_SPACE)              CoreRemoveIoSpace,                   // RemoveIoSpace
  (EFI_GET_IO_SPACE_DESCRIPTOR)      CoreGetIoSpaceDescriptor,            // GetIoSpaceDescriptor
  (EFI_GET_IO_SPACE_MAP)             CoreGetIoSpaceMap,                   // GetIoSpaceMap
  (EFI_DISPATCH)                     CoreDispatcher,                      // Dispatch
  (EFI_SCHEDULE)                     CoreSchedule,                        // Schedule
  (EFI_TRUST)                        CoreTrust,                           // Trust
  (EFI_PROCESS_FIRMWARE_VOLUME)      CoreProcessFirmwareVolume,           // ProcessFirmwareVolume
};

EFI_SYSTEM_TABLE mEfiSystemTableTemplate = {
  {
    EFI_SYSTEM_TABLE_SIGNATURE,                                           // Signature
    EFI_SYSTEM_TABLE_REVISION,                                            // Revision
    sizeof (EFI_SYSTEM_TABLE),                                            // HeaderSize
    0,                                                                    // CRC32
    0                                                                     // Reserved
  },
  NULL,                                                                   // FirmwareVendor
  0,                                                                      // FirmwareRevision
  NULL,                                                                   // ConsoleInHandle
  NULL,                                                                   // ConIn
  NULL,                                                                   // ConsoleOutHandle
  NULL,                                                                   // ConOut
  NULL,                                                                   // StandardErrorHandle
  NULL,                                                                   // StdErr
  NULL,                                                                   // RuntimeServices
  &mBootServices,                                                         // BootServices
  0,                                                                      // NumberOfConfigurationTableEntries
  NULL                                                                    // ConfigurationTable
};

EFI_RUNTIME_SERVICES mEfiRuntimeServicesTableTemplate = {
  {
    EFI_RUNTIME_SERVICES_SIGNATURE,                               // Signature
    EFI_RUNTIME_SERVICES_REVISION,                                // Revision
    sizeof (EFI_RUNTIME_SERVICES),                                // HeaderSize
    0,                                                            // CRC32
    0                                                             // Reserved
  },
  (EFI_GET_TIME)                    CoreEfiNotAvailableYetArg2,   // GetTime
  (EFI_SET_TIME)                    CoreEfiNotAvailableYetArg1,   // SetTime
  (EFI_GET_WAKEUP_TIME)             CoreEfiNotAvailableYetArg3,   // GetWakeupTime
  (EFI_SET_WAKEUP_TIME)             CoreEfiNotAvailableYetArg2,   // SetWakeupTime
  (EFI_SET_VIRTUAL_ADDRESS_MAP)     CoreEfiNotAvailableYetArg4,   // SetVirtualAddressMap
  (EFI_CONVERT_POINTER)             CoreEfiNotAvailableYetArg2,   // ConvertPointer
  (EFI_GET_VARIABLE)                CoreEfiNotAvailableYetArg5,   // GetVariable
  (EFI_GET_NEXT_VARIABLE_NAME)      CoreEfiNotAvailableYetArg3,   // GetNextVariableName
  (EFI_SET_VARIABLE)                CoreEfiNotAvailableYetArg5,   // SetVariable
  (EFI_GET_NEXT_HIGH_MONO_COUNT)    CoreEfiNotAvailableYetArg1,   // GetNextHighMonotonicCount
  (EFI_RESET_SYSTEM)                CoreEfiNotAvailableYetArg4,   // ResetSystem
  (EFI_UPDATE_CAPSULE)              CoreEfiNotAvailableYetArg3,   // UpdateCapsule
  (EFI_QUERY_CAPSULE_CAPABILITIES)  CoreEfiNotAvailableYetArg4,   // QueryCapsuleCapabilities
  (EFI_QUERY_VARIABLE_INFO)         CoreEfiNotAvailableYetArg4    // QueryVariableInfo
};

EFI_RUNTIME_ARCH_PROTOCOL gRuntimeTemplate = {
  INITIALIZE_LIST_HEAD_VARIABLE (gRuntimeTemplate.ImageHead),
  INITIALIZE_LIST_HEAD_VARIABLE (gRuntimeTemplate.EventHead),

  //
  // Make sure Size != sizeof (EFI_MEMORY_DESCRIPTOR). This will
  // prevent people from having pointer math bugs in their code.
  // now you have to use *DescriptorSize to make things work.
  //
  sizeof (EFI_MEMORY_DESCRIPTOR) + sizeof (UINT64) - (sizeof (EFI_MEMORY_DESCRIPTOR) % sizeof (UINT64)),
  EFI_MEMORY_DESCRIPTOR_VERSION,
  0,
  NULL,
  NULL,
  FALSE,
  FALSE
};

EFI_RUNTIME_ARCH_PROTOCOL *gRuntime = &gRuntimeTemplate;

//
// DXE Core Global Variables for the EFI System Table, Boot Services Table,
// DXE Services Table, and Runtime Services Table
//
EFI_DXE_SERVICES      *gDxeCoreDS = &mDxeServices;
EFI_SYSTEM_TABLE      *gDxeCoreST = NULL;

//
// For debug initialize gDxeCoreRT to template. gDxeCoreRT must be allocated from RT memory
//  but gDxeCoreRT is used for ASSERT () and DEBUG () type macros so lets give it
//  a value that will not cause debug infrastructure to crash early on.
//
EFI_RUNTIME_SERVICES  *gDxeCoreRT = &mEfiRuntimeServicesTableTemplate;
EFI_HANDLE            gDxeCoreImageHandle = NULL;

//
// For Loading modules at fixed address feature, the configuration table is to cache the top address below which to load
// Runtime code&boot time code
//
GLOBAL_REMOVE_IF_UNREFERENCED EFI_LOAD_FIXED_ADDRESS_CONFIGURATION_TABLE    gLoadModuleAtFixAddressConfigurationTable = {0, 0};

// Main entry point to the DXE Core
//

/**
  Main entry point to DXE Core.

  @param  HobStart               Pointer to the beginning of the HOB List from PEI.

  @return This function should never return.

**/
VOID
EFIAPI
DxeMain (
  IN  VOID *HobStart
  )
{
  EFI_STATUS                    Status;
  EFI_PHYSICAL_ADDRESS          MemoryBaseAddress;
  UINT64                        MemoryLength;
  PE_COFF_LOADER_IMAGE_CONTEXT  ImageContext;
  UINTN                         Index;
  EFI_HOB_GUID_TYPE             *GuidHob;
  EFI_VECTOR_HANDOFF_INFO       *VectorInfoList;
  EFI_VECTOR_HANDOFF_INFO       *VectorInfo;

  //
  // Setup the default exception handlers
  //
  VectorInfoList = NULL;
  GuidHob = GetNextGuidHob (&gEfiVectorHandoffInfoPpiGuid, HobStart);
  if (GuidHob != NULL) {
    VectorInfoList = (EFI_VECTOR_HANDOFF_INFO *) (GET_GUID_HOB_DATA(GuidHob));
  }
  Status = InitializeCpuExceptionHandlers (VectorInfoList);
  ASSERT_EFI_ERROR (Status);
  
  //
  // Initialize Debug Agent to support source level debug in DXE phase
  //
  InitializeDebugAgent (DEBUG_AGENT_INIT_DXE_CORE, HobStart, NULL);

  //
  // Initialize Memory Services
  //
  CoreInitializeMemoryServices (&HobStart, &MemoryBaseAddress, &MemoryLength);

  //
  // Allocate the EFI System Table and EFI Runtime Service Table from EfiRuntimeServicesData
  // Use the templates to initialize the contents of the EFI System Table and EFI Runtime Services Table
  //
  gDxeCoreST = AllocateRuntimeCopyPool (sizeof (EFI_SYSTEM_TABLE), &mEfiSystemTableTemplate);
  ASSERT (gDxeCoreST != NULL);

  gDxeCoreRT = AllocateRuntimeCopyPool (sizeof (EFI_RUNTIME_SERVICES), &mEfiRuntimeServicesTableTemplate);
  ASSERT (gDxeCoreRT != NULL);

  gDxeCoreST->RuntimeServices = gDxeCoreRT;

  //
  // Start the Image Services.
  //
  Status = CoreInitializeImageServices (HobStart);
  ASSERT_EFI_ERROR (Status);

  //
  // Call constructor for all libraries
  //
  ProcessLibraryConstructorList (gDxeCoreImageHandle, gDxeCoreST);
  PERF_END   (NULL,"PEI", NULL, 0) ;
  PERF_START (NULL,"DXE", NULL, 0) ;

  //
  // Report DXE Core image information to the PE/COFF Extra Action Library
  //
  ImageContext.ImageAddress = (EFI_PHYSICAL_ADDRESS)(UINTN)gDxeCoreLoadedImage->ImageBase;
  ImageContext.PdbPointer = PeCoffLoaderGetPdbPointer ((VOID*) (UINTN) ImageContext.ImageAddress);
  PeCoffLoaderRelocateImageExtraAction (&ImageContext);

  //
  // Initialize the Global Coherency Domain Services
  //
  Status = CoreInitializeGcdServices (&HobStart, MemoryBaseAddress, MemoryLength);
  ASSERT_EFI_ERROR (Status);

  //
  // Install the DXE Services Table into the EFI System Tables's Configuration Table
  //
  Status = CoreInstallConfigurationTable (&gEfiDxeServicesTableGuid, gDxeCoreDS);
  ASSERT_EFI_ERROR (Status);

  //
  // Install the HOB List into the EFI System Tables's Configuration Table
  //
  Status = CoreInstallConfigurationTable (&gEfiHobListGuid, HobStart);
  ASSERT_EFI_ERROR (Status);

  //
  // Install Memory Type Information Table into the EFI System Tables's Configuration Table
  //
  Status = CoreInstallConfigurationTable (&gEfiMemoryTypeInformationGuid, &gMemoryTypeInformation);
  ASSERT_EFI_ERROR (Status);

  //
  // If Loading modules At fixed address feature is enabled, install Load moduels at fixed address
  // Configuration Table so that user could easily to retrieve the top address to load Dxe and PEI
  // Code and Tseg base to load SMM driver.
  //
  if (PcdGet64(PcdLoadModuleAtFixAddressEnable) != 0) {
    Status = CoreInstallConfigurationTable (&gLoadFixedAddressConfigurationTableGuid, &gLoadModuleAtFixAddressConfigurationTable);
    ASSERT_EFI_ERROR (Status);
  }
  //
  // Report Status Code here for DXE_ENTRY_POINT once it is available
  //
  REPORT_STATUS_CODE (
    EFI_PROGRESS_CODE,
    (EFI_SOFTWARE_DXE_CORE | EFI_SW_DXE_CORE_PC_ENTRY_POINT)
    );

  DEBUG ((DEBUG_INFO | DEBUG_LOAD, "HOBLIST address in DXE = 0x%p\n", HobStart));

  DEBUG_CODE_BEGIN ();
    EFI_PEI_HOB_POINTERS               Hob;

    for (Hob.Raw = HobStart; !END_OF_HOB_LIST(Hob); Hob.Raw = GET_NEXT_HOB(Hob)) {
      if (GET_HOB_TYPE (Hob) == EFI_HOB_TYPE_MEMORY_ALLOCATION) {
        DEBUG ((DEBUG_INFO | DEBUG_LOAD, "Memory Allocation 0x%08x 0x%0lx - 0x%0lx\n", \
          Hob.MemoryAllocation->AllocDescriptor.MemoryType,                      \
          Hob.MemoryAllocation->AllocDescriptor.MemoryBaseAddress,               \
          Hob.MemoryAllocation->AllocDescriptor.MemoryBaseAddress + Hob.MemoryAllocation->AllocDescriptor.MemoryLength - 1));
      }
    }
    for (Hob.Raw = HobStart; !END_OF_HOB_LIST(Hob); Hob.Raw = GET_NEXT_HOB(Hob)) {
      if (GET_HOB_TYPE (Hob) == EFI_HOB_TYPE_FV2) {
        DEBUG ((DEBUG_INFO | DEBUG_LOAD, "FV2 Hob           0x%0lx - 0x%0lx\n", Hob.FirmwareVolume2->BaseAddress, Hob.FirmwareVolume2->BaseAddress + Hob.FirmwareVolume2->Length - 1));
      } else if (GET_HOB_TYPE (Hob) == EFI_HOB_TYPE_FV) {
        DEBUG ((DEBUG_INFO | DEBUG_LOAD, "FV Hob            0x%0lx - 0x%0lx\n", Hob.FirmwareVolume->BaseAddress, Hob.FirmwareVolume->BaseAddress + Hob.FirmwareVolume2->Length - 1));
      }
    }
  DEBUG_CODE_END ();

  //
  // Initialize the Event Services
  //
  Status = CoreInitializeEventServices ();
  ASSERT_EFI_ERROR (Status);

  //
  // Get persisted vector hand-off info from GUIDeed HOB again due to HobStart may be updated,
  // and install configuration table
  //
  GuidHob = GetNextGuidHob (&gEfiVectorHandoffInfoPpiGuid, HobStart);
  if (GuidHob != NULL) {
    VectorInfoList = (EFI_VECTOR_HANDOFF_INFO *) (GET_GUID_HOB_DATA(GuidHob));
    VectorInfo = VectorInfoList;
    Index = 1;
    while (VectorInfo->Attribute != EFI_VECTOR_HANDOFF_LAST_ENTRY) {
      VectorInfo ++;
      Index ++;
    }
    VectorInfo = AllocateCopyPool (sizeof (EFI_VECTOR_HANDOFF_INFO) * Index, (VOID *) VectorInfoList);
    ASSERT (VectorInfo != NULL);
    Status = CoreInstallConfigurationTable (&gEfiVectorHandoffTableGuid, (VOID *) VectorInfo);
    ASSERT_EFI_ERROR (Status);
  }

  //
  // Get the Protocols that were passed in from PEI to DXE through GUIDed HOBs
  //
  // These Protocols are not architectural. This implementation is sharing code between
  // PEI and DXE in order to save FLASH space. These Protocols could also be implemented
  // as part of the DXE Core. However, that would also require the DXE Core to be ported
  // each time a different CPU is used, a different Decompression algorithm is used, or a
  // different Image type is used. By placing these Protocols in PEI, the DXE Core remains
  // generic, and only PEI and the Arch Protocols need to be ported from Platform to Platform,
  // and from CPU to CPU.
  //

  //
  // Register for the GUIDs of the Architectural Protocols, so the rest of the
  // EFI Boot Services and EFI Runtime Services tables can be filled in.
  // Also register for the GUIDs of optional protocols.
  //
  CoreNotifyOnProtocolInstallation ();

  //
  // Produce Firmware Volume Protocols, one for each FV in the HOB list.
  //
  Status = FwVolDriverInit (gDxeCoreImageHandle, gDxeCoreST);
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize the DXE Dispatcher
  //
  PERF_START (NULL,"CoreInitializeDispatcher", "DxeMain", 0) ;
  CoreInitializeDispatcher ();
  PERF_END (NULL,"CoreInitializeDispatcher", "DxeMain", 0) ;

  //
  // Invoke the DXE Dispatcher
  //
  PERF_START (NULL, "CoreDispatcher", "DxeMain", 0);
  CoreDispatcher ();
  PERF_END (NULL, "CoreDispatcher", "DxeMain", 0);

  //
  // Display Architectural protocols that were not loaded if this is DEBUG build
  //
  DEBUG_CODE_BEGIN ();
    CoreDisplayMissingArchProtocols ();
  DEBUG_CODE_END ();

  //
  // Display any drivers that were not dispatched because dependency expression
  // evaluated to false if this is a debug build
  //
  DEBUG_CODE_BEGIN ();
    CoreDisplayDiscoveredNotDispatched ();
  DEBUG_CODE_END ();

  //
  // Assert if the Architectural Protocols are not present.
  //
  Status = CoreAllEfiServicesAvailable ();
  if (EFI_ERROR(Status)) {
    //
    // Report Status code that some Architectural Protocols are not present.
    //
    REPORT_STATUS_CODE (
      EFI_ERROR_CODE | EFI_ERROR_MAJOR,
      (EFI_SOFTWARE_DXE_CORE | EFI_SW_DXE_CORE_EC_NO_ARCH)
      );    
  }
  ASSERT_EFI_ERROR (Status);

  //
  // Report Status code before transfer control to BDS
  //
  REPORT_STATUS_CODE (
    EFI_PROGRESS_CODE,
    (EFI_SOFTWARE_DXE_CORE | EFI_SW_DXE_CORE_PC_HANDOFF_TO_NEXT)
    );

  //
  // Transfer control to the BDS Architectural Protocol
  //
  gBds->Entry (gBds);

  //
  // BDS should never return
  //
  ASSERT (FALSE);
  CpuDeadLoop ();
}



/**
  Place holder function until all the Boot Services and Runtime Services are
  available.

  @return EFI_NOT_AVAILABLE_YET

**/
EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg0 (
  VOID
  )
{
  //
  // This function should never be executed.  If it does, then the architectural protocols
  // have not been designed correctly.  The CpuBreakpoint () is commented out for now until the
  // DXE Core and all the Architectural Protocols are complete.
  //

  return EFI_NOT_AVAILABLE_YET;
}


/**
  Place holder function until all the Boot Services and Runtime Services are
  available.

  @param  Arg1                   Undefined

  @return EFI_NOT_AVAILABLE_YET

**/
EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg1 (
  UINTN Arg1
  )
{
  //
  // This function should never be executed.  If it does, then the architectural protocols
  // have not been designed correctly.  The CpuBreakpoint () is commented out for now until the
  // DXE Core and all the Architectural Protocols are complete.
  //

  return EFI_NOT_AVAILABLE_YET;
}


/**
  Place holder function until all the Boot Services and Runtime Services are available.

  @param  Arg1                   Undefined
  @param  Arg2                   Undefined

  @return EFI_NOT_AVAILABLE_YET

**/
EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg2 (
  UINTN Arg1,
  UINTN Arg2
  )
{
  //
  // This function should never be executed.  If it does, then the architectural protocols
  // have not been designed correctly.  The CpuBreakpoint () is commented out for now until the
  // DXE Core and all the Architectural Protocols are complete.
  //

  return EFI_NOT_AVAILABLE_YET;
}


/**
  Place holder function until all the Boot Services and Runtime Services are available.

  @param  Arg1                   Undefined
  @param  Arg2                   Undefined
  @param  Arg3                   Undefined

  @return EFI_NOT_AVAILABLE_YET

**/
EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg3 (
  UINTN Arg1,
  UINTN Arg2,
  UINTN Arg3
  )
{
  //
  // This function should never be executed.  If it does, then the architectural protocols
  // have not been designed correctly.  The CpuBreakpoint () is commented out for now until the
  // DXE Core and all the Architectural Protocols are complete.
  //

  return EFI_NOT_AVAILABLE_YET;
}


/**
  Place holder function until all the Boot Services and Runtime Services are available.

  @param  Arg1                   Undefined
  @param  Arg2                   Undefined
  @param  Arg3                   Undefined
  @param  Arg4                   Undefined

  @return EFI_NOT_AVAILABLE_YET

**/
EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg4 (
  UINTN Arg1,
  UINTN Arg2,
  UINTN Arg3,
  UINTN Arg4
  )
{
  //
  // This function should never be executed.  If it does, then the architectural protocols
  // have not been designed correctly.  The CpuBreakpoint () is commented out for now until the
  // DXE Core and all the Architectural Protocols are complete.
  //

  return EFI_NOT_AVAILABLE_YET;
}


/**
  Place holder function until all the Boot Services and Runtime Services are available.

  @param  Arg1                   Undefined
  @param  Arg2                   Undefined
  @param  Arg3                   Undefined
  @param  Arg4                   Undefined
  @param  Arg5                   Undefined

  @return EFI_NOT_AVAILABLE_YET

**/
EFI_STATUS
EFIAPI
CoreEfiNotAvailableYetArg5 (
  UINTN Arg1,
  UINTN Arg2,
  UINTN Arg3,
  UINTN Arg4,
  UINTN Arg5
  )
{
  //
  // This function should never be executed.  If it does, then the architectural protocols
  // have not been designed correctly.  The CpuBreakpoint () is commented out for now until the
  // DXE Core and all the Architectural Protocols are complete.
  //

  return EFI_NOT_AVAILABLE_YET;
}


/**
  Calcualte the 32-bit CRC in a EFI table using the service provided by the
  gRuntime service.

  @param  Hdr                    Pointer to an EFI standard header

**/
VOID
CalculateEfiHdrCrc (
  IN  OUT EFI_TABLE_HEADER    *Hdr
  )
{
  UINT32 Crc;

  Hdr->CRC32 = 0;

  //
  // If gBS->CalculateCrce32 () == CoreEfiNotAvailableYet () then
  //  Crc will come back as zero if we set it to zero here
  //
  Crc = 0;
  gBS->CalculateCrc32 ((UINT8 *)Hdr, Hdr->HeaderSize, &Crc);
  Hdr->CRC32 = Crc;
}


/**
  Terminates all boot services.

  @param  ImageHandle            Handle that identifies the exiting image.
  @param  MapKey                 Key to the latest memory map.

  @retval EFI_SUCCESS            Boot Services terminated
  @retval EFI_INVALID_PARAMETER  MapKey is incorrect.

**/
EFI_STATUS
EFIAPI
CoreExitBootServices (
  IN EFI_HANDLE   ImageHandle,
  IN UINTN        MapKey
  )
{
  EFI_STATUS                Status;

  //
  // Disable Timer
  //
  gTimer->SetTimerPeriod (gTimer, 0);

  //
  // Terminate memory services if the MapKey matches
  //
  Status = CoreTerminateMemoryMap (MapKey);
  if (EFI_ERROR (Status)) {
    //
    // Notify other drivers that ExitBootServices fail 
    //
    CoreNotifySignalList (&gEventExitBootServicesFailedGuid);
    return Status;
  }

  //
  // Notify other drivers that we are exiting boot services.
  //
  CoreNotifySignalList (&gEfiEventExitBootServicesGuid);

  //
  // Report that ExitBootServices() has been called
  //
  REPORT_STATUS_CODE (
    EFI_PROGRESS_CODE,
    (EFI_SOFTWARE_EFI_BOOT_SERVICE | EFI_SW_BS_PC_EXIT_BOOT_SERVICES)
    );

  //
  // Disable interrupt of Debug timer.
  //
  SaveAndSetDebugTimerInterrupt (FALSE);

  //
  // Disable CPU Interrupts
  //
  gCpu->DisableInterrupt (gCpu);

  //
  // Clear the non-runtime values of the EFI System Table
  //
  gDxeCoreST->BootServices        = NULL;
  gDxeCoreST->ConIn               = NULL;
  gDxeCoreST->ConsoleInHandle     = NULL;
  gDxeCoreST->ConOut              = NULL;
  gDxeCoreST->ConsoleOutHandle    = NULL;
  gDxeCoreST->StdErr              = NULL;
  gDxeCoreST->StandardErrorHandle = NULL;

  //
  // Recompute the 32-bit CRC of the EFI System Table
  //
  CalculateEfiHdrCrc (&gDxeCoreST->Hdr);

  //
  // Zero out the Boot Service Table
  //
  ZeroMem (gBS, sizeof (EFI_BOOT_SERVICES));
  gBS = NULL;

  //
  // Update the AtRuntime field in Runtiem AP.
  //
  gRuntime->AtRuntime = TRUE;

  return Status;
}

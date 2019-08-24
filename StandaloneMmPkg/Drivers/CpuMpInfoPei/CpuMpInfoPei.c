/** @file
Implementation of MP CPU driver for PEI phase.

This PEIM is to expose the MpService Ppi

  Copyright (c) 2012 - 2019, Intel Corporation. All rights reserved.<BR>

  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiPei.h>

#include <Ppi/MpServices.h>
#include <Ppi/MasterBootMode.h>
#include <Ppi/SecPlatformInformation.h>

#include <Guid/MpInformation.h>

#include <Library/DebugLib.h>
#include <Library/PeimEntryPoint.h>
#include <Library/BaseLib.h>
#include <Library/PeiServicesLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/HobLib.h>
#include <Library/PcdLib.h>
#include <Library/LocalApicLib.h>
#include <Library/SynchronizationLib.h>

#pragma pack(1)
typedef struct {
  UINT64                     NumberOfProcessors;
  UINT64                     NumberOfEnabledProcessors;
  EFI_PROCESSOR_INFORMATION  ProcessorInfoBuffer[FixedPcdGet32(PcdCpuMaxLogicalProcessorNumber)];
} MY_MP_INFORMATION_HOB_DATA;
#pragma pack()

/**
  This function will get CPU count from system,
  and build Hob to save the data.

  @param MaxCpuCount  The MaxCpuCount could be supported by system
**/
VOID
CountCpuNumber (
  IN CONST EFI_PEI_SERVICES     **PeiServices,
  IN UINTN                      MaxCpuCount
  )
{
  UINTN                      Index;
  EFI_STATUS                 Status;
  MY_MP_INFORMATION_HOB_DATA MpInformationData;
  EFI_PEI_MP_SERVICES_PPI    *MpPpi;
  UINTN                      NumberOfProcessors;
  UINTN                      NumberOfEnabledProcessors;
  EFI_PROCESSOR_INFORMATION  ProcessorInfoBuffer;

  Status = PeiServicesLocatePpi (&gEfiPeiMpServicesPpiGuid, 0, NULL, (VOID **)&MpPpi);
  ASSERT_EFI_ERROR(Status);

  Status = MpPpi->GetNumberOfProcessors(
                    PeiServices,
                    MpPpi,
                    &NumberOfProcessors,
                    &NumberOfEnabledProcessors
                    );
  ASSERT_EFI_ERROR(Status);
  DEBUG((EFI_D_ERROR, "PeiGetNumberOfProcessors - NumberOfProcessors - %x\n", NumberOfProcessors));
  DEBUG((EFI_D_ERROR, "PeiGetNumberOfProcessors - NumberOfEnabledProcessors - %x\n", NumberOfEnabledProcessors));

  //
  // Record MP information
  //
  MpInformationData.NumberOfProcessors        = NumberOfProcessors;
  MpInformationData.NumberOfEnabledProcessors = NumberOfEnabledProcessors;
  for (Index = 0; Index < NumberOfProcessors; Index++) {
    Status = MpPpi->GetProcessorInfo(
                      PeiServices,
                      MpPpi,
                      Index,
                      &ProcessorInfoBuffer
                      );
    ASSERT_EFI_ERROR(Status);
    DEBUG((EFI_D_ERROR, "PeiGetProcessorInfo - Index - %x\n", Index));
    DEBUG((EFI_D_ERROR, "PeiGetProcessorInfo - ProcessorId      - %016lx\n", ProcessorInfoBuffer.ProcessorId));
    DEBUG((EFI_D_ERROR, "PeiGetProcessorInfo - StatusFlag       - %08x\n", ProcessorInfoBuffer.StatusFlag));
    DEBUG((EFI_D_ERROR, "PeiGetProcessorInfo - Location.Package - %08x\n", ProcessorInfoBuffer.Location.Package));
    DEBUG((EFI_D_ERROR, "PeiGetProcessorInfo - Location.Core    - %08x\n", ProcessorInfoBuffer.Location.Core));
    DEBUG((EFI_D_ERROR, "PeiGetProcessorInfo - Location.Thread  - %08x\n", ProcessorInfoBuffer.Location.Thread));
    CopyMem (&MpInformationData.ProcessorInfoBuffer[Index], &ProcessorInfoBuffer, sizeof(ProcessorInfoBuffer));
  }

  BuildGuidDataHob (
    &gMpInformationHobGuid,
    (VOID *)&MpInformationData,
    sizeof(MpInformationData)
    );
}

VOID
EFIAPI
UnitTestFuncNull (
  IN OUT VOID  *Buffer
  )
{
}

VOID
EFIAPI
UnitTestFuncGetApicId (
  IN OUT VOID  *Buffer
  )
{
  UINT32   *ApicId;

  ApicId = (UINT32 *)Buffer;
  *ApicId = GetApicId ();
}

typedef struct {
  UINTN            SpinLock;
  UINT32           ApCount;
  UINT32           ApicId[FixedPcdGet32(PcdCpuMaxLogicalProcessorNumber)];
} APIC_ID_LIST;

VOID
EFIAPI
UnitTestFuncGetApicIdList (
  IN OUT VOID  *Buffer
  )
{
  APIC_ID_LIST   *ApicIdList;

  ApicIdList = (APIC_ID_LIST *)Buffer;
  
  AcquireSpinLock (&ApicIdList->SpinLock);

  ApicIdList->ApicId[ApicIdList->ApCount] = GetApicId ();
  InterlockedIncrement (&ApicIdList->ApCount);

  ReleaseSpinLock (&ApicIdList->SpinLock);
}

VOID
UnitTest (
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  EFI_STATUS                Status;
  UINTN                     NumberOfProcessors;
  UINTN                     NumberOfEnabledProcessors;
  UINTN                     Index;
  EFI_PROCESSOR_INFORMATION ProcessorInfoBuffer;
  UINTN                     BspIndex;
  UINT32                    ApicId;
  APIC_ID_LIST              ApicIdList;
  EFI_PEI_MP_SERVICES_PPI   *MpPpi;

  Status = PeiServicesLocatePpi (&gEfiPeiMpServicesPpiGuid, 0, NULL, (VOID **)&MpPpi);
  ASSERT_EFI_ERROR(Status);

  Status = MpPpi->GetNumberOfProcessors (
                    PeiServices,
                    MpPpi,
                    &NumberOfProcessors,
                    &NumberOfEnabledProcessors
                    );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_ERROR, "PeiGetNumberOfProcessors - NumberOfProcessors - %x\n", NumberOfProcessors));
  DEBUG ((EFI_D_ERROR, "PeiGetNumberOfProcessors - NumberOfEnabledProcessors - %x\n", NumberOfEnabledProcessors));

  for (Index = 0; Index < NumberOfProcessors; Index++) {
    Status = MpPpi->GetProcessorInfo (
                      PeiServices,
                      MpPpi,
                      Index,
                      &ProcessorInfoBuffer
                      );
    ASSERT_EFI_ERROR (Status);
    DEBUG ((EFI_D_ERROR, "PeiGetProcessorInfo - Index - %x\n", Index));
    DEBUG ((EFI_D_ERROR, "PeiGetProcessorInfo - ProcessorId      - %016lx\n", ProcessorInfoBuffer.ProcessorId));
    DEBUG ((EFI_D_ERROR, "PeiGetProcessorInfo - StatusFlag       - %08x\n", ProcessorInfoBuffer.StatusFlag));
    DEBUG ((EFI_D_ERROR, "PeiGetProcessorInfo - Location.Package - %08x\n", ProcessorInfoBuffer.Location.Package));
    DEBUG ((EFI_D_ERROR, "PeiGetProcessorInfo - Location.Core    - %08x\n", ProcessorInfoBuffer.Location.Core));
    DEBUG ((EFI_D_ERROR, "PeiGetProcessorInfo - Location.Thread  - %08x\n", ProcessorInfoBuffer.Location.Thread));
  }

  ZeroMem (&ApicIdList, sizeof(ApicIdList));
  InitializeSpinLock (&ApicIdList.SpinLock);
  Status = MpPpi->StartupAllAPs (
                    PeiServices,
                    MpPpi,
                    UnitTestFuncGetApicIdList,
                    FALSE,
                    0,
                    &ApicIdList
                    );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_ERROR, "PeiStartupAllAPs - SingleThread - FALSE, ApCount - %08x\n", ApicIdList.ApCount));
  for (Index = 0; Index < ApicIdList.ApCount; Index++) {
    DEBUG ((EFI_D_ERROR, "PeiStartupAllAPs - ApicId - %08x\n", ApicIdList.ApicId[Index]));
  }

  ZeroMem (&ApicIdList, sizeof(ApicIdList));
  InitializeSpinLock (&ApicIdList.SpinLock);
  Status = MpPpi->StartupAllAPs (
                    PeiServices,
                    MpPpi,
                    UnitTestFuncGetApicIdList,
                    TRUE,
                    0,
                    &ApicIdList
                    );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_ERROR, "PeiStartupAllAPs - SingleThread - TRUE, ApCount - %08x\n", ApicIdList.ApCount));
  for (Index = 0; Index < ApicIdList.ApCount; Index++) {
    DEBUG ((EFI_D_ERROR, "PeiStartupAllAPs - ApicId - %08x\n", ApicIdList.ApicId[Index]));
  }

  Status = MpPpi->WhoAmI (
                    PeiServices,
                    MpPpi,
                    &BspIndex
                    );
  ASSERT_EFI_ERROR (Status);
  DEBUG ((EFI_D_ERROR, "PeiWhoAmI - BspIndex - %x\n", BspIndex));

  for (Index = 0; Index < NumberOfProcessors; Index++) {
    if (Index == BspIndex) {
      continue;
    }
    Status = MpPpi->StartupThisAP (
                      PeiServices,
                      MpPpi,
                      UnitTestFuncGetApicId,
                      Index,
                      0,
                      &ApicId
                      );
    ASSERT_EFI_ERROR (Status);
    DEBUG ((EFI_D_ERROR, "PeiStartupThisAP - Index - %x\n", Index));
    DEBUG ((EFI_D_ERROR, "PeiStartupThisAP - ApicId - %08x\n", ApicId));
  }
}

/**
  The Entry point of the MP CPU PEIM

  This function is the Entry point of the MP CPU PEIM which will install the MpServicePpi
 
  @param  FileHandle  Handle of the file being invoked.
  @param  PeiServices Describes the list of possible PEI Services. 
                          
  @retval EFI_SUCCESS   MpServicePpi is installed successfully.

**/
EFI_STATUS
EFIAPI
CpuMpInfoPeimInit (
  IN       EFI_PEI_FILE_HANDLE  FileHandle,
  IN CONST EFI_PEI_SERVICES     **PeiServices
  )
{
  //
  // Collect info
  //
  CountCpuNumber (PeiServices, FixedPcdGet32(PcdCpuMaxLogicalProcessorNumber));

  //
  // Need call ProgramVirtualWireMode to enable APIC
  // This must be done to enable SMM.
  //
  DEBUG ((EFI_D_INFO, "ApicMode - 0x%x\n", GetApicMode ()));
  ProgramVirtualWireMode ();
  DEBUG ((EFI_D_INFO, "ProgramVirtualWireMode\n"));

  //
  // Unit Test
  //
  //UnitTest (PeiServices);
  return EFI_SUCCESS;
}

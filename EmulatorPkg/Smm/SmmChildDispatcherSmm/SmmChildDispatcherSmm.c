/** @file

  Copyright (c) 2015 - 2016, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/
#include "SmmChildDispatcherSmm.h"

#include <Library/SmiHandlerProfileLib.h>

PRIVATE_DATA          mPrivateData = {
  { NULL },
  NULL,
  NULL,
  {
    SmmSwDispatch2Register,
    SmmSwDispatch2UnRegister,
    MAXIMUM_SWI_VALUE
  }
};

EMU_SMM_INFO_PROTOCOL  *mSmmInfo;

EFI_STATUS
EFIAPI
InitializeSmmDispatcher (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
/*++

Routine Description:

  Initializes the PCH SMM Dispatcher

Arguments:

  ImageHandle   - Pointer to the loaded image protocol for this driver
  SystemTable   - Pointer to the EFI System Table

Returns:
  Status        - EFI_SUCCESS

--*/
{
  EFI_STATUS  Status;

  //
  // Register a callback function to handle subsequent SMIs.  This callback
  // will be called by SmmCoreDispatcher.
  //
  Status = gSmst->SmiHandlerRegister (SmmCoreDispatcher, NULL, &mPrivateData.SmiHandle);
  ASSERT_EFI_ERROR (Status);

  Status = gSmst->SmmInstallProtocolInterface (
                  &mPrivateData.SmmHandle,
                  &gEfiSmmSwDispatch2ProtocolGuid,
                  EFI_NATIVE_INTERFACE,
                  &mPrivateData.SmmSwDispatch2Protocols
                  );
  ASSERT_EFI_ERROR (Status);

  //
  // Initialize Callback DataBase
  //
  InitializeListHead (&mPrivateData.CallbackDataBase);

  Status = gSmst->SmmLocateProtocol (
                    &gEmuSmmInfoProtocolGuid,
                    NULL,
                    &mSmmInfo
                    );
  ASSERT_EFI_ERROR (Status);

  return EFI_SUCCESS;
}

EFI_STATUS
SmiInputValueDuplicateCheck (
  UINTN           FedSwSmiInputValue
  )
{
  DATABASE_RECORD *RecordInDb;
  LIST_ENTRY      *LinkInDb;

  LinkInDb = GetFirstNode (&mPrivateData.CallbackDataBase);
  while (!IsNull (&mPrivateData.CallbackDataBase, LinkInDb)) {
    RecordInDb = DATABASE_RECORD_FROM_LINK (LinkInDb);

    if (RecordInDb->RegisterContext.SwSmiInputValue == FedSwSmiInputValue) {
      return EFI_INVALID_PARAMETER;
    }

    LinkInDb = GetNextNode (&mPrivateData.CallbackDataBase, &RecordInDb->Link);
  }

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmmCoreDispatcher (
  IN     EFI_HANDLE               DispatchHandle,
  IN     CONST VOID               *RegisterContext,
  IN OUT VOID                     *CommBuffer,
  IN OUT UINTN                    *CommBufferSize
  )
{
  DATABASE_RECORD     *RecordInDb;
  LIST_ENTRY          *LinkInDb;
  EFI_SMM_SW_CONTEXT  SmmSwContext;
  UINTN               BufferSize;
  EFI_STATUS          Status;

  BufferSize = sizeof(SmmSwContext);
  SmmSwContext.SwSmiCpuIndex = 0;
  Status = mSmmInfo->GetInfo (mSmmInfo, &SmmSwContext.CommandPort, &SmmSwContext.DataPort);
  ASSERT_EFI_ERROR (Status);

  if (SmmSwContext.CommandPort == MAXIMUM_SWI_VALUE) {
    return EFI_WARN_INTERRUPT_SOURCE_PENDING;
  }

  //
  // If there are no registered child handlers, then return immediately
  //
  if (IsListEmpty (&mPrivateData.CallbackDataBase)) {
    return EFI_WARN_INTERRUPT_SOURCE_PENDING;
  }

  //
  // Mark all child handlers as not processed 
  //
  LinkInDb = GetFirstNode (&mPrivateData.CallbackDataBase);
  while (!IsNull (&mPrivateData.CallbackDataBase, LinkInDb)) {
    RecordInDb = DATABASE_RECORD_FROM_LINK (LinkInDb);
    if (RecordInDb->RegisterContext.SwSmiInputValue == SmmSwContext.CommandPort) {
      RecordInDb->DispatchFunction (
                          RecordInDb->DispatchHandle,
                          &RecordInDb->RegisterContext,
                          &SmmSwContext,
                          &BufferSize
                          );
      return EFI_SUCCESS;
    }
    LinkInDb = GetNextNode (&mPrivateData.CallbackDataBase, LinkInDb);
  }

  return EFI_WARN_INTERRUPT_SOURCE_PENDING;
}

EFI_STATUS
EFIAPI
SmmSwDispatch2Register (
  IN CONST EFI_SMM_SW_DISPATCH2_PROTOCOL  *This,
  IN       EFI_SMM_HANDLER_ENTRY_POINT2   DispatchFunction,
  IN OUT   EFI_SMM_SW_REGISTER_CONTEXT    *RegisterContext,
     OUT   EFI_HANDLE                     *DispatchHandle
  )
{
  EFI_STATUS                  Status;
  DATABASE_RECORD             *Record;
  INTN                        Index;

  if (RegisterContext == NULL || DispatchHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (RegisterContext->SwSmiInputValue == (UINTN)-1) {
    //
    // If SwSmiInputValue is set to (UINTN) -1 then a unique value will be assigned and returned in the structure.
    //
    Status = EFI_NOT_FOUND;
    for (Index = 1; Index < MAXIMUM_SWI_VALUE; Index++) {
      Status = SmiInputValueDuplicateCheck (Index);
      if (!EFI_ERROR (Status)) {
        RegisterContext->SwSmiInputValue = Index;
        break;
      }
    }
    if (RegisterContext->SwSmiInputValue == (UINTN)-1) {
      return EFI_OUT_OF_RESOURCES;
    }
  }

  if (RegisterContext->SwSmiInputValue >= MAXIMUM_SWI_VALUE) {
    return EFI_INVALID_PARAMETER;
  }

  Status = SmiInputValueDuplicateCheck (RegisterContext->SwSmiInputValue);
  if (EFI_ERROR (Status)) {
    return EFI_INVALID_PARAMETER;
  }

  //
  // Create database record and add to database
  //
  Record = (DATABASE_RECORD*) AllocateZeroPool (sizeof (DATABASE_RECORD));
  if (Record == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Record->Signature                       = DATABASE_RECORD_SIGNATURE;
  Record->DispatchFunction                = DispatchFunction;
  Record->RegisterContext.SwSmiInputValue = RegisterContext->SwSmiInputValue;
  InsertTailList (&mPrivateData.CallbackDataBase, &Record->Link);
  //
  // Child's handle will be the address linked list link in the record
  //
  Record->DispatchHandle    = (EFI_HANDLE) (&Record->Link);
  *DispatchHandle = (EFI_HANDLE)(&Record->Link);

  SmiHandlerProfileRegisterHandler (&gEfiSmmSwDispatch2ProtocolGuid, DispatchFunction, (UINTN)RETURN_ADDRESS (0), RegisterContext, sizeof(*RegisterContext));

  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
SmmSwDispatch2UnRegister (
  IN CONST EFI_SMM_SW_DISPATCH2_PROTOCOL  *This,
  IN EFI_HANDLE                           DispatchHandle
  )
{
  DATABASE_RECORD *RecordToDelete;

  if (DispatchHandle == NULL) {
    return EFI_INVALID_PARAMETER;
  }

  if (BASE_CR (DispatchHandle, DATABASE_RECORD, Link)->Signature != DATABASE_RECORD_SIGNATURE) {
    return EFI_INVALID_PARAMETER;
  }

  RecordToDelete = DATABASE_RECORD_FROM_LINK (DispatchHandle);

  SmiHandlerProfileUnregisterHandler (&gEfiSmmSwDispatch2ProtocolGuid, RecordToDelete->DispatchFunction, &RecordToDelete->RegisterContext, sizeof(RecordToDelete->RegisterContext));

  RemoveEntryList (&RecordToDelete->Link);

  FreePool (RecordToDelete);

  return EFI_SUCCESS;
}

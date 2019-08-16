/** @file
  export global variable g.

  Copyright (c) 2019, Intel Corporation. All rights reserved.<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent

**/

#include <PiDxe.h>
#include <Library/DebugLib.h>
#include <Library/HobLib.h>
#include <Library/EmuThunkLib.h>

#include <Protocol/EmuSocket.h>

EMU_SOCKET_THUNK_PROTOCOL *gEmuSocketThunk;

EFI_STATUS
EFIAPI
DxeEmuSocketLibConstructor (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_HOB_GUID_TYPE        *GuidHob;

  GuidHob = GetFirstGuidHob (&gEmuSocketThunkProtocolGuid);
  ASSERT (GuidHob != NULL);

  gEmuSocketThunk = (EMU_SOCKET_THUNK_PROTOCOL *)(*(UINTN *)(GET_GUID_HOB_DATA (GuidHob)));
  ASSERT (gEmuSocketThunk != NULL);

  return EFI_SUCCESS;
}

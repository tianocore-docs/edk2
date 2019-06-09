/** @file

  Copyright (c) 2017, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/DebugLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/UefiLib.h>
#include <Library/PrintLib.h>
#include <Protocol/LoadedImage.h>
#include <Guid/FileInfo.h>
#include <Guid/MemoryRangeCapsule.h>

#define CAPSULE_NAME  L"MemoryRange.Cap"

#define MAX_MEMORY_RANGE_COUNT  10

#pragma pack(1)

typedef struct {
  EFI_CAPSULE_HEADER        Header;
  EFI_MEMORY_TYPE           OsRequestedMemoryType;
  EFI_MEMORY_RANGE          MemoryForFirmwareUse;
  UINT64                    NumberOfMemoryRanges;
  EFI_MEMORY_RANGE          MemoryRanges[MAX_MEMORY_RANGE_COUNT];
} MY_EFI_MEMORY_RANGE_CAPSULE;

#pragma pack()

extern UINTN  Argc;
extern CHAR16 **Argv;

/**
  Read a file.

  @param[in]  FileName        The file to be read.
  @param[out] BufferSize      The file buffer size
  @param[out] Buffer          The file buffer

  @retval EFI_SUCCESS    Read file successfully
  @retval EFI_NOT_FOUND  File not found
**/
EFI_STATUS
ReadFileToBuffer (
  IN  CHAR16                               *FileName,
  OUT UINTN                                *BufferSize,
  OUT VOID                                 **Buffer
  );

/**
  Write a file.

  @param[in] FileName        The file to be written.
  @param[in] BufferSize      The file buffer size
  @param[in] Buffer          The file buffer

  @retval EFI_SUCCESS    Write file successfully
**/
EFI_STATUS
WriteFileFromBuffer (
  IN  CHAR16                               *FileName,
  IN  UINTN                                BufferSize,
  IN  VOID                                 *Buffer
  );

/**

  This function parse application ARG.

  @return Status
**/
EFI_STATUS
GetArg (
  VOID
  );

/**
  Create MemoryRange capsule.

  @retval EFI_SUCCESS            The capsule header is appended.
  @retval EFI_UNSUPPORTED        Input parameter is not valid.
  @retval EFI_OUT_OF_RESOURCES   No enough resource to create UX capsule.
**/
EFI_STATUS
CreateMemoryRangeCapsule (
  VOID
  )
{
  MY_EFI_MEMORY_RANGE_CAPSULE           MyMemoryRangeCapsule;
  UINTN                                 Index;
  EFI_STATUS                            Status;
  
  if (Argc < 9) {
    Print(L"GenMemoryRangeCapsuleApp: Invalid Parameter.\n");
    return EFI_UNSUPPORTED;
  }

  MyMemoryRangeCapsule.OsRequestedMemoryType = (UINT32)StrHexToUintn(Argv[2]);
  Print(L"OsRequestedMemoryType: 0x%x\n", MyMemoryRangeCapsule.OsRequestedMemoryType);
  
  MyMemoryRangeCapsule.MemoryForFirmwareUse.Address = StrHexToUint64(Argv[4]);
  MyMemoryRangeCapsule.MemoryForFirmwareUse.Length  = StrHexToUint64(Argv[5]);
    Print(
      L"MemoryForFirmwareUse: 0x%016lx - 0x%016lx\n",
      MyMemoryRangeCapsule.MemoryForFirmwareUse.Address,
      MyMemoryRangeCapsule.MemoryForFirmwareUse.Length
      );

  MyMemoryRangeCapsule.NumberOfMemoryRanges = (Argc - 7)/2;
  if (MyMemoryRangeCapsule.NumberOfMemoryRanges > MAX_MEMORY_RANGE_COUNT) {
    MyMemoryRangeCapsule.NumberOfMemoryRanges = MAX_MEMORY_RANGE_COUNT;
  }
  Print(L"NumberOfMemoryRanges: %d\n", MyMemoryRangeCapsule.NumberOfMemoryRanges);
  for (Index = 0; Index < MyMemoryRangeCapsule.NumberOfMemoryRanges; Index++) {
    MyMemoryRangeCapsule.MemoryRanges[Index].Address = StrHexToUint64(Argv[7 + Index * 2]);
    MyMemoryRangeCapsule.MemoryRanges[Index].Length  = StrHexToUint64(Argv[7 + Index * 2 + 1]);
    Print(
      L"MemoryRanges[%d]: 0x%016lx - 0x%016lx\n",
      Index,
      MyMemoryRangeCapsule.MemoryRanges[Index].Address,
      MyMemoryRangeCapsule.MemoryRanges[Index].Length
      );
  }

  CopyGuid(&MyMemoryRangeCapsule.Header.CapsuleGuid, &gEfiMemoryRangeCapsuleGuid);
  MyMemoryRangeCapsule.Header.HeaderSize = sizeof(MyMemoryRangeCapsule.Header);
  MyMemoryRangeCapsule.Header.Flags = EFI_MEMORY_RANGE_CAPSULE_FLAG;
  MyMemoryRangeCapsule.Header.CapsuleImageSize = (UINT32)(sizeof(EFI_MEMORY_RANGE_CAPSULE) + sizeof(EFI_MEMORY_RANGE) * (UINTN)MyMemoryRangeCapsule.NumberOfMemoryRanges);

  Status = WriteFileFromBuffer(CAPSULE_NAME, MyMemoryRangeCapsule.Header.CapsuleImageSize, &MyMemoryRangeCapsule);
  Print(L"GenMemoryRangeCapsuleApp: Write %s %r\n", CAPSULE_NAME, Status);

  return Status;
}

/**
  Print APP usage.
**/
VOID
PrintUsage (
  VOID
  )
{
  Print(L"GenMemoryRangeCapsuleApp:  usage\n");
  Print(L"  GenMemoryRangeCapsuleApp -T <MemType> -F <Base> <Length> -L <Base> <Length> [<Base> <Length>]\n");
  Print(L"    -T:      OsRequestedMemoryType\n");
  Print(L"    -F:      MemoryForFirmwareUse\n");
  Print(L"    -L:      OsReservedMemory\n");
  Print(L"    MemType: Hex based memory type\n");
  Print(L"             0x70000000..0x7FFFFFFF are reserved for OEM use.\n");
  Print(L"             0x80000000..0xFFFFFFFF are reserved for OS use.\n");
  Print(L"    Base:    Hex based memory base\n");
  Print(L"    Length:  Hex based memory length\n");
}

/**
  Update Capsule image.

  @param[in]  ImageHandle     The image handle.
  @param[in]  SystemTable     The system table.

  @retval EFI_SUCCESS            Command completed successfully.
  @retval EFI_INVALID_PARAMETER  Command usage error.
  @retval EFI_NOT_FOUND          The input file can't be found.
**/
EFI_STATUS
EFIAPI
UefiMain (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                    Status;

  Status = GetArg();
  if (EFI_ERROR(Status)) {
    Print(L"Please use UEFI SHELL to run this application!\n", Status);
    return Status;
  }
  if (Argc < 9) {
    PrintUsage();
    return EFI_INVALID_PARAMETER;
  }
  if ((StrCmp(Argv[1], L"-T") == 0) && (StrCmp(Argv[3], L"-F") == 0) && (StrCmp(Argv[6], L"-L") == 0)) {
    Status = CreateMemoryRangeCapsule();
    return Status;
  }
  return Status;
}

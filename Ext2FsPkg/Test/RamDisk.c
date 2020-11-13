/**@file

Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

//
// The package level header files this module uses
//
#include <Uefi.h>
//
// The protocols, PPI and GUID defintions for this module
//
#include <Protocol/BlockIo.h>
#include <Protocol/DevicePath.h>
#include <Protocol/LoadedImage.h>
#include <Protocol/SimpleFileSystem.h>
#include <Guid/FileInfo.h>
//
// The Library classes this module consumes
//
#include <Library/DebugLib.h>
#include <Library/BaseLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>

#include "RamDisk.h"

RAM_DISK_BLOCK_IO_PRIVATE   mRamDiskBlkIoPrivateTemplate = {
  RAM_DISK_BLOCK_IO_PRIVATE_SIGNATURE,
  {
    EFI_BLOCK_IO_PROTOCOL_REVISION2 - 1,
    &mRamDiskBlkIoPrivateTemplate.Media,
    RamDiskBlkIoReset,
    RamDiskBlkIoReadBlocks,
    RamDiskBlkIoWriteBlocks,
    RamDiskBlkIoFlushBlocks,
  },
  {
    1,     // MediaId;
    FALSE, // RemovableMedia;
    TRUE,  // MediaPresent;
    FALSE, // LogicalPartition;
    FALSE, // ReadOnly;
    FALSE, // WriteCaching;
    0,     // BlockSize; (TBD)
    1,     // IoAlign;
    0,     // LastBlock (TBD)
  },
};

EFI_GUID mEfiRamDiskGuid = EFI_RAM_DISK_GUID;

#define RAM_DISK_IMAGE_NAME  L"RamDisk.img"

#define RAM_DISK_BLOCK_SIZE 0x200

EFI_HANDLE  gImageHandle;

typedef struct {
  UINT64   Size;
  UINT64   FileSize;
  UINT64   PhysicalSize;
  EFI_TIME CreateTime;
  EFI_TIME LastAccessTime;
  EFI_TIME ModificationTime;
  UINT64   Attribute;
  CHAR16   FileName[1024];
} EFI_FILE_INFO_LOCAL;

EFI_STATUS
ReadBuffer (
  OUT VOID        **Buffer,
  OUT UINTN       *BufferSize
  )
{
  EFI_LOADED_IMAGE_PROTOCOL             *Image;
  EFI_DEVICE_PATH_PROTOCOL              *DevicePath;
  EFI_HANDLE                            DeviceHandle;
  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL       *SimpleFileSystem;
  EFI_FILE                              *Root;
  EFI_FILE                              *FileHandle;
  EFI_STATUS                            Status;
  EFI_FILE_INFO_LOCAL                   FileInfoLocal;
  UINTN                                 FileInfoSize;

  Status = gBS->HandleProtocol (
                 gImageHandle,
                 &gEfiLoadedImageProtocolGuid,
                 &Image
                 );
  if (EFI_ERROR(Status)) {
    Print (L"Error: HandleProtocol LoadedImage ! - %r\n", Status);
    return Status;
  }
  Status = gBS->HandleProtocol (
                 Image->DeviceHandle,
                 &gEfiDevicePathProtocolGuid,
                 &DevicePath
                 );
  if (EFI_ERROR(Status)) {
    Print (L"Error: HandleProtocol DevicePath ! - %r\n", Status);
    return Status;
  }
  Status = gBS->LocateDevicePath ( 
                 &gEfiSimpleFileSystemProtocolGuid,
                 &DevicePath,
                 &DeviceHandle
                 );
  if (EFI_ERROR (Status)) {
    Print (L"Error: LocateDevicePath SimpleFileSystem ! - %r\n", Status);
    return Status;
  }
  
  Status = gBS->HandleProtocol (
                 DeviceHandle, 
                 &gEfiSimpleFileSystemProtocolGuid,
                 (VOID*)&SimpleFileSystem
                 );
  if (EFI_ERROR (Status)) {
    Print (L"Error: HandleProtocol SimpleFileSystem ! - %r\n", Status);
    return Status;
  }
  Status = SimpleFileSystem->OpenVolume (
                               SimpleFileSystem,
                               &Root
                               );
  if (EFI_ERROR (Status)) {
    Print (L"Error: SimpleFileSystem->OpenVolume() ! - %r\n", Status);
    return Status;
  }
  Status = Root->Open (
                   Root,
                   &FileHandle,
                   RAM_DISK_IMAGE_NAME,
                   EFI_FILE_MODE_READ,
                   0
                   );
  if (EFI_ERROR (Status)) {
    Print (L"Error: Root->Open() ! - %r\n", Status);
    Root->Close (Root);
    return Status;
  }

  Status = FileHandle->SetPosition (FileHandle, 0); 
  if (EFI_ERROR (Status)) {
    Print (L"Error: Root->SetPosition() ! - %r\n", Status);
    Root->Close (Root);
    return Status;
  }

  FileInfoSize = sizeof (FileInfoLocal);
  ZeroMem (&FileInfoLocal, sizeof(FileInfoLocal));
  Status = FileHandle->GetInfo (FileHandle, &gEfiFileInfoGuid, &FileInfoSize, &FileInfoLocal);
  if (EFI_ERROR (Status)) {
    Print (L"Error: Root->GetInfo() ! - %r\n", Status);
    Root->Close (Root);
    return Status;
  }

  *BufferSize = (UINTN)FileInfoLocal.FileSize;
  *Buffer = AllocateZeroPool (*BufferSize);
  if (*Buffer == NULL) {
    Print (L"Error: AllocateZeroPool() ! - %x\n", *BufferSize);
    Root->Close (Root);
    return EFI_OUT_OF_RESOURCES;
  }
  
  Status = FileHandle->Read (FileHandle, BufferSize, *Buffer);
  FileHandle->Close (FileHandle);
  Root->Close (Root);

  if (EFI_ERROR (Status)) {
    Print (L"Error: FileHandle->Read() ! - %r\n", Status);
  }
  return Status;
}

/**
  The user Entry Point for module WinNtSimpleFileSystem. The user code starts with this function.

  @param[in] ImageHandle    The firmware allocated handle for the EFI image.  
  @param[in] SystemTable    A pointer to the EFI System Table.
  
  @retval EFI_SUCCESS       The entry point is executed successfully.
  @retval other             Some error occurs when executing this entry point.

**/
EFI_STATUS
EFIAPI
InitializeRamDiskBlkIo (
  IN EFI_HANDLE           ImageHandle,
  IN EFI_SYSTEM_TABLE     *SystemTable
  )
{
  EFI_STATUS              Status;
  EFI_HANDLE              Handle;
  VENDOR_DEVICE_PATH      *RamDiskDevicePath;
  EFI_DEVICE_PATH_PROTOCOL *EndDevicePath;
  UINT16                   RamDiskDevicePathLen;

  DEBUG ((EFI_D_ERROR, "InitializeRamDiskBlkIo - enter\n"));

  Status = ReadBuffer (
            &mRamDiskBlkIoPrivateTemplate.RamBuffer,
            &mRamDiskBlkIoPrivateTemplate.RamBufferSize
            );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_ERROR, "ReadBuffer - %r\n", Status));
    return Status;
  }
  DEBUG ((EFI_D_ERROR, "RamBufferSize - %x\n", mRamDiskBlkIoPrivateTemplate.RamBufferSize));
  DEBUG ((EFI_D_ERROR, "RamBuffer - %x\n", mRamDiskBlkIoPrivateTemplate.RamBuffer));

  mRamDiskBlkIoPrivateTemplate.Media.BlockSize = RAM_DISK_BLOCK_SIZE;
  mRamDiskBlkIoPrivateTemplate.Media.LastBlock = (mRamDiskBlkIoPrivateTemplate.RamBufferSize / mRamDiskBlkIoPrivateTemplate.Media.BlockSize) - 1;

  RamDiskDevicePathLen = sizeof(*RamDiskDevicePath);
  RamDiskDevicePath = AllocateZeroPool (RamDiskDevicePathLen + sizeof(EFI_DEVICE_PATH_PROTOCOL));
  RamDiskDevicePath->Header.Type = HARDWARE_DEVICE_PATH;
  RamDiskDevicePath->Header.SubType = HW_VENDOR_DP;
  CopyMem (&RamDiskDevicePath->Guid, &mEfiRamDiskGuid, sizeof(RamDiskDevicePath->Guid));
  CopyMem (&RamDiskDevicePath->Header.Length, &RamDiskDevicePathLen, sizeof(RamDiskDevicePath->Header.Length));
  EndDevicePath = (EFI_DEVICE_PATH_PROTOCOL *)((UINTN)RamDiskDevicePath + RamDiskDevicePathLen);
  EndDevicePath->Type = END_DEVICE_PATH_TYPE;
  EndDevicePath->SubType = END_ENTIRE_DEVICE_PATH_SUBTYPE;
  EndDevicePath->Length[0] = sizeof(EFI_DEVICE_PATH_PROTOCOL);

  Handle = NULL;
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Handle,
                  &gEfiBlockIoProtocolGuid,
                  &mRamDiskBlkIoPrivateTemplate.BlockIo,
                  &gEfiDevicePathProtocolGuid,
                  RamDiskDevicePath,
                  NULL
                  );

  DEBUG ((EFI_D_ERROR, "InitializeRamDiskBlkIo - exit (%r)\n", Status));

  return Status;
}

/**
  Reset the Block Device.

  @param  This                 Indicates a pointer to the calling context.
  @param  ExtendedVerification Driver may perform diagnostics on reset.

  @retval EFI_SUCCESS          The device was reset.
  @retval EFI_DEVICE_ERROR     The device is not functioning properly and could
                               not be reset.
**/
EFI_STATUS
EFIAPI
RamDiskBlkIoReset (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  BOOLEAN                ExtendedVerification
  )
{
  return EFI_SUCCESS;
}

/**
  Flush the Block Device.

  @param  This              Indicates a pointer to the calling context.

  @retval EFI_SUCCESS       All outstanding data was written to the device
  @retval EFI_DEVICE_ERROR  The device reported an error while writting back the data
  @retval EFI_NO_MEDIA      There is no media in the device.

**/
EFI_STATUS
EFIAPI
RamDiskBlkIoFlushBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This
  )
{
  return EFI_SUCCESS;
}

/**
  Write BufferSize bytes from Lba into Buffer.

  @param  This       Indicates a pointer to the calling context.
  @param  MediaId    The media ID that the write request is for.
  @param  Lba        The starting logical block address to be written. The caller is
                     responsible for writing to only legitimate locations.
  @param  BufferSize Size of Buffer, must be a multiple of device block size.
  @param  Buffer     A pointer to the source buffer for the data.

  @retval EFI_SUCCESS           The data was written correctly to the device.
  @retval EFI_WRITE_PROTECTED   The device can not be written to.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing the write.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHNAGED     The MediaId does not matched the current device.
  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  @retval EFI_INVALID_PARAMETER The write request contains LBAs that are not valid, 
                                or the buffer is not on proper alignment.

**/
EFI_STATUS
EFIAPI
RamDiskBlkIoWriteBlocks (
  IN EFI_BLOCK_IO_PROTOCOL  *This,
  IN UINT32                 MediaId,
  IN EFI_LBA                Lba,
  IN UINTN                  BufferSize,
  IN VOID                   *Buffer
  )
{
  RAM_DISK_BLOCK_IO_PRIVATE      *Private;

  DEBUG ((EFI_D_ERROR, "RamDiskBlkIoWriteBlocks - enter (%lx, %x)\n", Lba, BufferSize));

  Private = RAM_DISK_BLOCK_IO_PRIVATE_DATA_FROM_THIS(This);

  if (MediaId != Private->Media.MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  if (Lba > Private->Media.LastBlock) {
    return EFI_INVALID_PARAMETER;
  }

  if ((BufferSize % Private->Media.BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  CopyMem ((VOID *)(UINTN)((UINTN)Private->RamBuffer + MultU64x32 (Lba, Private->Media.BlockSize)), Buffer, BufferSize);

  DEBUG ((EFI_D_ERROR, "RamDiskBlkIoWriteBlocks - exit\n"));

  return EFI_SUCCESS;
}

/**
  Read BufferSize bytes from Lba into Buffer.

  @param  This       Indicates a pointer to the calling context.
  @param  MediaId    Id of the media, changes every time the media is replaced.
  @param  Lba        The starting Logical Block Address to read from
  @param  BufferSize Size of Buffer, must be a multiple of device block size.
  @param  Buffer     A pointer to the destination buffer for the data. The caller is
                     responsible for either having implicit or explicit ownership of the buffer.

  @retval EFI_SUCCESS           The data was read correctly from the device.
  @retval EFI_DEVICE_ERROR      The device reported an error while performing the read.
  @retval EFI_NO_MEDIA          There is no media in the device.
  @retval EFI_MEDIA_CHANGED     The MediaId does not matched the current device.
  @retval EFI_BAD_BUFFER_SIZE   The Buffer was not a multiple of the block size of the device.
  @retval EFI_INVALID_PARAMETER The read request contains LBAs that are not valid, 
                                or the buffer is not on proper alignment.

**/
EFI_STATUS
EFIAPI
RamDiskBlkIoReadBlocks (
  IN  EFI_BLOCK_IO_PROTOCOL  *This,
  IN  UINT32                 MediaId,
  IN  EFI_LBA                Lba,
  IN  UINTN                  BufferSize,
  OUT VOID                   *Buffer
  )
{
  RAM_DISK_BLOCK_IO_PRIVATE      *Private;

  DEBUG ((EFI_D_ERROR, "RamDiskBlkIoReadBlocks - enter (%lx, %x)\n", Lba, BufferSize));

  Private = RAM_DISK_BLOCK_IO_PRIVATE_DATA_FROM_THIS(This);

  if (MediaId != Private->Media.MediaId) {
    return EFI_MEDIA_CHANGED;
  }

  if (Lba > Private->Media.LastBlock) {
    return EFI_INVALID_PARAMETER;
  }

  if ((BufferSize % Private->Media.BlockSize) != 0) {
    return EFI_BAD_BUFFER_SIZE;
  }

  CopyMem (Buffer, (VOID *)(UINTN)((UINTN)Private->RamBuffer + MultU64x32 (Lba, Private->Media.BlockSize)), BufferSize);

  DEBUG ((EFI_D_ERROR, "RamDiskBlkIoReadBlocks - exit\n"));

  return EFI_SUCCESS;
}

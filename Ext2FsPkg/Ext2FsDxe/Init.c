/**@file

Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#include "Ext2FsDxe.h"

EFI_STATUS
Ext2AllocateVolume (
  IN  EFI_HANDLE                Handle,
  IN  EFI_DISK_IO_PROTOCOL      *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL     *BlockIo
  )
/*++

Routine Description:

  Allocates volume structure, detects EXT2 file system, installs protocol,
  and initialize cache.

Arguments:

  Handle                - The handle of parent device.
  DiskIo                - The DiskIo of parent device.
  BlockIo               - The BlockIo of parent devicel

Returns:

  EFI_SUCCESS           - Allocate a new volume successfully.
  EFI_OUT_OF_RESOURCES  - Can not allocate the memory.
  Others                - Allocating a new volume failed.

--*/
{
  EFI_STATUS   Status;
  EXT2_VOLUME  *Volume;

  //
  // Allocate a volume structure
  //
  Volume = AllocateZeroPool (sizeof (EXT2_VOLUME));
  if (Volume == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Initialize the structure
  //
  Volume->Signature                   = EXT2_VOLUME_SIGNATURE;
  Volume->Handle                      = Handle;
  Volume->DiskIo                      = DiskIo;
  Volume->BlockIo                     = BlockIo;
  Volume->MediaId                     = BlockIo->Media->MediaId;
  Volume->ReadOnly                    = BlockIo->Media->ReadOnly;
  Volume->VolumeInterface.Revision    = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_REVISION;
  Volume->VolumeInterface.OpenVolume  = Ext2OpenVolume;

  //
  // Check to see if there's a file system on the volume
  //
  Status = Ext2OpenDevice (Volume);
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Install our protocol interfaces on the device's handle
  //
  Status = gBS->InstallMultipleProtocolInterfaces (
                  &Volume->Handle,
                  &gEfiSimpleFileSystemProtocolGuid,
                  &Volume->VolumeInterface,
                  NULL
                  );
  if (EFI_ERROR (Status)) {
    goto Done;
  }
  //
  // Volume installed
  //
  DEBUG ((EFI_D_INFO, "Installed EXT2 filesystem on %p\n", Handle));
  Volume->Valid = TRUE;

Done:
  if (EFI_ERROR (Status)) {
    Ext2FreeVolume (Volume);
  }

  return Status;
}

EFI_STATUS
Ext2AbandonVolume (
  IN EXT2_VOLUME *Volume
  )
/*++

Routine Description:

  Called by FatDriverBindingStop(), Abandon the volume.

Arguments:

  Volume                - The volume to be abandoned.

Returns:

  EFI_SUCCESS           - Abandoned the volume successfully.
  Others                - Can not uninstall the protocol interfaces.

--*/
{
  FreePool (Volume);
  return EFI_SUCCESS;
}

static UINT8 mTempBuffer[EXT2_DEFAULT_BLOCK_SIZE];

EFI_STATUS
Ext2OpenDevice (
  IN OUT EXT2_VOLUME           *Volume
  )
/*++

Routine Description:

  Detects EXT2 file system on Disk and set relevant fields of Volume

Arguments:

  Volume                - The volume structure.

Returns:

  EFI_SUCCESS           - The EXT2 File System is detected successfully
  EFI_UNSUPPORTED       - The volume is not FAT file system.
  EFI_VOLUME_CORRUPTED  - The volume is corrupted.

--*/
{
  EFI_STATUS            Status;
  UINT64                Offset;
  UINTN                 Index;

  Volume->BlockSize = EXT2_DEFAULT_BLOCK_SIZE;

  //
  // Super block
  //
  Offset = EXT2_SUPER_BLOCK_INDEX * Volume->BlockSize;
  Status  = Ext2DiskIoRead (
              Volume,
              Offset,
              sizeof (EXT2_FS),
              &Volume->Ext2Fs
              );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ext2OpenDevice: ReadDisk failed %r\n", Status));
    return Status;
  }

  if (Volume->Ext2Fs.e2fs_magic != E2FS_MAGIC) {
    return EFI_UNSUPPORTED;
  }

  ASSERT (Volume->Ext2Fs.e2fs_log_bsize == 0);
  if (Volume->Ext2Fs.e2fs_rev > E2FS_REV0) {
    ASSERT (Volume->Ext2Fs.e2fs_inode_size == sizeof(EXT2FS_DINODE));
  }

  Ext2DumpSuperBlock (&Volume->Ext2Fs);

  Volume->Ext2GdCount = (Volume->Ext2Fs.e2fs_bcount + Volume->Ext2Fs.e2fs_bpg) / Volume->Ext2Fs.e2fs_bpg;
  DEBUG ((EFI_D_INFO, "Ext2GdCount: %d\n", Volume->Ext2GdCount));

  Volume->Ext2Gd = AllocateZeroPool (Volume->Ext2GdCount * sizeof(EXT2_GD));
  if (Volume->Ext2Gd == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Group descriptor
  //
  for (Index = 0; Index < Volume->Ext2GdCount; Index++) {
    Offset = (EXT2_SUPER_BLOCK_INDEX + 1) * Volume->BlockSize + Index * sizeof(EXT2_GD);
    Status  = Ext2DiskIoRead (
                Volume,
                Offset,
                sizeof (EXT2_GD),
                &Volume->Ext2Gd[Index]
                );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "Ext2OpenDevice: ReadDisk failed %r\n", Status));
      return Status;
    }
    DEBUG ((EFI_D_INFO, "Ext2Gd: %d\n", Index));
    Ext2DumpGroupDescriptor (&Volume->Ext2Gd[Index]);
  }
if (0) {
  //
  // Block Bit Map
  //
  for (Index = 0; Index < Volume->Ext2GdCount; Index++) {
    Status  = Ext2BlockRead (
                Volume,
                Volume->Ext2Gd[Index].ext2bgd_b_bitmap,
                mTempBuffer
                );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "Ext2OpenDevice: ReadDisk failed %r\n", Status));
      continue;
    }
    DEBUG ((EFI_D_INFO, "Block Bit Map: %d\n", Index));
    Ext2DumpBlock (mTempBuffer);
  }

  //
  // Inode Bit Map
  //
  for (Index = 0; Index < Volume->Ext2GdCount; Index++) {
    Status  = Ext2BlockRead (
                Volume,
                Volume->Ext2Gd[Index].ext2bgd_i_bitmap,
                mTempBuffer
                );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_INFO, "Ext2OpenDevice: ReadDisk failed %r\n", Status));
      continue;
    }

    DEBUG ((EFI_D_INFO, "Inode Bit Map: %d\n", Index));
    Ext2DumpBlock (mTempBuffer);
  }
}
  return EFI_SUCCESS;
}

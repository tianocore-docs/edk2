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
EFIAPI
Ext2OpenVolume (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL  *This,
  OUT EFI_FILE_PROTOCOL                **File
  )
/*++

Routine Description:

  Implements Simple File System Protocol interface function OpenVolume().

Arguments:

  This                  - Calling context.
  File                  - the Root Directory of the volume.

Returns:

  EFI_OUT_OF_RESOURCES  - Can not allocate the memory.
  EFI_VOLUME_CORRUPTED  - The FAT type is error.
  EFI_SUCCESS           - Open the volume successfully.

--*/
{
  EFI_STATUS  Status;
  EXT2_VOLUME *Volume;
  EXT2_IFILE  *IFile;
  EXT2_OFILE  *OFile;

  Volume = VOLUME_FROM_VOL_INTERFACE (This);
  Ext2AcquireLock ();

  //
  // Open Root file
  //
  OFile = AllocateZeroPool (sizeof(EXT2_OFILE));
  if (OFile == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  OFile->Signature = EXT2_OFILE_SIGNATURE;
  OFile->Volume = Volume;
  OFile->IsDir  = TRUE;
  Status  = Ext2InodeRead (
              Volume,
              EXT2_ROOTINO,
              &OFile->Inode
              );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ext2OpenVolume: ReadDisk failed %r\n", Status));
    goto Done;
  }

  DEBUG ((EFI_D_INFO, "Inode 0x%x:\n", EXT2_ROOTINO));
  Ext2DumpInode (&OFile->Inode);

  OFile->Entry.e2d_ino = EXT2_ROOTINO;
  OFile->Entry.e2d_reclen = 0xC;
  OFile->Entry.e2d_namlen = 1;
  OFile->Entry.e2d_type = EXT2_FT_DIR;
  OFile->Entry.e2d_name[0] = '\\';
  OFile->Entry.e2d_name[1] = 0;

  OFile->FullPath = AllocateZeroPool (sizeof(L"\\"));
  if (OFile->FullPath == NULL) {
    Status = EFI_OUT_OF_RESOURCES;
    goto Done;
  }
  OFile->FullPath[0] = L'\\';

  //
  // Open a new instance to the root
  //
  Status = Ext2AllocateIFile (OFile, &IFile);
  if (!EFI_ERROR (Status)) {
    *File = &IFile->Handle;
  }

  Volume->Root = OFile;

Done:
  Ext2ReleaseLock ();

  return Status;
}

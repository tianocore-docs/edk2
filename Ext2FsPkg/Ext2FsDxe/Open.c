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
Ext2AllocateIFile (
  IN EXT2_OFILE    *OFile,
  OUT EXT2_IFILE   **PtrIFile
  )
/*++

Routine Description:

  Create an Open instance for the existing OFile.
  The IFile of the newly opened file is passed out.

Arguments:

  OFile                 - The file that serves as a starting reference point.
  PtrIFile              - The newly generated IFile instance.

Returns:

  EFI_OUT_OF_RESOURCES  - Can not allocate the memory for the IFile
  EFI_SUCCESS           - Create the new IFile for the OFile successfully

--*/
{
  EXT2_IFILE *IFile;

  ASSERT_VOLUME_LOCKED (OFile->Volume);

  //
  // Allocate a new open instance
  //
  IFile = AllocateZeroPool (sizeof (EXT2_IFILE));
  if (IFile == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  IFile->Signature = EXT2_IFILE_SIGNATURE;

  CopyMem (&(IFile->Handle), &gExt2FileInterface, sizeof (EFI_FILE_PROTOCOL));

  IFile->OFile = OFile;

  *PtrIFile = IFile;
  return EFI_SUCCESS;
}

EFI_STATUS
Ext2OFileOpen (
  IN  EXT2_OFILE           *OFile,
  OUT EXT2_IFILE           **NewIFile,
  IN  CHAR16               *FileName,
  IN  UINT64               OpenMode,
  IN  UINT8                Attributes
  )
/*++

Routine Description:

  Open a file for a file name relative to an existing OFile.
  The IFile of the newly opened file is passed out.

Arguments:

  OFile                 - The file that serves as a starting reference point.
  NewIFile              - The newly generated IFile instance.
  FileName              - The file name relative to the OFile.
  OpenMode              - Open mode.
  Attributes            - Attributes to set if the file is created.

Returns:

  EFI_SUCCESS           - Open the file successfully.
  EFI_INVALID_PARAMETER - The open mode is conflict with the attributes
                          or the file name is not valid.
  EFI_NOT_FOUND         - Conficts between dir intention and attribute.
  EFI_WRITE_PROTECTED   - Can't open for write if the volume is read only.
  EFI_ACCESS_DENIED     - If the file's attribute is read only, and the
                          open is for read-write fail it.
  EFI_OUT_OF_RESOURCES  - Can not allocate the memory.

--*/
{
  EXT2_VOLUME *Volume;
  EFI_STATUS  Status;
  BOOLEAN     WriteMode;
  EXT2_OFILE  *NewOFile;

  Volume = OFile->Volume;
  ASSERT_VOLUME_LOCKED (Volume);
  WriteMode = (BOOLEAN) (OpenMode & EFI_FILE_MODE_WRITE);
  if (Volume->ReadOnly && WriteMode) {
    return EFI_WRITE_PROTECTED;
  }

  //
  // Get new OFile for the file
  //
  Status = Ext2LocateFile (OFile, FileName, &NewOFile);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  //
  // Create an open instance of the OFile
  //
  Status = Ext2AllocateIFile (NewOFile, NewIFile);
  if (EFI_ERROR (Status)) {
    return Status;
  }

  (*NewIFile)->ReadOnly = (BOOLEAN)!WriteMode;

  DEBUG ((EFI_D_INFO, "FSOpen: Open '%S' %r\n", FileName, Status));
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Ext2Open (
  IN  EFI_FILE_PROTOCOL   *FHand,
  OUT EFI_FILE_PROTOCOL   **NewHandle,
  IN  CHAR16              *FileName,
  IN  UINT64              OpenMode,
  IN  UINT64              Attributes
  )
/*++
Routine Description:

  Implements Open() of Simple File System Protocol.

Arguments:

  FHand                 - File handle of the file serves as a starting reference point.
  NewHandle             - Handle of the file that is newly opened.
  FileName              - File name relative to FHand.
  OpenMode              - Open mode.
  Attributes            - Attributes to set if the file is created.

Returns:

  EFI_INVALID_PARAMETER - The FileName is NULL or the file string is empty.
                          The OpenMode is not supported.
                          The Attributes is not the valid attributes.
  EFI_OUT_OF_RESOURCES  - Can not allocate the memory for file string.
  EFI_SUCCESS           - Open the file successfully.
  Others                - The status of open file.

--*/
{
  EXT2_IFILE  *IFile;
  EXT2_IFILE  *NewIFile;
  EXT2_OFILE  *OFile;
  EFI_STATUS  Status;

  //
  // Perform some parameter checking
  //
  if (FileName == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Check for a valid mode
  //
  switch (OpenMode) {
  case EFI_FILE_MODE_READ:
  case EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE:
  case EFI_FILE_MODE_READ | EFI_FILE_MODE_WRITE | EFI_FILE_MODE_CREATE:
    break;

  default:
    return EFI_INVALID_PARAMETER;
  }
  
  //
  // Check for valid Attributes for file creation case. 
  //
  if (((OpenMode & EFI_FILE_MODE_CREATE) != 0) && (Attributes & (EFI_FILE_READ_ONLY | (~EFI_FILE_VALID_ATTR))) != 0) {
    return EFI_INVALID_PARAMETER;
  }
  
  IFile = IFILE_FROM_FHAND (FHand);
  OFile = IFile->OFile;

  if (!OFile->IsDir) {
    DEBUG ((EFI_D_ERROR, "Ext2Open: try to open with this not being a dir\n"));
    return EFI_UNSUPPORTED;
  }

  DEBUG ((EFI_D_INFO, "Ext2Open - opening \"%s\" ...\n", FileName));

  //
  // Lock
  //
  Ext2AcquireLock ();

  //
  // Open the file
  //
  Status = Ext2OFileOpen (OFile, &NewIFile, FileName, OpenMode, (UINT8) Attributes);

  //
  // If the file was opened, return the handle to the caller
  //
  if (!EFI_ERROR (Status)) {
    *NewHandle = &NewIFile->Handle;
    DEBUG ((EFI_D_INFO, "Ext2Open - %r\n", Status));
    Ext2DumpInode (&NewIFile->OFile->Inode);
    Ext2DumpDirectory (&NewIFile->OFile->Entry);
  }
  //
  // Unlock
  //
  Ext2ReleaseLock ();

  return Status;
}

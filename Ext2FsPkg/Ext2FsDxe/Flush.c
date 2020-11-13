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
Ext2Flush (
  IN EFI_FILE_PROTOCOL  *FHand
  )
/*++

Routine Description:

  Flushes all data associated with the file handle.

Arguments:

  FHand                 - Handle to file to flush.

Returns:

  EFI_SUCCESS           - Flushed the file successfully.
  EFI_WRITE_PROTECTED   - The volume is read only.
  EFI_ACCESS_DENIED     - The file is read only.
  Others                - Flushing of the file failed.

--*/
{
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Ext2Close (
  IN EFI_FILE_PROTOCOL  *FHand
  )
/*++

Routine Description:

  Flushes & Closes the file handle.

Arguments:

  FHand                 - Handle to the file to delete.

Returns:

  EFI_SUCCESS           - Closed the file successfully.

--*/
{
  EXT2_IFILE  *IFile;
  EXT2_OFILE  *OFile;
  EXT2_VOLUME *Volume;

  IFile  = IFILE_FROM_FHAND (FHand);
  OFile  = IFile->OFile;
  Volume = OFile->Volume;

  DEBUG ((EFI_D_INFO, "Ext2Close - \'%s\'\n", OFile->FullPath));

  if (OFile != Volume->Root) {
    FreePool (IFile->OFile->FullPath);
    FreePool (IFile->OFile);
  }
  FreePool (IFile);

  return EFI_SUCCESS;
}

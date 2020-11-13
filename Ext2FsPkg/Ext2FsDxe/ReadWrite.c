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
Ext2GetPosition (
  IN  EFI_FILE_PROTOCOL *FHand,
  OUT UINT64            *Position
  )
/*++

Routine Description:

  Get the file's position of the file.

Arguments:

  FHand                 - The handle of file.
  Position              - The file's position of the file.

Returns:

  EFI_SUCCESS           - Get the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_UNSUPPORTED       - The open file is not a file.

--*/
{
  EXT2_IFILE *IFile;
  EXT2_OFILE *OFile;

  IFile = IFILE_FROM_FHAND (FHand);
  OFile = IFile->OFile;

  DEBUG ((EFI_D_INFO, "Ext2GetPosition - \'%s\'\n", OFile->FullPath));

  if (OFile->IsDir) {
    return EFI_UNSUPPORTED;
  }

  *Position = IFile->Position;
  return EFI_SUCCESS;
}

EFI_STATUS
EFIAPI
Ext2SetPosition (
  IN EFI_FILE_PROTOCOL  *FHand,
  IN UINT64             Position
  )
/*++

Routine Description:

  Set the file's position of the file.

Arguments:

  FHand                 - The handle of file.
  Position              - The file's position of the file.

Returns:

  EFI_SUCCESS           - Set the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_UNSUPPORTED       - Set a directory with a not-zero position.

--*/
{
  EXT2_IFILE *IFile;
  EXT2_OFILE *OFile;

  IFile = IFILE_FROM_FHAND (FHand);
  OFile = IFile->OFile;

  DEBUG ((EFI_D_INFO, "Ext2SetPosition - \'%s\'\n", OFile->FullPath));

  //
  // If this is a directory, we can only set back to position 0
  //
  if (OFile->IsDir) {
    if (Position != 0) {
      //
      // Reset current directory cursor;
      //
      return EFI_UNSUPPORTED;
    }
  }
  //
  // Set the position
  //
  if (Position == (UINT64)-1) {
    Position = OFile->Inode.e2di_size;
  }
  //
  // Set the position
  //
  IFile->Position = Position;
  return EFI_SUCCESS;
}

EFI_STATUS
Ext2ReadEfiDir (
  IN EXT2_IFILE       *IFile,
  IN OUT UINTN        *BufferSize,
  IN OUT VOID         *Buffer
  )
{
  EXT2_OFILE     *OFile;
  EXT2_VOLUME    *Volume;
  UINTN          MaxDirSize;
  VOID           *DirBuffer;
  EXT2FS_DIRECT  *Ext2Directory;
  UINTN          ThisSize;
  EFI_FILE_INFO  *FileInfo;
  EFI_STATUS     Status;
  EXT2_OFILE     NewOFile;

  OFile  = IFile->OFile;
  Volume = OFile->Volume;

  ASSERT (OFile->IsDir);

  MaxDirSize = OFile->Inode.e2di_size;
  DirBuffer = AllocatePool (MaxDirSize);
  if (DirBuffer == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  Status = Ext2ReadOFile (OFile, 0, &MaxDirSize, DirBuffer);
  if (EFI_ERROR (Status)) {
    FreePool (DirBuffer);
    return EFI_OUT_OF_RESOURCES;
  }

  Ext2Directory = (EXT2FS_DIRECT *)(UINTN)((UINTN)DirBuffer + IFile->Position);

  while (TRUE) {
    if ((IFile->Position >= MaxDirSize) || (Ext2Directory->e2d_reclen == 0)) {
      // End
      *BufferSize = 0;
      Status = EFI_SUCCESS;
      break;
    } else if (Ext2Directory->e2d_ino == EXT2_FT_UNKNOWN) {
      DEBUG ((EFI_D_INFO, "Ext2ReadEfiDir: empty\n"));
      Ext2DumpDirectory (Ext2Directory);
      // Empty entry, Skip
      IFile->Position += Ext2Directory->e2d_reclen;
      continue;
    } else {
      DEBUG ((EFI_D_INFO, "Ext2ReadEfiDir: ...\n"));
      Ext2DumpDirectory (Ext2Directory);

      ThisSize = SIZE_OF_EFI_FILE_INFO + (Ext2Directory->e2d_namlen + 1) * sizeof(UINT16);
      if (*BufferSize < ThisSize) {
        Status = EFI_BUFFER_TOO_SMALL;
      } else {
        FileInfo = Buffer;

        NewOFile.Signature = EXT2_OFILE_SIGNATURE;
        NewOFile.Volume = Volume;
        Status  = Ext2InodeRead (
                    Volume,
                    Ext2Directory->e2d_ino,
                    &NewOFile.Inode
                    );
        if (EFI_ERROR (Status)) {
          DEBUG ((EFI_D_INFO, "Ext2ReadEfiDir: ReadDisk failed %r\n", Status));
          goto Done;
        }
        if (S_ISDIR (NewOFile.Inode.e2di_mode)) {
          NewOFile.IsDir = TRUE;
        } else {
          NewOFile.IsDir = FALSE;
        }
        NewOFile.PosDisk = 0;
        NewOFile.PosRem = 0;
        CopyMem (&NewOFile.Entry, Ext2Directory, sizeof(EXT2FS_DIRECT));
        Status = Ext2GetFileInfo (&NewOFile, &ThisSize, FileInfo);
      }
      *BufferSize = ThisSize;
      if (!EFI_ERROR (Status)) {
        IFile->Position += Ext2Directory->e2d_reclen;
      }
      break;
    }
  }

Done:
  FreePool (DirBuffer);

  return Status;
}

EFI_STATUS
Ext2IFileRead (
  IN     EFI_FILE_PROTOCOL     *FHand,
  IN OUT UINTN                 *BufferSize,
  IN OUT VOID                  *Buffer
  )
/*++

Routine Description:

  Get the file info from the open file of the IFile into Buffer.

Arguments:

  FHand                 - The file handle to access.
  IoMode                - Indicate whether the access mode is reading or writing.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing read data.

Returns:

  EFI_SUCCESS           - Get the file info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  EFI_WRITE_PROTECTED   - The disk is write protect.
  EFI_ACCESS_DENIED     - The file is read-only.
  other                 - An error occurred when operating on the disk.

--*/
{
  EFI_STATUS  Status;
  EXT2_IFILE  *IFile;
  EXT2_OFILE  *OFile;
  EXT2_VOLUME *Volume;
  UINT64      EndPosition;

  IFile  = IFILE_FROM_FHAND (FHand);
  OFile  = IFile->OFile;
  Volume = OFile->Volume;

  //
  // If position is at EOF, then return device error
  //
  if (IFile->Position > OFile->Inode.e2di_size) {
    return EFI_DEVICE_ERROR;
  }

  DEBUG ((EFI_D_INFO, "Ext2Read - \'%s\'\n", OFile->FullPath));

  Ext2AcquireLock ();

  if (OFile->IsDir) {
    //
    // Access a directory
    //
    Status = Ext2ReadEfiDir (IFile, BufferSize, Buffer);

  } else if (S_ISREG(OFile->Inode.e2di_mode)) {
    //
    // Access a file
    //
    EndPosition = IFile->Position + *BufferSize;
    if (EndPosition > OFile->Inode.e2di_size) {
      //
      // The position goes beyond the end of file
      //

      //
      // Adjust the actual size read
      //
      *BufferSize -= (UINTN) EndPosition - OFile->Inode.e2di_size;
    }

    //
    // Access
    //
    Status = Ext2ReadOFile (OFile, IFile->Position, BufferSize, Buffer);

    IFile->Position += *BufferSize;

  } else {
    switch (OFile->Inode.e2di_mode & S_IFMT) {
    case S_IFLNK:
      DEBUG ((EFI_D_INFO, "Ext2Read Symbolic link - \'%a\'\n", OFile->Inode.e2di_blocks));
      //
      // Access a symbolic link
      //
      EndPosition = IFile->Position + *BufferSize;
      if (EndPosition > OFile->Inode.e2di_size) {
        //
        // The position goes beyond the end of file
        //

        //
        // Adjust the actual size read
        //
        *BufferSize -= (UINTN) EndPosition - OFile->Inode.e2di_size;
      }

      //
      // Access
      //
      CopyMem (Buffer, (UINT8 *)OFile->Inode.e2di_blocks + IFile->Position, *BufferSize);

      IFile->Position += *BufferSize;
      Status = EFI_SUCCESS;
      break;

    case S_IFCHR:
    case S_IFBLK:
    case S_IFIFO:
    case S_IFSOCK:
    default:
      DEBUG ((EFI_D_INFO, "Ext2Read Special File (0x%x) - %r\n", OFile->Inode.e2di_mode & S_IFMT, EFI_UNSUPPORTED));
      Status = EFI_UNSUPPORTED;
      break;
    }
  }

  Ext2ReleaseLock ();

  DEBUG ((EFI_D_INFO, "Ext2Read - %r\n", Status));

  return Status;
}

EFI_STATUS
EFIAPI
Ext2Read (
  IN     EFI_FILE_PROTOCOL  *FHand,
  IN OUT UINTN              *BufferSize,
     OUT VOID               *Buffer
  )
/*++

Routine Description:

  Get the file info.

Arguments:

  FHand                 - The handle of the file.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing read data.

Returns:

  EFI_SUCCESS           - Get the file info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_VOLUME_CORRUPTED  - The file type of open file is error.
  other                 - An error occurred when operation the disk.

--*/
{
  return Ext2IFileRead (FHand, BufferSize, Buffer);
}

EFI_STATUS
EFIAPI
Ext2Write (
  IN     EFI_FILE_PROTOCOL  *FHand,
  IN OUT UINTN              *BufferSize,
  IN     VOID               *Buffer
  )
/*++

Routine Description:

  Write the content of buffer into files.

Arguments:

  FHand                 - The handle of the file.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing write data.

Returns:

  EFI_SUCCESS           - Set the file info successfully.
  EFI_WRITE_PROTECTED   - The disk is write protect.
  EFI_ACCESS_DENIED     - The file is read-only.
  EFI_DEVICE_ERROR      - The OFile is not valid.
  EFI_UNSUPPORTED       - The open file is not a file.
                        - The writing file size is larger than 4GB.
  other                 - An error occurred when operation the disk.

--*/
{
  return EFI_WRITE_PROTECTED;
}

EFI_STATUS
Ext2ReadOFile (
  IN     EXT2_OFILE     *OFile,
  IN     UINT64         Position,
  IN OUT UINTN          *DataBufferSize,
  IN OUT UINT8          *UserBuffer
  )
/*++

Routine Description:

  This function reads data from a file or writes data to a file.
  It uses OFile->PosRem to determine how much data can be accessed in one time.

Arguments:

  OFile                 - The open file.
  IoMode                - Indicate whether the access mode is reading or writing.
  Position              - The position where data will be accessed.
  DataBufferSize        - Size of Buffer.
  UserBuffer            - Buffer containing data.

Returns:

  EFI_SUCCESS           - Access the data successfully.
  other                 - An error occurred when operating on the disk.

--*/
{
  EXT2_VOLUME *Volume;
  UINTN       Len;
  EFI_STATUS  Status;
  UINTN       BufferSize;

  BufferSize  = *DataBufferSize;
  Volume      = OFile->Volume;
  ASSERT_VOLUME_LOCKED (Volume);

  Status = EFI_SUCCESS;
  while (BufferSize > 0) {
    //
    // Seek the OFile to the file position
    //
    Status = Ext2OFilePosition (OFile, Position, BufferSize);
    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // Clip length to block run
    //
    Len = BufferSize > OFile->PosRem ? OFile->PosRem : BufferSize;

    //
    // Write the data
    //
    Status = Ext2DiskIoRead (Volume, OFile->PosDisk, Len, UserBuffer);
    if (EFI_ERROR (Status)) {
      break;
    }
    //
    // Data was successfully accessed
    //
    Position   += Len;
    UserBuffer += Len;
    BufferSize -= Len;
    //
    // Make sure no outbound occurred
    //
    ASSERT (Position <= OFile->Inode.e2di_size);
  }
  //
  // Update the number of bytes accessed
  //
  *DataBufferSize -= BufferSize;
  return Status;
}

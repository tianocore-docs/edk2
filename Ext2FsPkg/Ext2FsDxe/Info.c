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

UINT64
Ext2GetEfiDirSize (
  IN EXT2_OFILE       *OFile
  )
{
  UINTN          MaxDirSize;
  VOID           *DirBuffer;
  EXT2FS_DIRECT  *Ext2Directory;
  UINT64         TotalSize;
  EFI_STATUS     Status;

  ASSERT (OFile->IsDir);

  TotalSize = SIZE_OF_EFI_FILE_INFO + sizeof(L".") + SIZE_OF_EFI_FILE_INFO + sizeof(L"..");
  MaxDirSize = OFile->Inode.e2di_size;
  DirBuffer = AllocatePool (MaxDirSize);
  if (DirBuffer == NULL) {
    return TotalSize;
  }

  Status = Ext2ReadOFile (OFile, 0, &MaxDirSize, DirBuffer);
  if (EFI_ERROR (Status)) {
    FreePool (DirBuffer);
    return TotalSize;
  }

  TotalSize = 0;
  Ext2Directory = DirBuffer;
  while ((UINTN)Ext2Directory < (UINTN)DirBuffer + MaxDirSize) {
    if (Ext2Directory->e2d_reclen == 0) {
      break;
    }
    if (Ext2Directory->e2d_ino == EXT2_FT_UNKNOWN) {
      Ext2Directory = (EXT2FS_DIRECT *)((UINTN)Ext2Directory + Ext2Directory->e2d_reclen);
      continue;
    }
    TotalSize += SIZE_OF_EFI_FILE_INFO + (Ext2Directory->e2d_namlen + 1) * sizeof(UINT16);
    Ext2Directory = (EXT2FS_DIRECT *)((UINTN)Ext2Directory + Ext2Directory->e2d_reclen);
  }

  FreePool (DirBuffer);

  return TotalSize;
}

EFI_STATUS
Ext2GetFileInfo (
  IN EXT2_OFILE       *OFile,
  IN OUT UINTN        *BufferSize,
  OUT VOID            *Buffer
  )
/*++

Routine Description:

  Get the open file's info into Buffer.

Arguments:

  OFile                 - The open file.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing file info.

Returns:

  EFI_SUCCESS           - Get the file info successfully.
  EFI_BUFFER_TOO_SMALL  - The buffer is too small.

--*/
{
  UINTN               Size;
  UINTN               NameSize;
  UINTN               ResultSize;
  EFI_STATUS          Status;
  EFI_FILE_INFO       *Info;
  CHAR16              Name[EXT2FS_MAXNAMLEN + 1];
  UINTN               Index;

  ASSERT_VOLUME_LOCKED (Volume);

  Size        = SIZE_OF_EFI_FILE_INFO;
  for (Index = 0; Index < OFile->Entry.e2d_namlen; Index++) {
    Name[Index] = OFile->Entry.e2d_name[Index];
  }
  Name[Index] = 0;
  NameSize    = StrSize (Name);
  ResultSize  = Size + NameSize;

  Status      = EFI_BUFFER_TOO_SMALL;
  if (*BufferSize >= ResultSize) {
    Status      = EFI_SUCCESS;
    Info        = Buffer;
    Info->Size  = ResultSize;

    if (OFile->IsDir) {
      Info->FileSize      = Ext2GetEfiDirSize (OFile);
    } else {
      Info->FileSize      = OFile->Inode.e2di_size;
    }
    Info->PhysicalSize  = MultU64x32 (DivU64x32(Info->FileSize + EXT2_DEFAULT_BLOCK_SIZE, EXT2_DEFAULT_BLOCK_SIZE), EXT2_DEFAULT_BLOCK_SIZE);

    Ext2TimeToEfiTime (OFile->Inode.e2di_atime, &Info->LastAccessTime);
    Ext2TimeToEfiTime (OFile->Inode.e2di_ctime, &Info->CreateTime);
    Ext2TimeToEfiTime (OFile->Inode.e2di_mtime, &Info->ModificationTime);

    Info->Attribute = 0;
    if (OFile->IsDir) {
      Info->Attribute = EFI_FILE_DIRECTORY;
    } else if (S_ISREG(OFile->Inode.e2di_mode)) {
      Info->Attribute = 0;
    } else {
      switch (OFile->Inode.e2di_mode & S_IFMT) {
      case S_IFLNK:
        Info->Attribute = EFI_FILE_SYSTEM;
        break;
      case S_IFCHR:
      case S_IFBLK:
      case S_IFIFO:
      case S_IFSOCK:
      default:
        Info->Attribute = EFI_FILE_SYSTEM | EFI_FILE_HIDDEN;
        break;
      }
    }

    CopyMem ((CHAR8 *) Buffer + Size, Name, NameSize);
    Status      = EFI_SUCCESS;
  }

  *BufferSize = ResultSize;
  return Status;
}

EFI_STATUS
Ext2GetVolumeInfo (
  IN     EXT2_VOLUME    *Volume,
  IN OUT UINTN          *BufferSize,
     OUT VOID           *Buffer
  )
/*++

Routine Description:

  Get the volume's info into Buffer.

Arguments:

  Volume                - EXT2 file system volume.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing volume info.

Returns:

  EFI_SUCCESS           - Get the volume info successfully.
  EFI_BUFFER_TOO_SMALL  - The buffer is too small.

--*/
{
  UINTN                 Size;
  UINTN                 NameSize;
  UINTN                 ResultSize;
  CHAR16                Name[16 + 1];
  EFI_STATUS            Status;
  EFI_FILE_SYSTEM_INFO  *Info;
  UINTN                 Index;

  Size              = SIZE_OF_EFI_FILE_SYSTEM_INFO;
  AsciiStrToUnicodeStr (Volume->Ext2Fs.e2fs_vname, Name);
  NameSize          = StrSize (Name);
  ResultSize        = Size + NameSize;

  //
  // If we don't have valid info, compute it now
  //

  Status = EFI_BUFFER_TOO_SMALL;
  if (*BufferSize >= ResultSize) {
    Status  = EFI_SUCCESS;

    Info    = Buffer;
    ZeroMem (Info, SIZE_OF_EFI_FILE_SYSTEM_INFO);

    Info->Size        = ResultSize;
    Info->ReadOnly    = Volume->ReadOnly;
    Info->VolumeSize  = MultU64x32 (Volume->BlockSize, Volume->Ext2Fs.e2fs_bcount);
    Info->FreeSpace   = 0;
    for (Index = 0; Index < Volume->Ext2GdCount; Index++) {
      Info->FreeSpace += Volume->Ext2Gd[Index].ext2bgd_nbfree;
    }
    Info->FreeSpace   = MultU64x32 (Info->FreeSpace, Volume->BlockSize);
    Info->BlockSize   = Volume->BlockSize;
    CopyMem ((CHAR8 *) Buffer + Size, Name, NameSize);
  }

  *BufferSize = ResultSize;
  return Status;
}

EFI_STATUS
Ext2GetVolumeLabelInfo (
  IN EXT2_VOLUME      *Volume,
  IN OUT UINTN        *BufferSize,
  OUT VOID            *Buffer
  )
/*++

Routine Description:

  Get the volume's label info into Buffer.

Arguments:

  Volume                - EXT2 file system volume.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing volume's label info.

Returns:

  EFI_SUCCESS           - Get the volume's label info successfully.
  EFI_BUFFER_TOO_SMALL  - The buffer is too small.

--*/
{
  UINTN                             Size;
  UINTN                             NameSize;
  UINTN                             ResultSize;
  CHAR16                            Name[16 + 1];
  EFI_STATUS                        Status;

  Size        = SIZE_OF_EFI_FILE_SYSTEM_VOLUME_LABEL;
  AsciiStrToUnicodeStr (Volume->Ext2Fs.e2fs_vname, Name);
  NameSize    = StrSize (Name);
  ResultSize  = Size + NameSize;

  Status      = EFI_BUFFER_TOO_SMALL;
  if (*BufferSize >= ResultSize) {
    Status  = EFI_SUCCESS;
    CopyMem ((CHAR8 *) Buffer + Size, Name, NameSize);
  }

  *BufferSize = ResultSize;
  return Status;
}

EFI_STATUS
EFIAPI
Ext2GetInfo (
  IN     EFI_FILE_PROTOCOL   *FHand,
  IN     EFI_GUID            *Type,
  IN OUT UINTN               *BufferSize,
     OUT VOID                *Buffer
  )
/*++

Routine Description:

  Get the some types info of the file into Buffer.

Arguments:

  FHand                 - The handle of file.
  Type                  - The type of the info.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing volume info.

Returns:

  EFI_SUCCESS           - Get the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.

--*/
{
  EXT2_IFILE  *IFile;
  EXT2_OFILE  *OFile;
  EXT2_VOLUME *Volume;
  EFI_STATUS  Status;

  IFile   = IFILE_FROM_FHAND (FHand);
  OFile   = IFile->OFile;
  Volume  = OFile->Volume;

  DEBUG ((EFI_D_INFO, "Ext2GetInfo - \'%s\'\n", OFile->FullPath));

  Ext2AcquireLock ();

  Status = EFI_UNSUPPORTED;
  //
  // Get the proper information based on the request
  //
  if (CompareGuid (Type, &gEfiFileInfoGuid)) {
    DEBUG ((EFI_D_INFO, "Ext2GetFileInfo\n"));
    Status = Ext2GetFileInfo (OFile, BufferSize, Buffer);
  }

  if (CompareGuid (Type, &gEfiFileSystemInfoGuid)) {
    DEBUG ((EFI_D_INFO, "Ext2GetVolumeInfo\n"));
    Status = Ext2GetVolumeInfo (Volume, BufferSize, Buffer);
  }

  if (CompareGuid (Type, &gEfiFileSystemVolumeLabelInfoIdGuid)) {
    DEBUG ((EFI_D_INFO, "Ext2GetVolumeLabelInfo\n"));
    Status = Ext2GetVolumeLabelInfo (Volume, BufferSize, Buffer);
  }

  Ext2ReleaseLock ();

  DEBUG ((EFI_D_INFO, "Ext2GetInfo - %r\n", Status));
  return Status;
}

EFI_STATUS
EFIAPI
Ext2SetInfo (
  IN EFI_FILE_PROTOCOL  *FHand,
  IN EFI_GUID           *Type,
  IN UINTN              BufferSize,
  IN VOID               *Buffer
  )
/*++

Routine Description:

  Set the some types info of the file into Buffer.

Arguments:

  FHand                 - The handle of file.
  Type                  - The type of the info.
  BufferSize            - Size of Buffer
  Buffer                - Buffer containing volume info.

Returns:

  EFI_SUCCESS           - Set the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.

--*/
{
  return EFI_WRITE_PROTECTED;
}

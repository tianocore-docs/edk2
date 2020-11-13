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

BOOLEAN
Ext2FileNameInDirMatch (
  IN CHAR8  *Name,
  IN CHAR8  NameLen,
  IN CHAR16 *FileName
  )
{
  UINTN  Index;

  for (Index = 0; Index < NameLen; Index++) {
    if (Name[Index] != FileName[Index]) {
      return FALSE;
    }
  }
  if (FileName[NameLen] != 0) {
    return FALSE;
  }
  return TRUE;
}


EFI_STATUS
Ext2SearchDir (
  IN  EXT2_OFILE     *OFile,
  IN  CHAR16         *FileNameString,
  OUT EXT2FS_DIRECT  **PtrDirEnt
  )
{
  UINTN          MaxDirSize;
  VOID           *DirBuffer;
  EXT2FS_DIRECT  *Ext2Directory;
  EFI_STATUS     Status;

  if (!OFile->IsDir) {
    return EFI_UNSUPPORTED;
  }

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

  Status = EFI_NOT_FOUND;
  Ext2Directory = DirBuffer;
  while ((UINTN)Ext2Directory < (UINTN)DirBuffer + MaxDirSize) {
    if (Ext2Directory->e2d_reclen == 0) {
      break;
    }
    if (Ext2FileNameInDirMatch (Ext2Directory->e2d_name, Ext2Directory->e2d_namlen, FileNameString)) {
      *PtrDirEnt = AllocatePool (Ext2Directory->e2d_reclen);
      if (*PtrDirEnt == NULL) {
        Status = EFI_OUT_OF_RESOURCES;
      }
      CopyMem (*PtrDirEnt, Ext2Directory, Ext2Directory->e2d_reclen);
      Status = EFI_SUCCESS;
      break;
    }
    Ext2Directory = (EXT2FS_DIRECT *)((UINTN)Ext2Directory + Ext2Directory->e2d_reclen);
  }

  FreePool (DirBuffer);

  return Status;
}

EFI_STATUS
Ext2FilePathMoveToParent (
  IN OUT CHAR16         *FullFileName
  )
{
  CHAR16  *LastPtr;
  UINTN   Index;

  LastPtr = FullFileName;
  for (Index = 0; FullFileName[Index] != 0; Index++) {
    if (FullFileName[Index] == L'\\') {
      LastPtr = &FullFileName[Index];
    }
  }

  if (LastPtr == FullFileName) {
    if (*(LastPtr + 1) == 0) {
      //
      // Root handle does not have parent
      //
      return EFI_INVALID_PARAMETER;
    } else {
      //
      // Need return Root handle
      //
      LastPtr++;
    }
  }

  *LastPtr = 0;
  return EFI_SUCCESS;
}

EFI_STATUS
Ext2FilePathAppendToParent (
  IN OUT CHAR16         *FullFileName,
  IN     CHAR16         *FileName
  )
{
  if (StrCmp (FullFileName, L"\\") != 0) {
    StrCat (FullFileName, L"\\");
  }
  if (StrCmp (FileName, L"\\") != 0) {
    StrCat (FullFileName, FileName);
  }

  return EFI_SUCCESS;
}

/**
  "\"   If the filename starts with a "\" the relative location is the root directory that This resides on;
        otherwise "\" separates name components.

  "."   Opens the current location.

  ".."  Opens the parent directory for the current location.
        If the location is the root directory the request will return an error.

**/

EFI_STATUS
Ext2ConvertFileName (
  IN OUT CHAR16         *FullFileName,
  IN     CHAR16         *FileName
  )
{
  CHAR16      ComponentName[EXT2FS_MAXNAMLEN + 1];
  CHAR16      NewFileName[EXT2FS_MAXNAMLEN + 1];
  CHAR16      *Next;

  if (FileName[0] == L'\\') {
    StrCpy (FullFileName, L"\\");
    FileName++;
  }

  Next = FileName;
  for (;;) {
    //
    // Get the next component name
    //
    FileName = Next;
    Next     = Ext2GetNextNameComponent (FileName, ComponentName);

    //
    // If end of the file name, we're done
    //
    if (ComponentName[0] == 0) {
      break;
    }
    //
    // If "dot", then current
    //
    if (StrCmp (ComponentName, L".") == 0) {
      continue;
    }
    //
    // If "dot dot", then parent
    //
    if (StrCmp (ComponentName, L"..") == 0) {
      if (StrCmp (FullFileName, L"\\") == 0) {
        return EFI_INVALID_PARAMETER;
      }
      Ext2FilePathMoveToParent (FullFileName);
      continue;
    }

    if (!Ext2FileNameIsValid (ComponentName, NewFileName)) {
      return EFI_INVALID_PARAMETER;
    }

    Ext2FilePathAppendToParent (FullFileName, NewFileName);
  }

  return EFI_SUCCESS;
}

CHAR16 *
Ext2PartOfFilePath (
  IN CHAR16  *ParentFileName,
  IN CHAR16  *FullFileName
  )
{
  UINTN  Index;

  for (Index = 0; (ParentFileName[Index] != 0) && (FullFileName[Index] != 0); Index++) {
    if (ParentFileName[Index] != FullFileName[Index]) {
      return NULL;
    }
  }

  if (ParentFileName[Index] == 0) {
    if (StrCmp (ParentFileName, L"\\") == 0) {
      return &FullFileName[1];
    }
    if (FullFileName[Index] == 0) {
      return &FullFileName[Index];
    }
    if (FullFileName[Index] == L'\\') {
      return &FullFileName[Index + 1];
    }
  }
  return NULL;
}

EFI_STATUS
Ext2CreateOFileOnDirent (
  IN     EXT2_VOLUME    *Volume,
  IN     CHAR16         *ParentFilePath,
  IN     EXT2FS_DIRECT  *Direct,
  IN     BOOLEAN        CopyParentPath,
     OUT EXT2_OFILE     **NewOFile
  )
{
  EXT2_OFILE     *OFile;
  CHAR16         *Ptr;
  UINTN          Index;
  EFI_STATUS     Status;

  OFile = AllocateZeroPool (sizeof(EXT2_OFILE));
  if (OFile == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  //
  // NOTE: e2d_reclen might bigger than sizeof(EXT2FS_DIRECT).
  //
  CopyMem (&OFile->Entry, Direct, Direct->e2d_reclen > sizeof(EXT2FS_DIRECT) ? sizeof(EXT2FS_DIRECT): Direct->e2d_reclen);
  OFile->Signature = EXT2_OFILE_SIGNATURE;
  OFile->Volume = Volume;
  Status  = Ext2InodeRead (
              Volume,
              Direct->e2d_ino,
              &(OFile->Inode)
              );
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ext2CreateOFileOnDirent: ReadDisk failed %r\n", Status));
    FreePool (OFile);
    return Status;
  }
  OFile->IsDir = S_ISDIR (OFile->Inode.e2di_mode);

  OFile->FullPath = AllocateZeroPool (StrSize(ParentFilePath) + StrSize(L"\\") + Direct->e2d_namlen * sizeof(UINT16));
  if (OFile->FullPath == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }
  if (CopyParentPath) {
    StrCpy (OFile->FullPath, ParentFilePath);
  } else {
    if (StrCmp (ParentFilePath, L"\\") != 0) {
      StrCpy (OFile->FullPath, ParentFilePath);
    }
    StrCat (OFile->FullPath, L"\\");

    if (Direct->e2d_name[0] != '\\') {
      Ptr = OFile->FullPath + StrLen(OFile->FullPath);
      for (Index = 0; Index < Direct->e2d_namlen; Index++) {
        Ptr[Index] = Direct->e2d_name[Index];
      }
    }
  }

  *NewOFile = OFile;

  return EFI_SUCCESS;
}

EFI_STATUS
Ext2LocateFile (
  IN EXT2_OFILE         *ParentOFile,
  IN     CHAR16         *FileName,
     OUT EXT2_OFILE     **NewOFile
  )
{
  EFI_STATUS  Status;
  EXT2_VOLUME *Volume;
  CHAR16      ComponentName[EXT2FS_MAXNAMLEN + 1];
  UINTN       FileNameLen;
  CHAR16      *Next;
  EXT2FS_DIRECT  *Direct;
  EXT2_OFILE     *OFile;
  CHAR16      FullFileName[EXT2FS_MAXNAMLEN + 1];
  CHAR16      *PartOfFileName;

  FileNameLen = StrLen (FileName);
  if (FileNameLen == 0) {
    return EFI_INVALID_PARAMETER;
  }

  Direct = NULL;

  Volume = ParentOFile->Volume;

  StrCpy (FullFileName, ParentOFile->FullPath);
  Status = Ext2ConvertFileName (FullFileName, FileName);
  if (EFI_ERROR (Status)) {
    return Status;
  }
  FileName = FullFileName;
  FileNameLen = StrLen (FileName);

  PartOfFileName = Ext2PartOfFilePath (ParentOFile->FullPath, FullFileName);
  if (PartOfFileName != NULL) {
    //
    // Start from Parent
    //
    OFile = ParentOFile;
    FileName = PartOfFileName;
    FileNameLen = StrLen (FileName);
  } else {
    //
    // Start from Root
    //
    OFile = Volume->Root;
    FileName++;
    FileNameLen--;
  }

  //
  // Start at current location
  //
  Next = FileName;
  for (;;) {
    //
    // Get the next component name
    //
    FileName = Next;
    Next     = Ext2GetNextNameComponent (FileName, ComponentName);

    //
    // If end of the file name, we're done
    //
    if (ComponentName[0] == 0) {
      if (Direct == NULL) {
        Direct = AllocateZeroPool (sizeof(EXT2FS_DIRECT));
        if (Direct == NULL) {
          return EFI_OUT_OF_RESOURCES;
        }
        CopyMem (Direct, &OFile->Entry, sizeof(EXT2FS_DIRECT));
        Status = Ext2CreateOFileOnDirent (Volume, OFile->FullPath, Direct, TRUE, NewOFile);
      }
      break;
    }
    //
    // If "dot", then current
    //
    if (StrCmp (ComponentName, L".") == 0) {
      ASSERT (FALSE);
      break;
    }
    //
    // If "dot dot", then parent
    //
    if (StrCmp (ComponentName, L"..") == 0) {
      ASSERT (FALSE);
      break;
    }

    //
    // Search the compName in the directory
    //
    Status = Ext2SearchDir (OFile, ComponentName, &Direct);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    //
    // Create Temp New OFile
    //
    Status = Ext2CreateOFileOnDirent (Volume, OFile->FullPath, Direct, FALSE, NewOFile);
    if (EFI_ERROR (Status)) {
      return Status;
    }

    if (OFile != ParentOFile && OFile != Volume->Root) {
      FreePool (OFile->FullPath);
      FreePool (OFile);
    }
    FreePool (Direct);

    OFile = *NewOFile;
    continue;
  }

  return EFI_SUCCESS;
}

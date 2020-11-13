/**@file

Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef _EFI_EXT2_FS_DXE_H_
#define _EFI_EXT2_FS_DXE_H_

#include <Uefi.h>

#include <Guid/FileInfo.h>
#include <Guid/FileSystemInfo.h>
#include <Guid/FileSystemVolumeLabelInfo.h>
#include <Protocol/BlockIo.h>
#include <Protocol/DiskIo.h>
#include <Protocol/SimpleFileSystem.h>
#include <Protocol/UnicodeCollation.h>

#include <Library/PcdLib.h>
#include <Library/DebugLib.h>
#include <Library/UefiLib.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/UefiDriverEntryPoint.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>

#include <IndustryStandard/ExtFileSystem.h>

//
// The EXT2 signature
//
#define EXT2_VOLUME_SIGNATURE         SIGNATURE_32 ('e', 'x', 't', 'v')
#define EXT2_IFILE_SIGNATURE          SIGNATURE_32 ('e', 'x', 't', 'i')
#define EXT2_ODIR_SIGNATURE           SIGNATURE_32 ('e', 'x', 't', 'd')
#define EXT2_DIRENT_SIGNATURE         SIGNATURE_32 ('e', 'x', 't', 'e')
#define EXT2_OFILE_SIGNATURE          SIGNATURE_32 ('e', 'x', 't', 'o')

#define ASSERT_VOLUME_LOCKED(a)      ASSERT_LOCKED (&gExt2FsLock)

#define IFILE_FROM_FHAND(a)          CR (a, EXT2_IFILE, Handle, EXT2_IFILE_SIGNATURE)

#define DIRENT_FROM_LINK(a)          CR (a, EXT2_DIRENT, Link, EXT2_DIRENT_SIGNATURE)

#define VOLUME_FROM_ROOT_DIRENT(a)   CR (a, EXT2_VOLUME, RootDirEnt, EXT2_VOLUME_SIGNATURE)

#define VOLUME_FROM_VOL_INTERFACE(a) CR (a, EXT2_VOLUME, VolumeInterface, EXT2_VOLUME_SIGNATURE);

//
// The directory entry for opened directory
//
typedef struct _EXT2_DIRENT {
  UINTN               Signature;
  struct _EXT2_OFILE  *OFile;                 // The OFile of the corresponding directory entry
} EXT2_DIRENT;

typedef struct {
  UINTN               Signature;
  EFI_FILE_PROTOCOL   Handle;
  UINT64              Position;
  BOOLEAN             ReadOnly;
  struct _EXT2_OFILE  *OFile;
} EXT2_IFILE;

//
// FAT_OFILE - Each opened file
//
typedef struct _EXT2_OFILE {
  UINTN               Signature;
  struct _EXT2_VOLUME *Volume;

  EXT2FS_DINODE       Inode;
  BOOLEAN             IsDir;

  UINT64              PosDisk;  // on the disk
  UINTN               PosRem;   // remaining in this disk run

  EXT2FS_DIRECT       Entry;    // Describe itself, e.g. file name.

  CHAR16              *FullPath; // normalized full path
} EXT2_OFILE;

typedef struct _EXT2_VOLUME {
  UINTN                           Signature;

  EFI_HANDLE                      Handle;
  BOOLEAN                         Valid;
  BOOLEAN                         DiskError;

  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL VolumeInterface;

  //
  // If opened, the parent handle and BlockIo interface
  //
  EFI_BLOCK_IO_PROTOCOL           *BlockIo;
  EFI_DISK_IO_PROTOCOL            *DiskIo;
  UINT32                          MediaId;
  BOOLEAN                         ReadOnly;

  //
  // Ext2 fs info
  //
  UINT32                          BlockSize;

  EXT2_FS                         Ext2Fs;

  UINTN                           Ext2GdCount;
  EXT2_GD                         *Ext2Gd;

  EXT2_OFILE                      *Root;

} EXT2_VOLUME;

//
// Function Prototypes
//
EFI_STATUS
EFIAPI
Ext2Open (
  IN  EFI_FILE_PROTOCOL *FHand,
  OUT EFI_FILE_PROTOCOL **NewHandle,
  IN  CHAR16            *FileName,
  IN  UINT64            OpenMode,
  IN  UINT64            Attributes
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
;

EFI_STATUS
EFIAPI
Ext2GetPosition (
  IN  EFI_FILE_PROTOCOL *FHand,
  OUT UINT64            *Position
  )
/*++

Routine Description:

  Get the file's position of the file

Arguments:

  FHand                 - The handle of file.
  Position              - The file's position of the file.

Returns:

  EFI_SUCCESS           - Get the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.
  EFI_UNSUPPORTED       - The open file is not a file.

--*/
;

EFI_STATUS
EFIAPI
Ext2GetInfo (
  IN     EFI_FILE_PROTOCOL      *FHand,
  IN     EFI_GUID               *Type,
  IN OUT UINTN                  *BufferSize,
     OUT VOID                   *Buffer
  )
/*++

Routine Description:

  Get the some types info of the file into Buffer

Arguments:

  FHand                 - The handle of file.
  Type                  - The type of the info.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing volume info.

Returns:

  EFI_SUCCESS           - Get the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.

--*/
;

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

  Set the some types info of the file into Buffer

Arguments:

  FHand                 - The handle of file.
  Type                  - The type of the info.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing volume info.

Returns:

  EFI_SUCCESS           - Set the info successfully.
  EFI_DEVICE_ERROR      - Can not find the OFile for the file.

--*/
;

EFI_STATUS
EFIAPI
Ext2Flush (
  IN EFI_FILE_PROTOCOL  *FHand
  )
/*++

Routine Description:

  Flushes all data associated with the file handle

Arguments:

  FHand                 - Handle to file to flush

Returns:

  EFI_SUCCESS           - Flushed the file successfully
  EFI_WRITE_PROTECTED   - The volume is read only
  EFI_ACCESS_DENIED     - The volume is not read only
                          but the file is read only
  Others                - Flushing of the file is failed

--*/
;

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
;

EFI_STATUS
EFIAPI
Ext2Delete (
  IN EFI_FILE_PROTOCOL  *FHand
  )
/*++

Routine Description:

  Deletes the file & Closes the file handle.

Arguments:

  FHand                    - Handle to the file to delete.

Returns:

  EFI_SUCCESS              - Delete the file successfully.
  EFI_WARN_DELETE_FAILURE  - Fail to delete the file.

--*/
;

EFI_STATUS
EFIAPI
Ext2SetPosition (
  IN EFI_FILE_PROTOCOL  *FHand,
  IN UINT64             Position
  )
/*++

Routine Description:

  Set the file's position of the file

Arguments:

  FHand                 - The handle of file
  Position              - The file's position of the file

Returns:

  EFI_SUCCESS           - Set the info successfully
  EFI_DEVICE_ERROR      - Can not find the OFile for the file
  EFI_UNSUPPORTED       - Set a directory with a not-zero position

--*/
;

EFI_STATUS
EFIAPI
Ext2Read (
  IN     EFI_FILE_PROTOCOL    *FHand,
  IN OUT UINTN                *BufferSize,
     OUT VOID                 *Buffer
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
;

EFI_STATUS
EFIAPI
Ext2Write (
  IN     EFI_FILE_PROTOCOL      *FHand,
  IN OUT UINTN                  *BufferSize,
  IN     VOID                   *Buffer
  )
/*++

Routine Description:

  Set the file info.

Arguments:

  FHand                 - The handle of the file.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing write data.

Returns:

  EFI_SUCCESS           - Set the file info successfully.
  EFI_WRITE_PROTECTED   - The disk is write protected.
  EFI_ACCESS_DENIED     - The file is read-only.
  EFI_DEVICE_ERROR      - The OFile is not valid.
  EFI_UNSUPPORTED       - The open file is not a file.
                        - The writing file size is larger than 4GB.
  other                 - An error occurred when operation the disk.

--*/
;

//
// FileSpace.c
//
EFI_STATUS
Ext2OFilePosition (
  IN EXT2_OFILE           *OFile,
  IN UINT64               Position,
  IN UINTN                PosLimit
  );

//
// Info.c
//
EFI_STATUS
Ext2GetFileInfo (
  IN EXT2_OFILE       *OFile,
  IN OUT UINTN        *BufferSize,
  OUT VOID            *Buffer
  );

//
// Init.c
//
EFI_STATUS
Ext2AllocateVolume (
  IN  EFI_HANDLE                     Handle,
  IN  EFI_DISK_IO_PROTOCOL           *DiskIo,
  IN  EFI_BLOCK_IO_PROTOCOL          *BlockIo
  );

EFI_STATUS
Ext2OpenDevice (
  IN OUT EXT2_VOLUME     *Volume
  );

EFI_STATUS
Ext2AbandonVolume (
  IN EXT2_VOLUME         *Volume
  );

//
// Misc.c
//

EFI_STATUS
Ext2DiskIoRead (
  IN EXT2_VOLUME        *Volume,
  IN UINT64             Offset,
  IN UINTN              BufferSize,
  IN OUT VOID           *Buffer
  );

EFI_STATUS
Ext2InodeRead (
  IN     EXT2_VOLUME      *Volume,
  IN     UINT32           InodeNo,
  IN OUT VOID             *Inode
  );

EFI_STATUS
Ext2BlockRead (
  IN     EXT2_VOLUME      *Volume,
  IN     UINT32           BlockNo,
  IN OUT VOID             *BlockBuffer
  );

VOID
Ext2AcquireLock (
  VOID
  );

VOID
Ext2ReleaseLock (
  VOID
  );

EFI_STATUS
Ext2AcquireLockOrFail (
  VOID
  );

VOID
Ext2FreeVolume (
  IN EXT2_VOLUME         *Volume
  );

VOID
Ext2TimeToEfiTime (
  IN  UINT32            Ext2Time,
  OUT EFI_TIME          *ETime
  );

//
// UnicodeCollation.c
//
EFI_STATUS
InitializeUnicodeCollationSupport (
  IN EFI_HANDLE    AgentHandle
  );

VOID
Ext2FatToStr (
  IN UINTN              FatSize,
  IN CHAR8              *Fat,
  OUT CHAR16            *String
  );

BOOLEAN
Ext2StrToFat (
  IN  CHAR16            *String,
  IN  UINTN             FatSize,
  OUT CHAR8             *Fat
  );

VOID
Ext2StrLwr (
  IN CHAR16             *Str
  );

VOID
Ext2StrUpr (
  IN CHAR16             *Str
  );

INTN
Ext2StriCmp (
  IN CHAR16             *Str1,
  IN CHAR16             *Str2
  );

//
// Open.c
//
EFI_STATUS
Ext2AllocateIFile (
  IN EXT2_OFILE          *OFile,
  OUT EXT2_IFILE         **PtrIFile
  );

//
// OpenVolume.c
//
EFI_STATUS
EFIAPI
Ext2OpenVolume (
  IN  EFI_SIMPLE_FILE_SYSTEM_PROTOCOL *This,
  OUT EFI_FILE_PROTOCOL               **File
  );

//
// ReadWrite.c
//
EFI_STATUS
Ext2ReadOFile (
  IN EXT2_OFILE         *OFile,
  IN UINT64             Position,
  IN UINTN              *DataBufferSize,
  IN UINT8              *UserBuffer
  );


//
// DirectoryManage.c
//
EFI_STATUS
Ext2LocateFile (
  IN EXT2_OFILE         *ParentOFile,
  IN     CHAR16         *FileName,
     OUT EXT2_OFILE     **NewOFile
  );

//
// FileName.c
//
CHAR16*
Ext2GetNextNameComponent (
  IN  CHAR16            *Path,
  OUT CHAR16            *Name
  );

BOOLEAN
Ext2FileNameIsValid (
  IN  CHAR16  *InputFileName,
  OUT CHAR16  *OutputFileName
  );

//
// Other supported functions
//
VOID
Ext2DumpSuperBlock (
  IN EXT2_FS               *Ext2Fs
  );

VOID
Ext2DumpGroupDescriptor (
  IN EXT2_GD  *Ext2Gd
  );

VOID
Ext2DumpInode (
  IN EXT2FS_DINODE  *Ext2Inode
  );

VOID
Ext2DumpDirectory (
  IN EXT2FS_DIRECT  *Ext2Directory
  );

VOID
Ext2DumpBlock (
  IN UINT8 *Buffer
  );

BOOLEAN
Ext2IsAllZero (
  IN UINT8  *Buffer,
  IN UINTN  BufferSize
  );

//
// Global Variables
//
extern EFI_DRIVER_BINDING_PROTOCOL     gExt2DriverBinding;
extern EFI_COMPONENT_NAME_PROTOCOL     gExt2ComponentName;
extern EFI_COMPONENT_NAME2_PROTOCOL    gExt2ComponentName2;
extern EFI_LOCK                        gExt2FsLock;
extern EFI_FILE_PROTOCOL               gExt2FileInterface;

#endif

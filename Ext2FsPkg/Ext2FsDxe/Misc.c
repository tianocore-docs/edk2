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
Ext2DiskIoRead (
  IN     EXT2_VOLUME      *Volume,
  IN     UINT64           Offset,
  IN     UINTN            BufferSize,
  IN OUT VOID             *Buffer
  )
/*++

Routine Description:

  General disk access function

Arguments:

  Volume                - FAT file system volume.
  IoMode                - The access mode (disk read/write or cache access).
  Offset                - The starting byte offset to read from.
  BufferSize            - Size of Buffer.
  Buffer                - Buffer containing read data.

Returns:

  EFI_SUCCESS           - The operation is performed successfully.
  EFI_VOLUME_CORRUPTED  - The accesss is
  Others                - The status of read/write the disk

--*/
{
  EFI_STATUS            Status;
  EFI_DISK_IO_PROTOCOL  *DiskIo;

  //
  // Access disk directly
  //
  DiskIo      = Volume->DiskIo;
  Status      = DiskIo->ReadDisk (DiskIo, Volume->MediaId, Offset, BufferSize, Buffer);
  if (EFI_ERROR (Status)) {
    DEBUG ((EFI_D_INFO, "Ext2DiskIoRead: 0x%lx - error %r\n", Offset, Status));
  }

  return Status;
}

EFI_STATUS
Ext2InodeRead (
  IN     EXT2_VOLUME      *Volume,
  IN     UINT32           InodeNo,
  IN OUT VOID             *Inode
  )
{
  EFI_STATUS            Status;
  UINTN                 GroupNumber;
  UINTN                 InodeInGroup;

  if (InodeNo == 0) {
    ASSERT (FALSE);
  }

  ASSERT (InodeNo != 0);

  //
  // NOTE: e2d_ino is 1-based number, so e2d_ino == inode index + 1.
  //
  InodeNo --;

  GroupNumber = InodeNo / Volume->Ext2Fs.e2fs_ipg;
  InodeInGroup = InodeNo % Volume->Ext2Fs.e2fs_ipg;

  Status  = Ext2DiskIoRead (
              Volume,
              MultU64x32 (Volume->Ext2Gd[GroupNumber].ext2bgd_i_tables, Volume->BlockSize) + InodeInGroup * sizeof (EXT2FS_DINODE),
              sizeof (EXT2FS_DINODE),
              Inode
              );

  return Status;
}

EFI_STATUS
Ext2BlockRead (
  IN     EXT2_VOLUME      *Volume,
  IN     UINT32           BlockNo,
  IN OUT VOID             *BlockBuffer
  )
{
  EFI_STATUS            Status;

  Status  = Ext2DiskIoRead (
              Volume,
              BlockNo * Volume->BlockSize,
              Volume->BlockSize,
              BlockBuffer
              );

  return Status;
}

VOID
Ext2AcquireLock (
  VOID
  )
{
  EfiAcquireLock (&gExt2FsLock);
}

EFI_STATUS
Ext2AcquireLockOrFail (
  VOID
  )
{
  return EfiAcquireLockOrFail (&gExt2FsLock);
}

VOID
Ext2ReleaseLock (
  VOID
  )
{
  EfiReleaseLock (&gExt2FsLock);
}

VOID
Ext2FreeVolume (
  IN EXT2_VOLUME       *Volume
  )
/*++

Routine Description:

  Free volume structure (including the contents of directory cache and disk cache).

Arguments:

  Volume                - The volume structure to be freed.

Returns:

  None.

--*/
{
  FreePool (Volume);
}

//
// -- Time Management Routines --
//

#define IsLeap(y)   (((y) % 4) == 0 && (((y) % 100) != 0 || ((y) % 400) == 0))
#define SECSPERMIN  (60)
#define SECSPERHOUR (60 * 60)
#define SECSPERDAY  (24 * SECSPERHOUR)

//
//  The arrays give the cumulative number of days up to the first of the
//  month number used as the index (1 -> 12) for regular and leap years.
//  The value at index 13 is for the whole year.
//
UINTN CumulativeDays[2][14] = {
  {
    0,
    0,
    31,
    31 + 28,
    31 + 28 + 31,
    31 + 28 + 31 + 30,
    31 + 28 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31
  },
  {
    0,
    0,
    31,
    31 + 29,
    31 + 29 + 31,
    31 + 29 + 31 + 30,
    31 + 29 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30,
    31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31 
  }
};

VOID
Ext2TimeToEfiTime (
  IN  UINT32            Ext2Time,
  OUT EFI_TIME          *ETime
  )
/*++

Routine Description:

  Translate Fat time to EFI time.

Arguments:

  FTime                 - The time of EXT2 Time.
  ETime                 - The time of EFI_TIME.

Returns:

  None.

--*/
{
  UINT16     DayNo;
  UINT16     DayRemainder;
  UINT32     Year;
  UINT32     YearNo;
  UINT16     TotalDays;
  UINT16     MonthNo;

  DayNo        = (UINT16) (Ext2Time / SECSPERDAY);
  DayRemainder = (UINT16) (Ext2Time % SECSPERDAY);

  for (Year = 1970, YearNo = 0; DayNo > 0; Year++) {
    TotalDays = (UINT16) (IsLeap (Year) ? 366 : 365);
    if (DayNo >= TotalDays) {
      DayNo = (UINT16) (DayNo - TotalDays);
      YearNo++;
    } else {
      break;
    }
  }

  for (MonthNo = 12; MonthNo > 1; MonthNo--) {
    if (DayNo >= CumulativeDays[IsLeap(Year)][MonthNo]) {
      DayNo = (UINT16) (DayNo - (UINT16) (CumulativeDays[IsLeap(Year)][MonthNo]));
      break;
    }
  }

  ETime->Year       = (UINT16) (YearNo + 1970);
  ETime->Month      = (UINT8) MonthNo;
  ETime->Day        = (UINT8) DayNo + 1;

  ETime->Hour       = (UINT8) (DayRemainder / SECSPERHOUR);
  ETime->Minute     = (UINT8) ((DayRemainder % SECSPERHOUR) / SECSPERMIN);
  ETime->Second     = (UINT8) (DayRemainder % SECSPERMIN);

  ETime->Nanosecond = 0;
  ETime->TimeZone   = EFI_UNSPECIFIED_TIMEZONE;
  ETime->Daylight   = 0;
}

typedef struct {
  UINT32 Value;
  CHAR8  *Name;
} EXT2_NAME_VALUE_STRUCT;

VOID
Ext2DumpSuperBlock (
  IN EXT2_FS               *Ext2Fs
  )
{
  DEBUG ((EFI_D_INFO, "ext2fs:\n"));
  DEBUG ((EFI_D_INFO, "  e2fs_icount            - 0x%08x\n", Ext2Fs->e2fs_icount));
  DEBUG ((EFI_D_INFO, "  e2fs_bcount            - 0x%08x\n", Ext2Fs->e2fs_bcount));
  DEBUG ((EFI_D_INFO, "  e2fs_rbcount           - 0x%08x\n", Ext2Fs->e2fs_rbcount));
  DEBUG ((EFI_D_INFO, "  e2fs_fbcount           - 0x%08x\n", Ext2Fs->e2fs_fbcount));
  DEBUG ((EFI_D_INFO, "  e2fs_ficount           - 0x%08x\n", Ext2Fs->e2fs_ficount));
  DEBUG ((EFI_D_INFO, "  e2fs_first_dblock      - 0x%08x\n", Ext2Fs->e2fs_first_dblock));
  DEBUG ((EFI_D_INFO, "  e2fs_log_bsize         - 0x%08x\n", Ext2Fs->e2fs_log_bsize));
  DEBUG ((EFI_D_INFO, "  e2fs_log_fsize         - 0x%08x\n", Ext2Fs->e2fs_log_fsize));
  DEBUG ((EFI_D_INFO, "  e2fs_bpg               - 0x%08x\n", Ext2Fs->e2fs_bpg));
  DEBUG ((EFI_D_INFO, "  e2fs_fpg               - 0x%08x\n", Ext2Fs->e2fs_fpg));
  DEBUG ((EFI_D_INFO, "  e2fs_ipg               - 0x%08x\n", Ext2Fs->e2fs_ipg));
  DEBUG ((EFI_D_INFO, "  e2fs_mtime             - 0x%08x\n", Ext2Fs->e2fs_mtime));
  DEBUG ((EFI_D_INFO, "  e2fs_wtime             - 0x%08x\n", Ext2Fs->e2fs_wtime));
  DEBUG ((EFI_D_INFO, "  e2fs_mnt_count         - 0x%04x\n", Ext2Fs->e2fs_mnt_count));
  DEBUG ((EFI_D_INFO, "  e2fs_max_mnt_count     - 0x%04x\n", Ext2Fs->e2fs_max_mnt_count));
  DEBUG ((EFI_D_INFO, "  e2fs_magic             - 0x%04x\n", Ext2Fs->e2fs_magic));
  DEBUG ((EFI_D_INFO, "  e2fs_state             - 0x%04x\n", Ext2Fs->e2fs_state));
  DEBUG ((EFI_D_INFO, "  e2fs_beh               - 0x%04x\n", Ext2Fs->e2fs_beh));
  DEBUG ((EFI_D_INFO, "  e2fs_minrev            - 0x%04x\n", Ext2Fs->e2fs_minrev));
  DEBUG ((EFI_D_INFO, "  e2fs_lastfsck          - 0x%08x\n", Ext2Fs->e2fs_lastfsck));
  DEBUG ((EFI_D_INFO, "  e2fs_fsckintv          - 0x%08x\n", Ext2Fs->e2fs_fsckintv));
  DEBUG ((EFI_D_INFO, "  e2fs_creator           - 0x%08x\n", Ext2Fs->e2fs_creator));
  DEBUG ((EFI_D_INFO, "  e2fs_rev               - 0x%08x\n", Ext2Fs->e2fs_rev));
  DEBUG ((EFI_D_INFO, "  e2fs_ruid              - 0x%04x\n", Ext2Fs->e2fs_ruid));
  DEBUG ((EFI_D_INFO, "  e2fs_rgid              - 0x%04x\n", Ext2Fs->e2fs_rgid));
  DEBUG ((EFI_D_INFO, "  e2fs_first_ino         - 0x%08x\n", Ext2Fs->e2fs_first_ino));
  DEBUG ((EFI_D_INFO, "  e2fs_inode_size        - 0x%04x\n", Ext2Fs->e2fs_inode_size));
  DEBUG ((EFI_D_INFO, "  e2fs_block_group_nr    - 0x%04x\n", Ext2Fs->e2fs_block_group_nr));
  DEBUG ((EFI_D_INFO, "  e2fs_features_compat   - 0x%08x\n", Ext2Fs->e2fs_features_compat));
  DEBUG ((EFI_D_INFO, "  e2fs_features_incompat - 0x%08x\n", Ext2Fs->e2fs_features_incompat));
  DEBUG ((EFI_D_INFO, "  e2fs_features_rocompat - 0x%08x\n", Ext2Fs->e2fs_features_rocompat));
  DEBUG ((EFI_D_INFO, "  e2fs_uuid              - %g\n", Ext2Fs->e2fs_uuid));
  DEBUG ((EFI_D_INFO, "  e2fs_vname             - '"));
  {
    UINTN Index;
    for (Index = 0; Index < sizeof(Ext2Fs->e2fs_vname); Index++) {
      DEBUG ((EFI_D_INFO, "%c", Ext2Fs->e2fs_vname[Index]));
    }
    DEBUG ((EFI_D_INFO, "'\n"));
  }
  DEBUG ((EFI_D_INFO, "  e2fs_fsmnt             - '"));
  {
    UINTN Index;
    for (Index = 0; Index < sizeof(Ext2Fs->e2fs_fsmnt); Index++) {
      DEBUG ((EFI_D_INFO, "%c", Ext2Fs->e2fs_fsmnt[Index]));
    }
    DEBUG ((EFI_D_INFO, "'\n"));
  }
  DEBUG ((EFI_D_INFO, "  e2fs_algo              - 0x%08x\n", Ext2Fs->e2fs_algo));
  DEBUG ((EFI_D_INFO, "  e2fs_prealloc          - 0x%02x\n", Ext2Fs->e2fs_prealloc));
  DEBUG ((EFI_D_INFO, "  e2fs_dir_prealloc      - 0x%02x\n", Ext2Fs->e2fs_dir_prealloc));
}

VOID
Ext2DumpGroupDescriptor (
  IN EXT2_GD  *Ext2Gd
  )
{
  DEBUG ((EFI_D_INFO, "ext2_gd:\n"));
  DEBUG ((EFI_D_INFO, "  ext2bgd_b_bitmap       - 0x%08x\n", Ext2Gd->ext2bgd_b_bitmap));
  DEBUG ((EFI_D_INFO, "  ext2bgd_i_bitmap       - 0x%08x\n", Ext2Gd->ext2bgd_i_bitmap));
  DEBUG ((EFI_D_INFO, "  ext2bgd_i_tables       - 0x%08x\n", Ext2Gd->ext2bgd_i_tables));
  DEBUG ((EFI_D_INFO, "  ext2bgd_nbfree         - 0x%04x\n", Ext2Gd->ext2bgd_nbfree));
  DEBUG ((EFI_D_INFO, "  ext2bgd_nifree         - 0x%04x\n", Ext2Gd->ext2bgd_nifree));
  DEBUG ((EFI_D_INFO, "  ext2bgd_ndirs          - 0x%04x\n", Ext2Gd->ext2bgd_ndirs));
  DEBUG ((EFI_D_INFO, "  ext4bgd_flags          - 0x%04x\n", Ext2Gd->ext4bgd_flags));
  DEBUG ((EFI_D_INFO, "  ext4bgd_x_bitmap       - 0x%08x\n", Ext2Gd->ext4bgd_x_bitmap));
  DEBUG ((EFI_D_INFO, "  ext4bgd_b_bmap_csum    - 0x%04x\n", Ext2Gd->ext4bgd_b_bmap_csum));
  DEBUG ((EFI_D_INFO, "  ext4bgd_i_bmap_csum    - 0x%04x\n", Ext2Gd->ext4bgd_i_bmap_csum));
  DEBUG ((EFI_D_INFO, "  ext4bgd_i_unused       - 0x%04x\n", Ext2Gd->ext4bgd_i_unused));
  DEBUG ((EFI_D_INFO, "  ext4bgd_csum           - 0x%04x\n", Ext2Gd->ext4bgd_csum));
}

GLOBAL_REMOVE_IF_UNREFERENCED EXT2_NAME_VALUE_STRUCT  mExt2InodeFileType[] = {
  {S_IFIFO,         "fifo"},
  {S_IFCHR,         "char special"},
  {S_IFDIR,         "directory"},
  {S_IFBLK,         "block special"},
  {S_IFREG,         "regular file"},
  {S_IFLNK,         "symbolic link"},
  {S_IFSOCK,        "socket"},
  {S_IFWHT,         "whiteout"},
};

VOID
Ext2DumpInodeFileType (
  IN UINT16  e2di_mode
  )
{
  UINTN  Index;

  DEBUG ((EFI_D_INFO, "    "));
  for (Index = 0; Index < sizeof(mExt2InodeFileType)/sizeof(mExt2InodeFileType[0]); Index++) {
    if ((e2di_mode & S_IFMT) == mExt2InodeFileType[Index].Value) {
      DEBUG ((EFI_D_INFO, "%a", mExt2InodeFileType[Index].Name));
    }
  }
  DEBUG ((EFI_D_INFO, "\n"));
}

GLOBAL_REMOVE_IF_UNREFERENCED EXT2_NAME_VALUE_STRUCT  mExt2InodeFlags[] = {
  {EXT2_SECRM,         "SECRM"},
  {EXT2_UNRM,          "UNRM"},
  {EXT2_COMPR,         "COMPR"},
  {EXT2_SYNC,          "SYNC"},
  {EXT2_IMMUTABLE,     "IMMUTABLE"},
  {EXT2_APPEND,        "APPEND"},
  {EXT2_NODUMP,        "NODUMP"},
  {EXT2_NOATIME,       "NOATIME"},
  {EXT4_INDEX,         "INDEX"},
  {EXT4_IMAGIC,        "IMAGIC"},
  {EXT4_JOURNAL_DATA,  "JOURNAL_DATA"},
  {EXT4_NOTAIL,        "NOTAIL"},
  {EXT4_DIRSYNC,       "DIRSYNC"},
  {EXT4_TOPDIR,        "TOPDIR"},
  {EXT4_HUGE_FILE,     "HUGE_FILE"},
  {EXT4_EXTENTS,       "EXTENTS"},
  {EXT4_EOFBLOCKS,     "EOFBLOCKS"},
};

VOID
Ext2DumpInodeFlags (
  IN UINT32  e2di_flags
  )
{
  UINTN   Index;

  if (e2di_flags == 0) {
    return ;
  }

  DEBUG ((EFI_D_INFO, "    "));
  for (Index = 0; Index < sizeof(mExt2InodeFlags)/sizeof(mExt2InodeFlags[0]); Index++) {
    if ((e2di_flags & mExt2InodeFlags[Index].Value) != 0) {
      DEBUG ((EFI_D_INFO, "|%a", mExt2InodeFlags[Index].Name));
    }
  }
  DEBUG ((EFI_D_INFO, "\n"));
}

VOID
Ext2DumpInodeExtentHeader (
  IN EXT4_EXTENT_HEADER  *Ext4ExtHeader
  )
{
  DEBUG ((EFI_D_INFO, "  ext4_extent_header:\n"));
  DEBUG ((EFI_D_INFO, "    eh_magic             - 0x%04x\n", Ext4ExtHeader->eh_magic));
  DEBUG ((EFI_D_INFO, "    eh_ecount            - 0x%04x\n", Ext4ExtHeader->eh_ecount));
  DEBUG ((EFI_D_INFO, "    eh_max               - 0x%04x\n", Ext4ExtHeader->eh_max));
  DEBUG ((EFI_D_INFO, "    eh_depth             - 0x%04x\n", Ext4ExtHeader->eh_depth));
  DEBUG ((EFI_D_INFO, "    eh_gen               - 0x%08x\n", Ext4ExtHeader->eh_gen));
}

VOID
Ext2DumpInodeExtent (
  IN EXT4_EXTENT  *Ext4Ext
  )
{
  DEBUG ((EFI_D_INFO, "  ext4_extent:\n"));
  DEBUG ((EFI_D_INFO, "    e_blk                - 0x%08x\n", Ext4Ext->e_blk));
  DEBUG ((EFI_D_INFO, "    e_len                - 0x%04x\n", Ext4Ext->e_len));
  DEBUG ((EFI_D_INFO, "    e_start_hi           - 0x%04x\n", Ext4Ext->e_start_hi));
  DEBUG ((EFI_D_INFO, "    e_start_lo           - 0x%08x\n", Ext4Ext->e_start_lo));
}

VOID
Ext2DumpInodeExtentIndex (
  IN EXT4_EXTENT_INDEX  *Ext4ExtIndex
  )
{
  DEBUG ((EFI_D_INFO, "  ext4_extent_index:\n"));
  DEBUG ((EFI_D_INFO, "    ei_blk               - 0x%08x\n", Ext4ExtIndex->ei_blk));
  DEBUG ((EFI_D_INFO, "    ei_leaf_lo           - 0x%08x\n", Ext4ExtIndex->ei_leaf_lo));
  DEBUG ((EFI_D_INFO, "    ei_leaf_hi           - 0x%04x\n", Ext4ExtIndex->ei_leaf_hi));
}

VOID
Ext2DumpInode (
  IN EXT2FS_DINODE  *Ext2Inode
  )
{
  UINTN               Index;
  EXT4_EXTENT_HEADER  *Ext4ExtHeader;
  EXT4_EXTENT         *Ext4Ext;
  EXT4_EXTENT_INDEX   *Ext4ExtIndex;

  DEBUG ((EFI_D_INFO, "ext2fs_dinode:\n"));
  DEBUG ((EFI_D_INFO, "  e2di_mode              - 0x%04x\n", Ext2Inode->e2di_mode));
  Ext2DumpInodeFileType (Ext2Inode->e2di_mode);
  DEBUG ((EFI_D_INFO, "  e2di_uid               - 0x%04x\n", Ext2Inode->e2di_uid));
  DEBUG ((EFI_D_INFO, "  e2di_size              - 0x%08x\n", Ext2Inode->e2di_size));
  DEBUG ((EFI_D_INFO, "  e2di_atime             - 0x%08x\n", Ext2Inode->e2di_atime));
  DEBUG ((EFI_D_INFO, "  e2di_ctime             - 0x%08x\n", Ext2Inode->e2di_ctime));
  DEBUG ((EFI_D_INFO, "  e2di_mtime             - 0x%08x\n", Ext2Inode->e2di_mtime));
  DEBUG ((EFI_D_INFO, "  e2di_dtime             - 0x%08x\n", Ext2Inode->e2di_dtime));
  DEBUG ((EFI_D_INFO, "  e2di_gid               - 0x%04x\n", Ext2Inode->e2di_gid));
  DEBUG ((EFI_D_INFO, "  e2di_nlink             - 0x%04x\n", Ext2Inode->e2di_nlink));
  DEBUG ((EFI_D_INFO, "  e2di_nblock            - 0x%08x\n", Ext2Inode->e2di_nblock));
  DEBUG ((EFI_D_INFO, "  e2di_flags             - 0x%08x\n", Ext2Inode->e2di_flags));
  Ext2DumpInodeFlags (Ext2Inode->e2di_flags);
  DEBUG ((EFI_D_INFO, "  e2di_version           - 0x%08x\n", Ext2Inode->e2di_version));
  if ((Ext2Inode->e2di_flags & EXT4_EXTENTS) == 0) {
    for (Index = 0; Index < EXT2_N_BLOCKS; Index++) {
      DEBUG ((EFI_D_INFO, "  e2di_blocks[%x]         - 0x%08x\n", Index, Ext2Inode->e2di_blocks[Index]));
    }
  } else {
    Ext4ExtHeader = (EXT4_EXTENT_HEADER *)&Ext2Inode->e2di_blocks[0];
    Ext2DumpInodeExtentHeader (Ext4ExtHeader);
    Ext4Ext = (EXT4_EXTENT *)(Ext4ExtHeader + 1);
    Ext4ExtIndex = (EXT4_EXTENT_INDEX *)Ext4Ext;
    for (Index = 0; (Index < 4) && (Index < Ext4ExtHeader->eh_ecount); Index++) {
      if (Ext4ExtHeader->eh_depth == 0) {
        Ext2DumpInodeExtent (&Ext4Ext[Index]);
      } else {
        Ext2DumpInodeExtentIndex (&Ext4ExtIndex[Index]);
      }
    }
  }
  DEBUG ((EFI_D_INFO, "  e2di_gen               - 0x%08x\n", Ext2Inode->e2di_gen));
  DEBUG ((EFI_D_INFO, "  e2di_facl              - 0x%08x\n", Ext2Inode->e2di_facl));
  DEBUG ((EFI_D_INFO, "  e2di_size_high         - 0x%08x\n", Ext2Inode->e2di_size_high));
  DEBUG ((EFI_D_INFO, "  e2di_faddr             - 0x%08x\n", Ext2Inode->e2di_faddr));
}

GLOBAL_REMOVE_IF_UNREFERENCED EXT2_NAME_VALUE_STRUCT  mExt2DirectoryFileType[] = {
  {EXT2_FT_UNKNOWN,     "Unknown"},
  {EXT2_FT_REG_FILE,    "regular file"},
  {EXT2_FT_DIR,         "directory"},
  {EXT2_FT_CHRDEV,      "char device"},
  {EXT2_FT_BLKDEV,      "block device"},
  {EXT2_FT_FIFO,        "fifo"},
  {EXT2_FT_SOCK,        "socket"},
  {EXT2_FT_SYMLINK,     "symbolic link"},
};

VOID
Ext2DumpDirectoryFileType (
  IN UINT8  e2d_type
  )
{
  UINTN  Index;

  DEBUG ((EFI_D_INFO, "    "));
  for (Index = 0; Index < sizeof(mExt2DirectoryFileType)/sizeof(mExt2DirectoryFileType[0]); Index++) {
    if (e2d_type == mExt2DirectoryFileType[Index].Value) {
      DEBUG ((EFI_D_INFO, "%a", mExt2DirectoryFileType[Index].Name));
    }
  }
  DEBUG ((EFI_D_INFO, "\n"));
}

VOID
Ext2DumpDirectory (
  IN EXT2FS_DIRECT  *Ext2Directory
  )
{
  DEBUG ((EFI_D_INFO, "ext2fs_direct:\n"));
  DEBUG ((EFI_D_INFO, "  e2d_ino                - 0x%08x\n", Ext2Directory->e2d_ino));
  DEBUG ((EFI_D_INFO, "  e2d_reclen             - 0x%04x\n", Ext2Directory->e2d_reclen));
  DEBUG ((EFI_D_INFO, "  e2d_namlen             - 0x%02x\n", Ext2Directory->e2d_namlen));
  DEBUG ((EFI_D_INFO, "  e2d_type               - 0x%02x\n", Ext2Directory->e2d_type));
  Ext2DumpDirectoryFileType (Ext2Directory->e2d_type);
  DEBUG ((EFI_D_INFO, "  e2d_name               - '"));
  {
    UINTN Index;
    for (Index = 0; Index < Ext2Directory->e2d_namlen; Index++) {
      DEBUG ((EFI_D_INFO, "%c", Ext2Directory->e2d_name[Index]));
    }
    DEBUG ((EFI_D_INFO, "'\n"));
  }
}

VOID
Ext2DumpBlock (
  IN UINT8 *Buffer
  )
{
  UINTN  Index;
  UINTN  SubIndex;

#define BYTE_PER_LINE  16

  for (Index = 0; Index < 1024 / BYTE_PER_LINE; Index++) {
    for (SubIndex = 0; SubIndex < BYTE_PER_LINE; SubIndex++) {
      DEBUG ((EFI_D_INFO, "%02x ", Buffer[Index * BYTE_PER_LINE + SubIndex]));
    }
    DEBUG ((EFI_D_INFO, "\n"));
  }
}

BOOLEAN
Ext2IsAllZero (
  IN UINT8  *Buffer,
  IN UINTN  BufferSize
  )
{
  UINTN  Index;
  for (Index = 0; Index < BufferSize; Index++) {
    if (Buffer[0] != 0) {
      return FALSE;
    }
  }
  return TRUE;
}

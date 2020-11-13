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

#define EXT2_NDIR_BLOCKS_COUNT  (EXT2_NDIR_BLOCKS)
#define EXT2_IND_BLOCKS_COUNT   (EXT2_NDIR_BLOCKS_COUNT + (EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32)))
#define EXT2_DIND_BLOCKS_COUNT  (EXT2_IND_BLOCKS_COUNT  + (UINT64)(EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32)) * (EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32)))
#define EXT2_TIND_BLOCKS_COUNT  (EXT2_DIND_BLOCKS_COUNT + (UINT64)(EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32)) * (EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32)) * (EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32)))

static UINT32 mTempBuffer[EXT2_DEFAULT_BLOCK_SIZE/sizeof(UINT32)];

EFI_STATUS
Ext2OFilePositionTradition (
  IN EXT2_OFILE           *OFile,
  IN UINT64               Position,
  IN UINTN                PosLimit
  )
/*++

Routine Description:

  Seek OFile to requested position, and calculate the number of
  consecutive clusters from the position in the file

Arguments:

  OFile                 - The open file.
  Position              - The file's position which will be accessed.
  PosLimit              - The maximum length current reading/writing may access

Returns:

  EFI_SUCCESS           - Set the info successfully.
  EFI_VOLUME_CORRUPTED  - Cluster chain corrupt.

--*/
{
  EXT2_VOLUME  *Volume;
  UINTN        BlockNumber;
  UINTN        BlockOffset;
  EFI_STATUS   Status;

  Volume      = OFile->Volume;

  ASSERT_VOLUME_LOCKED (Volume);

  BlockNumber = DivU64x32 (Position, Volume->BlockSize);
  BlockOffset = 0;

  if (BlockNumber < EXT2_NDIR_BLOCKS_COUNT) {
    // direct
    BlockOffset = OFile->Inode.e2di_blocks[BlockNumber];
    if (BlockOffset == 0) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - direct(0x%x) error\n", BlockOffset));
      return EFI_VOLUME_CORRUPTED;
    }

  } else if (BlockNumber < EXT2_IND_BLOCKS_COUNT) {
    // indirect
    BlockNumber -= EXT2_NDIR_BLOCKS_COUNT;
    Status = Ext2BlockRead (
               Volume,
               OFile->Inode.e2di_blocks[EXT2_IND_BLOCK],
               mTempBuffer
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - indirect(0x%x) error\n", OFile->Inode.e2di_blocks[EXT2_IND_BLOCK]));
      return Status;
    }
    BlockOffset = mTempBuffer[BlockNumber];
    if (BlockOffset == 0) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - indirect2(0x%x) error\n", BlockOffset));
      return EFI_VOLUME_CORRUPTED;
    }

  } else if (BlockNumber < EXT2_DIND_BLOCKS_COUNT) {
    // double indirect
    BlockNumber -= EXT2_IND_BLOCKS_COUNT;
    Status = Ext2BlockRead (
               Volume,
               OFile->Inode.e2di_blocks[EXT2_DIND_BLOCK],
               mTempBuffer
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - double indirect(0x%x) error\n", OFile->Inode.e2di_blocks[EXT2_DIND_BLOCK]));
      return Status;
    }
    BlockOffset = mTempBuffer[BlockNumber/(EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32))];
    if (BlockOffset == 0) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - double indirect2(0x%x) error\n", BlockOffset));
      return EFI_VOLUME_CORRUPTED;
    }
    BlockNumber %= (EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32));

    Status = Ext2BlockRead (
               Volume,
               BlockOffset,
               mTempBuffer
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - double indirect3(0x%x) error\n", BlockOffset));
      return Status;
    }
    BlockOffset = mTempBuffer[BlockNumber];
    if (BlockOffset == 0) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - double indirect4(0x%x) error\n", BlockOffset));
      return EFI_VOLUME_CORRUPTED;
    }

  } else if (BlockNumber < EXT2_TIND_BLOCKS_COUNT) {
    // triple indirect
    BlockNumber -= EXT2_DIND_BLOCKS_COUNT;
    Status = Ext2BlockRead (
               Volume,
               OFile->Inode.e2di_blocks[EXT2_TIND_BLOCK],
               mTempBuffer
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - triple indirect(0x%x) error\n", OFile->Inode.e2di_blocks[EXT2_TIND_BLOCK]));
      return Status;
    }
    BlockOffset = mTempBuffer[BlockNumber/((EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32)) * (EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32)))];
    if (BlockOffset == 0) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - triple indirect2(0x%x) error\n", BlockOffset));
      return EFI_VOLUME_CORRUPTED;
    }
    BlockNumber %= ((EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32)) * (EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32)));

    Status = Ext2BlockRead (
               Volume,
               BlockOffset,
               mTempBuffer
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - triple indirect3(0x%x) error\n", BlockOffset));
      return Status;
    }
    BlockOffset = mTempBuffer[BlockNumber/(EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32))];
    if (BlockOffset == 0) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - triple indirect4(0x%x) error\n", BlockOffset));
      return EFI_VOLUME_CORRUPTED;
    }
    BlockNumber %= (EXT2_DEFAULT_BLOCK_SIZE / sizeof(UINT32));

    Status = Ext2BlockRead (
               Volume,
               BlockOffset,
               mTempBuffer
               );
    if (EFI_ERROR (Status)) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - triple indirect5(0x%x) error\n", BlockOffset));
      return Status;
    }
    BlockOffset = mTempBuffer[BlockNumber];
    if (BlockOffset == 0) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - triple indirect6(0x%x) error\n", BlockOffset));
      return EFI_VOLUME_CORRUPTED;
    }
  } else {
    DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - block too large (0x%x)\n", BlockNumber));
    return EFI_OUT_OF_RESOURCES;
  }

  OFile->PosDisk = MultU64x32 (BlockOffset, Volume->BlockSize) + ModU64x32 (Position, Volume->BlockSize);
  OFile->PosRem = Volume->BlockSize - ModU64x32 (Position, Volume->BlockSize);

  return EFI_SUCCESS;
}

EFI_STATUS
Ext2OFilePositionExtent (
  IN EXT2_OFILE           *OFile,
  IN UINT64               Position,
  IN UINTN                PosLimit
  )
{
  EXT2_VOLUME         *Volume;
  UINTN               BlockNumber;
  UINTN               BlockOffset;
  EFI_STATUS          Status;
  EXT4_EXTENT_HEADER  *Ext4ExtHeader;
  EXT4_EXTENT         *Ext4Ext;
  EXT4_EXTENT_INDEX   *Ext4ExtIndex;
  UINTN               Index;
  UINTN               MaxCount;

  Volume      = OFile->Volume;

  ASSERT_VOLUME_LOCKED (Volume);

  BlockNumber = DivU64x32 (Position, Volume->BlockSize);
  BlockOffset = 0;

  Ext4ExtHeader = (EXT4_EXTENT_HEADER *)&OFile->Inode.e2di_blocks[0];
  MaxCount = 4;

  while (TRUE) {
    if (Ext4ExtHeader->eh_magic != EXT4_EXT_MAGIC) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionExtent - eh_magic (0x%x) error\n", Ext4ExtHeader->eh_magic));
      return EFI_UNSUPPORTED;
    }
    if (Ext4ExtHeader->eh_ecount > MaxCount) {
      DEBUG ((EFI_D_ERROR, "Ext2OFilePositionExtent - eh_ecount (0x%x) error\n", Ext4ExtHeader->eh_ecount));
      return EFI_UNSUPPORTED;
    }
    if (Ext4ExtHeader->eh_depth == 0) {
      Ext4Ext = (EXT4_EXTENT *)(Ext4ExtHeader + 1);
      for (Index = 0; Index < Ext4ExtHeader->eh_ecount; Index++) {
        if ((BlockNumber >= Ext4Ext[Index].e_blk) &&
            (BlockNumber < Ext4Ext[Index].e_blk + Ext4Ext[Index].e_len)) {
          BlockOffset = (BlockNumber - Ext4Ext[Index].e_blk) + Ext4Ext[Index].e_start_lo + RShiftU64 (Ext4Ext[Index].e_start_hi, 32);
          break;
        }
      }

      if (BlockOffset == 0) {
        DEBUG ((EFI_D_ERROR, "Ext2OFilePositionExtent - block not found in Ext (0x%x) error\n", BlockNumber));
        return EFI_VOLUME_CORRUPTED;
      }

      //
      // Leaf node, finish
      //
      break;

    } else {
      Ext4ExtIndex = (EXT4_EXTENT_INDEX *)(Ext4ExtHeader + 1);
      for (Index = 0; Index < Ext4ExtHeader->eh_ecount; Index++) {
        if (BlockNumber >= Ext4ExtIndex[Index].ei_blk) {
          BlockOffset = Ext4ExtIndex[Index].ei_leaf_lo + RShiftU64 (Ext4ExtIndex[Index].ei_leaf_hi, 32);
        }
      }

      if (BlockOffset == 0) {
        DEBUG ((EFI_D_ERROR, "Ext2OFilePositionExtent - block not found in ExtIndex (0x%x) error\n", BlockNumber));
        return EFI_VOLUME_CORRUPTED;
      }

      Status = Ext2BlockRead (
                 Volume,
                 BlockOffset,
                 mTempBuffer
                 );
      if (EFI_ERROR (Status)) {
        DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - ExtIndex read (0x%x) error\n", BlockOffset));
        return Status;
      }

      Ext4ExtHeader = (EXT4_EXTENT_HEADER *)mTempBuffer;

      if (Ext4ExtHeader->eh_depth <= ((EXT4_EXTENT_HEADER *)mTempBuffer)->eh_depth) {
        DEBUG ((EFI_D_ERROR, "Ext2OFilePositionTradition - old eh_depth(0x%x) <= new eh_depth(0x%x)\n", Ext4ExtHeader->eh_depth, ((EXT4_EXTENT_HEADER *)mTempBuffer)->eh_depth));
        return EFI_UNSUPPORTED;
      }

      MaxCount = (Volume->BlockSize / sizeof(EXT4_EXTENT)) - 1;
      //
      // index node, continue
      //
      continue;
    }
  }

  OFile->PosDisk = MultU64x32 (BlockOffset, Volume->BlockSize) + ModU64x32 (Position, Volume->BlockSize);
  OFile->PosRem = Volume->BlockSize - ModU64x32 (Position, Volume->BlockSize);

  return EFI_SUCCESS;
}

EFI_STATUS
Ext2OFilePosition (
  IN EXT2_OFILE           *OFile,
  IN UINT64               Position,
  IN UINTN                PosLimit
  )
{
  if ((OFile->Inode.e2di_flags & EXT4_EXTENTS) == 0) {
    return Ext2OFilePositionTradition (OFile, Position, PosLimit);
  } else {
    return Ext2OFilePositionExtent (OFile, Position, PosLimit);
  }

}
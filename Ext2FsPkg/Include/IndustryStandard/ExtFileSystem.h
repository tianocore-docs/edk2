/**@file

Copyright (c) 2014, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials                          
are licensed and made available under the terms and conditions of the BSD License         
which accompanies this distribution.  The full text of the license may be found at        
http://opensource.org/licenses/bsd-license.php                                            
                                                                                          
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef __EXT_FILE_SYSTEM_H__
#define __EXT_FILE_SYSTEM_H__

/**

  EXT2 layout:
  +-----------------------+
  | Boot Block (1)        |
  +-----------------------+ --
  | Super Block (1)       |  |
  +-----------------------+  |
  | Group Descriptor (n)  |  |
  +-----------------------+  |
  | Data Block Bitmap (1) |  |
  +-----------------------+  |-> Block Group 0
  | Inode Bitmap (1)      |  |
  +-----------------------+  |
  | Inode Table (n)       |  |
  +-----------------------+  |
  | Data Blocks (n)       |  |
  +-----------------------+ --
  | Super Block (1)       |  |
  +-----------------------+  |
  | Group Descriptor (n)  |  |
  +-----------------------+  |
  | Data Block Bitmap (1) |  |
  +-----------------------+  |-> Block Group N
  | Inode Bitmap (1)      |  |
  +-----------------------+  |
  | Inode Table (n)       |  |
  +-----------------------+  |
  | Data Blocks (n)       |  |
  +-----------------------+ --

   Inode layout:
  +-----------------------+
  | mode                  |
  +-----------------------+
  | size                  |
  +-----------------------+
  | time                  |
  +-----------------------+       ------
  | e2di_blocks[0-11]     | ---> | Data | (Direct Blocks)
  +-----------------------+       ------       ------
  | e2di_blocks[12]       | ---> POINTER ---> | Data | (Indirect Blocks)
  +-----------------------+                    ------       ------
  | e2di_blocks[13]       | ---> POINTER ---> POINTER ---> | Data | (Double Indirect Blocks)
  +-----------------------+                                 ------       ------
  | e2di_blocks[14]       | ---> POINTER ---> POINTER ---> POINTER ---> | Data | (Triple Indirect Blocks)
  +-----------------------+                                              ------

  Directory:
  +-----------------------+
  | Inode 0               |
  +-----------------------+
  |                       |
  +-----------------------+
  |                       | ---------------------------
  +-----------------------+                            |
  |                       |                            |
  +-----------------------+                            V
  |                       | <---------------         Directory
  +-----------------------+                 |   +-----------------+
  |                       |                  -- | Inode A | Name  |
  +-----------------------+                     +-----------------+
  |                       | <------------------ | Inode B | Name  |
  +-----------------------+                     +-----------------+
  |                       |              ------ | Inode C | Name  |
  +-----------------------+             |       +-----------------+
  |                       |             |
  +-----------------------+             |
  |                       | <-----------
  +-----------------------+
  | Inode Z               |
  +-----------------------+

   Inode extention layout: (eh_depth == 0)
  +-----------------------+
  | mode                  |
  +-----------------------+
  | size                  |
  +-----------------------+
  | time                  |
  +-----------------------+
  | ext4_extent_header    |
  +-----------------------+       ------
  | ext4_extent[0~3]      | ---> | Data |
  +-----------------------+       ------

   Inode extention layout: (eh_depth > 0)
  +-----------------------+
  | mode                  |
  +-----------------------+
  | size                  |
  +-----------------------+
  | time                  |
  +-----------------------+
  | ext4_extent_header    |
  +-----------------------+       -----------------------
  | ext4_extent_index[0~3]| ---> | ext4_extent_header    |
  +-----------------------+       -----------------------        -----------------------
                                 | ext4_extent_index[0~N]| ---> | ext4_extent_header    |
                                  -----------------------        -----------------------        ------
                                                                | ext4_extent_index[0~N]| ---> | Data |
                                                                 -----------------------        ------





**/

typedef UINT8  uint8_t;
typedef UINT16 uint16_t;
typedef UINT32 uint32_t;
typedef UINT64 uint64_t;
typedef	uint32_t	ino_t;	/* inode number */

//
// Special definition for EXT2 FS
//
#define EXT2_DEFAULT_BLOCK_SIZE            1024
#define EXT2_SUPER_BLOCK_INDEX             1

#pragma pack(1)
#include <IndustryStandard\ext2fs.h>
typedef struct ext2fs  EXT2_FS;
typedef struct ext2_gd EXT2_GD;
#pragma pack()

#pragma pack(1)
#include <IndustryStandard\ext2_dinode.h>
typedef struct ext2fs_dinode EXT2FS_DINODE;
#pragma pack()

#pragma pack(1)
#include <IndustryStandard\ext2_dir.h>
//
// Use ext2fs_direct_2, but NEVER use e2d_type for compatibility consideration
//
typedef struct ext2fs_direct_2 EXT2FS_DIRECT;
#pragma pack()

#pragma pack(1)
#include <IndustryStandard\ext2_extents.h>
typedef struct ext4_extent EXT4_EXTENT;
typedef struct ext4_extent_index EXT4_EXTENT_INDEX;
typedef struct ext4_extent_header EXT4_EXTENT_HEADER;
#pragma pack()

#endif

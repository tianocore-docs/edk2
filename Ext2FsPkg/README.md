# This is sample implementation for ReadOnly EXT2/3/4 File system in UEFI.

It is refering to FreeBSD 9.0 OS Ext2 File System driver, which is BSD license.

# Feature
1. Read Only EXT2/3/4 FS.
2. Block Size - 1024
   Inode Size - 128
   Support EXT4.INODE_EXTENT.
3. Dir, Regular file - read only
   Symbolic Link - read will show link name
   Char/Block/Fifo/Socket - show info only, read unsupported.

# Known limitation
This package is only the sample code to show the concept. It does not have a full validation such as robustness functional test and fuzzing test. It does not meet the production quality yet. Any codes including the API definition, the libary and the drivers are subject to change.
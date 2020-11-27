=============================
  1. Feature
=============================
This package is to create 64K TinyQuark firmware. It can boot Yocto Linux on board flash on Kipsbay Green board.
http://www.intel.com/content/www/us/en/do-it-yourself/galileo-maker-quark-board.html
The purpose of this package is to show the possibility of creating a tiny BIOS (64K) but it is still 
EFI Yocto bootable on Quark platform.

Here is final result:
  +-----------------------------+
  | Module  | Size (K) | Percent|
  +-----------------------------+
  | Generic | 31       | 49%    |
  |-----------------------------|
  | Silicon | 10       | 15%    |
  |-----------------------------|
  | Platform| 23       | 36%    |
  +-----------------------------+

=============================
  2. Goal
=============================

  +-----------------------------------------------------------+
  | Goal is :                    | Goal is NOT :              |
  +-----------------------------------------------------------+
  |   Follow UEFI specification. |   Follow PI specification. |
  |-----------------------------------------------------------|
  |   Reduce features.           |   Support all features.    |
  |-----------------------------------------------------------|
  |   Quark specific.            |   General for all platform.|
  |-----------------------------------------------------------|
  |   Boot Yocto only.           |   Boot all OS.             |
  +-----------------------------------------------------------+

=============================
  3. Platform support
=============================
This package is validated on Quark Kipsbay Platform, Green Board.
If user are using other platform, it might not work directly and porting is needed.

=============================
  4. Build Image
=============================
1) Build Linux image
NOTE: If user fail to create Yocto binary with below step, he/she can use UEFI Shell as payload instead.
User just need set BOOT_LINUX_ON_FLASH to FALSE in QuarkPlatformPkgConfig.dsc.

Goto https://communities.intel.com/docs/DOC-22226
Download Yocto souce and build a small Yocto image.
Please control the kernal size is less than 2M, ramdisk size is less than 2.5M.

After build, please copy below image:
  grub.conf        -> QuarkPayloadBinPkg\Binary\grub.conf.bin
  grub.efi         -> QuarkPayloadBinPkg\Binary\grub.efi
  kernel           -> QuarkPayloadBinPkg\Binary\kernel.bin
  layout.conf_dump -> QuarkPayloadBinPkg\Binary\layout.conf_dump.bin
  ramdisk          -> QuarkPayloadBinPkg\Binary\ramdisk.bin

Please also copy find signed-key-module and svn-area in sysimage, and copy them:
  signed-key-module.bin -> QuarkPayloadBinPkg\Binary\signed-key-module.bin
  svn-area.bin          -> QuarkPayloadBinPkg\Binary\svn-area.bin

Please generate security header to put to:
  header for grub.conf            -> QuarkPayloadBinPkg\Binary\grub.conf.bin.hdr
  header for grub.firmware volume -> QuarkPayloadBinPkg\Binary\grub.fv.hdr
  header for kernel               -> QuarkPayloadBinPkg\Binary\kernel.bin.hdr
  header for ramdisk              -> QuarkPayloadBinPkg\Binary\ramdisk.bin.hdr

2) Build BIOS
The Quark firmware reuses the code on https://communities.intel.com/docs/DOC-22226. User can compare them if he/she wants.

For build, please goto http://sourceforge.net/apps/mediawiki/tianocore/index.php?title=UDK2014
Download UDK2014 BIOS and copy all the package under TinyQuark_EDKII, then build the BIOS. (Only Visual Studio 2008 is validated.)

After build, user may find EDKII_BOOT_STAGE1_IMAGE1.Fv which is 64K BIOS.
User may also find 8M QUARK.fd, which can be burned to on board flash, and boot the linux image built at step 1, or UEFI shell
based on user choice.

=============================
  END
=============================

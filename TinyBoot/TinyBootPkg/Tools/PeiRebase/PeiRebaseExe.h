/*++

Copyright(c) 2014 Intel Corporation. All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions
are met:

* Redistributions of source code must retain the above copyright
notice, this list of conditions and the following disclaimer.
* Redistributions in binary form must reproduce the above copyright
notice, this list of conditions and the following disclaimer in
the documentation and/or other materials provided with the
distribution.
* Neither the name of Intel Corporation nor the names of its
contributors may be used to endorse or promote products derived
from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.


Module Name:

  PeiRebaseExe.h

Abstract:

  Definitions for the PeiRebase exe utility.

--*/

#ifndef _EFI_PEIM_FIXUP_EXE_H
#define _EFI_PEIM_FIXUP_EXE_H

#include <stdlib.h>

#include <Common/UefiBaseTypes.h>
#include <Common/UefiCapsule.h>

#include <Common/PiFirmwareFile.h>
#include <Common/PiFirmwareVolume.h>
#include <Guid/PiFirmwareFileSystem.h>
#include <IndustryStandard/PeImage.h>

#include "CommonLib.h"
#include "ParseInf.h"
#include "EfiUtilityMsgs.h"
#include "FvLib.h"
#include "PeCoffLib.h"

//
// Utility Name
//
#define UTILITY_NAME  "PeiRebase"

//
// Utility version information
//
#define UTILITY_MAJOR_VERSION 0
#define UTILITY_MINOR_VERSION 1
#define UTILITY_DATE          __DATE__

//
// The maximum number of arguments accepted from the command line.
//
#define MAX_ARGS  7

//
// The file copy buffer size
//
#define FILE_COPY_BUFFER_SIZE 512

//
// The function that displays general utility information
//
VOID
PrintUtilityInfo (
  VOID
  );

//
// The function that displays the utility usage message.
//
VOID
PrintUsage (
  VOID
  );

//
// Internal function declarations
//
EFI_STATUS
FfsRebaseImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINT32  *ReadSize,
  OUT    VOID    *Buffer
  );

EFI_STATUS
FfsRebase (
  IN OUT  EFI_FFS_FILE_HEADER       *FfsFile,
  IN      UINT32                    Flags,
  IN OUT  EFI_PHYSICAL_ADDRESS      XipBase,
  IN OUT  EFI_PHYSICAL_ADDRESS      *BsBase,
  IN OUT  EFI_PHYSICAL_ADDRESS      *RtBase,
  OUT     FILE                      *LogFile
  );

#endif

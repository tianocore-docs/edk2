/** @file
  Implements functions to write firmware file

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

**/

#include "DxeMain.h"
#include "FwVolDriver.h"


/**
  Writes one or more files to the firmware volume.

  @param  This                   Indicates the calling context.
  @param  NumberOfFiles          Number of files.
  @param  WritePolicy            WritePolicy indicates the level of reliability
                                 for the write in the event of a power failure or
                                 other system failure during the write operation.
  @param  FileData               FileData is an pointer to an array of
                                 EFI_FV_WRITE_DATA. Each element of array
                                 FileData represents a file to be written.

  @retval EFI_SUCCESS            Files successfully written to firmware volume
  @retval EFI_OUT_OF_RESOURCES   Not enough buffer to be allocated.
  @retval EFI_DEVICE_ERROR       Device error.
  @retval EFI_WRITE_PROTECTED    Write protected.
  @retval EFI_NOT_FOUND          Not found.
  @retval EFI_INVALID_PARAMETER  Invalid parameter.
  @retval EFI_UNSUPPORTED        This function not supported.

**/
EFI_STATUS
EFIAPI
FvWriteFile (
  IN CONST EFI_FIRMWARE_VOLUME2_PROTOCOL  *This,
  IN       UINT32                         NumberOfFiles,
  IN       EFI_FV_WRITE_POLICY            WritePolicy,
  IN       EFI_FV_WRITE_FILE_DATA         *FileData
  )
{
  return EFI_UNSUPPORTED;
}



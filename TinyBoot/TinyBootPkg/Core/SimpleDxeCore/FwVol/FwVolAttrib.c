/** @file
  Implements get/set firmware volume attributes

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
  Retrieves attributes, insures positive polarity of attribute bits, returns
  resulting attributes in output parameter.

  @param  This             Calling context
  @param  Attributes       output buffer which contains attributes

  @retval EFI_SUCCESS      Successfully got volume attributes

**/
EFI_STATUS
EFIAPI
FvGetVolumeAttributes (
  IN  CONST EFI_FIRMWARE_VOLUME2_PROTOCOL *This,
  OUT       EFI_FV_ATTRIBUTES             *Attributes
  )
{
  FV_DEVICE                                 *FvDevice;
  EFI_FVB_ATTRIBUTES_2                      FvbAttributes;
  EFI_FIRMWARE_VOLUME_HEADER                *FwVolHeader;

  FvDevice = FV_DEVICE_FROM_THIS (This);

  FwVolHeader = FvDevice->FwVolHeader;

  //
  // First get the Firmware Volume Block Attributes
  //
  FvbAttributes = FwVolHeader->Attributes & ~EFI_FVB2_WRITE_STATUS;

  //
  // Mask out Fvb bits that are not defined in FV
  //
  FvbAttributes &= 0xfffff0ff;

  *Attributes = (EFI_FV_ATTRIBUTES)FvbAttributes;

  return EFI_SUCCESS;
}



/**
  Sets current attributes for volume

  @param  This             Calling context
  @param  Attributes       At input, contains attributes to be set.  At output
                           contains new value of FV

  @retval EFI_UNSUPPORTED  Could not be set.

**/
EFI_STATUS
EFIAPI
FvSetVolumeAttributes (
  IN     CONST EFI_FIRMWARE_VOLUME2_PROTOCOL  *This,
  IN OUT       EFI_FV_ATTRIBUTES              *Attributes
  )
{
  return EFI_UNSUPPORTED;
}


/**
  Return information of type InformationType for the requested firmware
  volume.

  @param  This             Pointer to EFI_FIRMWARE_VOLUME2_PROTOCOL.
  @param  InformationType  InformationType for requested.
  @param  BufferSize       On input, size of Buffer.On output, the amount of data
                           returned in Buffer.
  @param  Buffer           A poniter to the data buffer to return.

  @retval EFI_SUCCESS      Successfully got volume Information.

**/
EFI_STATUS
EFIAPI
FvGetVolumeInfo (
  IN  CONST EFI_FIRMWARE_VOLUME2_PROTOCOL       *This,
  IN  CONST EFI_GUID                            *InformationType,
  IN OUT UINTN                                  *BufferSize,
  OUT VOID                                      *Buffer
  )
{
  return EFI_UNSUPPORTED;
}



/**
  Set information of type InformationType for the requested firmware
  volume.

  @param  This             Pointer to EFI_FIRMWARE_VOLUME2_PROTOCOL.
  @param  InformationType  InformationType for requested.
  @param  BufferSize       On input, size of Buffer.On output, the amount of data
                           returned in Buffer.
  @param  Buffer           A poniter to the data buffer to return.

  @retval EFI_SUCCESS      Successfully set volume Information.

**/
EFI_STATUS
EFIAPI
FvSetVolumeInfo (
  IN  CONST EFI_FIRMWARE_VOLUME2_PROTOCOL       *This,
  IN  CONST EFI_GUID                            *InformationType,
  IN  UINTN                                     BufferSize,
  IN CONST  VOID                                *Buffer
  )
{
  return EFI_UNSUPPORTED;
}




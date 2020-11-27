/** @file
  NULL Decompress Library 

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

#include "PiDxe.h"

/**
  Given a compressed source buffer, this function retrieves the size of 
  the uncompressed buffer and the size of the scratch buffer required 
  to decompress the compressed source buffer.

  Retrieves the size of the uncompressed buffer and the temporary scratch buffer 
  required to decompress the buffer specified by Source and SourceSize.
  If the size of the uncompressed buffer or the size of the scratch buffer cannot
  be determined from the compressed data specified by Source and SourceData, 
  then RETURN_INVALID_PARAMETER is returned.  Otherwise, the size of the uncompressed
  buffer is returned in DestinationSize, the size of the scratch buffer is returned
  in ScratchSize, and RETURN_SUCCESS is returned.
  This function does not have scratch buffer available to perform a thorough 
  checking of the validity of the source data.  It just retrieves the "Original Size"
  field from the beginning bytes of the source data and output it as DestinationSize.
  And ScratchSize is specific to the decompression implementation.

  If Source is NULL, then ASSERT().
  If DestinationSize is NULL, then ASSERT().
  If ScratchSize is NULL, then ASSERT().

  @param  Source          The source buffer containing the compressed data.
  @param  SourceSize      The size, in bytes, of the source buffer.
  @param  DestinationSize A pointer to the size, in bytes, of the uncompressed buffer
                          that will be generated when the compressed buffer specified
                          by Source and SourceSize is decompressed..
  @param  ScratchSize     A pointer to the size, in bytes, of the scratch buffer that
                          is required to decompress the compressed buffer specified 
                          by Source and SourceSize.

  @retval  RETURN_SUCCESS The size of the uncompressed data was returned 
                          in DestinationSize and the size of the scratch 
                          buffer was returned in ScratchSize.
  @retval  RETURN_INVALID_PARAMETER 
                          The size of the uncompressed data or the size of 
                          the scratch buffer cannot be determined from 
                          the compressed data specified by Source 
                          and SourceSize.
**/
RETURN_STATUS
EFIAPI
UefiDecompressGetInfo (
  IN  CONST VOID  *Source,
  IN  UINT32      SourceSize,
  OUT UINT32      *DestinationSize,
  OUT UINT32      *ScratchSize
  )
{
  return RETURN_UNSUPPORTED;
}

/**
  Decompresses a UEFI compressed source buffer.

  Extracts decompressed data to its original form.
  This function is designed so that the decompression algorithm can be implemented
  without using any memory services.  As a result, this function is not allowed to
  call any memory allocation services in its implementation.  It is the caller's 
  responsibility to allocate and free the Destination and Scratch buffers.
  If the compressed source data specified by Source is successfully decompressed 
  into Destination, then RETURN_SUCCESS is returned.  If the compressed source data 
  specified by Source is not in a valid compressed data format,
  then RETURN_INVALID_PARAMETER is returned.

  If Source is NULL, then ASSERT().
  If Destination is NULL, then ASSERT().
  If the required scratch buffer size > 0 and Scratch is NULL, then ASSERT().

  @param  Source      The source buffer containing the compressed data.
  @param  Destination The destination buffer to store the decompressed data
  @param  Scratch     A temporary scratch buffer that is used to perform the decompression.
                      This is an optional parameter that may be NULL if the 
                      required scratch buffer size is 0.

  @retval  RETURN_SUCCESS Decompression completed successfully, and 
                          the uncompressed buffer is returned in Destination.
  @retval  RETURN_INVALID_PARAMETER 
                          The source buffer specified by Source is corrupted 
                          (not in a valid compressed format).
**/
RETURN_STATUS
EFIAPI
UefiDecompress (
  IN CONST VOID  *Source,
  IN OUT VOID    *Destination,
  IN OUT VOID    *Scratch  OPTIONAL
  )
{
  return RETURN_UNSUPPORTED;
}

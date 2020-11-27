/** @file
  Section Extraction Protocol implementation.

  Stream database is implemented as a linked list of section streams,
  where each stream contains a linked list of children, which may be leaves or
  encapsulations.

  Children that are encapsulations generate new stream entries
  when they are created.  Streams can also be created by calls to
  SEP->OpenSectionStream().

  The database is only created far enough to return the requested data from
  any given stream, or to determine that the requested data is not found.

  If a GUIDed encapsulation is encountered, there are three possiblilites.

  1) A support protocol is found, in which the stream is simply processed with
     the support protocol.

  2) A support protocol is not found, but the data is available to be read
     without processing.  In this case, the database is built up through the
     recursions to return the data, and a RPN event is set that will enable
     the stream in question to be refreshed if and when the required section
     extraction protocol is published.This insures the AuthenticationStatus
     does not become stale in the cache.

  3) A support protocol is not found, and the data is not available to be read
     without it.  This results in EFI_PROTOCOL_ERROR.

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

//
// Local defines and typedefs
//
#define CORE_SECTION_CHILD_SIGNATURE  SIGNATURE_32('S','X','C','S')
#define CHILD_SECTION_NODE_FROM_LINK(Node) \
  CR (Node, CORE_SECTION_CHILD_NODE, Link, CORE_SECTION_CHILD_SIGNATURE)

typedef struct {
  UINT32                      Signature;
  LIST_ENTRY                  Link;
  UINT32                      Type;
  UINT32                      Size;
  //
  // StreamBase + OffsetInStream == pointer to section header in stream.  The
  // stream base is always known when walking the sections within.
  //
  UINT32                      OffsetInStream;
  //
  // If the section REQUIRES an extraction protocol, register for RPN 
  // when the required GUIDed extraction protocol becomes available.
  //
  EFI_EVENT                   Event;
} CORE_SECTION_CHILD_NODE;

#define CORE_SECTION_STREAM_SIGNATURE SIGNATURE_32('S','X','S','S')
#define STREAM_NODE_FROM_LINK(Node) \
  CR (Node, CORE_SECTION_STREAM_NODE, Link, CORE_SECTION_STREAM_SIGNATURE)

typedef struct {
  UINT32                      Signature;
  LIST_ENTRY                  Link;
  UINTN                       StreamHandle;
  UINT8                       *StreamBuffer;
  UINTN                       StreamLength;
  LIST_ENTRY                  Children;
  //
  // Authentication status is from GUIDed encapsulations.
  //
  UINT32                      AuthenticationStatus;
} CORE_SECTION_STREAM_NODE;

#define NULL_STREAM_HANDLE    0

typedef struct {
  CORE_SECTION_CHILD_NODE     *ChildNode;
  CORE_SECTION_STREAM_NODE    *ParentStream;
  VOID                        *Registration;
} RPN_EVENT_CONTEXT;

//
// Module globals
//
LIST_ENTRY mStreamRoot = INITIALIZE_LIST_HEAD_VARIABLE (mStreamRoot);

EFI_HANDLE mSectionExtractionHandle = NULL;

/**
  Check if a stream is valid.

  @param  SectionStream          The section stream to be checked
  @param  SectionStreamLength    The length of section stream

  @return A boolean value indicating the validness of the section stream.

**/
BOOLEAN
IsValidSectionStream (
  IN  VOID              *SectionStream,
  IN  UINTN             SectionStreamLength
  )
{
  UINTN                       TotalLength;
  UINTN                       SectionLength;
  EFI_COMMON_SECTION_HEADER   *SectionHeader;
  EFI_COMMON_SECTION_HEADER   *NextSectionHeader;

  TotalLength = 0;
  SectionHeader = (EFI_COMMON_SECTION_HEADER *)SectionStream;

  while (TotalLength < SectionStreamLength) {
    if (IS_SECTION2 (SectionHeader)) {
      SectionLength = SECTION2_SIZE (SectionHeader);
    } else {
      SectionLength = SECTION_SIZE (SectionHeader);
    }
    TotalLength += SectionLength;

    if (TotalLength == SectionStreamLength) {
      return TRUE;
    }

    //
    // Move to the next byte following the section...
    //
    SectionHeader = (EFI_COMMON_SECTION_HEADER *) ((UINT8 *) SectionHeader + SectionLength);

    //
    // Figure out where the next section begins
    //
    NextSectionHeader = ALIGN_POINTER(SectionHeader, 4);
    TotalLength += (UINTN) NextSectionHeader - (UINTN) SectionHeader;
    SectionHeader = NextSectionHeader;
  }

  ASSERT (FALSE);
  return FALSE;
}


/**
  Worker function.  Constructor for section streams.

  @param  SectionStreamLength    Size in bytes of the section stream.
  @param  SectionStream          Buffer containing the new section stream.
  @param  AllocateBuffer         Indicates whether the stream buffer is to be
                                 copied or the input buffer is to be used in
                                 place. AuthenticationStatus- Indicates the
                                 default authentication status for the new
                                 stream.
  @param  AuthenticationStatus   A pointer to a caller-allocated UINT32 that
                                 indicates the authentication status of the
                                 output buffer. If the input section's
                                 GuidedSectionHeader.Attributes field
                                 has the EFI_GUIDED_SECTION_AUTH_STATUS_VALID
                                 bit as clear, AuthenticationStatus must return
                                 zero. Both local bits (19:16) and aggregate
                                 bits (3:0) in AuthenticationStatus are returned
                                 by ExtractSection(). These bits reflect the
                                 status of the extraction operation. The bit
                                 pattern in both regions must be the same, as
                                 the local and aggregate authentication statuses
                                 have equivalent meaning at this level. If the
                                 function returns anything other than
                                 EFI_SUCCESS, the value of *AuthenticationStatus
                                 is undefined.
  @param  SectionStreamHandle    A pointer to a caller allocated section stream
                                 handle.

  @retval EFI_SUCCESS            Stream was added to stream database.
  @retval EFI_OUT_OF_RESOURCES   memory allocation failed.

**/
EFI_STATUS
OpenSectionStreamEx (
  IN     UINTN                                     SectionStreamLength,
  IN     VOID                                      *SectionStream,
  IN     BOOLEAN                                   AllocateBuffer,
  IN     UINT32                                    AuthenticationStatus,
     OUT UINTN                                     *SectionStreamHandle
  )
{
  CORE_SECTION_STREAM_NODE    *NewStream;
  EFI_TPL                     OldTpl;

  //
  // Allocate a new stream
  //
  NewStream = AllocatePool (sizeof (CORE_SECTION_STREAM_NODE));
  if (NewStream == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  if (AllocateBuffer) {
    //
    // if we're here, we're double buffering, allocate the buffer and copy the
    // data in
    //
    if (SectionStreamLength > 0) {
      NewStream->StreamBuffer = AllocatePool (SectionStreamLength);
      if (NewStream->StreamBuffer == NULL) {
        CoreFreePool (NewStream);
        return EFI_OUT_OF_RESOURCES;
      }
      //
      // Copy in stream data
      //
      CopyMem (NewStream->StreamBuffer, SectionStream, SectionStreamLength);
    } else {
      //
      // It's possible to have a zero length section stream.
      //
      NewStream->StreamBuffer = NULL;
    }
  } else {
    //
    // If were here, the caller has supplied the buffer (it's an internal call)
    // so just assign the buffer.  This happens when we open section streams
    // as a result of expanding an encapsulating section.
    //
    NewStream->StreamBuffer = SectionStream;
  }

  //
  // Initialize the rest of the section stream
  //
  NewStream->Signature = CORE_SECTION_STREAM_SIGNATURE;
  NewStream->StreamHandle = (UINTN) NewStream;
  NewStream->StreamLength = SectionStreamLength;
  InitializeListHead (&NewStream->Children);
  NewStream->AuthenticationStatus = AuthenticationStatus;

  //
  // Add new stream to stream list
  //
  OldTpl = CoreRaiseTpl (TPL_NOTIFY);
  InsertTailList (&mStreamRoot, &NewStream->Link);
  CoreRestoreTpl (OldTpl);

  *SectionStreamHandle = NewStream->StreamHandle;

  return EFI_SUCCESS;
}


/**
  SEP member function.  This function creates and returns a new section stream
  handle to represent the new section stream.

  @param  SectionStreamLength    Size in bytes of the section stream.
  @param  SectionStream          Buffer containing the new section stream.
  @param  SectionStreamHandle    A pointer to a caller allocated UINTN that on
                                 output contains the new section stream handle.

  @retval EFI_SUCCESS            The section stream is created successfully.
  @retval EFI_OUT_OF_RESOURCES   memory allocation failed.
  @retval EFI_INVALID_PARAMETER  Section stream does not end concident with end
                                 of last section.

**/
EFI_STATUS
EFIAPI
OpenSectionStream (
  IN     UINTN                                     SectionStreamLength,
  IN     VOID                                      *SectionStream,
     OUT UINTN                                     *SectionStreamHandle
  )
{
  //
  // Check to see section stream looks good...
  //
  if (!IsValidSectionStream (SectionStream, SectionStreamLength)) {
    return EFI_INVALID_PARAMETER;
  }

  return OpenSectionStreamEx (
           SectionStreamLength,
           SectionStream,
           TRUE,
           0,
           SectionStreamHandle
           );
}



/**
  Worker function.  Determine if the input stream:child matches the input type.

  @param  Stream                 Indicates the section stream associated with the
                                 child
  @param  Child                  Indicates the child to check
  @param  SearchType             Indicates the type of section to check against
                                 for
  @param  SectionDefinitionGuid  Indicates the GUID to check against if the type
                                 is EFI_SECTION_GUID_DEFINED

  @retval TRUE                   The child matches
  @retval FALSE                  The child doesn't match

**/
BOOLEAN
ChildIsType (
  IN CORE_SECTION_STREAM_NODE *Stream,
  IN CORE_SECTION_CHILD_NODE  *Child,
  IN EFI_SECTION_TYPE         SearchType,
  IN EFI_GUID                 *SectionDefinitionGuid
  )
{
  EFI_GUID_DEFINED_SECTION    *GuidedSection;

  if (SearchType == EFI_SECTION_ALL) {
    return TRUE;
  }
  if (Child->Type != SearchType) {
    return FALSE;
  }
  if ((SearchType != EFI_SECTION_GUID_DEFINED) || (SectionDefinitionGuid == NULL)) {
    return TRUE;
  }
  GuidedSection = (EFI_GUID_DEFINED_SECTION * )(Stream->StreamBuffer + Child->OffsetInStream);
  if (IS_SECTION2 (GuidedSection)) {
    return CompareGuid (&(((EFI_GUID_DEFINED_SECTION2 *) GuidedSection)->SectionDefinitionGuid), SectionDefinitionGuid);
  } else {
    return CompareGuid (&GuidedSection->SectionDefinitionGuid, SectionDefinitionGuid);
  }
}

/**
  Worker function.  Constructor for new child nodes.

  @param  Stream                 Indicates the section stream in which to add the
                                 child.
  @param  ChildOffset            Indicates the offset in Stream that is the
                                 beginning of the child section.
  @param  ChildNode              Indicates the Callee allocated and initialized
                                 child.

  @retval EFI_SUCCESS            Child node was found and returned.
                                 EFI_OUT_OF_RESOURCES- Memory allocation failed.
  @retval EFI_PROTOCOL_ERROR     Encapsulation sections produce new stream
                                 handles when the child node is created.  If the
                                 section type is GUID defined, and the extraction
                                 GUID does not exist, and producing the stream
                                 requires the GUID, then a protocol error is
                                 generated and no child is produced. Values
                                 returned by OpenSectionStreamEx.

**/
EFI_STATUS
CreateChildNode (
  IN     CORE_SECTION_STREAM_NODE              *Stream,
  IN     UINT32                                ChildOffset,
  OUT    CORE_SECTION_CHILD_NODE               **ChildNode
  )
{
  EFI_COMMON_SECTION_HEADER                    *SectionHeader;
  CORE_SECTION_CHILD_NODE                      *Node;

  SectionHeader = (EFI_COMMON_SECTION_HEADER *) (Stream->StreamBuffer + ChildOffset);

  //
  // Allocate a new node
  //
  *ChildNode = AllocateZeroPool (sizeof (CORE_SECTION_CHILD_NODE));
  Node = *ChildNode;
  if (Node == NULL) {
    return EFI_OUT_OF_RESOURCES;
  }

  //
  // Now initialize it
  //
  Node->Signature = CORE_SECTION_CHILD_SIGNATURE;
  Node->Type = SectionHeader->Type;
  if (IS_SECTION2 (SectionHeader)) {
    Node->Size = SECTION2_SIZE (SectionHeader);
  } else {
    Node->Size = SECTION_SIZE (SectionHeader);
  }
  Node->OffsetInStream = ChildOffset;

  //
  // If it's an encapsulating section, then create the new section stream also
  //
  switch (Node->Type) {
    case EFI_SECTION_COMPRESSION:
      ASSERT (FALSE);
      return EFI_UNSUPPORTED;
      break;
    case EFI_SECTION_GUID_DEFINED:
      ASSERT (FALSE);
      return EFI_UNSUPPORTED;
      break;
    default:
      //
      // Nothing to do if it's a leaf
      //
      break;
  }

  //
  // Last, add the new child node to the stream
  //
  InsertTailList (&Stream->Children, &Node->Link);

  return EFI_SUCCESS;
}


/**
  Worker function  Recursively searches / builds section stream database
  looking for requested section.

  @param  SourceStream           Indicates the section stream in which to do the
                                 search.
  @param  SearchType             Indicates the type of section to search for.
  @param  SectionInstance        Indicates which instance of section to find.
                                 This is an in/out parameter to deal with
                                 recursions.
  @param  SectionDefinitionGuid  Guid of section definition
  @param  FoundChild             Output indicating the child node that is found.
  @param  FoundStream            Output indicating which section stream the child
                                 was found in.  If this stream was generated as a
                                 result of an encapsulation section, the
                                 streamhandle is visible within the SEP driver
                                 only.
  @param  AuthenticationStatus   Indicates the authentication status of the found section.

  @retval EFI_SUCCESS            Child node was found and returned.
                                 EFI_OUT_OF_RESOURCES- Memory allocation failed.
  @retval EFI_NOT_FOUND          Requested child node does not exist.
  @retval EFI_PROTOCOL_ERROR     a required GUIDED section extraction protocol
                                 does not exist

**/
EFI_STATUS
FindChildNode (
  IN     CORE_SECTION_STREAM_NODE                   *SourceStream,
  IN     EFI_SECTION_TYPE                           SearchType,
  IN OUT UINTN                                      *SectionInstance,
  IN     EFI_GUID                                   *SectionDefinitionGuid,
  OUT    CORE_SECTION_CHILD_NODE                    **FoundChild,
  OUT    CORE_SECTION_STREAM_NODE                   **FoundStream,
  OUT    UINT32                                     *AuthenticationStatus
  )
{
  CORE_SECTION_CHILD_NODE                       *CurrentChildNode;
  UINT32                                        NextChildOffset;
  EFI_STATUS                                    ErrorStatus;
  EFI_STATUS                                    Status;

  CurrentChildNode = NULL;
  ErrorStatus = EFI_NOT_FOUND;

  if (SourceStream->StreamLength == 0) {
    return EFI_NOT_FOUND;
  }

  if (IsListEmpty (&SourceStream->Children) &&
      SourceStream->StreamLength >= sizeof (EFI_COMMON_SECTION_HEADER)) {
    //
    // This occurs when a section stream exists, but no child sections
    // have been parsed out yet.  Therefore, extract the first child and add it
    // to the list of children so we can get started.
    // Section stream may contain an array of zero or more bytes.
    // So, its size should be >= the size of commen section header.
    //
    Status = CreateChildNode (SourceStream, 0, &CurrentChildNode);
    if (EFI_ERROR (Status)) {
      return Status;
    }
  }

  //
  // At least one child has been parsed out of the section stream.  So, walk
  // through the sections that have already been parsed out looking for the
  // requested section, if necessary, continue parsing section stream and
  // adding children until either the requested section is found, or we run
  // out of data
  //
  CurrentChildNode = CHILD_SECTION_NODE_FROM_LINK (GetFirstNode(&SourceStream->Children));

  for (;;) {
    ASSERT (CurrentChildNode != NULL);
    if (ChildIsType (SourceStream, CurrentChildNode, SearchType, SectionDefinitionGuid)) {
      //
      // The type matches, so check the instance count to see if it's the one we want
      //
      (*SectionInstance)--;
      if (*SectionInstance == 0) {
        //
        // Got it!
        //
        *FoundChild = CurrentChildNode;
        *FoundStream = SourceStream;
        *AuthenticationStatus = SourceStream->AuthenticationStatus;
        return EFI_SUCCESS;
      }
    }

    if ((CurrentChildNode->Type == EFI_SECTION_GUID_DEFINED) && (SearchType != EFI_SECTION_GUID_DEFINED)) {
      //
      // When Node Type is GUIDED section, but Node has no encapsulated data, Node data should not be parsed
      // because a required GUIDED section extraction protocol does not exist.
      // If SearchType is not GUIDED section, EFI_PROTOCOL_ERROR should return.
      //
      ErrorStatus = EFI_PROTOCOL_ERROR;
    }

    if (!IsNodeAtEnd (&SourceStream->Children, &CurrentChildNode->Link)) {
      //
      // We haven't found the child node we're interested in yet, but there's
      // still more nodes that have already been parsed so get the next one
      // and continue searching..
      //
      CurrentChildNode = CHILD_SECTION_NODE_FROM_LINK (GetNextNode (&SourceStream->Children, &CurrentChildNode->Link));
    } else {
      //
      // We've exhausted children that have already been parsed, so see if
      // there's any more data and continue parsing out more children if there
      // is.
      //
      NextChildOffset = CurrentChildNode->OffsetInStream + CurrentChildNode->Size;
      //
      // Round up to 4 byte boundary
      //
      NextChildOffset += 3;
      NextChildOffset &= ~(UINTN) 3;
      if (NextChildOffset <= SourceStream->StreamLength - sizeof (EFI_COMMON_SECTION_HEADER)) {
        //
        // There's an unparsed child remaining in the stream, so create a new child node
        //
        Status = CreateChildNode (SourceStream, NextChildOffset, &CurrentChildNode);
        if (EFI_ERROR (Status)) {
          return Status;
        }
      } else {
        ASSERT (EFI_ERROR (ErrorStatus));
        return ErrorStatus;
      }
    }
  }
}


/**
  Worker function.  Search stream database for requested stream handle.

  @param  SearchHandle           Indicates which stream to look for.
  @param  FoundStream            Output pointer to the found stream.

  @retval EFI_SUCCESS            StreamHandle was found and *FoundStream contains
                                 the stream node.
  @retval EFI_NOT_FOUND          SearchHandle was not found in the stream
                                 database.

**/
EFI_STATUS
FindStreamNode (
  IN  UINTN                                     SearchHandle,
  OUT CORE_SECTION_STREAM_NODE                  **FoundStream
  )
{
  CORE_SECTION_STREAM_NODE                      *StreamNode;

  if (!IsListEmpty (&mStreamRoot)) {
    StreamNode = STREAM_NODE_FROM_LINK (GetFirstNode (&mStreamRoot));
    for (;;) {
      if (StreamNode->StreamHandle == SearchHandle) {
        *FoundStream = StreamNode;
        return EFI_SUCCESS;
      } else if (IsNodeAtEnd (&mStreamRoot, &StreamNode->Link)) {
        break;
      } else {
        StreamNode = STREAM_NODE_FROM_LINK (GetNextNode (&mStreamRoot, &StreamNode->Link));
      }
    }
  }

  return EFI_NOT_FOUND;
}


/**
  SEP member function.  Retrieves requested section from section stream.

  @param  SectionStreamHandle   The section stream from which to extract the
                                requested section.
  @param  SectionType           A pointer to the type of section to search for.
  @param  SectionDefinitionGuid If the section type is EFI_SECTION_GUID_DEFINED,
                                then SectionDefinitionGuid indicates which of
                                these types of sections to search for.
  @param  SectionInstance       Indicates which instance of the requested
                                section to return.
  @param  Buffer                Double indirection to buffer.  If *Buffer is
                                non-null on input, then the buffer is caller
                                allocated.  If Buffer is NULL, then the buffer
                                is callee allocated.  In either case, the
                                requried buffer size is returned in *BufferSize.
  @param  BufferSize            On input, indicates the size of *Buffer if
                                *Buffer is non-null on input.  On output,
                                indicates the required size (allocated size if
                                callee allocated) of *Buffer.
  @param  AuthenticationStatus  A pointer to a caller-allocated UINT32 that
                                indicates the authentication status of the
                                output buffer. If the input section's
                                GuidedSectionHeader.Attributes field
                                has the EFI_GUIDED_SECTION_AUTH_STATUS_VALID
                                bit as clear, AuthenticationStatus must return
                                zero. Both local bits (19:16) and aggregate
                                bits (3:0) in AuthenticationStatus are returned
                                by ExtractSection(). These bits reflect the
                                status of the extraction operation. The bit
                                pattern in both regions must be the same, as
                                the local and aggregate authentication statuses
                                have equivalent meaning at this level. If the
                                function returns anything other than
                                EFI_SUCCESS, the value of *AuthenticationStatus
                                is undefined.
  @param  IsFfs3Fv              Indicates the FV format.

  @retval EFI_SUCCESS           Section was retrieved successfully
  @retval EFI_PROTOCOL_ERROR    A GUID defined section was encountered in the
                                section stream with its
                                EFI_GUIDED_SECTION_PROCESSING_REQUIRED bit set,
                                but there was no corresponding GUIDed Section
                                Extraction Protocol in the handle database.
                                *Buffer is unmodified.
  @retval EFI_NOT_FOUND         An error was encountered when parsing the
                                SectionStream.  This indicates the SectionStream
                                is not correctly formatted.
  @retval EFI_NOT_FOUND         The requested section does not exist.
  @retval EFI_OUT_OF_RESOURCES  The system has insufficient resources to process
                                the request.
  @retval EFI_INVALID_PARAMETER The SectionStreamHandle does not exist.
  @retval EFI_WARN_TOO_SMALL    The size of the caller allocated input buffer is
                                insufficient to contain the requested section.
                                The input buffer is filled and section contents
                                are truncated.

**/
EFI_STATUS
EFIAPI
GetSection (
  IN UINTN                                              SectionStreamHandle,
  IN EFI_SECTION_TYPE                                   *SectionType,
  IN EFI_GUID                                           *SectionDefinitionGuid,
  IN UINTN                                              SectionInstance,
  IN VOID                                               **Buffer,
  IN OUT UINTN                                          *BufferSize,
  OUT UINT32                                            *AuthenticationStatus,
  IN BOOLEAN                                            IsFfs3Fv
  )
{
  CORE_SECTION_STREAM_NODE                              *StreamNode;
  EFI_TPL                                               OldTpl;
  EFI_STATUS                                            Status;
  CORE_SECTION_CHILD_NODE                               *ChildNode;
  CORE_SECTION_STREAM_NODE                              *ChildStreamNode;
  UINTN                                                 CopySize;
  UINT32                                                ExtractedAuthenticationStatus;
  UINTN                                                 Instance;
  UINT8                                                 *CopyBuffer;
  UINTN                                                 SectionSize;
  EFI_COMMON_SECTION_HEADER                             *Section;


  OldTpl = CoreRaiseTpl (TPL_NOTIFY);
  Instance = SectionInstance + 1;

  //
  // Locate target stream
  //
  Status = FindStreamNode (SectionStreamHandle, &StreamNode);
  if (EFI_ERROR (Status)) {
    Status = EFI_INVALID_PARAMETER;
    goto GetSection_Done;
  }

  //
  // Found the stream, now locate and return the appropriate section
  //
  if (SectionType == NULL) {
    //
    // SectionType == NULL means return the WHOLE section stream...
    //
    CopySize = StreamNode->StreamLength;
    CopyBuffer = StreamNode->StreamBuffer;
    *AuthenticationStatus = StreamNode->AuthenticationStatus;
  } else {
    //
    // There's a requested section type, so go find it and return it...
    //
    Status = FindChildNode (
               StreamNode,
               *SectionType,
               &Instance,
               SectionDefinitionGuid,
               &ChildNode,
               &ChildStreamNode,
               &ExtractedAuthenticationStatus
               );
    if (EFI_ERROR (Status)) {
      goto GetSection_Done;
    }

    Section = (EFI_COMMON_SECTION_HEADER *) (ChildStreamNode->StreamBuffer + ChildNode->OffsetInStream);

    if (IS_SECTION2 (Section)) {
      ASSERT (SECTION2_SIZE (Section) > 0x00FFFFFF);
      if (!IsFfs3Fv) {
        DEBUG ((DEBUG_ERROR, "It is a FFS3 formatted section in a non-FFS3 formatted FV.\n"));
        Status = EFI_NOT_FOUND;
        goto GetSection_Done;
      }
      CopySize = SECTION2_SIZE (Section) - sizeof (EFI_COMMON_SECTION_HEADER2);
      CopyBuffer = (UINT8 *) Section + sizeof (EFI_COMMON_SECTION_HEADER2);
    } else {
      CopySize = SECTION_SIZE (Section) - sizeof (EFI_COMMON_SECTION_HEADER);
      CopyBuffer = (UINT8 *) Section + sizeof (EFI_COMMON_SECTION_HEADER);
    }
    *AuthenticationStatus = ExtractedAuthenticationStatus;
  }

  SectionSize = CopySize;
  if (*Buffer != NULL) {
    //
    // Caller allocated buffer.  Fill to size and return required size...
    //
    if (*BufferSize < CopySize) {
      Status = EFI_WARN_BUFFER_TOO_SMALL;
      CopySize = *BufferSize;
    }
  } else {
    //
    // Callee allocated buffer.  Allocate buffer and return size.
    //
    *Buffer = AllocatePool (CopySize);
    if (*Buffer == NULL) {
      Status = EFI_OUT_OF_RESOURCES;
      goto GetSection_Done;
    }
  }
  CopyMem (*Buffer, CopyBuffer, CopySize);
  *BufferSize = SectionSize;

GetSection_Done:
  CoreRestoreTpl (OldTpl);

  return Status;
}


/**
  Worker function.  Destructor for child nodes.

  @param  ChildNode              Indicates the node to destroy

**/
VOID
FreeChildNode (
  IN  CORE_SECTION_CHILD_NODE                   *ChildNode
  )
{
  ASSERT (ChildNode->Signature == CORE_SECTION_CHILD_SIGNATURE);
  //
  // Remove the child from it's list
  //
  RemoveEntryList (&ChildNode->Link);

  if (ChildNode->Event != NULL) {
    gBS->CloseEvent (ChildNode->Event);
  }

  //
  // Last, free the child node itself
  //
  CoreFreePool (ChildNode);
}


/**
  SEP member function.  Deletes an existing section stream

  @param  StreamHandleToClose    Indicates the stream to close

  @retval EFI_SUCCESS            The section stream is closed sucessfully.
  @retval EFI_OUT_OF_RESOURCES   Memory allocation failed.
  @retval EFI_INVALID_PARAMETER  Section stream does not end concident with end
                                 of last section.

**/
EFI_STATUS
EFIAPI
CloseSectionStream (
  IN  UINTN                                     StreamHandleToClose
  )
{
  CORE_SECTION_STREAM_NODE                      *StreamNode;
  EFI_TPL                                       OldTpl;
  EFI_STATUS                                    Status;
  LIST_ENTRY                                    *Link;
  CORE_SECTION_CHILD_NODE                       *ChildNode;

  OldTpl = CoreRaiseTpl (TPL_NOTIFY);

  //
  // Locate target stream
  //
  Status = FindStreamNode (StreamHandleToClose, &StreamNode);
  if (!EFI_ERROR (Status)) {
    //
    // Found the stream, so close it
    //
    RemoveEntryList (&StreamNode->Link);
    while (!IsListEmpty (&StreamNode->Children)) {
      Link = GetFirstNode (&StreamNode->Children);
      ChildNode = CHILD_SECTION_NODE_FROM_LINK (Link);
      FreeChildNode (ChildNode);
    }
    CoreFreePool (StreamNode->StreamBuffer);
    CoreFreePool (StreamNode);
    Status = EFI_SUCCESS;
  } else {
    Status = EFI_INVALID_PARAMETER;
  }

  CoreRestoreTpl (OldTpl);
  return Status;
}


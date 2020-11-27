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

  PeiRebaseExe.c

Abstract:

  This contains all code necessary to build the PeiRebase.exe utility.
  This utility relies heavily on the PeiRebase DLL.  Definitions for both
  can be found in the PEI Rebase Utility Specification, review draft.

--*/

//
// File included in build
//
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "PeiRebaseExe.h"

STATIC
EFI_STATUS
ReadHeader (
  IN FILE       *InputFile,
  OUT UINT32    *FvSize,
  OUT BOOLEAN   *ErasePolarity
  );

STATIC
BOOLEAN
CheckXipImage (
  IN EFI_IMAGE_NT_HEADERS32   *PeHdr
  );

int
main (
  int  argc,
  char **argv
  )
/*++

Routine Description:

  This utility relocates PEI XIP PE32s in a FV.

Arguments:

  argc          - Number of command line arguments
  argv[]:
  BaseAddress     The base address to use for rebasing the FV.  The correct
                  format is a hex number preceded by 0x.
  InputFileName   The name of the input FV file.
  OutputFileName  The name of the output FV file.

  Arguments come in pair in any order.
    -I InputFileName
    -O OutputFileName
    -B BaseAddress

Returns:

  0   No error conditions detected.
  1   One or more of the input parameters is invalid.
  2   A resource required by the utility was unavailable.
      Most commonly this will be memory allocation or file creation.
  3   PeiRebase.dll could not be loaded.
  4   Error executing the PEI rebase.

--*/
{
  UINT8                       Index;
  CHAR8                       InputFileName[_MAX_PATH];
  CHAR8                       OutputFileName[_MAX_PATH];
  EFI_PHYSICAL_ADDRESS        XipBase, BsBase, RtBase;
  UINT32                      BaseTypes;
  EFI_STATUS                  Status;
  FILE                        *InputFile;
  FILE                        *OutputFile;
  FILE                        *LogFile;
  UINT64                      FvOffset;
  UINT32                      FileCount;
  int                         BytesRead;
  EFI_FIRMWARE_VOLUME_HEADER  *FvImage;
  UINT32                      FvSize;
  EFI_FFS_FILE_HEADER         *CurrentFile;
  BOOLEAN                     ErasePolarity;

  ErasePolarity = FALSE;
  //
  // Set utility name for error/warning reporting purposes.
  //
  SetUtilityName (UTILITY_NAME);
  //
  // Verify the correct number of arguments
  //
  if (argc < MAX_ARGS) {
    PrintUsage ();
    return STATUS_ERROR;
  }

  //
  // Initialize variables
  //
  InputFileName[0]  = 0;
  OutputFileName[0] = 0;
  XipBase = BsBase = RtBase = 0;
  BaseTypes         = 0;
  FvOffset          = 0;
  FileCount         = 0;
  ErasePolarity     = FALSE;
  InputFile         = NULL;
  OutputFile        = NULL;
  LogFile           = NULL;
  FvImage           = NULL;

  //
  // Parse the command line arguments
  //
  for (Index = 1; Index < argc; Index += 2) {
    //
    // Make sure argument pair begin with - or /
    //
    if (argv[Index][0] != '-' && argv[Index][0] != '/') {
      PrintUsage ();
      Error (NULL, 0, 0, argv[Index], "unrecognized option");
      return STATUS_ERROR;
    }
    //
    // Make sure argument specifier is only one letter
    //
    if (argv[Index][2] != 0) {
      PrintUsage ();
      Error (NULL, 0, 0, argv[Index], "unrecognized option");
      return STATUS_ERROR;
    }
    //
    // Determine argument to read
    //
    switch (argv[Index][1]) {
    case 'I':
    case 'i':
      if (strlen (InputFileName) == 0) {
        strcpy (InputFileName, argv[Index + 1]);
      } else {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "only one -i InputFileName may be specified");
        return STATUS_ERROR;
      }
      break;

    case 'O':
    case 'o':
      if (strlen (OutputFileName) == 0) {
        strcpy (OutputFileName, argv[Index + 1]);
      } else {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "only one -o OutputFileName may be specified");
        return STATUS_ERROR;
      }
      break;

    case 'B':
    case 'b':
      if (BaseTypes & 1) {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "-b XipBaseAddress may be specified only once");
        return STATUS_ERROR;
      }

      Status = AsciiStringToUint64 (argv[Index + 1], FALSE, &XipBase);
      if (EFI_ERROR (Status)) {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "invalid hex digit given for XIP base address");
        return STATUS_ERROR;
      }

      BaseTypes |= 1;
      break;

    case 'D':
    case 'd':
      if (BaseTypes & 2) {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "-d BsBaseAddress may be specified only once");
        return STATUS_ERROR;
      }

      Status = AsciiStringToUint64 (argv[Index + 1], FALSE, &BsBase);
      if (EFI_ERROR (Status)) {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "invalid hex digit given for BS_DRIVER base address");
        return STATUS_ERROR;
      }

      BaseTypes |= 2;
      break;

    case 'R':
    case 'r':
      if (BaseTypes & 4) {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "-r RtBaseAddress may be specified only once");
        return STATUS_ERROR;
      }

      Status = AsciiStringToUint64 (argv[Index + 1], FALSE, &RtBase);
      if (EFI_ERROR (Status)) {
        PrintUsage ();
        Error (NULL, 0, 0, argv[Index + 1], "invalid hex digit given for RT_DRIVER base address");
        return STATUS_ERROR;
      }

      BaseTypes |= 4;
      break;

    default:
      PrintUsage ();
      Error (NULL, 0, 0, argv[Index], "unrecognized argument");
      return STATUS_ERROR;
      break;
    }
  }
  //
  // Open the file containing the FV
  //
  InputFile = fopen (InputFileName, "rb");
  if (InputFile == NULL) {
    Error (NULL, 0, 0, InputFileName, "could not open input file for reading");
    return STATUS_ERROR;
  }

  //
  // Open the log file
  //
  strcat (InputFileName, ".log");
  LogFile = fopen (InputFileName, "wb");
  if (LogFile == NULL) {
    Error (NULL, 0, 0, InputFileName, "could not open log file for writing");
  }

  //
  // Determine size of FV
  //
  Status = ReadHeader (InputFile, &FvSize, &ErasePolarity);
  if (EFI_ERROR (Status)) {
    Error (NULL, 0, 0, "could not parse the FV header", NULL);
    goto Finish;
  }
  //
  // Allocate a buffer for the FV image
  //
  FvImage = malloc (FvSize);
  if (FvImage == NULL) {
    Error (NULL, 0, 0, "application error", "memory allocation failed");
    goto Finish;
  }
  //
  // Read the entire FV to the buffer
  //
  BytesRead = fread (FvImage, 1, FvSize, InputFile);
  fclose (InputFile);
  InputFile = NULL;
  if ((unsigned int) BytesRead != FvSize) {
    Error (NULL, 0, 0, InputFileName, "failed to read from file");
    goto Finish;
  }
  //
  // Prepare to walk the FV image
  //
  InitializeFvLib (FvImage, FvSize);
  //
  // Get the first file
  //
  Status = GetNextFile (NULL, &CurrentFile);
  if (EFI_ERROR (Status)) {
    Error (NULL, 0, 0, "cannot find the first file in the FV image", NULL);
    goto Finish;
  }
  //
  // Check if each file should be rebased
  //
  while (CurrentFile != NULL) {
    //
    // Rebase this file
    //
    Status = FfsRebase (
              CurrentFile,
              BaseTypes,
              XipBase + (UINTN)CurrentFile - (UINTN)FvImage,
              &BsBase,
              &RtBase,
              LogFile
              );

    if (EFI_ERROR (Status)) {
      switch (Status) {

      case EFI_INVALID_PARAMETER:
        Error (NULL, 0, 0, "invalid parameter passed to FfsRebase", NULL);
        break;

      case EFI_ABORTED:
        Error (NULL, 0, 0, "error detected while rebasing -- aborted", NULL);
        break;

      case EFI_OUT_OF_RESOURCES:
        Error (NULL, 0, 0, "FfsRebase could not allocate required resources", NULL);
        break;

      case EFI_NOT_FOUND:
        Error (NULL, 0, 0, "FfsRebase could not locate a PE32 section", NULL);
        break;

      default:
        Error (NULL, 0, 0, "FfsRebase returned unknown status", "status=0x%08X", Status);
        break;
      }

      goto Finish;
    }
    //
    // Get the next file
    //
    Status = GetNextFile (CurrentFile, &CurrentFile);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "cannot find the next file in the FV image", NULL);
      goto Finish;
    }
  }
  //
  // Open the output file
  //
  OutputFile = fopen (OutputFileName, "wb");
  if (OutputFile == NULL) {
    Error (NULL, 0, 0, OutputFileName, "failed to open output file");
    goto Finish;
  }

  if (fwrite (FvImage, 1, FvSize, OutputFile) != FvSize) {
    Error (NULL, 0, 0, "failed to write to output file", 0);
    goto Finish;
  }

Finish:
  if (InputFile != NULL) {
    fclose (InputFile);
  }
  //
  // If we created an output file, and there was an error, remove it so
  // subsequent builds will rebuild it.
  //
  if (OutputFile != NULL) {
    if (GetUtilityStatus () == STATUS_ERROR) {
      remove (OutputFileName);
    }

    fclose (OutputFile);
  }

  if (LogFile != NULL) {
    fclose (LogFile);
  }

  if (FvImage != NULL) {
    free (FvImage);
  }

  return GetUtilityStatus ();
}

STATIC
EFI_STATUS
ReadHeader (
  IN FILE       *InputFile,
  OUT UINT32    *FvSize,
  OUT BOOLEAN   *ErasePolarity
  )
/*++

Routine Description:

  This function determines the size of the FV and the erase polarity.  The
  erase polarity is the FALSE value for file state.

Arguments:

  InputFile       The file that contains the FV image.
  FvSize          The size of the FV.
  ErasePolarity   The FV erase polarity.

Returns:

  EFI_SUCCESS             Function completed successfully.
  EFI_INVALID_PARAMETER   A required parameter was NULL or is out of range.
  EFI_ABORTED             The function encountered an error.

--*/
{
  EFI_FIRMWARE_VOLUME_HEADER  VolumeHeader;
  EFI_FV_BLOCK_MAP_ENTRY      BlockMap;
  UINTN                       Signature[2];
  UINTN                       BytesRead;
  UINT32                      Size;

  BytesRead = 0;
  Size      = 0;
  //
  // Check input parameters
  //
  if ((InputFile == NULL) || (FvSize == NULL) || (ErasePolarity == NULL)) {
    Error (NULL, 0, 0, "ReadHeader()", "invalid input parameter");
    return EFI_INVALID_PARAMETER;
  }
  //
  // Read the header
  //
  fread (&VolumeHeader, sizeof (EFI_FIRMWARE_VOLUME_HEADER) - sizeof (EFI_FV_BLOCK_MAP_ENTRY), 1, InputFile);
  BytesRead     = sizeof (EFI_FIRMWARE_VOLUME_HEADER) - sizeof (EFI_FV_BLOCK_MAP_ENTRY);
  Signature[0]  = VolumeHeader.Signature;
  Signature[1]  = 0;

  //
  // Get erase polarity
  //
  if (VolumeHeader.Attributes & EFI_FVB2_ERASE_POLARITY) {
    *ErasePolarity = TRUE;
  }

  do {
    fread (&BlockMap, sizeof (EFI_FV_BLOCK_MAP_ENTRY), 1, InputFile);
    BytesRead += sizeof (EFI_FV_BLOCK_MAP_ENTRY);

    if (BlockMap.NumBlocks != 0) {
      Size += BlockMap.NumBlocks * BlockMap.Length;
    }

  } while (!(BlockMap.NumBlocks == 0 && BlockMap.Length == 0));

  if (VolumeHeader.FvLength != Size) {
    Error (NULL, 0, 0, "volume size not consistant with block maps", NULL);
    return EFI_ABORTED;
  }

  *FvSize = Size;

  rewind (InputFile);

  return EFI_SUCCESS;
}

VOID
PrintUtilityInfo (
  VOID
  )
/*++

Routine Description:

  Displays the standard utility information to SDTOUT

Arguments:

  None

Returns:

  None

--*/
{
  printf (
    "%s, PEI Rebase Utility. Version %i.%i, %s.\n\n",
    UTILITY_NAME,
    UTILITY_MAJOR_VERSION,
    UTILITY_MINOR_VERSION,
    UTILITY_DATE
    );
}

VOID
PrintUsage (
  VOID
  )
/*++

Routine Description:

  Displays the utility usage syntax to STDOUT

Arguments:

  None

Returns:

  None

--*/
{
  printf (
    "Usage: %s -I InputFileName -O OutputFileName -B BaseAddress\n",
    UTILITY_NAME
    );
  printf ("  [-D BootDriverBaseAddress] [-R RuntimeDriverBaseAddress]\n");
  printf ("  Where:\n");
  printf ("    InputFileName is the name of the EFI FV file to rebase.\n");
  printf ("    OutputFileName is the desired output file name.\n");
  printf ("    BaseAddress is the FV base address to rebase agains.\n");
  printf ("    BootDriverBaseAddress is the rebase address for all boot drivers in this fv image.\n");
  printf ("    RuntimeDriverBaseAddress is the rebase address for all runtime drivers in this fv image.\n");
  printf ("  Argument pair may be in any order.\n\n");
}

EFI_STATUS
FfsRebase (
  IN OUT  EFI_FFS_FILE_HEADER       *FfsFile,
  IN      UINT32                    Flags,
  IN OUT  EFI_PHYSICAL_ADDRESS      XipBase,
  IN OUT  EFI_PHYSICAL_ADDRESS      *BsBase,
  IN OUT  EFI_PHYSICAL_ADDRESS      *RtBase,
  OUT     FILE                      *LogFile
  )
/*++

Routine Description:

  This function determines if a file is XIP and should be rebased.  It will
  rebase any PE32 sections found in the file using the base address.

Arguments:

  FfsFile           A pointer to Ffs file image.
  BaseAddress       The base address to use for rebasing the file image.

Returns:

  EFI_SUCCESS             The image was properly rebased.
  EFI_INVALID_PARAMETER   An input parameter is invalid.
  EFI_ABORTED             An error occurred while rebasing the input file image.
  EFI_OUT_OF_RESOURCES    Could not allocate a required resource.
  EFI_NOT_FOUND           No compressed sections could be found.

--*/
{
  EFI_STATUS                            Status;
  PE_COFF_LOADER_IMAGE_CONTEXT          ImageContext;
  EFI_PHYSICAL_ADDRESS                  NewPe32BaseAddress;
  UINTN                                 Index;
  UINTN                                 MemoryImagePointer;
  EFI_FILE_SECTION_POINTER              CurrentPe32Section;
  EFI_FFS_FILE_STATE                    SavedState;
  EFI_IMAGE_NT_HEADERS32                *PeHdr;
  EFI_IMAGE_NT_HEADERS64                *PePlusHdr;
  EFI_TE_IMAGE_HEADER                   *TEImageHeader;
  UINT8                                 FileGuidString[80];
  EFI_PHYSICAL_ADDRESS                  *BaseToUpdate;
  EFI_IMAGE_SECTION_HEADER              *SectionHeader;

  //
  // Verify input parameters
  //
  if (FfsFile == NULL) {
    return EFI_INVALID_PARAMETER;
  }
  //
  // Convert the GUID to a string so we can at least report which file
  // if we find an error.
  //
  PrintGuidToBuffer (&FfsFile->Name, FileGuidString, sizeof (FileGuidString), TRUE);
  //
  // Do some cursory checks on the FFS file contents
  //
  Status = VerifyFfsFile (FfsFile);
  if (EFI_ERROR (Status)) {
    Error (NULL, 0, 0, "file does not appear to be a valid FFS file, cannot be rebased", FileGuidString);
    return EFI_INVALID_PARAMETER;
  }

  //
  // We only process files potentially containing PE32 sections.
  //
  switch (FfsFile->Type) {
    case EFI_FV_FILETYPE_PEI_CORE:
    case EFI_FV_FILETYPE_PEIM:
    case EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER:
    case EFI_FV_FILETYPE_DRIVER:
    case EFI_FV_FILETYPE_DXE_CORE:
      break;
    case EFI_FV_FILETYPE_SECURITY_CORE:
      printf ("FfsRebase - Skip SecCore\n");
    default:
      return EFI_SUCCESS;
  }

  printf ("FfsRebase - %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
    FfsFile->Name.Data1,
    FfsFile->Name.Data2,
    FfsFile->Name.Data3,
    FfsFile->Name.Data4[0],
    FfsFile->Name.Data4[1],
    FfsFile->Name.Data4[2],
    FfsFile->Name.Data4[3],
    FfsFile->Name.Data4[4],
    FfsFile->Name.Data4[5],
    FfsFile->Name.Data4[6],
    FfsFile->Name.Data4[7]
    );

  printf ("FfsRebase - XIP base - 0x%08x\n", XipBase);

  //
  // Rebase each PE32 section
  //
  Status      = EFI_SUCCESS;
  for (Index = 1;; Index++) {
    Status = GetSectionByType (FfsFile, EFI_SECTION_PE32, Index, &CurrentPe32Section);
    if (EFI_ERROR (Status)) {
      break;
    }

    printf ("FfsRebase - handle PE32 section ...\n");

    //
    // Initialize context
    //
    memset (&ImageContext, 0, sizeof (ImageContext));
    ImageContext.Handle     = (VOID *) ((UINTN) CurrentPe32Section.Pe32Section + sizeof (EFI_PE32_SECTION));
    ImageContext.ImageRead  = (PE_COFF_LOADER_READ_FILE) FfsRebaseImageRead;
    Status                  = PeCoffLoaderGetImageInfo (&ImageContext);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "GetImageInfo() call failed on rebase", FileGuidString);
      return Status;
    }

    //
    // Get PeImage Header
    //
    ImageContext.ImageAddress = (UINTN) CurrentPe32Section.Pe32Section + sizeof (EFI_PE32_SECTION);
    PeHdr = (EFI_IMAGE_NT_HEADERS32 *) (UINTN) (ImageContext.ImageAddress + ImageContext.PeCoffHeaderOffset); 
    
    //
    // Calculate the PE32 base address, based on file type
    //
    switch (FfsFile->Type) {
      case EFI_FV_FILETYPE_SECURITY_CORE:
      case EFI_FV_FILETYPE_PEI_CORE:
      case EFI_FV_FILETYPE_PEIM:
      case EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER:
        if ((Flags & 1) == 0) {
          //
          // We aren't relocating XIP code, so skip it.
          //
          printf ("FfsRebase - not relocating XIP code\n");
          return EFI_SUCCESS;
        }

        if (CheckXipImage (PeHdr) == FALSE) {
          Error (NULL, 0, 0, "This file is not one valid XIP image", FileGuidString);
          return EFI_ABORTED;
        }

        NewPe32BaseAddress = XipBase + (UINTN)ImageContext.ImageAddress - (UINTN)FfsFile;
        BaseToUpdate = &XipBase;
        break;

      case EFI_FV_FILETYPE_DRIVER:
        switch (PeHdr->OptionalHeader.Subsystem) {
          case EFI_IMAGE_SUBSYSTEM_EFI_RUNTIME_DRIVER:
            if ((Flags & 4) == 0) {
              //
              // RT drivers aren't supposed to be relocated
              //
              continue;
            }

            NewPe32BaseAddress = *RtBase;
            BaseToUpdate = RtBase;
            break;

          default:
            //
            // We treat all other subsystems the same as BS_DRIVER
            //
            if ((Flags & 2) == 0) {
              //
              // Skip all BS_DRIVER's
              //
              continue;
            }

            NewPe32BaseAddress = *BsBase;
            BaseToUpdate = BsBase;
            break;
        }
        break;

      case EFI_FV_FILETYPE_DXE_CORE:
        if ((Flags & 2) == 0) {
          //
          // Skip DXE core
          //
          return EFI_SUCCESS;
        }

        NewPe32BaseAddress = *BsBase;
        BaseToUpdate = BsBase;
        break;

      default:
        //
        // Not supported file type
        //
        return EFI_SUCCESS;
    }

    if (PeHdr->FileHeader.Machine == EFI_IMAGE_MACHINE_IA32) {
      NewPe32BaseAddress = (EFI_PHYSICAL_ADDRESS)(UINT32)NewPe32BaseAddress;
    }

    //
    // Load and Relocate Image Data
    //
    MemoryImagePointer = (UINTN) malloc ((UINTN) ImageContext.ImageSize + ImageContext.SectionAlignment);
    if (MemoryImagePointer == 0) {
      Error (NULL, 0, 0, "Can't allocate enough memory on rebase", FileGuidString);
      return Status;
    }
    memset ((VOID *) MemoryImagePointer, 0, (UINTN) ImageContext.ImageSize + ImageContext.SectionAlignment);
    ImageContext.ImageAddress = (MemoryImagePointer + ImageContext.SectionAlignment - 1) & (~(ImageContext.SectionAlignment - 1));
    
    Status =  PeCoffLoaderLoadImage (&ImageContext);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "LocateImage() call failed on rebase", FileGuidString);
      free ((VOID *) MemoryImagePointer);
      return Status;
    }

    if (ImageContext.RelocationsStripped) {
      printf ("FfsRebase - RelocationsStripped!\n");
    }

    ImageContext.DestinationAddress = NewPe32BaseAddress;
    Status                          = PeCoffLoaderRelocateImage (&ImageContext);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "RelocateImage() call failed on rebase", FileGuidString);
      free ((VOID *) MemoryImagePointer);
      return Status;
    }

    //
    // Copy Relocated data to raw image file.
    //
    PePlusHdr = (EFI_IMAGE_NT_HEADERS64 *) PeHdr;
    if (PeHdr->FileHeader.Machine == EFI_IMAGE_MACHINE_IA32) {
      PeHdr->OptionalHeader.ImageBase     = (UINT32) NewPe32BaseAddress;
    } else if (PeHdr->FileHeader.Machine == EFI_IMAGE_MACHINE_IA64 || 
               PeHdr->FileHeader.Machine == EFI_IMAGE_MACHINE_X64) {
      PePlusHdr->OptionalHeader.ImageBase = NewPe32BaseAddress;
    } else {
      Error (
        NULL,
        0,
        0,
        "unknown machine type in PE32 image",
        "machine type=0x%X, file=%s",
        (UINT32) PeHdr->FileHeader.Machine,
        FileGuidString
        );
      free ((VOID *) MemoryImagePointer);
      return EFI_ABORTED;
    }      

    SectionHeader = (EFI_IMAGE_SECTION_HEADER *) (
                       (UINTN) ImageContext.ImageAddress +
                       ImageContext.PeCoffHeaderOffset +
                       sizeof (UINT32) + 
                       sizeof (EFI_IMAGE_FILE_HEADER) +  
                       PeHdr->FileHeader.SizeOfOptionalHeader
                       );
    
    for (Index = 0; Index < PeHdr->FileHeader.NumberOfSections; Index ++, SectionHeader ++) {
      CopyMem (
        (UINT8 *) ImageContext.Handle + SectionHeader->PointerToRawData, 
        (VOID*) (UINTN) (ImageContext.ImageAddress + SectionHeader->VirtualAddress), 
        SectionHeader->SizeOfRawData
        );
    }
    
    free ((VOID *) MemoryImagePointer);

    //
    // Update BASE address
    //
    fprintf (
      LogFile,
      "%s %016I64X\n",
      FileGuidString,
      ImageContext.DestinationAddress
      );
    *BaseToUpdate += EFI_SIZE_TO_PAGES (ImageContext.ImageSize) * EFI_PAGE_SIZE;

    //
    // Now update file checksum
    //
    if (FfsFile->Attributes & FFS_ATTRIB_CHECKSUM) {
      SavedState  = FfsFile->State;
      FfsFile->IntegrityCheck.Checksum.File = 0;
      FfsFile->State                        = 0;
      if (FfsFile->Attributes & FFS_ATTRIB_CHECKSUM) {
        FfsFile->IntegrityCheck.Checksum.File = CalculateChecksum8 (
                                                  (UINT8 *) FfsFile,
                                                  GetLength (FfsFile->Size)
                                                  );
      } else {
        FfsFile->IntegrityCheck.Checksum.File = FFS_FIXED_CHECKSUM;
      }

      FfsFile->State = SavedState;
    }
  }

  if ((Flags & 1) == 0 || (
      FfsFile->Type != EFI_FV_FILETYPE_SECURITY_CORE &&
      FfsFile->Type != EFI_FV_FILETYPE_PEI_CORE &&

      FfsFile->Type != EFI_FV_FILETYPE_PEIM &&
      FfsFile->Type != EFI_FV_FILETYPE_COMBINED_PEIM_DRIVER
      )) {
    //
    // Only XIP code may have a TE section
    //
    return EFI_SUCCESS;
  }

  //
  // Now process TE sections
  //
  for (Index = 1;; Index++) {
    Status = GetSectionByType (FfsFile, EFI_SECTION_TE, Index, &CurrentPe32Section);
    if (EFI_ERROR (Status)) {
      break;
    }

    printf ("FfsRebase - Handle TE section ...\n");

    //
    // Calculate the TE base address, the FFS file base plus the offset of the TE section less the size stripped off
    // by GenTEImage
    //
    TEImageHeader = (EFI_TE_IMAGE_HEADER *) ((UINT8 *) CurrentPe32Section.Pe32Section + sizeof (EFI_COMMON_SECTION_HEADER));

    NewPe32BaseAddress = XipBase +
      (
        (UINTN) TEImageHeader + 
        sizeof (EFI_TE_IMAGE_HEADER) - TEImageHeader->StrippedSize -
        (UINTN) FfsFile
      );

    if (TEImageHeader->Machine == EFI_IMAGE_MACHINE_IA32) {
      NewPe32BaseAddress = (EFI_PHYSICAL_ADDRESS)(UINT32)NewPe32BaseAddress;
    }

    //
    // Initialize context
    //
    memset (&ImageContext, 0, sizeof (ImageContext));
    ImageContext.Handle     = (VOID *) TEImageHeader;
    ImageContext.ImageRead  = (PE_COFF_LOADER_READ_FILE) FfsRebaseImageRead;

    Status                  = PeCoffLoaderGetImageInfo (&ImageContext);

    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "GetImageInfo() call failed on rebase of TE image", FileGuidString);
      return Status;
    }

    if (ImageContext.RelocationsStripped) {
      printf ("FfsRebase - RelocationsStripped!\n");
    }

    //
    // Don't reload TeImage
    //
    ImageContext.ImageAddress = (UINTN) TEImageHeader;

    //
    // Reloacate TeImage
    // 
    ImageContext.DestinationAddress = NewPe32BaseAddress;
    Status                          = PeCoffLoaderRelocateImage (&ImageContext);
    if (EFI_ERROR (Status)) {
      Error (NULL, 0, 0, "RelocateImage() call failed on rebase of TE image", FileGuidString);
      return Status;
    }

    //
    // Now update file checksum
    //
    if (FfsFile->Attributes & FFS_ATTRIB_CHECKSUM) {
      SavedState  = FfsFile->State;
      FfsFile->IntegrityCheck.Checksum.File = 0;
      FfsFile->State                        = 0;
      if (FfsFile->Attributes & FFS_ATTRIB_CHECKSUM) {
        FfsFile->IntegrityCheck.Checksum.File = CalculateChecksum8 (
                                                  (UINT8 *) FfsFile,
                                                  GetLength (FfsFile->Size)
                                                  );
      } else {
        FfsFile->IntegrityCheck.Checksum.File = FFS_FIXED_CHECKSUM;
      }

      FfsFile->State = SavedState;
    }
    fprintf (
      LogFile,
      "%s %016I64X\n",
      FileGuidString,
      ImageContext.DestinationAddress
      );
  }

  return EFI_SUCCESS;
}

EFI_STATUS
FfsRebaseImageRead (
  IN     VOID    *FileHandle,
  IN     UINTN   FileOffset,
  IN OUT UINT32  *ReadSize,
  OUT    VOID    *Buffer
  )
/*++

Routine Description:

  Support routine for the PE/COFF Loader that reads a buffer from a PE/COFF file

Arguments:

  FileHandle - The handle to the PE/COFF file

  FileOffset - The offset, in bytes, into the file to read

  ReadSize   - The number of bytes to read from the file starting at FileOffset

  Buffer     - A pointer to the buffer to read the data into.

Returns:

  EFI_SUCCESS - ReadSize bytes of data were read into Buffer from the PE/COFF file starting at FileOffset

--*/
{
  CHAR8   *Destination8;
  CHAR8   *Source8;
  UINT32  Length;

  Destination8  = Buffer;
  Source8       = (CHAR8 *) ((UINTN) FileHandle + FileOffset);
  Length        = *ReadSize;
  while (Length--) {
    *(Destination8++) = *(Source8++);
  }

  return EFI_SUCCESS;
}

STATIC
BOOLEAN
CheckXipImage (
  IN EFI_IMAGE_NT_HEADERS32   *PeHdr
  )
/*++

Routine Description:

  This function checks whether the input PeImage is one XIP Image.

Arguments:
  PeHdr - Pointer to PeImage Header

Returns:
  FALSE - This image is not one valid xip image.
  TRUE  - This image is one valid xip image.

--*/
{
  EFI_IMAGE_SECTION_HEADER  *SectionHeader;
  UINT8                     Index;

  if (PeHdr->OptionalHeader.SectionAlignment != PeHdr->OptionalHeader.FileAlignment) {
    return FALSE;
  }

  SectionHeader = (EFI_IMAGE_SECTION_HEADER *) (
                     (UINTN) &(PeHdr->OptionalHeader) +
                     PeHdr->FileHeader.SizeOfOptionalHeader
                     );
  
  for (Index = 0; Index < PeHdr->FileHeader.NumberOfSections; Index ++, SectionHeader ++) {
    if ((SectionHeader->VirtualAddress != SectionHeader->PointerToRawData) || 
        (SectionHeader->SizeOfRawData < SectionHeader->Misc.VirtualSize)) {
      return FALSE;
    }
  }
  
  return TRUE;
}

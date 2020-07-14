/** @file  
  Application for RSA Key Retrieving (from PEM and X509) & Signature Validation.

Copyright (c) 2010 - 2011, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include "Cryperf.h"

extern GLOBAL_REMOVE_IF_UNREFERENCED UINT8 TestRootCer[];
extern UINTN TestRootCerSize;

extern GLOBAL_REMOVE_IF_UNREFERENCED UINT8 TestRootKey[];
extern UINTN TestRootKeySize;

//
// Message Hash for Signing & Verification Validation.
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST UINT8 MsgHash[] = {
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
  0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07, 0x08, 0x09,
  0x00, 0x01
  };

//
// Payload for PKCS#7 Signing & Verification Validation.
//
GLOBAL_REMOVE_IF_UNREFERENCED CONST CHAR8 *Payload = "Payload Data for PKCS#7 Signing";

UINT8 MsgSignature[] = {
  0x8F, 0xC3, 0xB8, 0xF6, 0xC6, 0xE8, 0x20, 0x6C, 0xF3, 0x9B, 0xB9, 0x08, 0x43, 0x8E, 0x33, 0x0E,
  0x96, 0x9C, 0x07, 0xE4, 0x7A, 0x3F, 0x79, 0x26, 0x50, 0x96, 0x5D, 0x40, 0x7D, 0xFF, 0xB6, 0xF0,
  0x33, 0xC2, 0x3D, 0x91, 0x44, 0xF3, 0xA3, 0xBE, 0x80, 0xAA, 0xB2, 0x21, 0xA5, 0x1B, 0xD1, 0xE6,
  0x8B, 0x25, 0x6E, 0xC1, 0x7D, 0x09, 0x13, 0xDB, 0xA9, 0x91, 0x76, 0xE4, 0xCA, 0x8B, 0xE2, 0xE2,
  0x98, 0x41, 0xE5, 0x93, 0x73, 0x2E, 0x28, 0x9D, 0x6A, 0xEF, 0xE3, 0x6C, 0xEE, 0xA3, 0x63, 0x7D,
  0xD3, 0x73, 0x75, 0xBD, 0x8F, 0x72, 0x50, 0x99, 0xB3, 0x13, 0x03, 0x41, 0x5C, 0xEF, 0x2F, 0x0B,
  0x2D, 0xA3, 0x7F, 0x7A, 0x05, 0x23, 0xB9, 0x18, 0xA3, 0x63, 0x74, 0xB2, 0x37, 0x2C, 0xFD, 0x97,
  0xC1, 0x8D, 0xDB, 0x70, 0x77, 0xC6, 0x50, 0xA5, 0xD8, 0x14, 0xC8, 0x69, 0x4D, 0xA6, 0xF6, 0x89,
  0x03, 0x28, 0xEE, 0x95, 0x90, 0xE7, 0xEC, 0x8B, 0x4A, 0x1D, 0x1C, 0x19, 0xD9, 0x99, 0xAC, 0xD7,
  0x60, 0x63, 0xE6, 0xC3, 0xC8, 0xD0, 0xD2, 0xFF, 0xE0, 0x0B, 0x4E, 0x3A, 0x5F, 0xE6, 0x94, 0xC1,
  0x2E, 0x2D, 0x9E, 0x0C, 0xDD, 0x25, 0xFA, 0x85, 0x42, 0x42, 0x78, 0x63, 0x51, 0x9A, 0xC6, 0xE8,
  0x15, 0xED, 0x91, 0xE7, 0x7F, 0xE4, 0xA3, 0xAA, 0xD7, 0xC9, 0x5F, 0x27, 0xF7, 0x21, 0x80, 0x25,
  0xAD, 0xCD, 0x92, 0x98, 0x99, 0x14, 0xA2, 0xE8, 0x6C, 0xAA, 0x16, 0x20, 0xD4, 0x6E, 0x1E, 0x28,
  0xBF, 0x77, 0x36, 0x64, 0xC0, 0xDF, 0x38, 0x61, 0x93, 0x0B, 0x57, 0x28, 0x1C, 0xD0, 0xD3, 0xC8,
  0xBF, 0x86, 0x0E, 0x0F, 0xAD, 0xD6, 0xA6, 0x24, 0x81, 0x21, 0x17, 0xA3, 0x98, 0x8D, 0x11, 0x77,
  0x07, 0xF7, 0x1B, 0x87, 0x01, 0x5C, 0xCF, 0x35, 0x57, 0x8B, 0xB7, 0x54, 0xF3, 0x19, 0xDA, 0xE5, 
};

/**
  Validate UEFI-OpenSSL RSA Key Retrieving & Signature Interfaces.

  @retval  EFI_SUCCESS  Validation succeeded.
  @retval  EFI_ABORTED  Validation failed.

**/
EFI_STATUS
ValidateCryptRsa2 (
  VOID
  )
{
  BOOLEAN        Status;
  VOID           *RsaPrivKey;
  VOID           *RsaPubKey;
  UINT8          *Subject;
  UINTN          SubjectSize;
  RETURN_STATUS  ReturnStatus;
  CHAR8          CommonName[64];
  CHAR16         CommonNameUnicode[64];
  UINTN          CommonNameSize;
  UINT64         StartTsc;
  UINT64         EndTsc;
  UINTN          Iteration = GetIteration();
  UINTN          Index;

  Print (L"\nUEFI-OpenSSL RSA Key Retrieving Testing: ");

  //
  // Retrieve RSA private key from encrypted PEM data.
  //
  Print (L"\n- Retrieve RSA Private Key for PEM ...");
  Status = RsaGetPrivateKeyFromPem (TestRootKey, TestRootKeySize, NULL, &RsaPrivKey);
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  } else {
    Print (L"[Pass]");
  }

  //
  // Retrieve RSA public key from X509 Certificate.
  //
  Print (L"\n- Retrieve RSA Public Key from X509 ... ");
  RsaPubKey = NULL;
  Status    = RsaGetPublicKeyFromX509 (TestRootCer, TestRootCerSize, &RsaPubKey);
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  } else {
    Print (L"[Pass]");
  }

  //
  // Verify RSA PKCS#1-encoded Signature.
  //
  Print (L"\n- PKCS#1 Signature Verification ... ");
  StartTsc = AsmReadTsc ();
  for (Index = 0; Index < Iteration; Index++) {
    Status = RsaPkcs1Verify (RsaPubKey, MsgHash, SHA256_DIGEST_SIZE, MsgSignature, sizeof(MsgSignature));
  }
  EndTsc = AsmReadTsc ();
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  } else {
    Print (L"[Pass] - %duS\n", TscToMicrosecond((EndTsc - StartTsc) / Iteration));
  }

  //
  // X509 Certificate Subject Retrieving.
  //
  Print (L"\n- X509 Certificate Subject Bytes Retrieving ... ");
  SubjectSize = 0;
  Status  = X509GetSubjectName (TestRootCer, TestRootCerSize, NULL, &SubjectSize);
  Subject = (UINT8 *)AllocatePool (SubjectSize);
  Status  = X509GetSubjectName (TestRootCer, TestRootCerSize, Subject, &SubjectSize);
  if (!Status) {
    Print (L"[Fail]");
    return EFI_ABORTED;
  } else {
    Print (L"[Pass]");
  }

  //
  // Get CommonName from X509 Certificate Subject
  //
  CommonNameSize = 64;
  ZeroMem (CommonName, CommonNameSize);
  ReturnStatus = X509GetCommonName (TestRootCer, TestRootCerSize, CommonName, &CommonNameSize);
  if (RETURN_ERROR (ReturnStatus)) {
    Print (L"\n  - Retrieving Common Name - [Fail]");
//    return EFI_ABORTED;
  } else {
    AsciiStrToUnicodeStrS (CommonName, CommonNameUnicode, CommonNameSize);
    Print (L"\n  - Retrieving Common Name = \"%s\" (Size = %d)", CommonNameUnicode, CommonNameSize);
  }

  //
  // X509 Certificate Verification.
  //
  Print (L"\n- X509 Certificate Verification with Trusted CA ...");
  StartTsc = AsmReadTsc ();
  for (Index = 0; Index < Iteration; Index++) {
    Status = X509VerifyCert (TestRootCer, TestRootCerSize, TestRootCer, TestRootCerSize);
  }
  EndTsc = AsmReadTsc ();
  if (!Status) {
    Print (L"[Fail]\n");
    return EFI_ABORTED;
  } else {
    Print (L"[Pass] - %duS\n", TscToMicrosecond((EndTsc - StartTsc) / Iteration));
  }

  //
  // Release Resources.
  //
  RsaFree  (RsaPubKey);
  RsaFree  (RsaPrivKey);
  FreePool (Subject);

  return EFI_SUCCESS;
}

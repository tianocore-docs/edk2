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

void test_pki_verify_sign ();
void test_pki_verify_cert ();

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
  UINT64         StartTsc;
  UINT64         EndTsc;
  UINTN          Iteration = GetIteration();
  UINTN          Index;

  Print (L"test_pki_verify_sign\n");
  StartTsc = AsmReadTsc ();
  for (Index = 0; Index < Iteration; Index++) {
    test_pki_verify_sign ();
  }
  EndTsc = AsmReadTsc ();

  Print (L"test_pki_verify_cert\n");
  StartTsc = AsmReadTsc ();
  for (Index = 0; Index < Iteration; Index++) {
    test_pki_verify_cert ();
  }
  EndTsc = AsmReadTsc ();

  return EFI_SUCCESS;
}

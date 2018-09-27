/** @file
  This file declares EDKII SMM Cryptographic Protocol.

  This protocol provides basic cryptograhpic services (including Hash, HMAC,
  RSA, DH, PKCS7, etc.) for SMM environment.

  Copyright (c) 2018, Intel Corporation. All rights reserved.<BR>
  This program and the accompanying materials
  are licensed and made available under the terms and conditions of the BSD License
  which accompanies this distribution.  The full text of the license may be found at
  http://opensource.org/licenses/bsd-license.php

  THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
  WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#ifndef __EDKII_SMM_CRYPT_PROTOCOL_H__
#define __EDKII_SMM_CRYPT_PROTOCOL_H__

#include <Protocol/Crypt.h>

// {47F43018-D33B-43F0-878C-87CE092C3B58}
#define EDKII_SMM_CRYPT_PROTOCOL_GUID \
  { 0x47f43018, 0xd33b, 0x43f0, { 0x87, 0x8c, 0x87, 0xce, 0x9, 0x2c, 0x3b, 0x58 } };

typedef struct _EDKII_SMM_CRYPT_PROTOCOL EDKII_SMM_CRYPT_PROTOCOL;

///
/// SMM Cryptographic Services.
///
struct _EDKII_SMM_CRYPT_PROTOCOL {
  //
  // MD5 Hash
  //
  EDKII_CRYPT_MD5_GET_CONTEXTSIZE                 Md5GetContextSize;
  EDKII_CRYPT_MD5_INIT                            Md5Init;
  EDKII_CRYPT_MD5_DUPLICATE                       Md5Duplicate;
  EDKII_CRYPT_MD5_UPDATE                          Md5Update;
  EDKII_CRYPT_MD5_FINAL                           Md5Final;
  EDKII_CRYPT_MD5_HASHALL                         Md5HashAll;
  //
  // SHA1 Hash
  //
  EDKII_CRYPT_SHA1_GET_CONTEXTSIZE                Sha1GetContextSize;
  EDKII_CRYPT_SHA1_INIT                           Sha1Init;
  EDKII_CRYPT_SHA1_DUPLICATE                      Sha1Duplicate;
  EDKII_CRYPT_SHA1_UPDATE                         Sha1Update;
  EDKII_CRYPT_SHA1_FINAL                          Sha1Final;
  EDKII_CRYPT_SHA1_HASHALL                        Sha1HashAll;
  //
  // SHA256 Hash
  //
  EDKII_CRYPT_SHA256_GET_CONTEXTSIZE              Sha256GetContextSize;
  EDKII_CRYPT_SHA256_INIT                         Sha256Init;
  EDKII_CRYPT_SHA256_DUPLICATE                    Sha256Duplicate;
  EDKII_CRYPT_SHA256_UPDATE                       Sha256Update;
  EDKII_CRYPT_SHA256_FINAL                        Sha256Final;
  EDKII_CRYPT_SHA256_HASHALL                      Sha256HashAll;

  //
  // HMAC-SHA256
  //
  EDKII_CRYPT_HMAC_SHA256_GET_CONTEXTSIZE         HmacSha256GetContextSize;
  EDKII_CRYPT_HMAC_SHA256_NEW                     HmacSha256New;
  EDKII_CRYPT_HMAC_SHA256_FREE                    HmacSha256Free;
  EDKII_CRYPT_HMAC_SHA256_INIT                    HmacSha256Init;
  EDKII_CRYPT_HMAC_SHA256_DUPLICATE               HmacSha256Duplicate;
  EDKII_CRYPT_HMAC_SHA256_UPDATE                  HmacSha256Update;
  EDKII_CRYPT_HMAC_SHA256_FINAL                   HmacSha256Final;

  //
  // AES
  //
  EDKII_CRYPT_AES_GET_CONTEXTSIZE                 AesGetContextSize;
  EDKII_CRYPT_AES_INIT                            AesInit;
  EDKII_CRYPT_AES_ECB_ENCRYPT                     AesEcbEncrypt;
  EDKII_CRYPT_AES_ECB_DECRYPT                     AesEcbDecrypt;
  EDKII_CRYPT_AES_CBC_ENCRYPT                     AesCbcEncrypt;
  EDKII_CRYPT_AES_CBC_DECRYPT                     AesCbcDecrypt;

  //
  // RSA
  //
  EDKII_CRYPT_RSA_NEW                             RsaNew;
  EDKII_CRYPT_RSA_FREE                            RsaFree;
  EDKII_CRYPT_RSA_SET_KEY                         RsaSetKey;
  EDKII_CRYPT_RSA_PKCS1_VERIFY                    RsaPkcs1Verify;
  EDKII_CRYPT_RSA_GET_PRIVATE_KEY_FROM_PEM        RsaGetPrivateKeyFromPem;

  //
  // X.509
  //
  EDKII_CRYPT_X509_GET_SUBJECT_NAME               X509GetSubjectName;
  EDKII_CRYPT_X509_GET_COMMON_NAME                X509GetCommonName;
  EDKII_CRYPT_X509_VERIFY_CERT                    X509VerifyCert;
  EDKII_CRYPT_X509_CONSTRUCT_CERTIFICATE          X509ConstructCertificate;
  EDKII_CRYPT_X509_CONSTRUCT_CERTIFICATE_STACK    X509ConstructCertificateStack;
  EDKII_CRYPT_X509_FREE                           X509Free;
  EDKII_CRYPT_X509_STACK_FREE                     X509StackFree;
  EDKII_CRYPT_X509_GET_TBSCERT                    X509GetTBSCert;
  //
  // PKCS#5
  //
  EDKII_CRYPT_PKCS5_HASH_PASSWORD                 Pkcs5HashPassword;
  //
  // PKCS#7
  //
  EDKII_CRYPT_PKCS7_VERIFY                        Pkcs7Verify;

  //
  // Random Generation
  //
  EDKII_CRYPT_RANDOM_SEED                         RandomSeed;
  EDKII_CRYPT_RANDOM_BYTES                        RandomBytes;
};

extern EFI_GUID gEdkiiSmmCryptProtocolGuid;

#endif // __EDKII_SMM_CRYPT_PROTOCOL_H__

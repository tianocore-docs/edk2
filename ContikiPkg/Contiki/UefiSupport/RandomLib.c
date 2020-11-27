/** @file
  ACPI Sdt Fuzzing Test Driver

Copyright (c) 2014, Intel Corporation. All rights reserved.
This program and the accompanying materials
are licensed and made available under the terms and conditions of the BSD License
which accompanies this distribution.  The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php

THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.

**/

#include <Base.h>
#include <Uefi.h>
#include <Library/BaseLib.h>
#include <Library/BaseMemoryLib.h>

typedef
UINT32
(* HASH_TRANSFORM_FUNC) (
  IN      UINT32                      A,
  IN      UINT32                      B,
  IN      UINT32                      C
  );

#define _ROTATE_LEFT(v,s) \
  (((v) << (s)) | ((v) >> (32 - (s))))

#define ROTATE_LEFT(v,s)  \
  _ROTATE_LEFT(v, s)

//
// Transform functions for MD5 & SHA1
//

UINT32
TF1 (
  UINT32 A,
  UINT32 B,
  UINT32 C
  )
{
  return (A & B) | (~A & C);
}

UINT32
TF2 (
  UINT32 A,
  UINT32 B,
  UINT32 C
  )
{
  return (A & C) | (B & ~C);
}

UINT32
TF3 (
  UINT32 A,
  UINT32 B,
  UINT32 C
  )
{
  return A ^ B ^ C;
}

UINT32
TF4 (
  UINT32 A,
  UINT32 B,
  UINT32 C
  )
{
  return B ^ (A | ~C);
}

UINT32
TF5 (
  UINT32 A,
  UINT32 B,
  UINT32 C
  )
{
  return (A & B) | (A & C) | (B & C);
}

//
//  Hash constants
// 
CONST UINT8 gEfiMd5Sha1Init[] = {
  0x01, 0x23, 0x45, 0x67,
  0x89, 0xab, 0xcd, 0xef,
  0xfe, 0xdc, 0xba, 0x98,
  0x76, 0x54, 0x32, 0x10,
  0xf0, 0xe1, 0xd2, 0xc3
};

CONST UINT8 gEfiHashPadding[] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};

//
// SHA1 algo
//

#define SHA1_HASHSIZE 20

typedef struct __SHA1_CTX {
  EFI_STATUS                          Status;
  UINT64                              Length;
  UINT32                              States[SHA1_HASHSIZE / sizeof(UINT32)];
  UINT8                               M[64];
  UINTN                               Count;
} SHA1_CTX;

CONST HASH_TRANSFORM_FUNC  SHA1_F[] = {
  TF1,
  TF3,
  TF5,
  TF3
};

CONST UINT32                   SHA1_K[] = {
  0x5A827999,
  0x6ED9EBA1,
  0x8F1BBCDC,
  0xCA62C1D6
};

#define REVERSE_ENDIAN32(x) \
    ((((x) >> 24) & 0xFF) | (((x) >> 8) & 0xFF00) | (((x) << 8) & 0xFF0000) | (((x) << 24) & 0xFF000000))

VOID
SHA1_transform(
  IN      SHA1_CTX                    *Sha1Ctx
  )
{
  UINT32  i;
  UINT32  s;
  UINT32  e;
  UINT32  t;
  UINT32  temp;
  UINT32  W[80];
  UINT32  S[SHA1_HASHSIZE / sizeof(UINT32)];

  CopyMem (S, Sha1Ctx->States, SHA1_HASHSIZE);

  for (t = 0; t < 16; t++) {
    W[t] = REVERSE_ENDIAN32 (*(UINT32 *) &Sha1Ctx->M[t << 2]);
  }

  for (; t < 80; t++) {
    W[t]  = W[t - 3] ^ W[t - 8] ^ W[t - 14] ^ W[t - 16];
    W[t]  = ROTATE_LEFT (W[t], 1);
  }

  for (i = 0; i < 4; i++) {
    s = i * 20;
    e = s + 20;
    for (t = s; t < e; t++) {
      temp  = ROTATE_LEFT (S[0], 5) + (*SHA1_F[i]) (S[1], S[2], S[3]) + S[4] + W[t] + SHA1_K[i];
      S[4]  = S[3];
      S[3]  = S[2];
      S[2]  = ROTATE_LEFT (S[1], 30);
      S[1]  = S[0];
      S[0]  = temp;
    }
  }

  for (i = 0; i < SHA1_HASHSIZE / sizeof (UINT32); i++) {
    Sha1Ctx->States[i] += S[i];
  }
}

EFI_STATUS
SHA1_init (
  IN      SHA1_CTX                    *Sha1Ctx
  )
{
  ZeroMem (Sha1Ctx, sizeof (*Sha1Ctx));
  CopyMem (Sha1Ctx->States, gEfiMd5Sha1Init, sizeof (Sha1Ctx->States));
  return EFI_SUCCESS;
}

EFI_STATUS
SHA1_update (
  IN      SHA1_CTX                    *Sha1Ctx,
  IN      VOID                        *Data,
  IN      UINTN                       DataLen
  )
{
  UINTN Limit;
  UINT8 *D;

  if (EFI_ERROR (Sha1Ctx->Status) || DataLen == 0) {
    return Sha1Ctx->Status;
  }

  D = (UINT8 *) Data;
  Sha1Ctx->Length += DataLen;
  for (Limit = 64 - Sha1Ctx->Count; DataLen >= Limit; Limit = 64) {
    CopyMem (Sha1Ctx->M + Sha1Ctx->Count, D, Limit);
    SHA1_transform (Sha1Ctx);
    Sha1Ctx->Count = 0;
    D += Limit;
    DataLen -= Limit;
  }

  CopyMem (Sha1Ctx->M + Sha1Ctx->Count, D, DataLen);
  Sha1Ctx->Count += DataLen;
  return EFI_SUCCESS;
}

EFI_STATUS
SHA1_final (
  IN      SHA1_CTX                    *Sha1Ctx,
  OUT     UINT8                       **HashVal
  )
{
  UINTN  PadLen;
  UINTN  i;
  UINT32 LenInBits[3];
  UINT64 Data64;

  if (Sha1Ctx->Status == EFI_ALREADY_STARTED) {
    *HashVal = (UINT8 *) Sha1Ctx->States;
    return EFI_SUCCESS;
  }

  if (EFI_ERROR (Sha1Ctx->Status)) {
    return Sha1Ctx->Status;
  }

  PadLen = Sha1Ctx->Count < 56 ? 64 - 8 : (64 << 1) - 8;
  PadLen -= Sha1Ctx->Count;
  
  Data64 = LShiftU64 (Sha1Ctx->Length, 3);
  CopyMem (&LenInBits[1], &Data64, sizeof(Data64));
  LenInBits[0]                = REVERSE_ENDIAN32 (LenInBits[2]);
  LenInBits[1]                = REVERSE_ENDIAN32 (LenInBits[1]);
  SHA1_update (Sha1Ctx, (VOID *) gEfiHashPadding, PadLen);
  SHA1_update (Sha1Ctx, &LenInBits, sizeof (Sha1Ctx->Length));
  for (i = 0; i < SHA1_HASHSIZE / sizeof (UINT32); i++) {
    Sha1Ctx->States[i] = REVERSE_ENDIAN32 (Sha1Ctx->States[i]);
  }

  ZeroMem (Sha1Ctx->M, sizeof (Sha1Ctx->M));
  Sha1Ctx->Length = 0;
  Sha1Ctx->Status = EFI_ALREADY_STARTED;
  return SHA1_final (Sha1Ctx, HashVal);
}

EFI_STATUS
SHA1Init(
  IN      VOID                        *Context
  )
{
  return SHA1_init((SHA1_CTX*)Context);
}

EFI_STATUS
SHA1Update(
  IN      VOID                        *Context,
  IN      VOID                        *Data,
  IN      UINTN                       DataLen
  )
{
  return SHA1_update((SHA1_CTX*)Context, Data, DataLen);
}

EFI_STATUS
SHA1Final(
  IN      VOID                        *Context,
  OUT     UINT8                       **HashVal
  )
{
  return SHA1_final((SHA1_CTX*)Context, HashVal);
}

//
// MD5 algo
//

#define MD5_HASHSIZE 16

typedef struct __MD5_CTX {
  EFI_STATUS                          Status;
  UINT64                              Length;
  UINT32                              States[MD5_HASHSIZE / sizeof(UINT32)];
  UINT8                               M[64];
  UINTN                               Count;
} MD5_CTX;

CONST HASH_TRANSFORM_FUNC MD5_F[] = {
  TF1, TF2, TF3, TF4
};

CONST UINT32                MD5_K[][2] = {
  { 0, 1 },
  { 1, 5 },
  { 5, 3 },
  { 0, 7 }
};

CONST  UINT32               MD5_S[][4] = {
  { 7, 22, 17, 12 },
  { 5, 20, 14, 9 },
  { 4, 23, 16 ,11 },
  { 6, 21, 15, 10 },
};

CONST  UINT32                MD5_T[] = {
  0xD76AA478, 0xE8C7B756, 0x242070DB, 0xC1BDCEEE,
  0xF57C0FAF, 0x4787C62A, 0xA8304613, 0xFD469501,
  0x698098D8, 0x8B44F7AF, 0xFFFF5BB1, 0x895CD7BE,
  0x6B901122, 0xFD987193, 0xA679438E, 0x49B40821,
  0xF61E2562, 0xC040B340, 0x265E5A51, 0xE9B6C7AA,
  0xD62F105D, 0x02441453, 0xD8A1E681, 0xE7D3FBC8,
  0x21E1CDE6, 0xC33707D6, 0xF4D50D87, 0x455A14ED,
  0xA9E3E905, 0xFCEFA3F8, 0x676F02D9, 0x8D2A4C8A,
  0xFFFA3942, 0x8771F681, 0x6D9D6122, 0xFDE5380C,
  0xA4BEEA44, 0x4BDECFA9, 0xF6BB4B60, 0xBEBFBC70,
  0x289B7EC6, 0xEAA127FA, 0xD4EF3085, 0x04881D05,
  0xD9D4D039, 0xE6DB99E5, 0x1FA27CF8, 0xC4AC5665,
  0xF4292244, 0x432AFF97, 0xAB9423A7, 0xFC93A039,
  0x655B59C3, 0x8F0CCC92, 0xFFEFF47D, 0x85845DD1,
  0x6FA87E4F, 0xFE2CE6E0, 0xA3014314, 0x4E0811A1,
  0xF7537E82, 0xBD3AF235, 0x2AD7D2BB, 0xEB86D391
};

#define SA                         S[j & 3]
#define SB                         S[(j + 1) & 3]
#define SC                         S[(j + 2) & 3]
#define SD                         S[(j + 3) & 3]

VOID
MD5_transform(
  IN      MD5_CTX                     *Md5Ctx
  )
{
  UINT32  i;
  UINT32  j;
  UINT32  S[MD5_HASHSIZE >> 2];
  UINT32  *X;
  UINT32  k;
  UINT32  t;

  X = (UINT32 *) Md5Ctx->M;

  //
  // Copy MD5 states to S
  //
  CopyMem (S, Md5Ctx->States, MD5_HASHSIZE);

  t = 0;
  for (i = 0; i < 4; i++) {
    k = MD5_K[i][0];
    for (j = 16; j > 0; j--) {
      SA += (*MD5_F[i]) (SB, SC, SD) + X[k] + MD5_T[t];
      SA = ROTATE_LEFT (SA, MD5_S[i][j & 3]);
      SA += SB;

      k += MD5_K[i][1];
      k &= 15;

      t++;
    }
  }

  for (i = 0; i < 4; i++) {
    Md5Ctx->States[i] += S[i];
  }
}

VOID
MD5_update_block(
  IN      MD5_CTX                     *Md5Ctx,
  IN      const UINT8                 *Data,
  IN      UINTN                       DataLen
  )
{
  UINTN Limit;

  for (Limit = 64 - Md5Ctx->Count; DataLen >= 64 - Md5Ctx->Count; Limit = 64) {
    CopyMem (Md5Ctx->M + Md5Ctx->Count, Data, Limit);
    MD5_transform (Md5Ctx);
    Md5Ctx->Count = 0;
    Data += Limit;
    DataLen -= Limit;
  }

  CopyMem (Md5Ctx->M + Md5Ctx->Count, Data, DataLen);
  Md5Ctx->Count += DataLen;
}

EFI_STATUS
MD5_init(
  IN      MD5_CTX                     *Md5Ctx
  )
{
  ZeroMem (Md5Ctx, sizeof (*Md5Ctx));
  CopyMem (Md5Ctx->States, gEfiMd5Sha1Init, sizeof (Md5Ctx->States));
  return EFI_SUCCESS;
}

EFI_STATUS
MD5_update(
  IN      MD5_CTX                     *Md5Ctx,
  IN      VOID                        *Data,
  IN      UINTN                       DataLen
  )
{
  if (EFI_ERROR (Md5Ctx->Status)) {
    return Md5Ctx->Status;
  }

  MD5_update_block (Md5Ctx, (const UINT8 *) Data, DataLen);
  Md5Ctx->Length += DataLen;
  return EFI_SUCCESS;
}

EFI_STATUS
MD5_final(
  IN      MD5_CTX                     *Md5Ctx,
  OUT     UINT8                       **HashVal
  )
{
  UINTN PadLength;

  if (Md5Ctx->Status == EFI_ALREADY_STARTED) {
    *HashVal = (UINT8 *) Md5Ctx->States;
    return EFI_SUCCESS;
  }

  if (EFI_ERROR (Md5Ctx->Status)) {
    return Md5Ctx->Status;
  }

  PadLength = Md5Ctx->Count >= 56 ? 120 : 56;
  PadLength -= Md5Ctx->Count;
  MD5_update_block (Md5Ctx, gEfiHashPadding, PadLength);
  Md5Ctx->Length = LShiftU64 (Md5Ctx->Length, 3);
  MD5_update_block (Md5Ctx, (const UINT8 *) &Md5Ctx->Length, 8);

  ZeroMem (Md5Ctx->M, sizeof (Md5Ctx->M));
  Md5Ctx->Length  = 0;
  Md5Ctx->Status  = EFI_ALREADY_STARTED;
  return MD5_final (Md5Ctx, HashVal);
}

EFI_STATUS
MD5Init(
  IN      VOID                        *Context
  )
{
  return MD5_init((MD5_CTX*)Context);
}

EFI_STATUS
MD5Update(
  IN      VOID                        *Context,
  IN      VOID                        *Data,
  IN      UINTN                       DataLen
  )
{
  return MD5_update((MD5_CTX*)Context, Data, DataLen);
}

EFI_STATUS
MD5Final(
  IN      VOID                        *Context,
  OUT     UINT8                       **HashVal
  )
{
  return MD5_final((MD5_CTX*)Context, HashVal);
}

//
// GenRandom
//
SHA1_CTX                     mSha1Seed;

VOID
Nonce(
  IN OUT  UINT64                      *Nonce
  )
{
  UINT64  Tsc;

#ifdef MDE_CPU_IPF
  Tsc = AsmReadItc ();
#else
  Tsc = AsmReadTsc ();
#endif

  *Nonce += Tsc;

  return ;
}

VOID
Seed(
  OUT     SHA1_CTX                    **Ctx
  )
{
  UINT64                              t;

  // K8 Error ignore:
  // Do not initialize it purposely, because we need use random value in stack.
  Nonce(&t);

  SHA1_update(&mSha1Seed, &t, sizeof(t));
  *Ctx = &mSha1Seed;
  return ;
}

VOID
Gen16Bytes(
  OUT     UINT8                       *Buffer
  )
{
  UINT8                               *Digest;
  SHA1_CTX                            *Ctx;
  SHA1_CTX                            GenCtx;
  MD5_CTX                             md5Ctx;

  Seed(&Ctx);

  CopyMem(&GenCtx, Ctx, sizeof(SHA1_CTX));

  SHA1_final(&GenCtx, &Digest);
  CopyMem(Buffer, Digest,MD5_HASHSIZE);

  MD5_init(&md5Ctx);
  MD5_update(&md5Ctx, Digest, SHA1_HASHSIZE);
  MD5_final(&md5Ctx, &Digest);
  SHA1_update(Ctx, Digest, MD5_HASHSIZE);
  return ;
}

VOID
GenerateRandom(
  IN OUT  UINT8                       *Buffer,
  IN      UINTN                       BufLen
  )
{
  UINT8                               RanBuf[16];
  UINTN                               i;
  SHA1_CTX                            *Ctx;

  Seed(&Ctx);
  SHA1_update(Ctx, RanBuf, sizeof(RanBuf));
  SHA1_update(Ctx, Buffer, BufLen);
  SHA1_update(Ctx, (UINT8*)&Buffer, sizeof(UINT8*));
  SHA1_update(Ctx, (UINT8*)&BufLen, sizeof(BufLen));

  i = 0;
  if (BufLen > 16) {
    for (i = 0; i < BufLen - 16; i += 16) {
      Gen16Bytes(RanBuf);
      CopyMem(Buffer + i, RanBuf, 16);
    }
  }

  Gen16Bytes(RanBuf);
  CopyMem(Buffer + i, RanBuf, BufLen - i);
  return ;
}

void srand(unsigned seed)
{
  SHA1_CTX                            *Ctx;

  Seed(&Ctx);
}

int rand(void)
{
  int R;

  GenerateRandom ((UINT8 *)&R, sizeof(R));
  return R;
}

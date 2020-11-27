/** @file
  Null instance of Sec Platform Hook Lib.

  Copyright(c) 2013 Intel Corporation. All rights reserved.

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

//
// The package level header files this module uses
//
#include "QuarkSecLib.h"

/**
  Library constructor for platform SEC library.

  The constructor will be invoked by SecCore module in IA32FamilyCpuBasePkg once the transition
  to C code at SecStartup() function.
  
**/
RETURN_STATUS
EFIAPI
QNCSecLibConstructor (
  VOID
  )
{
  SecCpuInitTimer ();
  
  return RETURN_SUCCESS;
}

/*
boot_mode          - 0x00000001
uart_mmio_base     - 0xA0019000
dram_width         - 0x00
ddr_speed          - 0x00
ddr_type           - 0x00
ecc_enables        - 0x00
scrambling_enables - 0x01
rank_enables       - 0x00000001
channel_enables    - 0x00000001
channel_width      - 0x00000001
address_mode       - 0x00000000
refresh_rate       - 0x03
sr_temp_range      - 0x00
ron_value          - 0x00
rtt_nom_value      - 0x02
rd_odt_value       - 0x00
board_id           - 0x00000000
hte_setup          - 0x00
menu_after_mrc     - 0x00
power_down_disable - 0x00
tune_rcvn          - 0x00
channel_size[0]    - 0x00000000
column_bits[0]     - 0x00000000
row_bits[0]        - 0x00000000
mrs1               - 0x00000000
status             - 0x00000000
mem_size           - 0x00000000
MRC density=01 tCL=06 tRAS=0000927c tWTR=00002710 tRRD=00002710 tFAW=00009c40
*/

GLOBAL_REMOVE_IF_UNREFERENCED UINT8  mMrcDataBin[] = {
0x01, 0x00, 0x00, 0x00, 0x00, 0x90, 0x01, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00,
0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x03, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x01, 0x06, 0x00, 0x00, 0x7C, 0x92, 0x00, 0x00,
0x10, 0x27, 0x00, 0x00, 0x10, 0x27, 0x00, 0x00, 0x40, 0x9C, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
};


VOID
MrcConfigureFromMcFuses (
  OUT MRC_PARAMS                          *MrcData
  )
{
  // MRC McFuseStat 0x00000429
  // MRC Fuse : fus_dun_ecc_dis.
  MrcData->ecc_enables = 0;
  return;
}

VOID
MrcConfigureFromInfoHob (
  OUT MRC_PARAMS                          *MrcData
  )
{
  // MRC dram_width 0
  // MRC rank_enables 1
  // MRC ddr_speed 0
  // MRC flags: SCRAMBLE_EN
  // MRC density=1 tCL=6 tRAS=37500 tWTR=10000 tRRD=10000 tFAW=40000
#if 0
  if (sizeof(*MrcData) != sizeof(mMrcDataBin)) {
    DEBUG_WRITE_STRING ("sizeof(MrcData) ");
    DEBUG_WRITE_UINT32 (sizeof(MrcData));
    DEBUG_WRITE_STRING ("\n");
    DEBUG_WRITE_STRING ("sizeof(mMrcDataBin)\n");
    DEBUG_WRITE_UINT32 (sizeof(mMrcDataBin));
    DEBUG_WRITE_STRING ("\n");
    CpuDeadLoop ();
  }
#endif
  MrcData->channel_enables     = 1;
  MrcData->channel_width       = 1;
  MrcData->address_mode        = 0;
  // Enable scrambling if requested.
  MrcData->scrambling_enables  = 1;
  MrcData->ddr_type            = 0;
  MrcData->dram_width          = 0;
  MrcData->ddr_speed           = 0;
  // Enable ECC if requested.
  MrcData->rank_enables        = 1;
  MrcData->params.DENSITY      = 1;
  MrcData->params.tCL          = 6;
  MrcData->params.tRAS         = 37500;
  MrcData->params.tWTR         = 10000;
  MrcData->params.tRRD         = 10000;
  MrcData->params.tFAW         = 40000;

  MrcData->refresh_rate        = 3;
  MrcData->sr_temp_range       = 0;
  MrcData->ron_value           = 0;
  MrcData->rtt_nom_value       = 2;
  MrcData->rd_odt_value        = 0;

}

VOID
MrcUartConfig(
  MRC_PARAMS *MrcData
  )
{
	UINT8    UartIdx;
	UINT32   RegData32;
	UINT8    IohUartBus;
	UINT8    IohUartDev;

	UartIdx    = PcdGet8(PcdIohUartFunctionNumber);
	IohUartBus = PcdGet8(PcdIohUartBusNumber);
	IohUartDev = PcdGet8(PcdIohUartDevNumber);

	RegData32 = PciRead32 (PCI_LIB_ADDRESS(IohUartBus,  IohUartDev, UartIdx, PCI_BASE_ADDRESSREG_OFFSET));
	MrcData->uart_mmio_base = RegData32 & 0xFFFFFFF0;
  DEBUG_WRITE_STRING ("MRC uart_mmio_base ");
  DEBUG_WRITE_UINT32 (MrcData->uart_mmio_base);
  DEBUG_WRITE_STRING ("\n");
}

VOID
PrepareData (
    IN OUT MRCParams_t *MrcData
    )
{
  MrcData->boot_mode = bmCold;

  MrcConfigureFromMcFuses (MrcData);
  MrcConfigureFromInfoHob (MrcData);
  MrcUartConfig (MrcData);


  DEBUG_WRITE_STRING ("MRC dram_width ");
  DEBUG_WRITE_UINT8 (MrcData->dram_width);
  DEBUG_WRITE_STRING ("\n");
  DEBUG_WRITE_STRING ("MRC rank_enables ");
  DEBUG_WRITE_UINT32 (MrcData->rank_enables);
  DEBUG_WRITE_STRING ("\n");
  DEBUG_WRITE_STRING ("MRC ddr_speed ");
  DEBUG_WRITE_UINT8 (MrcData->ddr_speed);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("MRC flags: ");
  DEBUG_WRITE_UINT8 (MrcData->scrambling_enables);
  DEBUG_WRITE_STRING ("\n");

  DEBUG_WRITE_STRING ("MRC density=");
  DEBUG_WRITE_UINT8 (MrcData->params.DENSITY);
  DEBUG_WRITE_STRING (" tCL=");
  DEBUG_WRITE_UINT8 (MrcData->params.tCL);
  DEBUG_WRITE_STRING (" tRAS=");
  DEBUG_WRITE_UINT32 (MrcData->params.tRAS);
  DEBUG_WRITE_STRING (" tWTR=");
  DEBUG_WRITE_UINT32 (MrcData->params.tWTR);
  DEBUG_WRITE_STRING (" tRRD=");
  DEBUG_WRITE_UINT32 (MrcData->params.tRRD);
  DEBUG_WRITE_STRING (" tFAW=");
  DEBUG_WRITE_UINT32 (MrcData->params.tFAW);
  DEBUG_WRITE_STRING ("\n");

  return ;
}

typedef struct {
  UINT32  Index;
  UINT64  Data;
} MTRR_SETTING;

UINT32   mMsrIndex[] = {
  MTRR_LIB_IA32_MTRR_CAP, // 0x0FE
//  MTRR_LIB_IA32_MTRR_CAP_VCNT_MASK, // 0x0FF
  MTRR_LIB_IA32_MTRR_FIX64K_00000, // 0x250
  MTRR_LIB_IA32_MTRR_FIX16K_80000, // 0x258
  MTRR_LIB_IA32_MTRR_FIX16K_A0000, // 0x259
  MTRR_LIB_IA32_MTRR_FIX4K_C0000, // 0x268
  MTRR_LIB_IA32_MTRR_FIX4K_C8000, // 0x269
  MTRR_LIB_IA32_MTRR_FIX4K_D0000, // 0x26A
  MTRR_LIB_IA32_MTRR_FIX4K_D8000, // 0x26B
  MTRR_LIB_IA32_MTRR_FIX4K_E0000, // 0x26C
  MTRR_LIB_IA32_MTRR_FIX4K_E8000, // 0x26D
  MTRR_LIB_IA32_MTRR_FIX4K_F0000, // 0x26E
  MTRR_LIB_IA32_MTRR_FIX4K_F8000, // 0x26F
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE, // 0x200
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 1,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 2,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 3,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 4,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 5,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 6,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 7,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 8,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 9,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 0xA,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 0xB,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 0xC,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 0xD,
  MTRR_LIB_IA32_VARIABLE_MTRR_BASE + 0xE,
  MTRR_LIB_IA32_VARIABLE_MTRR_END, // 0x20F
  MTRR_LIB_IA32_MTRR_DEF_TYPE, // 0x2FF
};

MTRR_SETTINGS mMtrrSetting = {
  // Fixed
  {
    0x0606060606060606ull,
    0x0606060606060606ull,
  },
  // Variables
  {
    0xFFFE0006ull,
    0xFFFE0800ull,
    0x00000006ull,
    0xF0000800ull,
    0x0FE00000ull,
    0xFFE00800ull,
  },
  // MtrrDefType
  MTRR_LIB_CACHE_MTRR_ENABLED | MTRR_LIB_CACHE_FIXED_MTRR_ENABLED,
};


VOID
SetMtrr (
  VOID
  )
{
  AsmInvd ();
  MtrrSetAllMtrrs(&mMtrrSetting);
}

VOID
SetTseg (
  IN UINT32  MemSize
  )
{
  UINT32  Register;

  Register = (UINT32)((MemSize - 1) & SMM_END_MASK);

  MemSize = MemSize - FixedPcdGet32(PcdTSegSize);

  //
  // Update QuarkNcSoc HSMMCTL register
  //
  Register |= (UINT32)(((RShiftU64(MemSize, 16)) & SMM_START_MASK) + (SMM_WRITE_OPEN | SMM_READ_OPEN | SMM_CODE_RD_OPEN));

  DEBUG_WRITE_STRING ("HSMMCTL - ");
  DEBUG_WRITE_UINT32 (Register);
  DEBUG_WRITE_STRING ("\n");

  QncHsmmcWrite (Register);
}

/**
  A developer supplied function to perform platform specific operations.

  It's a developer supplied function to perform any operations appropriate to a
  given platform. It's invoked just before passing control to PEI core by SEC
  core. Platform developer may modify the SecCoreData and PPI list that is
  passed to PEI Core. 

  @param  SecCoreData           The same parameter as passing to PEI core. It
                                could be overridden by this function.
  @param  PpiList               The default PPI list passed from generic SEC
                                part.

  @return The final PPI list that platform wishes to passed to PEI core.

**/
VOID *
EFIAPI
SecPlatformMain (
  IN OUT   VOID        *SecCoreData
  )
{
  MRCParams_t MrcData;
  VOID        *MemoryTop;

  DEBUG_WRITE_STRING ("SecPlatformMain\n");

  DEBUG_WRITE_STRING ("IntelQNCAcpiTimerLibConstructor\n");

  IntelQNCAcpiTimerLibConstructor ();

  DEBUG_WRITE_STRING ("QNCSecLibConstructor\n");

  QNCSecLibConstructor ();

  DEBUG_WRITE_STRING ("PeiQNCPreMemInit\n");

  PeiQNCPreMemInit ();

  DEBUG_WRITE_STRING ("PrepareData\n");

  ZeroMem (&MrcData, sizeof(MrcData));

  PrepareData (&MrcData); 

  DEBUG_WRITE_STRING ("Mrc ...\n");
	//
	// Do memory initialization
	//
	Mrc (&MrcData);

  DEBUG_WRITE_STRING ("Mcr done.\n");
  DEBUG_WRITE_STRING ("  mem_size - ");
  DEBUG_WRITE_UINT32 (MrcData.mem_size);
  DEBUG_WRITE_STRING ("\n");

  SetMtrr ();

  if (MrcData.mem_size != FixedPcdGet32 (PcdMemorySize)) {
    CpuDeadLoop ();
  }

  MemoryTop = (VOID *)(UINTN)(FixedPcdGet32(PcdPlatformBootFvMemBase));

  SetTseg (FixedPcdGet32 (PcdMemorySize));

  //
  // Here decompress the pei fv
  //
  CopyRomFv (SecCoreData);
  return MemoryTop;
}

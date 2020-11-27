#------------------------------------------------------------------------------
#
# Copyright(c) 2013 Intel Corporation. All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# * Redistributions of source code must retain the above copyright
# notice, this list of conditions and the following disclaimer.
# * Redistributions in binary form must reproduce the above copyright
# notice, this list of conditions and the following disclaimer in
# the documentation and/or other materials provided with the
# distribution.
# * Neither the name of Intel Corporation nor the names of its
# contributors may be used to endorse or promote products derived
# from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
# A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
# OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
# SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
# LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
# DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
# THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
#
# Module Name:
#
#  Flat32.S
#
# Abstract:
#
#  This is the code that goes from real-mode to protected mode.
#  It consumes the reset vector, configures the stack, and enters PEI.
#
#
#------------------------------------------------------------------------------


#
# CR0 cache control bit definition
#
.equ                    CR0_CACHE_DISABLE, 0x040000000
.equ                    CR0_NO_WRITE,      0x020000000

.macro RET32
    jmp    %esp
.endm

#
# ROM/SPI/MEMORY Definitions
#
.equ  QUARK_DDR3_MEM_BASE_ADDRESS, (0x000000000)		# Memory Base Address = 0
.equ  QUARK_MAX_DDR3_MEM_SIZE_BYTES, (0x80000000)		# DDR3 Memory Size = 2GB
.equ  QUARK_ESRAM_MEM_BASE_ADDRESS, (QUARK_DDR3_MEM_BASE_ADDRESS + QUARK_MAX_DDR3_MEM_SIZE_BYTES)		# eSRAM Memory above DDR3
.equ  QUARK_ESRAM_MEM_SIZE_BYTES, (0x00080000)		# eSRAM Memory Size = 512K
.equ  QUARK_STACK_SIZE_BYTES, (0x008000)			# Quark stack size = 32K
.equ	QUARK_STACK_BASE_ADDRESS, (QUARK_ESRAM_MEM_BASE_ADDRESS + QUARK_ESRAM_MEM_SIZE_BYTES) - QUARK_STACK_SIZE_BYTES		# Top of eSRAM - stack size

#
# RTC/CMOS definitions
#
.equ  RTC_INDEX, (0x70)
.equ    NMI_DISABLE, (0x80)	# Bit7=1 disables NMI
.equ    NMI_ENABLE, (0x00)	# Bit7=0 disables NMI
.equ  RTC_DATA, (0x71)

#
# PCI Configuration definitions
#
.equ  PCI_CFG, (0x80000000) # PCI configuration access mechanism
.equ  PCI_ADDRESS_PORT, (0xCF8)
.equ  PCI_DATA_PORT, (0xCFC)

#
# Quark PCI devices
#
.equ  HOST_BRIDGE_PFA, (0x0000)   # B0:D0:F0 (Host Bridge)
.equ  ILB_PFA, (0x00F8)	    # B0:D31:F0 (Legacy Block)

#
# ILB PCI Config Registers
#
.equ  BDE, (0x0D4)                                # BIOS Decode Enable register
.equ    DECODE_ALL_REGIONS_ENABLE, (0xFF000000)	  # Decode all BIOS decode ranges

#
# Host Bridge PCI Config Registers
#
.equ  MESSAGE_BUS_CONTROL_REG, (0xD0)       # Message Bus Control Register
.equ    SB_OPCODE_FIELD, (0x18)	            # Bit location of Opcode field
.equ      OPCODE_SIDEBAND_REG_READ, (0x10)	# Read opcode
.equ      OPCODE_SIDEBAND_REG_WRITE, (0x11) # Write opcode
.equ    SB_PORT_FIELD, (0x10)               # Bit location of Port ID field
.equ      MEMORY_ARBITER_PORT_ID, (0x00)
.equ      HOST_BRIDGE_PORT_ID, (0x03)
.equ      MEMORY_MANAGER_PORT_ID, (0x05)
.equ    SB_ADDR_FIELD, (0x08)               # Bit location of Register field
.equ    SB_BE_FIELD, (0x04)	                # Bit location of Byte Enables field
.equ      ALL_BYTE_EN, (0x0F)	              # All Byte Enables
.equ  MESSAGE_DATA_REG, (0xD4)              # Message Data Register

#
# Memory Arbiter Config Registers
#
.equ  AEC_CTRL_OFFSET, (0x00)

#
# Host Bridge Config Registers
#
.equ  HMISC2_OFFSET, (0x03) # PCI configuration access mechanism
.equ    OR_PM_FIELD, (0x10)

.equ  HMBOUND_OFFSET, (0x08)
.equ    HMBOUND_ADDRESS, (QUARK_DDR3_MEM_BASE_ADDRESS + QUARK_MAX_DDR3_MEM_SIZE_BYTES + QUARK_ESRAM_MEM_SIZE_BYTES)
.equ  HECREG_OFFSET, (0x09)
.equ    EC_BASE, (0xE0000000)
.equ    EC_ENABLE, (0x01)
.equ  HLEGACY_OFFSET, (0x0A)
.equ    NMI, (0x00004000)                 # Bit location of Register field
.equ    SMI, (0x00001000)                 # Bit location of Register field
.equ    INTR, (0x00000400)                # Bit location of Register field

#
# Memory Manager Config Registers
#
.equ  ESRAMPGCTRL_BLOCK_OFFSET, (0x82)
.equ  ESRAM_ADDRESS_2G, (0x10000080)

#
# Contrary to the name, this file contains 16 bit code as well.
#
.text
#
# Protected mode portion initializes stack, configures cache, and calls C entry point
#

#----------------------------------------------------------------------------
#
# Procedure:    ProtectedModeEntryPoint
#
# Input:				Executing in 32 Bit Protected (flat) mode
#								cs: 0-4GB
#								ds: 0-4GB
#								es: 0-4GB
#								fs: 0-4GB
#								gs: 0-4GB
#								ss: 0-4GB
#								Maskable interrupts disabled (IF=0)
#               NMI disabled (RTC port)
#               SMI disabled (HMISC2 register)
#
# Output:       This function never returns
#
# Destroys:
#               eax
#               ebx
#               ecx
#	              esi
#               edi
#               ebp
#	              esp
#
# Description:
#								Perform any essential early platform initilaisation
#               Setup a stack
#               Call the main EDKII Sec C code
#
#----------------------------------------------------------------------------
#
# This signature is used by build tools to locate this entry point in the 32-bit SPI code
#
ASM_GLOBAL ASM_PFX(_ModuleEntryPoint)
ASM_PFX(_ModuleEntryPoint):

	leal	L0, %esp
	jmp	stackless_EarlyPlatformInit
L0:

  #
  # Set up stack pointer
  #
  movl    $QUARK_STACK_BASE_ADDRESS, %esp
  movl    $QUARK_STACK_SIZE_BYTES, %esi
  addl    %esi, %esp                          # ESP = top of stack (stack grows downwards).

  #
  # Store the the BIST value in EBP
  #
  movl    $0, %ebp    # No processor BIST on Quark

  #
  # Push processor count to stack first, then BIST status (AP then BSP)
  #
  movl    $1, %eax
  cpuid
  shrl    $16, %ebx
  andl    $0x000000FF, %ebx
  cmpb    $1, %bl
  jae     PushProcessorCount

  #
  # Some processors report 0 logical processors.  Effectively 0 = 1.
  # So we fix up the processor count
  #
  incl    %ebx

PushProcessorCount:  
  pushl   %ebx

  #
  # We need to implement a long-term solution for BIST capture.  For now, we just copy BSP BIST
  # for all processor threads
  #
  xorl    %ecx, %ecx
  movb    %bl, %cl

PushBist: 
  pushl   %ebp
  loop    PushBist

  #
  # Pass entry point of the PEI core
  #
  movl $0xFFFFFFE0, %edi
  pushl %ds:(%edi)

  #
  # Pass BFV into the PEI Core
  # Sec/Pei FV Base Address in eSRAM is QUARK_ESRAM_MEM_BASE_ADDRESS + PcdFvSecurityHeaderSize.
  #
  movl    _gPcd_FixedAtBuild_PcdFvSecurityHeaderSize, %eax
  addl    $QUARK_ESRAM_MEM_BASE_ADDRESS, %eax
  pushl   %eax

  #
  # Pass Temp Ram Base into the PEI Core
  #
  pushl   $QUARK_STACK_BASE_ADDRESS

  #
  # Pass stack size into the PEI Core
  #
  pushl   $QUARK_STACK_SIZE_BYTES

  #
  # Pass Control into the PEI Core
  #
  call SecStartup

  #
  # PEI Core should never return to here, this is just to capture an invalid return.
  #
  jmp     .

#----------------------------------------------------------------------------
#
# Procedure:    stackless_EarlyPlatformInit
#
# Input:        esp - Return address
#
# Output:       None
#
# Destroys:     Assume all registers
#
# Description:
#				Any early platform initialisation required
#
# Return:
#			None
#
#----------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(stackless_EarlyPlatformInit)
ASM_PFX(stackless_EarlyPlatformInit):

  #
  # Save return address
  #
  movl  %esp, %ebp
  
  #
  # Ensure cache is disabled.
  #
  movl %cr0, %eax
  orl $(CR0_CACHE_DISABLE + CR0_NO_WRITE), %eax
  invd
  movl    %eax, %cr0

  #
  # Enable NMI operation
  # Good convention suggests you should read back RTC data port after
  # accessing the RTC index port.
  #
  movb $(NMI_ENABLE), %al
  movw $(RTC_INDEX), %dx
  outb %al, %dx
  movw $(RTC_DATA), %dx
  inb  %dx, %al

  #
  # Clear Host Bridge SMI, NMI, INTR fields
  #
  movl	$((OPCODE_SIDEBAND_REG_READ << SB_OPCODE_FIELD) | (HOST_BRIDGE_PORT_ID << SB_PORT_FIELD) | (HLEGACY_OFFSET << SB_ADDR_FIELD)), %ecx
	leal	L3, %esp
	jmp	stackless_SideBand_Read
L3:
  andl $~(NMI + SMI + INTR), %eax      # Clear NMI, SMI, INTR fields
  movl	$((OPCODE_SIDEBAND_REG_WRITE << SB_OPCODE_FIELD) | (HOST_BRIDGE_PORT_ID << SB_PORT_FIELD) | (HLEGACY_OFFSET << SB_ADDR_FIELD)), %ecx
	leal	L4, %esp
	jmp	stackless_SideBand_Write
L4:
  
  #
  # Set up the HMBOUND register
  #  
  movl $(HMBOUND_ADDRESS), %eax      # Data (Set HMBOUND location)
  movl	$((OPCODE_SIDEBAND_REG_WRITE << SB_OPCODE_FIELD) | (HOST_BRIDGE_PORT_ID << SB_PORT_FIELD) | (HMBOUND_OFFSET << SB_ADDR_FIELD)), %ecx
	leal	L5, %esp
	jmp	stackless_SideBand_Write
L5:  

  #
  # Enable PCIEXBAR
  #  
  movl $(EC_BASE + EC_ENABLE), %eax      # Data
  movl	$((OPCODE_SIDEBAND_REG_WRITE << SB_OPCODE_FIELD) | (MEMORY_ARBITER_PORT_ID << SB_PORT_FIELD) | (AEC_CTRL_OFFSET << SB_ADDR_FIELD)), %ecx
	leal	L6, %esp
	jmp	stackless_SideBand_Write
L6: 

  movl $(EC_BASE + EC_ENABLE), %eax      # Data
  movl	$((OPCODE_SIDEBAND_REG_WRITE << SB_OPCODE_FIELD) | (HOST_BRIDGE_PORT_ID << SB_PORT_FIELD) | (HECREG_OFFSET << SB_ADDR_FIELD)), %ecx
	leal	L7, %esp
	jmp	stackless_SideBand_Write
L7: 

  #
  # Restore return address
  #
  movl	%ebp, %esp
  RET32  
    
#----------------------------------------------------------------------------
#
# Procedure:    stackless_SideBand_Read
#
# Input:        esp - return address
#								ecx[15:8] - Register offset
#								ecx[23:16] - Port ID
#								ecx[31:24] - Opcode
#
# Output:       eax - Data read
#
# Destroys:
#								eax
#								ebx
#								cl
#								esi
#
# Description:
#				Perform requested sideband read
#
#----------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(stackless_SideBand_Read)
ASM_PFX(stackless_SideBand_Read):

  movl	%esp, %esi      # Save the return address
  
	#
	# Load the SideBand Packet Register to generate the transaction
	#
  movl	$((PCI_CFG) | (HOST_BRIDGE_PFA << 8) | (MESSAGE_BUS_CONTROL_REG)), %ebx   # PCI Configuration address
	movb  $(ALL_BYTE_EN << SB_BE_FIELD), %cl      # Set all Byte Enable bits
  xchgl	%ecx, %eax
	leal	L8, %esp
	jmp	stackless_PCIConfig_Write
L8:
  xchgl	%ecx, %eax

	#
	# Read the SideBand Data Register
	#	
  movl	$((PCI_CFG) | (HOST_BRIDGE_PFA << 8) | (MESSAGE_DATA_REG)), %ebx   # PCI Configuration address
	leal	L9, %esp
	jmp	stackless_PCIConfig_Read
L9:
	
  movl	%esi, %esp      # Restore the return address
  RET32
  

#----------------------------------------------------------------------------
#
# Procedure:    stackless_SideBand_Write
#
# Input:        esp - return address
#								eax - Data
#								ecx[15:8] - Register offset
#								ecx[23:16] - Port ID
#								ecx[31:24] - Opcode
#
# Output:       None
#
# Destroys:
#								ebx
#								cl
#								esi
#
# Description:
#				Perform requested sideband write
#
#
#----------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(stackless_SideBand_Write)
ASM_PFX(stackless_SideBand_Write):

  movl	%esp, %esi      # Save the return address

	#
	# Load the SideBand Data Register with the data
	#
  movl	$((PCI_CFG) | (HOST_BRIDGE_PFA << 8) | (MESSAGE_DATA_REG)), %ebx   # PCI Configuration address
	leal	L10, %esp
	jmp	stackless_PCIConfig_Write
L10:
	
	#
	# Load the SideBand Packet Register to generate the transaction
	#	
  movl	$((PCI_CFG) | (HOST_BRIDGE_PFA << 8) | (MESSAGE_BUS_CONTROL_REG)), %ebx   # PCI Configuration address
	movb  $(ALL_BYTE_EN << SB_BE_FIELD), %cl      # Set all Byte Enable bits
  xchgl	%ecx, %eax
	leal	L11, %esp
	jmp	stackless_PCIConfig_Write
L11:
  xchgl	%ecx, %eax
  	
  movl	%esi, %esp      # Restore the return address
  RET32
  
  
#----------------------------------------------------------------------------
#
# Procedure:    stackless_PCIConfig_Write
#
# Input:        esp - return address
#								eax - Data to write
#								ebx - PCI Config Address
#
# Output:       None
#
# Destroys:
#								dx
#
# Description:
#				Perform a DWORD PCI Configuration write
#
#----------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(stackless_PCIConfig_Write)
ASM_PFX(stackless_PCIConfig_Write):
  
	#
	# Write the PCI Config Address to the address port
	#
	xchgl	%ebx, %eax
	movw	$(PCI_ADDRESS_PORT), %dx
	outl	%eax, %dx
	xchgl	%ebx, %eax
	  
	#
	# Write the PCI DWORD Data to the data port
	#
	movw	$(PCI_DATA_PORT), %dx
	outl	%eax, %dx
		  
  RET32
		
		
#----------------------------------------------------------------------------
#
# Procedure:    stackless_PCIConfig_Read
#
# Input:        esp - return address
#								ebx - PCI Config Address
#
# Output:       eax - Data read
#
# Destroys:
#								eax
#								dx
#
# Description:
#				Perform a DWORD PCI Configuration read
#
#----------------------------------------------------------------------------
ASM_GLOBAL ASM_PFX(stackless_PCIConfig_Read)
ASM_PFX(stackless_PCIConfig_Read):
		  
	#
	# Write the PCI Config Address to the address port
	#
	xchgl	%ebx, %eax
	movw	$(PCI_ADDRESS_PORT), %dx
	outl	%eax, %dx
	xchgl	%ebx, %eax
			  
	#
	# Read the PCI DWORD Data from the data port
	#
	movw	$(PCI_DATA_PORT), %dx
	inl	%dx, %eax
	
	RET32


// Copyright (c) 2020 Intel Corporation
//
// SPDX-License-Identifier: BSD-2-Clause-Patent

#[macro_use]
use crate::logger;

use crate::pcd;
use crate::pi;
use crate::pi::hob;
use crate::pi::fv;
use crate::pi::fv_lib;
use crate::elf;

use r_efi::efi;
use core::panic::PanicInfo;
use core::ffi::c_void;
use core::convert::TryInto;

pub const SIZE_4KB  :u64 = 0x00001000u64;
pub const SIZE_1MB  :u64 = 0x00100000u64;
pub const SIZE_16MB :u64 = 0x01000000u64;

#[allow(non_snake_case)]
#[cfg(not(test))]
pub fn GetSystemMemorySizeBelow4Gb() -> (u64, u64) {
    let mut cmos0x34: u8 = 0u8;
    let mut cmos0x35: u8 = 0u8;

    unsafe {
        cmos0x34 = x86::io::inb(0x34u16);
        cmos0x35 = x86::io::inb(0x35u16);
    }

    let mut res: u64 = 0;
    res = ((cmos0x35 as u64) << 8 + (cmos0x34 as u64) << 16) + SIZE_16MB;
    (res, res-SIZE_16MB)
}

#[allow(non_snake_case)]
#[cfg(not(test))]
pub fn EfiSizeToPage(size: u64) -> u64
{
    (size + SIZE_4KB - 1) / SIZE_4KB
}

#[allow(non_snake_case)]
#[cfg(not(test))]
pub fn EfiPageToSize(page: u64) -> u64
{
    page * SIZE_4KB
}

/// flag  ture align to low address else high address
#[allow(non_snake_case)]
fn AlignValue(value: u64, align: u64, flag: bool) -> u64
{
    if flag {
        value & ((!(align - 1)) as u64)
    } else {
        value - (value & (align - 1)) as u64 + align
    }
}

#[allow(non_snake_case)]
#[cfg(not(test))]
pub fn FindAndReportEntryPoint(firmwareVolumePtr: * const fv::FirmwareVolumeHeader) -> u64
{
    let firmwareVolume = unsafe{&(*firmwareVolumePtr)};
    log!("FirmwareVolumeHeader: \n fv_length: {:x}\n signature: {:x}\n", firmwareVolume.fv_length, firmwareVolume.signature);

    let (image, size) = fv_lib::get_image_from_fv(firmwareVolumePtr as u64, pcd::pcd_get_PcdOvmfDxeMemFvSize() as u64, fv::FV_FILETYPE_DXE_CORE, fv::SECTION_PE32);

    // parser file and get entry point
    let data_slice = unsafe {core::slice::from_raw_parts(image as *const u8, size as usize)};
    let data_header = data_slice[0..4].try_into().unwrap();
    let mut entry = 0u64;
    match data_header {
        elf::ELFMAG => {
            log!("Elf FW image\n");
            match data_slice[elf::EI_CLASS] {
                elf::ELFCLASS32 => FindAndReportEntryPointElf32(image, data_slice),
                elf::ELFCLASS64 => FindAndReportEntryPointElf64(image, data_slice),
                _ => {log!("Invalid EI class"); 0u64}
            }
        }
        _ =>{
            log!("Not support\n");
            0u64
        }
    }
}

#[allow(non_snake_case)]
#[cfg(not(test))]
fn FindAndReportEntryPointElf64(image: *const c_void, data_slice: &[u8]) -> u64 {
    let elf_header = elf::ELFHeader64::from_bytes(data_slice);
    log!("--Elf: {:?}", elf_header);
    let phdr_slice = unsafe {core::slice::from_raw_parts((image as u64 + elf_header.e_phoff as u64) as *const u8 , (elf_header.e_ehsize * elf_header.e_phnum) as usize)};

    let pheaders = elf::ProgramHeader64::slice_from_bytes(phdr_slice);

    let mut bottom: u64 = 0xFFFFFFFFu64;
    let mut top:    u64 = 0u64;

    for ph in pheaders.into_iter() {
        log!("pheader: {:?}", ph);
        if ph.p_type == elf::PT_LOAD {
            if bottom > ph.p_vaddr {
                bottom = ph.p_vaddr;
            }
            if top < ph.p_vaddr + ph.p_memsz {
                top = ph.p_vaddr + ph.p_memsz;
            }
        }
    }
    bottom = AlignValue(bottom, SIZE_4KB, true);
    top = AlignValue(top, SIZE_4KB, false);
    log!("BaseHypervisorFw - 0x{:x}\n", bottom);
    log!("Size - 0x{:x}\n", top - bottom);
    log!("HypervisorFwEntryPoint - 0x{:x}\n", elf_header.e_entry);

    // load per program header
    for ph in pheaders.into_iter() {
        if ph.p_type == elf::PT_LOAD {
            unsafe {
                core::ptr::copy_nonoverlapping((image as u64 + ph.p_offset as u64) as *mut u8, ph.p_vaddr as *const u8 as *mut u8, ph.p_filesz as usize)
            };
        }
    }
    elf_header.e_entry as u64
}

#[allow(non_snake_case)]
#[cfg(not(test))]
fn FindAndReportEntryPointElf32(image: *const c_void, data_slice: &[u8]) -> u64 {
    let elf_header = elf::ELFHeader32::from_bytes(data_slice);
    log!("--Elf: {:?}", elf_header);
    let phdr_slice = unsafe {core::slice::from_raw_parts((image as u64 + elf_header.e_phoff as u64) as *const u8 , (elf_header.e_ehsize * elf_header.e_phnum) as usize)};

    let pheaders = elf::ProgramHeader32::slice_from_bytes(phdr_slice);

    let mut bottom: u32 = 0xFFFFFFFFu32;
    let mut top:    u32 = 0u32;

    for ph in pheaders.into_iter() {
        log!("pheader: {:?}", ph);
        if ph.p_type == elf::PT_LOAD {
            if bottom > ph.p_vaddr {
                bottom = ph.p_vaddr;
            }
            if top < ph.p_vaddr + ph.p_memsz {
                top = ph.p_vaddr + ph.p_memsz;
            }
        }
    }
    bottom = AlignValue(bottom as u64, SIZE_4KB, true) as u32;
    top = AlignValue(top as u64, SIZE_4KB, false) as u32;
    log!("BaseHypervisorFw - 0x{:x}\n", bottom);
    log!("Size - 0x{:x}\n", top - bottom);
    log!("HypervisorFwEntryPoint - 0x{:x}\n", elf_header.e_entry);

    // load per program header
    for ph in pheaders.into_iter() {
        if ph.p_type == elf::PT_LOAD {
            unsafe {
                core::ptr::copy_nonoverlapping((image as u64 + ph.p_offset as u64) as *mut u8, ph.p_vaddr as *const u8 as *mut u8, ph.p_filesz as usize)
            };
        }
    }
    elf_header.e_entry as u64
}

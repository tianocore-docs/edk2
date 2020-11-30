// Copyright (c) 2020 Intel Corporation
//
// SPDX-License-Identifier: BSD-2-Clause-Patent

pub const PCI_CONFIGURATION_ADDRESS_PORT:  u16 = 0xCF8;
pub const PCI_CONFIGURATION_DATA_PORT: u16 = 0xCFC;

#[allow(non_snake_case)]
#[cfg(not(test))]
pub fn PciCf8Read32(bus: u8, device: u8, fnc: u8, reg: u8) -> u32
{
    let data = u32::from(bus) << 16;
    let data = data | u32::from(device) << 11;
    let data = data | u32::from(fnc) << 8;
    let data = data | u32::from(reg & 0xfc);
    let data = data | 1u32 << 31;

    let mut result: u32 = 0u32;
    unsafe {
        x86::io::outl(PCI_CONFIGURATION_ADDRESS_PORT, data);
        result = x86::io::inl(PCI_CONFIGURATION_DATA_PORT);
    }
    result
}

#[allow(non_snake_case)]
#[cfg(not(test))]
pub fn PciCf8Write32(bus: u8, device: u8, fnc: u8, reg: u8, value: u32)
{
    let data = u32::from(bus) << 16;
    let data = data | u32::from(device) << 11;
    let data = data | u32::from(fnc) << 8;
    let data = data | u32::from(reg & 0xfc);
    let data = data | 1u32 << 31;

    unsafe {
        x86::io::outl(PCI_CONFIGURATION_ADDRESS_PORT, data);
        x86::io::outl(PCI_CONFIGURATION_DATA_PORT, value);
    }
}

#[allow(non_snake_case)]
#[cfg(not(test))]
pub fn PciCf8Write8(bus: u8, device: u8, fnc: u8, reg: u8, value: u8)
{
    let data = u32::from(bus) << 16;
    let data = data | u32::from(device) << 11;
    let data = data | u32::from(fnc) << 8;
    let data = data | u32::from(reg & 0xfc);
    let data = data | 1u32 << 31;

    unsafe {
        x86::io::outl(PCI_CONFIGURATION_ADDRESS_PORT, data);
        x86::io::outb(PCI_CONFIGURATION_DATA_PORT + (data & 3) as u16, value);
    }
}

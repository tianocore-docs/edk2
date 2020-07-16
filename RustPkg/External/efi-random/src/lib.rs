// Copyright (c) 2020 Intel Corporation
//
// SPDX-License-Identifier: BSD-2-Clause-Patent

#![no_std]
#![feature(llvm_asm)]

pub struct Unsupported;

#[cfg(any(target_os = "uefi"))]
pub fn generate(dest: &mut [u8]) -> Result<(), ()> {
    for i in 0..dest.len() {
        unsafe{
            match rdrand16() {
                Err(_) => (),
                Ok(val) => {dest[i] = val as u8}
            };
        }
    }
    Ok(())
}

#[cfg(any(target_os = "uefi"))]
unsafe fn rdrand16() -> Result<u16, Unsupported> {
    let mut retval: u16;
    let mut randval: u16;
    llvm_asm!(
        "
        .byte  0xf, 0xc7, 0xf0
        jnc    rn16_failed
        mov    cx, ax
        mov    ax,  1
        jmp    rn16_end
    rn16_failed:
        xor    ax, ax
    rn16_end:
        "
        : "={ax}"(retval), "={cx}"(randval)
        ::: "intel"
    );
    if retval == 0 {
        Err(Unsupported)
    } else {
        Ok(randval)
    }
}

#[cfg(test)]
mod tests {
    #[test]
    fn it_works() {

    }
}

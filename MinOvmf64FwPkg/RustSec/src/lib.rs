// Copyright (c) 2020 Intel Corporation
//
// SPDX-License-Identifier: BSD-2-Clause-Patent

#![allow(unused)]

#![feature(llvm_asm)]
#![cfg_attr(not(test), no_std)]
#![cfg_attr(not(test), no_main)]
#![cfg_attr(test, allow(unused_imports))]

#[macro_use]
mod logger;

use core::panic::PanicInfo;

use core::ffi::c_void;

#[cfg(not(test))]
#[panic_handler]
#[allow(clippy::empty_loop)]
fn panic(_info: &PanicInfo) -> ! {
    // log!("panic ... {:?}\n", _info);
    loop {}
}

#[cfg(not(test))]
#[no_mangle]
#[export_name = "SecCoreStartupWithStack"]
pub extern "win64" fn _start(boot_fv: *const c_void, top_of_stack: *const c_void) -> ! {

    log!("Starting SEC boot_fv - {:p}, Top of stack - {:p} \n", boot_fv, top_of_stack);
    loop {}
}

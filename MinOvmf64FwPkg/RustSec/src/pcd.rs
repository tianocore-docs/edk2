// Copyright (c) 2020 Intel Corporation
//
// SPDX-License-Identifier: BSD-2-Clause-Patent

// import PCD from AutoGen.c

extern "C" {
    #[no_mangle]
    static _gPcd_FixedAtBuild_PcdOvmfSecPeiTempRamBase: u32;
    #[no_mangle]
    static _gPcd_FixedAtBuild_PcdOvmfSecPeiTempRamSize: u32;
    #[no_mangle]
    static _gPcd_FixedAtBuild_PcdOvmfSecPageTablesBase: u32;
    #[no_mangle]
    static _gPcd_FixedAtBuild_PcdOvmfDxeMemFvBase: u32;
    #[no_mangle]
    static _gPcd_FixedAtBuild_PcdOvmfDxeMemFvSize: u32;
    #[no_mangle]
    static _gPcd_FixedAtBuild_PcdOvmfPeiMemFvBase: u32;
    #[no_mangle]
    static _gPcd_FixedAtBuild_PcdOvmfPeiMemFvSize: u32;
    #[no_mangle]
    static _gPcd_FixedAtBuild_PcdPciExpressBaseAddress: u64;
}

#[allow(non_snake_case)]
pub fn pcd_get_PcdOvmfSecPeiTempRamSize() -> u32 {
    unsafe{
        _gPcd_FixedAtBuild_PcdOvmfSecPeiTempRamSize
    }
}

#[allow(non_snake_case)]
pub fn pcd_get_PcdOvmfSecPeiTempRamBase() -> u32 {
    unsafe{
        _gPcd_FixedAtBuild_PcdOvmfSecPeiTempRamBase
    }
}

#[allow(non_snake_case)]
pub fn pcd_get_PcdOvmfSecPageTablesBase() -> u32 {
    unsafe{
        _gPcd_FixedAtBuild_PcdOvmfSecPageTablesBase
    }
}

#[allow(non_snake_case)]
pub fn pcd_get_PcdOvmfDxeMemFvBase() -> u32 {
    unsafe{
        _gPcd_FixedAtBuild_PcdOvmfDxeMemFvBase
    }
}

#[allow(non_snake_case)]
pub fn pcd_get_PcdOvmfDxeMemFvSize() -> u32 {
    unsafe{
        _gPcd_FixedAtBuild_PcdOvmfDxeMemFvSize
    }
}

#[allow(non_snake_case)]
pub fn pcd_get_PcdOvmfPeiMemFvBase() -> u32 {
    unsafe{
        _gPcd_FixedAtBuild_PcdOvmfPeiMemFvBase
    }
}

#[allow(non_snake_case)]
pub fn pcd_get_PcdOvmfPeiMemFvSize() -> u32 {
    unsafe{
        _gPcd_FixedAtBuild_PcdOvmfPeiMemFvSize
    }
}

#[allow(non_snake_case)]
pub fn pcd_get_PcdPciExpressBaseAddress() -> u64 {
    unsafe{
        _gPcd_FixedAtBuild_PcdPciExpressBaseAddress
    }
}

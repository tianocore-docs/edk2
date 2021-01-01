# Rust Based SEC-IPL

## Environment Setting

See https://github.com/jyao1/edk2/tree/edkii-rust/RustPkg/

Rust UEFI core: https://github.com/jyao1/rust-hypervisor-firmware

Checkout required crate update for rust-crypto and put to the same directory as edk2.

r-efi: https://github.com/jyao1/edk2/tree/edkii-rust/RustPkg/External/r-efi

plain: https://github.com/jyao1/plain/tree/minovmf

## Build

Open Visual Studio 2019 X64 command windows.

`build -p MinOvmf64FwPkg\MinOvmf64RustFwPkg.dsc -a X64 -t CLANGPDB [-b RELEASE]`

Size comparison (SecMain):

======
C    RELEASE:     9,600 bytes
C    DEBUG:      30,752 bytes
RUST RELEASE:    47,712 bytes
RUST DEBUG:   1,219,264 bytes
======

## Run

Take Windows QEMU as an example:

```
set QEMU_BIN="C:\Program Files\qemu\qemu-system-x86_64.exe"
set BIOS_BIN=<edk2 path>\Build\MinOvmf64Fw\DEBUG_CLANGPDB\FV\OVMF64RUSTFW.fd
@REM set BIOS_BIN=<edk2 path>\Build\Ovmf3264\DEBUG_VS2015x86\FV\OVMF.fd
set SERIAL_LOG=-serial file:serial.log

%QEMU_BIN% -machine q35 -drive if=pflash,format=raw,unit=0,file=%BIOS_BIN%,readonly=on %SERIAL_LOG%
```


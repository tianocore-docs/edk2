// Copyright (c) 2020 Intel Corporation
//
// SPDX-License-Identifier: BSD-2-Clause-Patent
#![cfg_attr(not(test), no_std)]

#[cfg(not(test))]
use r_efi::efi;

//GlobalAlloc and alloc_error_handler installed by efi_services
#[cfg(not(test))]
use efi_services;

#[cfg(not(test))]
extern crate alloc;
#[cfg(not(test))]
use alloc::vec::Vec;

#[cfg(not(test))]
#[panic_handler]
fn panic_handler(panic_info: &core::panic::PanicInfo) -> ! {
    loop {}
}

#[cfg(not(test))]
use ring::signature::{self, RsaEncoding, VerificationAlgorithm};

#[cfg(not(test))]
mod pki_data;
#[cfg(not(test))]
fn test_pki_verify_sign(){
    use webpki::EndEntityCert;
    use ring::rand;
    let cert_der = untrusted::Input::from(&pki_data::PKI_RSA_CA_CERT_DER).as_slice_less_safe();
    let cert = EndEntityCert::from(cert_der).unwrap();

    let key_bytes_der =
    untrusted::Input::from(&pki_data::PKI_RSA_CA_KEY_DER).as_slice_less_safe();
    let key_pair: signature::RsaKeyPair = signature::RsaKeyPair::from_der(key_bytes_der).unwrap();

    const MESSAGE: &'static [u8] = b"hello, world";
    let rng = rand::SystemRandom::new();

    let mut sign = alloc::vec::Vec::new();
    for _i in 0..key_pair.public_modulus_len() {
        sign.push(0u8);
    }
    key_pair.sign(&signature::RSA_PKCS1_SHA256, &rng, &MESSAGE, &mut sign).unwrap();

    //RSA_PSS_SHA256
    cert.verify_signature(&webpki::RSA_PKCS1_2048_8192_SHA256, &MESSAGE, &sign).unwrap();
}

#[cfg(not(test))]
fn test_pki_verify_cert() {
    static ALL_SIGALGS: &[&webpki::SignatureAlgorithm] = &[
        &webpki::RSA_PKCS1_2048_8192_SHA256,
        &webpki::RSA_PKCS1_3072_8192_SHA384,
    ];

    let ee =  &pki_data::PKI_RSA_END_CERT_DER;
    let inter = &pki_data::PKI_RSA_INTER_CERT_DER;
    let ca = &pki_data::PKI_RSA_CA_CERT_DER;

    let mut anchors = alloc::vec::Vec::new();
    anchors.push(webpki::trust_anchor_util::cert_der_as_trust_anchor(ca).unwrap());
    //let anchors = vec![webpki::trust_anchor_util::cert_der_as_trust_anchor(ca).unwrap()];
    let anchors = webpki::TLSServerTrustAnchors(&anchors);

    let time = webpki::Time::from_seconds_since_unix_epoch(1593482917);
    let cert = webpki::EndEntityCert::from(ee).unwrap();
    let _ = cert
        .verify_is_valid_tls_server_cert(ALL_SIGALGS, &anchors, &[inter], time)
        .unwrap();
}

#[cfg(not(test))]
#[no_mangle]
#[export_name = "UEFI_Main"]
pub extern fn main(_h: efi::Handle, st: *mut efi::SystemTable) -> efi::Status {
    unsafe { efi_services::init(_h, st); }

    test_pki_verify_sign();
    test_pki_verify_cert();

    efi::Status::SUCCESS
}


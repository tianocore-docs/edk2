/** @file
  GUIDs for gBS->CreateEventEx Event Groups. Defined in UEFI spec 2.0 and PI 1.2.1.

Copyright (c) 2006 - 2012, Intel Corporation. All rights reserved.<BR>
This program and the accompanying materials are licensed and made available under 
the terms and conditions of the BSD License that accompanies this distribution.  
The full text of the license may be found at
http://opensource.org/licenses/bsd-license.php.                                          
    
THE PROGRAM IS DISTRIBUTED UNDER THE BSD LICENSE ON AN "AS IS" BASIS,                     
WITHOUT WARRANTIES OR REPRESENTATIONS OF ANY KIND, EITHER EXPRESS OR IMPLIED.             

**/

#ifndef __EVENT_GROUP_GUID__
#define __EVENT_GROUP_GUID__

#define EFI_EVENT_GROUP_DXE_DISPATCH_GUID \
  { 0x7081e22f, 0xcac6, 0x4053, { 0x94, 0x68, 0x67, 0x57, 0x82, 0xcf, 0x88, 0xe5 }}

extern EFI_GUID gEfiEventDxeDispatchGuid;

#define EFI_END_OF_DXE_EVENT_GROUP_GUID \
  { 0x2ce967a, 0xdd7e, 0x4ffc, { 0x9e, 0xe7, 0x81, 0xc, 0xf0, 0x47, 0x8, 0x80 } }

extern EFI_GUID gEfiEndOfDxeEventGroupGuid;

#endif

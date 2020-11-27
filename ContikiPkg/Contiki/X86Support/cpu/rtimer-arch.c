/*
 * Copyright (c) 2010, Loughborough University - Computer Science
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         Hardware-dependent functions used to support the
 *         contiki rtimer module.
 *
 *         clock_init() has set our tick speed prescaler already, so we
 *         are ticking with 500 kHz freq.
 *
 *         Contiki typedefs rtimer_clock_t as unsigned short (16bit)
 *         It thus makes sense to use the 16bit timer (Timer 1)
 *
 *         This file contains an ISR and must reside in the HOME bank
 *
 * \author
 *         jiewen yao - <jiewen.yao@intel.com>
 */

#include "sys/rtimer.h"
#include "sys/energest.h"

#include "debug.h"
#include <stdio.h>

rtimer_clock_t
rtimer_arch_now(void)
{
  return AsmReadTsc ();
}

/*---------------------------------------------------------------------------*/
void
rtimer_arch_init(void)
{
  // JYAO1 - TBD
}
/*---------------------------------------------------------------------------*/
void
rtimer_arch_schedule(rtimer_clock_t t)
{
  // JYAO1 - TBD
}
/*---------------------------------------------------------------------------*/
/* avoid referencing bits, we don't call code which use them */
void
rtimer_isr(void)
{
  // JYAO1 - TBD
  rtimer_run_next();
}

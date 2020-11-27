/*
 * Copyright (c) 2009, Swedish Institute of Computer Science.
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
 */

/**
 * \file
 *         Implementation of the clock functions
 * \author
 *         jiewen yao - <jiewen.yao@intel.com>
 */
#include "sys/clock.h"
#include "sys/etimer.h"

#include <Library/UefiBootServicesTableLib.h>

/*---------------------------------------------------------------------------*/
#if CLOCK_CONF_STACK_FRIENDLY
volatile uint8_t sleep_flag;
#endif
static volatile clock_time_t count = 0; /* Uptime in ticks */
static volatile clock_time_t seconds = 0; /* Uptime in secs */
/*---------------------------------------------------------------------------*/
/**
 * Each iteration is ~1.0xy usec, so this function delays for roughly len usec
 */
void
clock_delay_usec(uint16_t len)
{
//  MicroSecondDelay (len);
  gBS->Stall (len);
}
/*---------------------------------------------------------------------------*/
/**
 * Wait for a multiple of ~8 ms (a tick)
 */
void
clock_wait(clock_time_t i)
{
  clock_time_t start;

  start = clock_time();
  while(clock_time() - start < (clock_time_t)i);
}
/*---------------------------------------------------------------------------*/
CCIF clock_time_t
clock_time(void)
{
  return count;
}
/*---------------------------------------------------------------------------*/
CCIF unsigned long
clock_seconds(void)
{
  return seconds;
}
/*---------------------------------------------------------------------------*/
/*
 * Initialize clock
 */
void
clock_init(void)
{
  // JYAO1 - TBD - need hook this interrupt handle to timer
}

void
PlatformIsrHandler ();

/*---------------------------------------------------------------------------*/
/* avoid referencing bits, we don't call code which use them */
void
clock_isr(void)
{
  ++count;
  if(ModU64x32 (count, CLOCK_CONF_SECOND) == 0) {
    ++seconds;
  }

#if CLOCK_CONF_STACK_FRIENDLY
  sleep_flag = 1;
#else
  // JYAO1
  if(etimer_pending()
      && (etimer_next_expiration_time() - count - 1) > MAX_TICKS) {
    etimer_request_poll();
  }
  PlatformIsrHandler ();
#endif
}
/*---------------------------------------------------------------------------*/

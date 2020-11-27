/*
 * \file
 *	This file contains a set of configuration for using SDCC as a compiler.
 *
 * \author
 *	 jiewen yao - <jiewen.yao@intel.com>
 */

#ifndef __X86_PLATFORM_DEF_H__
#define __X86_PLATFORM_DEF_H__

#include <stdint.h>

/* In watchdog mode, our WDT can't be stopped once started
 * Include watchdog_stop()'s declaration and then trash it */
//#include "dev/watchdog.h"
//#define watchdog_stop() watchdog_periodic()


//
// The PCAT 8253/8254 has an input clock at 1.193182 MHz and Timer 0 is
// initialized as a 16 bit free running counter that generates an interrupt(IRQ0)
// each time the counter rolls over.
//
//   65536 counts
// ---------------- * 1,000,000 uS/S = 54925.4 uS = 549254 * 100 ns
//   1,193,182 Hz
//

//
// The maximum tick duration for 8254 timer
//
#define MAX_TIMER_TICK_DURATION     549254
//
// The default timer tick duration is set to 10 ms = 100000 100 ns units
//
#define DEFAULT_TIMER_TICK_DURATION 100000

/* Generic types. */
typedef unsigned short uip_stats_t;

/* Time type. */
typedef unsigned long long clock_time_t;
#define MAX_TICKS (~((clock_time_t)0) / 2)
/* Defines tick counts for a second. */
// 1 second = 10000000 100ns
#define CLOCK_CONF_SECOND   (10000000 / DEFAULT_TIMER_TICK_DURATION)

/* Compiler configurations */
#define CCIF
#define CLIF

#endif /* __8051_DEF_H__ */

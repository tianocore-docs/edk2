/**
 * \file
 *
 *   Definition of some debugging functions.
 *
 *   writestring() and writehex()
 *
 * \author
 *         jiewen yao - <jiewen.yao@intel.com>
 */

#include <UefiContikiSupport.h>

#include "debug.h"

#include <Library/DebugLib.h>

static const char hexconv[] = "0123456789abcdef";
static const char binconv[] = "01";
/*---------------------------------------------------------------------------*/
void
writestring(char *s)
{
#if 1
  DebugPrint (DEBUG_ERROR, "%a", s);
#else
  while(*s) {
    writechar(*s++);
  }
#endif
}
/*---------------------------------------------------------------------------*/
void
writehex(uint8_t c)
{
  writechar(hexconv[c >> 4]);
  writechar(hexconv[c & 0x0f]);
}
/*---------------------------------------------------------------------------*/
void
writebin(uint8_t c)
{
  unsigned char i = 0x80;
  while(i) {
    writechar(binconv[(c & i) != 0]);
    i >>= 1;
  }
}
/*---------------------------------------------------------------------------*/
void
writedec(uint8_t c)
{
  uint8_t div;
  uint8_t hassent = 0;
  for(div = 100; div > 0; div /= 10) {
    uint8_t disp = c / div;
    c %= div;
    if((disp != 0) || (hassent) || (div == 1)) {
      hassent = 1;
      writechar('0' + disp);
    }
  }
}

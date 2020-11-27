/**
 * \file
 *         hardware-specific writechar() routine
 *
 * \author
 *         jiewen yao - <jiewen.yao@intel.com>
 */

/*---------------------------------------------------------------------------*/

#include <UefiContikiSupport.h>

#include <Library/DebugLib.h>

void
writechar(char c)
{
  DebugPrint (DEBUG_ERROR, "%c", c);
}

/*
www.sourceforge.net/projects/dfhack
Copyright (c) 2009 Petr Mrázek (peterix), Kenneth Ferland (Impaler[WrG]), dorf, doomchild

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#ifndef ITEMS_C_API
#define ITEMS_C_API

#include "DFHack_C.h"
#include "dfhack/DFProcess.h"
#include "dfhack-c/DFTypes_C.h"
#include "dfhack/modules/Items.h"

#ifdef __cplusplus
extern "C" {
#endif

DFHACK_EXPORT int Items_Start(DFHackObject* items);
DFHACK_EXPORT int Items_Finish(DFHackObject* items);

DFHACK_EXPORT char* Items_getItemDescription(DFHackObject* items, uint32_t itemptr, DFHackObject* mats);
DFHACK_EXPORT char* Items_getItemClass(DFHackObject* items, int32_t index);
DFHACK_EXPORT int Items_getItemData(DFHackObject* items, uint32_t itemptr, t_item* item);

#ifdef __cplusplus
}
#endif

#endif

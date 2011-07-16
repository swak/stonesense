/*
https://github.com/peterix/dfhack
Copyright (c) 2009-2011 Petr Mrázek (peterix@gmail.com)

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

#pragma once
#ifndef CL_MOD_ITEMS
#define CL_MOD_ITEMS

/*
* Items!
*/
#include "dfhack/Export.h"
#include "dfhack/Module.h"
#include "dfhack/Types.h"

/**
 * \defgroup grp_items Items module and its types
 * @ingroup grp_modules
 */

namespace DFHack
{

class Context;
class DFContextShared;
class Creatures;

/**
 * Item flags. A bit fuzzy.
 * Mostly from http://dwarffortresswiki.net/index.php/User:Rick/Memory_research
 * \ingroup grp_items
 */
union t_itemflags
{
    uint32_t whole; ///< the whole struct. all 32 bits of it, as an integer
    struct
    {
        unsigned int on_ground : 1;      ///< 0000 0001 Item on ground
        unsigned int in_job : 1;         ///< 0000 0002 Item currently being used in a job
        unsigned int u_ngrd1 : 1;        ///< 0000 0004 unknown, unseen
        unsigned int in_inventory : 1;   ///< 0000 0008 Item in a creature or workshop inventory

        unsigned int u_ngrd2 : 1;        ///< 0000 0010 unknown, lost (artifact)?, unseen
        unsigned int in_building : 1;    ///< 0000 0020 Part of a building (including mechanisms, bodies in coffins)
        unsigned int u_ngrd3 : 1;        ///< 0000 0040 unknown, unseen
        unsigned int dead_dwarf : 1;     ///< 0000 0080 Dwarf's dead body or body part

        unsigned int rotten : 1;         ///< 0000 0100 Rotten food
        unsigned int spider_web : 1;     ///< 0000 0200 Thread in spider web
        unsigned int construction : 1;   ///< 0000 0400 Material used in construction
        unsigned int u_ngrd5 : 1;        ///< 0000 0800 unknown, unseen

        unsigned int unk3 : 1;           ///< 0000 1000 unknown, unseen
        unsigned int u_ngrd6 : 1;        ///< 0000 2000 unknown, unseen
        unsigned int foreign : 1;        ///< 0000 4000 Item is imported
        unsigned int u_ngrd7 : 1;        ///< 0000 8000 unknown, unseen

        unsigned int owned : 1;          ///< 0001 0000 Item is owned by a dwarf
        unsigned int garbage_colect : 1; ///< 0002 0000 Marked for deallocation by DF it seems
        unsigned int artifact1 : 1;      ///< 0004 0000 Artifact ?
        unsigned int forbid : 1;         ///< 0008 0000 Forbidden item

        unsigned int unk5 : 1;           ///< 0010 0000 unknown, unseen
        unsigned int dump : 1;           ///< 0020 0000 Designated for dumping
        unsigned int on_fire: 1;         ///< 0040 0000 Indicates if item is on fire, Will Set Item On Fire if Set!
        unsigned int melt : 1;           ///< 0080 0000 Designated for melting, if applicable

        unsigned int hidden : 1;       ///< 0100 0000 Hidden item
        unsigned int in_chest : 1;     ///< 0200 0000 Stored in chest/part of well?
        unsigned int unk6 : 1;         ///< 0400 0000 unknown, unseen
        unsigned int artifact2 : 1;    ///< 0800 0000 Artifact ?

        unsigned int unk8 : 1;         ///< 1000 0000 unknown, unseen
        unsigned int unk9 : 1;         ///< 2000 0000 unknown, set when viewing details
        unsigned int unk10 : 1;        ///< 4000 0000 unknown, unseen
        unsigned int unk11 : 1;        ///< 8000 0000 unknown, unseen
    };
};

/**
 * Basic item data, read as a single chunk
 * \ingroup grp_items
 */
struct t_item
{
    uint32_t vtable;
    int16_t x;
    int16_t y;
    int16_t z;
    t_itemflags flags;
};

/**
 * Type for holding an item read from DF
 * \ingroup grp_items
 */
struct dfh_item
{
    int32_t id;
    t_item base;
    t_material matdesc;
    int32_t quantity;
    int32_t quality;
    int16_t wear_level;
    uint32_t origin;
};

/**
 * Type for holding item improvements. broken/unused.
 * \ingroup grp_items
 */
struct t_improvement
{
    t_material matdesc;
    int32_t quality;
};

/**
 * The Items module
 * \ingroup grp_modules
 * \ingroup grp_items
 */
class DFHACK_EXPORT Items : public Module
{
public:
    Items();
    ~Items();
    bool Start();
    bool Finish();

    bool readItemVector(std::vector<uint32_t> &items);
    uint32_t findItemByID(int32_t id);

    /// get a string describing an item
    std::string getItemDescription(const dfh_item & item, Materials * Materials);
    /// get a short name for an item
    std::string getItemClass(int32_t index);
    std::string getItemClass(const dfh_item & item);
    /// read an item, including the extra attributes
    bool readItem(uint32_t itemptr, dfh_item & item);
    /// write item base (position and flags only = t_item part of dfh_item)
    bool writeItem(const dfh_item & item);
    /// dump offsets used by accessors to a string
    std::string dumpAccessors(const dfh_item & item);

    /// who owns this item we already read?
    int32_t getItemOwnerID(const dfh_item & item);
    /// which item is it contained in?
    int32_t getItemContainerID(const dfh_item & item);
    /// which items does it contain?
    bool getContainedItems(const dfh_item & item, std::vector<int32_t> &items);

    /// wipe out the owner records
    bool removeItemOwner(dfh_item &item, Creatures *creatures);

    bool readItemRefs(const dfh_item &item, const ClassNameCheck &classname, std::vector<int32_t> &values);
private:
    class Private;
    Private* d;
};
}
#endif

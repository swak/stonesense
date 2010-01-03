#pragma once
#include "common.h"
#include "dfhack/library/tinyxml/tinyxml.h"

static t_SpriteWithOffset spriteItem_NA = {0, 0, 0,-1,1+2+4+8+16+32};

void ReadItems(API& DF);
void clearItemCache();
void clearCachedItem(uint32_t x,uint32_t y,uint32_t z);
void getCachedItem(uint32_t x, uint32_t y, uint32_t z, t_CachedItem &item);

void DrawItem( BITMAP* target, int drawx, int drawy, t_CachedItem& item );
t_SpriteWithOffset GetItemsSpriteMap( t_CachedItem* item );

class ItemConfiguration
{
	public:
	t_SpriteWithOffset sprite;
	int matType;
	int matIndex;
	
	ItemConfiguration(){}
	ItemConfiguration(t_SpriteWithOffset &sprite, int matType, int matIndex);
	~ItemConfiguration(void);
};


bool addItemConfig( TiXmlElement* elemRoot, vector<vector<ItemConfiguration>*>& knownItem );

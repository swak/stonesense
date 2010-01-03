#include "Items.h"

vector<t_CachedItem*> itemCache;
uint32_t itemIndex;
uint32_t imapxsize;
uint32_t imapysize;
uint32_t imapzsize;

void initItemVector(API& DF)
{
	DF.getSize(imapxsize, imapysize, imapzsize);
    imapxsize *= 16;
    imapysize *= 16;
	itemCache.resize(imapxsize*imapysize*imapzsize,NULL);
	itemIndex = 0;
}

void clearItemCache()
{
	int max_i = itemCache.size();
	for (int i=0;i<max_i;i++)
	{
		if (itemCache[i] != NULL)	
			delete(itemCache[i]);
	}
	itemCache.clear();
}

void clearCachedItem(uint32_t x, uint32_t y, uint32_t z)
{
	uint32_t itemLoc = x + (y * imapxsize) +
		(z * imapxsize * imapysize);
	if (itemCache[itemLoc])
		delete(itemCache[itemLoc]);
	itemCache[itemLoc] = NULL;
}

void getCachedItem(uint32_t x, uint32_t y, uint32_t z, t_CachedItem &item)
{
	uint32_t itemLoc = x + (y * imapxsize) +
		(z * imapxsize * imapysize);
	if (itemCache[itemLoc])
		item = *(itemCache[itemLoc]);
	else
		item.itemType = INVALID_INDEX;
}

inline void handleItem(API& DF, t_item &tempItem)
{
	//WriteErr("hi+ @%d\n",itemIndex);
	DF.ReadItem(itemIndex, tempItem);
	//WriteErr("hi r ed\n");
	if (tempItem.x == 35536 || tempItem.y == 35536 || tempItem.z == 35536)
			return;
	//WriteErr("got\n");
	uint32_t itemLoc = tempItem.x + (tempItem.y * imapxsize) +
		(tempItem.z * imapxsize * imapysize);
	//WriteErr("loc %d %d %d = %d\n",tempItem.x,tempItem.y,tempItem.z,itemLoc);
	t_CachedItem* cachedTemp = itemCache[itemLoc];
	//WriteErr("to %d\n",(int)cachedTemp);
	if (cachedTemp == NULL)
	{
		cachedTemp = new t_CachedItem;
		itemCache[itemLoc] = cachedTemp;
	}
	//WriteErr("tweak\n");
	cachedTemp->itemType=tempItem.type;
	cachedTemp->matType=tempItem.material.type;
	cachedTemp->matIndex=tempItem.material.index;
	//WriteErr("hi-\n",itemIndex);
}

void ReadItems(API& DF)
{
	//WriteErr("ri+\n");
	if (itemCache.size() == 0)
	{
		//WriteErr("ri iiv\n");
		initItemVector(DF);
	}
	t_item tempItem;
	//WriteErr("ri iri\n");
	uint32_t itemv_size = DF.InitReadItems();
	uint32_t max_item = itemIndex + config.itemsPerFrame;
	uint32_t extra_item = 0;
	//WriteErr("ri %d of %d\n",itemIndex, itemv_size);
	//WriteErr("ics %d\n",itemCache.size());
	if (max_item > itemv_size)
	{
		extra_item = max_item - itemv_size;
		max_item = itemv_size;
	}
	if (extra_item > itemIndex)
	{
		extra_item = itemIndex = 0;
		max_item = itemv_size;
	}
	//WriteErr("ri hi1\n");
	for (;itemIndex<max_item;itemIndex++)
	{
		handleItem(DF,tempItem);
	}
	//WriteErr("ri hi2?\n");
	if (extra_item)
	{
		//WriteErr("ri hi2\n");
		for (itemIndex = 0;itemIndex<extra_item;itemIndex++)
		{
			handleItem(DF,tempItem);
		}
	}
	//WriteErr("ri fri\n");
	DF.FinishReadItems();	
	//WriteErr("ri-\n");
}

void DrawItem( BITMAP* target, int drawx, int drawy, t_CachedItem& item )
{
	textprintf(target, font, drawx, drawy-10, 0xFFffFF, "%d", item.itemType );
}

t_SpriteWithOffset GetItemSpriteMap( t_CachedItem* item )
{	
	return spriteItem_NA;
}

bool addItemConfig( TiXmlElement* elemRoot, vector<vector<ItemConfiguration>*>& knownItem )
{
	return true;
}
ItemConfiguration::ItemConfiguration(t_SpriteWithOffset &sprite, int matType, int matIndex)
{}
ItemConfiguration::~ItemConfiguration(void)
{}
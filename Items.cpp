#include "Items.h"
#include "GUI.h"
#include "ContentLoader.h"

vector<t_CachedItem*> itemCache;
uint32_t itemIndex = 0;
uint32_t imapxsize;
uint32_t imapysize;
uint32_t imapzsize;
uint32_t checked_size = 0;
uint32_t cachePass = 0;

#define ITEM_VECTOR_PRIORITY_AREA 10

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
	itemIndex = 0;
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

uint32_t item_hidden_flag = 1 << 24;

inline void handleItem(API& DF, t_item &tempItem, bool all_items)
{
	DF.ReadItem(itemIndex, tempItem);
	if (tempItem.x == 35536 || tempItem.y == 35536 || tempItem.z == 35536)
			return;
	// TODO integrate DFHack flags when they arrive
	if (!(tempItem.flags & 1)) //item on ground
		return;
	uint32_t itemLoc = tempItem.x + (tempItem.y * imapxsize) +
		(tempItem.z * imapxsize * imapysize);
	t_CachedItem* cachedTemp = itemCache[itemLoc];
	if (cachedTemp == NULL)
	{
		cachedTemp = new t_CachedItem;
		itemCache[itemLoc] = cachedTemp;
		cachedTemp->cachePass = cachePass-2; // ensure write
	}
	if (!all_items)
	{
		// check if cached item is the same as us
		if (cachedTemp->itemID == tempItem.ID)
		{
			// check if we have a full cycle without state change
			if (cachedTemp->cachePass == cachePass-1 &&
				(cachedTemp->flags & item_hidden_flag) == (tempItem.flags & item_hidden_flag))
			{
				cachedTemp->fullPass = true;
			}
			else
			{
				cachedTemp->fullPass = false;
			}
			//update changing state
			cachedTemp->cachePass = cachePass;
			cachedTemp->flags=tempItem.flags;
			return;
		}
		// check if cached item has priority
		if (cachedTemp->cachePass == cachePass)
		{
			if (cachedTemp->itemIndex > itemIndex &&
					(!(cachedTemp->flags & item_hidden_flag) || (tempItem.flags & item_hidden_flag)))
				return;
		}
		else if (cachedTemp->cachePass == cachePass - 1)
		{
			if (cachedTemp->itemIndex < itemIndex && (tempItem.flags & item_hidden_flag))
				return;
		}
	}
	cachedTemp->itemType=tempItem.type;
	cachedTemp->matType=tempItem.material.type;
	cachedTemp->matIndex=tempItem.material.index;
	cachedTemp->flags=tempItem.flags;
	cachedTemp->itemIndex=itemIndex;
	cachedTemp->itemID=tempItem.ID;
	cachedTemp->cachePass = cachePass;
	cachedTemp->fullPass = all_items; // if we are running all items at once, we know we checked everything
}

void ReadItems(API& DF)
{
	if (itemCache.size() == 0)
	{
		initItemVector(DF);
	}
	t_item tempItem;
	uint32_t itemv_size = DF.InitReadItems();
	// check for first time setup
	if (itemIndex == 0)
	{
		itemIndex == itemv_size - 1;
	}
	// note: everything is unsigned, so we need to be careful about
	// not dropping below zero
	uint32_t min_item; // stop point for base search [min_item .. current itemIndex]
	uint32_t extra_item; // stop point for extra search [extra_item .. itemv_size)
	uint32_t item_rem;	// scratch for remaining search size
	// uint32_t itemv_size // item vector size
	// uint32_t checked_size // last cycles itemv_size
	
	extra_item = itemv_size;
	item_rem = config.itemsPerFrame;
	// dont worry about cache speed if we are loading manually
	// also, handle the case where there are few enough items
	// to just load everything
	if (config.automatic_reload_time == 0 || itemv_size <= item_rem)
	{
		itemIndex = itemv_size - 1;
		min_item = 0;
		cachePass++; // why reset it periodically when you can let it overflow?
		// use ascending loop for simplicity
		for (itemIndex = 0;itemIndex < itemv_size;itemIndex++)
		{
			handleItem(DF,tempItem,true);	
		}
		itemIndex = 0;
	}
	else
	{
		// danger: much end-casey maths
		// check for pathological vector shrinkage case
		// 
		if (itemIndex >= itemv_size) //know either itemIndex < checked_size or
										// are in initial state
		{
			itemIndex = itemv_size - 1;	
		}
		// run as much as we can of newly created items preferentially
		// this way dead critters make a bloody mess rather than spraying unidentified objects
		// (if we cant run em all, we'll forget the rest... but how often will that happen?)
		// we'll also miss stuff as a result of simultaneous delete/create
		if (checked_size > 0 && checked_size < itemv_size)
		{
			extra_item = itemv_size - item_rem; // know itemv_size > item_rem (outer if)
			if (checked_size >= extra_item) // know checked_size < itemv_size
			{
				extra_item = checked_size;
			}
			if (itemIndex >= extra_item) // itemIndex in [0 .. itemv_size)
			{
				extra_item = itemIndex + 1;
			}
			item_rem -= itemv_size - extra_item; // at this stage, extra item in (0 .. itemv_size]
		}
		// run as much as we can from itemIndex
		if (item_rem <= itemIndex)
		{
			min_item = itemIndex - item_rem;
		}
		else // have remainder after rest of set- run more from top
		{
			min_item = 0;
			extra_item -= item_rem - itemIndex;
		}
		// store amount checked for next time
		checked_size = itemv_size;
		// first pass- current sweep;
		itemIndex++;
		while (itemIndex>min_item) // this construct avoids problems when min_item == 0
		{
			itemIndex--;
			handleItem(DF,tempItem,false);
		}
		// second pass- 'extra' sweep
		// update pass if we have looped
		if (min_item == 0)
		{
			cachePass++; // why reset it periodically when you can let it overflow?
		}
		if (extra_item < itemv_size)
		{
			for (itemIndex = itemv_size - 1;itemIndex>=extra_item;itemIndex--)
			{
				handleItem(DF,tempItem,false);
			}
		}
		// work out where we left off
		if (min_item)
		{
			itemIndex = min_item;	
		}
		else
		{
			itemIndex = extra_item;	
		}
	}
	DF.FinishReadItems();
}

void DrawItem( BITMAP* target, int drawx, int drawy, t_CachedItem& item )
{
	t_SpriteWithOffset sprite = GetItemSpriteMap( item );
	if (sprite.sheetIndex == INVALID_INDEX)
		return;
  	BITMAP* itemSheet;
    if (sprite.fileIndex == INVALID_INDEX)
    {
    	itemSheet = IMGObjectSheet;
	}
    else
    {
    	itemSheet = getImgFile(sprite.fileIndex);
	} 
	DrawSpriteFromSheet( sprite.sheetIndex, target, itemSheet, drawx, drawy );
}

t_SpriteWithOffset GetItemSpriteMap( t_CachedItem& item )
{	
	vector<ItemConfiguration>* testVector;
	if (item.flags & item_hidden_flag)
	{
		if (item.fullPass)
			return spriteItem_None;
		return spriteItem_NA;
	}
	uint32_t num = (uint32_t)contentLoader.itemConfigs.size();
	if (item.itemType >= num || item.itemType < 0)
	{
		return spriteItem_NA;
	}
	testVector = contentLoader.itemConfigs[item.itemType];
	if (testVector == NULL || testVector->size() == 0)
	{
		return spriteItem_NA;
	}
	int maxv = testVector->size();
	int mat = item.matType;
	for (int i=0;i<maxv;i++)
	{
		if (mat == (*testVector)[i].matType || (*testVector)[i].matType == INVALID_INDEX)
			return (*testVector)[i].sprite;
	}
	return spriteItem_NA;
}

void pushItemConfig( vector<vector<ItemConfiguration>*>& knownItems, int gameID, ItemConfiguration& iconf)
{
	//get or make the vector to store it
	vector<ItemConfiguration>* itemVector;
	if (knownItems.size() <= gameID)
	{
		//resize using hint from creature name list
		int newsize = gameID +1;
		// wont work yet
		//if (newsize <= contentLoader.itemNameStrings.size())
		//{
		//	newsize = contentLoader.itemNameStrings.size() + 1;
		//}
		knownItems.resize(newsize,NULL);
	}
	itemVector = knownItems[gameID];
	if (itemVector == NULL)
	{
		itemVector = new vector<ItemConfiguration>;
		knownItems[gameID] = itemVector;
	}
	//add a copy to known items
	itemVector->push_back(iconf);
}

int getItemnameFromString(const char* strType)
{
  for (uint32_t i=0; i<contentLoader.buildingNameStrings.size(); i++){
		if (contentLoader.buildingNameStrings[i].compare(strType) == 0)
		{
			return i;
		}
	}
	return INVALID_INDEX;	
}

bool addSingleItemConfig( TiXmlElement* elemItem, vector<vector<ItemConfiguration>*>& knownItems, int basefile ){
	// names are stored in with buildings at the moment...
	int gameID = getItemnameFromString(elemItem->Attribute("gameID"));
	if (gameID == INVALID_INDEX)
	{
		WriteErr("Item name %d not recognised\n",elemItem->Attribute("gameID"));
		contentError("in item config",elemItem);
		return false;
	}
	// no item names yet!
	//const char* gameIdStr = elemItem->Attribute("gameID");
	//int gameID = atoi(gameIdStr);
	//if (gameID == 0 && gameIdStr[0] != '0')
	//	return false;
	const char* sheetIndexStr;
	t_SpriteWithOffset sprite;
	sprite.fileIndex=basefile;
	sprite.x=0;
	sprite.y=0;
	sprite.animFrames=ALL_FRAMES;
	const char* filename = elemItem->Attribute("file");
	if (filename != NULL && filename[0] != 0)
	{
		sprite.fileIndex = loadConfigImgFile((char*)filename,elemItem);
	}
	int mattype = INVALID_INDEX;
	const char* matname = elemItem->Attribute("material");
	if (matname != NULL && matname[0] != 0)
	{
		mattype = lookupMaterialType(matname);
		if (mattype == INVALID_INDEX)
		{
			WriteErr("Material name %d not recognised\n",matname);
			contentError("in item config",elemItem);			
		}
	}
	//create default config
	sheetIndexStr = elemItem->Attribute("sheetIndex");
	sprite.animFrames = ALL_FRAMES;
	sprite.sheetIndex = atoi( sheetIndexStr );
	ItemConfiguration iconf(sprite, mattype, INVALID_INDEX);
	pushItemConfig(knownItems, gameID, iconf);
	return true;
}

bool addItemsConfig( TiXmlElement* elemRoot, vector<vector<ItemConfiguration>*>& knownItems )
{
  int basefile = -1;
  const char* filename = elemRoot->Attribute("file");
  if (filename != NULL && filename[0] != 0)
  {
	basefile = loadConfigImgFile((char*)filename,elemRoot);
  } 
  TiXmlElement* elemItem = elemRoot->FirstChildElement("item");
  if (elemItem == NULL)
  {
     contentError("No items found",elemRoot);
     return false;
  }
  while( elemItem ){
	addSingleItemConfig(elemItem,knownItems,basefile );
	elemItem = elemItem->NextSiblingElement("item");
  }
  return true;
}

ItemConfiguration::ItemConfiguration(t_SpriteWithOffset &sprite, int matType, int matIndex)
{
	this->sprite=sprite;
	this->matType=matType;
	this->matIndex=matIndex;
}

ItemConfiguration::~ItemConfiguration(void)
{}

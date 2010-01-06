#include "Items.h"
#include "GUI.h"
#include "ContentLoader.h"

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
	DF.ReadItem(itemIndex, tempItem);
	if (tempItem.x == 35536 || tempItem.y == 35536 || tempItem.z == 35536)
			return;
	uint32_t itemLoc = tempItem.x + (tempItem.y * imapxsize) +
		(tempItem.z * imapxsize * imapysize);
	t_CachedItem* cachedTemp = itemCache[itemLoc];
	if (cachedTemp == NULL)
	{
		cachedTemp = new t_CachedItem;
		itemCache[itemLoc] = cachedTemp;
	}
	cachedTemp->itemType=tempItem.type;
	cachedTemp->matType=tempItem.material.type;
	cachedTemp->matIndex=tempItem.material.index;
}

void ReadItems(API& DF)
{
	if (itemCache.size() == 0)
	{
		initItemVector(DF);
	}
	t_item tempItem;
	uint32_t itemv_size = DF.InitReadItems();
	uint32_t max_item = itemIndex + config.itemsPerFrame;
	uint32_t extra_item = 0;
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
	for (;itemIndex<max_item;itemIndex++)
	{
		handleItem(DF,tempItem);
	}
	if (extra_item)
	{
		for (itemIndex = 0;itemIndex<extra_item;itemIndex++)
		{
			handleItem(DF,tempItem);
		}
	}
	DF.FinishReadItems();	
}

void DrawItem( BITMAP* target, int drawx, int drawy, t_CachedItem& item )
{
	t_SpriteWithOffset sprite = GetItemSpriteMap( item );
  	BITMAP* itemSheet;
    if (sprite.fileIndex == -1)
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
	return (*testVector)[0].sprite;
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

bool addSingleItemConfig( TiXmlElement* elemItem, vector<vector<ItemConfiguration>*>& knownItems, int basefile ){
	//int gameID = lookupIndexedType(elemItem->Attribute("gameID"),contentLoader.itemNameStrings);
	//if (gameID == INVALID_INDEX)
	//	return false;
	// no item names yet!
	const char* gameIdStr = elemItem->Attribute("gameID");
	int gameID = atoi(gameIdStr);
	if (gameID == 0 && gameIdStr[0] != '0')
		return false;
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
	
	//create default config
	sheetIndexStr = elemItem->Attribute("sheetIndex");
	sprite.animFrames = ALL_FRAMES;
	sprite.sheetIndex = atoi( sheetIndexStr );
	ItemConfiguration iconf(sprite, INVALID_INDEX, INVALID_INDEX);
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
	this->matType=matIndex;
	this->matIndex=matIndex;
}

ItemConfiguration::~ItemConfiguration(void)
{}
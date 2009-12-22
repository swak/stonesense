#include "Block.h"
#include "common.h"
#include "GUI.h"
#include "SpriteMaps.h"
#include "GameBuildings.h"
#include "Creatures.h"
#include "WorldSegment.h"
#include "BlockFactory.h"
#include "SpriteCache.h"

#include "dfhack/library/DFTypes.h"






Block::Block(WorldSegment* ownerSegment)
{
	//clear out own memory
	memset(this, 0, sizeof(Block));

	this->ownerSegment = ownerSegment;

	building.info.type = BUILDINGTYPE_NA;
	building.index = -1;

	this->material.type = INVALID_INDEX;
	this->material.index = INVALID_INDEX;
}



Block::~Block(void){
	if( creature )
		delete(creature);
}

void* Block::operator new (size_t size){
	return blockFactory.allocateBlock( );
}
void Block::operator delete (void *p){
	blockFactory.deleteBlock( (Block*)p );
}

inline BITMAP* imageSheet(t_SpriteWithOffset sprite, BITMAP* defaultBmp)
{
	if (sprite.fileIndex == -1)
	{
		return defaultBmp;
	}
	else
	{
		return getImgFile(sprite.fileIndex);
	}
}

#include "newblockdraw.h"

void Block::DrawRamptops(BITMAP* target){
	if (ramp.type > 0)
	{

		int sheetOffsetX, sheetOffsetY;
  /*if(config.hide_outer_blocks){
    if(x == ownerSegment->x || x == ownerSegment->x + ownerSegment->sizex - 1) return;
    if(y == ownerSegment->y || y == ownerSegment->y + ownerSegment->sizey - 1) return;
  }*/
		int32_t drawx = x;
		int32_t drawy = y;
		int32_t drawz = z+1; //- ownerSegment->sizez + 1;

		correctBlockForSegmetOffset( drawx, drawy, drawz);
		correctBlockForRotation( drawx, drawy, drawz);
		pointToScreen((int*)&drawx, (int*)&drawy, drawz);
		drawx -= TILEWIDTH>>1;

		t_SpriteWithOffset sprite = GetBlockSpriteMap(ramp.type,material);
		if (sprite.sheetIndex == UNCONFIGURED_INDEX)
		{
			sprite.sheetIndex = 0;
			sprite.fileIndex = INVALID_INDEX;
		}
		if (sprite.sheetIndex != INVALID_INDEX)
		{
			sheetOffsetX = SPRITEWIDTH * ramp.index;
			sheetOffsetY = (TILEHEIGHT + FLOORHEIGHT + SPRITEHEIGHT) * sprite.sheetIndex;

			masked_blit( imageSheet(sprite,IMGRampSheet), target, sheetOffsetX,sheetOffsetY, drawx,drawy, SPRITEWIDTH, TILEHEIGHT + FLOORHEIGHT);	

		}
	}
}

bool hasWall(Block* b){
	if(!b) return false;
	return b->wallType > 0;
}

bool hasBuildingOfID(Block* b, int ID){
	if(!b) return false;
	return b->building.info.type == ID;
}

bool hasBuildingIdentity(Block* b, uint32_t index, int buildingOcc){
	if(!b) return false;
	if (!(b->building.index == index)) return false;
	return b->occ.bits.building == buildingOcc;
}

bool hasBuildingOfIndex(Block* b, uint32_t index){
	if(!b) return false;
	return b->building.index == index;
}

bool wallShouldNotHaveBorders( int in ){
	switch( in ){
	case 65: //stone fortification
	case 436: //minstone fortification
	case 326: //lavastone fortification
	case 327: //featstone fortification
	case 494: //constructed fortification
		return true;
		break;
	};
	return false;
}

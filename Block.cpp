#include "Block.h"
#include "common.h"
#include "GUI.h"
#include "SpriteMaps.h"
#include "GameBuildings.h"
#include "Creatures.h"
#include "WorldSegment.h"

#include "dfhack/library/DFTypes.h"


BITMAP* temptile=0;



Block::Block(WorldSegment* ownerSegment)
{
  //clear out own memory
  memset(this, 0, sizeof(Block));

  this->ownerSegment = ownerSegment;

  building.info.type = BUILDINGTYPE_NA;
  building.index = -1;

  this->materialIndex = INVALID_INDEX;
}



Block::~Block(void){
  if( creature )
    delete(creature);
}


void Block::Draw(BITMAP* target){
	int sheetOffsetX, sheetOffsetY;
  if(config.hide_outer_blocks){
    if(x == ownerSegment->x || x == ownerSegment->x + ownerSegment->sizex - 1) return;
    if(y == ownerSegment->y || y == ownerSegment->y + ownerSegment->sizey - 1) return;
  }

	int32_t drawx = x;
	int32_t drawy = y;
  int32_t drawz = z; //- ownerSegment->sizez + 1;

	correctBlockForSegmetOffset( drawx, drawy, drawz);
  correctBlockForRotation( drawx, drawy, drawz);
	drawx *= TILEWIDTH;
	drawy *= TILEWIDTH;
	pointToScreen((int*)&drawx, (int*)&drawy, drawz * BLOCKHEIGHT);
	drawx -= TILEWIDTH>>1;

  int tileBorderColor = makecol(85,85,85);

	//Draw Floor
  if(floorType > 0 || wallType > 0 || ramp.type > 0){
    int floorSpriteIndex = GetFloorSpriteMap(this->floorType, materialIndex);
    //if floor is muddy, override regular floor
    if( occ.bits.mud )
      floorSpriteIndex = SPRITEFLOOR_WATERFLOOR;

    //If tile has no floor, look for a Filler Floor from it's wall
    if(floorSpriteIndex == SPRITEFLOOR_NA)
      floorSpriteIndex = GetWallSpriteMap(this->wallType, this->materialIndex, true);
    
    //TODO: need configurable Filler Floors for ramps as well
	  if(floorSpriteIndex == SPRITEFLOOR_NA && (ramp.type > 0))
		  floorSpriteIndex = 3;

    sheetOffsetX = TILEWIDTH * (floorSpriteIndex % SHEET_OBJECTSWIDE);
    sheetOffsetY = (TILEHEIGHT + FLOORHEIGHT) * (floorSpriteIndex / SHEET_OBJECTSWIDE);
		masked_blit(IMGFloorSheet, target, sheetOffsetX,sheetOffsetY, drawx,drawy, TILEWIDTH,TILEHEIGHT + FLOORHEIGHT);

    //Northern frame
    if(this->depthBorderNorth)
      line(target, drawx + (TILEWIDTH>>1), drawy, drawx+TILEWIDTH-1, drawy+(TILEHEIGHT>>1)-1, tileBorderColor);

    //Western frame
    if(this->depthBorderWest)
      line(target, drawx, drawy+(TILEHEIGHT>>1)-1, drawx+(TILEWIDTH>>1)-1, drawy, tileBorderColor);
	}

	//Draw Ramp
  if(ramp.type > 0){
    sheetOffsetX = SPRITEWIDTH * ramp.index;
    sheetOffsetY = SPRITEHEIGHT * GetRampMaterialTypeMap(ramp.type);

		masked_blit(IMGRampSheet, target, sheetOffsetX,sheetOffsetY, drawx,drawy - (WALLHEIGHT), SPRITEWIDTH, SPRITEHEIGHT);
	}


	//vegitation
	if(tree.index > 0 || tree.type > 0){
    int spriteNum =  GetSpriteVegitation( (VegetationType) getVegetationType( this->floorType ), tree.index );
    DrawSpriteFromSheet( spriteNum, target, IMGObjectSheet, drawx, drawy );
	}

	//Building
  bool skipBuilding = 
    (building.info.type == BUILDINGTYPE_STOCKPILE && !config.show_stockpiles) ||
    (building.info.type == BUILDINGTYPE_ZONE && !config.show_zones);
  
  if(building.info.type != BUILDINGTYPE_NA && !skipBuilding){

    int spriteNum =  SPRITEOBJECT_NA; //getBuildingSprite(this->building, mirroredBuilding);

   for(uint32_t i=0; i < building.sprites.size(); i++){
      spriteNum = building.sprites[i].sheetIndex;
      BITMAP* objectSheet;
	    if (building.sprites[i].fileIndex == -1)
	    {
	    	objectSheet = IMGObjectSheet;
    	}
	    else
	    {
	    	objectSheet = getImgFile(building.sprites[i].fileIndex);
    	}
      DrawSpriteFromSheet(spriteNum , target, objectSheet, 
        drawx + building.sprites[i].x,
        drawy + building.sprites[i].y);
    }
	}




	//Draw Stairs
	if(stairType > 0){
		//down part
		int spriteNum = GetDownStairTypeMap(stairType);
    //DrawSpriteFromSheet( spriteNum, target, IMGObjectSheet, drawx, drawy );

		//up part
    bool mirrored = false;
    if(findWallCloseTo(ownerSegment, this) == eSimpleW)
      mirrored = true;
		spriteNum = GetUpStairTypeMap(stairType, mirrored);
    if(spriteNum)
      DrawSpriteFromSheet( spriteNum, target, IMGObjectSheet, drawx, drawy );
	}

  //Draw Walls
	if(wallType > 0){
    //draw wall
    int spriteNum =  GetWallSpriteMap(wallType, materialIndex, false);

    if( config.truncate_walls && this->z == ownerSegment->z + ownerSegment->sizez - 1){
      int sheetx = spriteNum % SHEET_OBJECTSWIDE;
      int sheety = spriteNum / SHEET_OBJECTSWIDE;
      //draw a tiny bit of wall
      masked_blit(IMGObjectSheet, target,
        sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT+WALL_CUTOFF_HEIGHT,
        drawx, drawy - (WALLHEIGHT)+WALL_CUTOFF_HEIGHT, SPRITEWIDTH, SPRITEHEIGHT-WALL_CUTOFF_HEIGHT);
      //draw cut-off floor thing
      masked_blit(IMGFloorSheet, target,
        TILEWIDTH * SPRITEFLOOR_CUTOFF, 0,
        drawx, drawy-(SPRITEHEIGHT-WALL_CUTOFF_HEIGHT)/2, SPRITEWIDTH, SPRITEWIDTH);
    }
    else {
      DrawSpriteFromSheet( spriteNum, target, IMGObjectSheet, drawx, drawy );

      drawy -= (WALLHEIGHT);
      //Northern border
      if(this->depthBorderNorth)
        line(target, drawx + (TILEWIDTH>>1), drawy, drawx+TILEWIDTH-1, drawy+(TILEHEIGHT>>1)-1, tileBorderColor);

      //Western border
      if(this->depthBorderWest)
        line(target, drawx, drawy+(TILEHEIGHT>>1)-1, drawx+(TILEWIDTH>>1)-1, drawy, tileBorderColor);
      
      drawy += (WALLHEIGHT);
    }
	}

	//water
	if(water.index > 0){
		int spriteNum = 0;
    int waterlevel = water.index;

    //if(waterlevel == 7) waterlevel--;

		if(water.type == 0)
			spriteNum = SPRITEOBJECT_WATERLEVEL1 + waterlevel - 1;
		if(water.type == 1)
			spriteNum = SPRITEOBJECT_WATERLEVEL1_LAVA + waterlevel - 1;
		DrawSpriteFromSheet( spriteNum, target, IMGObjectSheet, drawx, drawy );
	}

  //creature
  if(creature != null){
    DrawCreature( target, drawx, drawy, creature);
  }
}

void Block::DrawRamptops(BITMAP* target){
	if (ramp.type > 0)
	{
	
	int sheetOffsetX, sheetOffsetY;
  if(config.hide_outer_blocks){
    if(x == ownerSegment->x || x == ownerSegment->x + ownerSegment->sizex - 1) return;
    if(y == ownerSegment->y || y == ownerSegment->y + ownerSegment->sizey - 1) return;
  }

	int32_t drawx = x;
	int32_t drawy = y;
  	int32_t drawz = z+1; //- ownerSegment->sizez + 1;

	correctBlockForSegmetOffset( drawx, drawy, drawz);
  	correctBlockForRotation( drawx, drawy, drawz);
	drawx *= TILEWIDTH;
	drawy *= TILEWIDTH;
	pointToScreen((int*)&drawx, (int*)&drawy, drawz * BLOCKHEIGHT);
	drawx -= TILEWIDTH>>1;

    sheetOffsetX = SPRITEWIDTH * ramp.index;
    sheetOffsetY = (TILEHEIGHT + FLOORHEIGHT) * GetRampMaterialTypeMap(ramp.type);

		masked_blit(IMGRamptopSheet, target, sheetOffsetX,sheetOffsetY, drawx,drawy, SPRITEWIDTH, TILEHEIGHT + FLOORHEIGHT);	
		
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
    case ID_WOODFORTIFICATION:
      return true;
      break;
  };
  return false;
}
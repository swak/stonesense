#include "common.h"
#include "ConditionalSprite.h"
#include "Block.h"
#include "GameBuildings.h"


ConditionalSprite::ConditionalSprite(void)
{
  memset(this, 0, sizeof(ConditionalSprite));
  //
  cMaterialType = INVALID_INDEX;
  cMaterialIndex = INVALID_INDEX;
  cPositionIndex = INVALID_INDEX;
  cNeighbourHasWall = eSimpleInvalid;
}

bool ConditionalSprite::matchPosition(Block *b){
  int x = b->x - b->building.x1;
  int y = b->y - b->building.y1;
  int w = b->building.x2 - b->building.x1 + 1 ;
  int pos = y * w + x;

  return pos == cPositionIndex;
}

bool ConditionalSprite::matchMaterialType(Block *b){
  return b->building.material.type == cMaterialType;
}


bool ConditionalSprite::matchNeighbourHasWall(Block *b){
    dirTypes closebyWalls = findWallCloseTo(b->ownerSegment,b);
    
    return closebyWalls == cNeighbourHasWall;
}
bool ConditionalSprite::BlockMatches(Block* b){
  bool okSoFar = true;
  if(cMaterialType != INVALID_INDEX){
    if( !this->matchMaterialType(b) ) okSoFar = false;
  }
  if(cPositionIndex != INVALID_INDEX){
    if( !this->matchPosition(b) ) okSoFar = false;
  }
  if(cNeighbourHasWall != INVALID_INDEX){
    if( !this->matchNeighbourHasWall(b) ) okSoFar = false;
  }
  return okSoFar;
}
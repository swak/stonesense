#pragma once

#include "common.h"

class WorldSegment;


class Block
{
public:
	Block(WorldSegment* ownerSegment);
	~Block(void);
  static void* operator new (size_t size); 
  static void operator delete (void *p);

  WorldSegment* ownerSegment;
  
	uint32_t x, y, z;
	int floorType;
	int wallType;
	int stairType;
  t_matglossPair material;

  bool depthBorderNorth;
  bool depthBorderWest;
  int shadow;

	t_matglossPair ramp;
	t_matglossPair water;
    t_occupancy occ;
	
  t_creature* creature;
  //bool mirroredBuilding;
	t_matglossPair tree;

  struct {
    t_building info;
    vector<t_SpriteWithOffset> sprites;
	uint32_t index;
  } building;
  
	bool IsVisible(){
		return (floorType || wallType) != 0;
	}
	void Draw(BITMAP* target);
	void DrawRamptops(BITMAP* target);
	
private:

};


//find a better place for these
bool hasWall(Block* b);
bool hasBuildingOfID(Block* b, int ID);
bool hasBuildingIdentity(Block* b, uint32_t index, int buildingOcc);
bool hasBuildingOfIndex(Block* b, uint32_t index);
bool wallShouldNotHaveBorders( int in );

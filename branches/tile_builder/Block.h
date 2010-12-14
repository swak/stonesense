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

	int tileType;
	t_matglossPair material;

	uint8_t openborders;
	uint8_t lightborders;
	
	bool creaturePresent;
	bool buildingPresent;
	t_matglossPair ramp;
	t_matglossPair water;

	t_occupancy occ;
	t_matglossPair tree;
	t_designation designation;

	int mudlevel;
	int snowlevel;
	int bloodlevel;
	ALLEGRO_COLOR bloodcolor;

	struct {
		t_building info;
		uint32_t index;
	} building;

	bool IsVisible(){
		return tileTypeTable[tileType].c != EMPTY;
	}
	void Draw_pixel(bool * bitmask);

	uint16_t consForm;

private:

};
void initRandomCube();

//find a better place for these
bool hasBuildingOfID(Block* b, int ID);
bool hasBuildingIdentity(Block* b, uint32_t index, int buildingOcc);
bool hasBuildingOfIndex(Block* b, uint32_t index);
bool wallShouldNotHaveBorders( int in );
void drawFloorBlood ( Block *b, int32_t drawx, int32_t drawy );

ALLEGRO_COLOR getSpriteColor(t_subSprite &sprite, t_matglossPair material, t_matglossPair layerMaterial, t_matglossPair veinMaterial);
ALLEGRO_COLOR getSpriteColor(t_SpriteWithOffset &sprite, t_matglossPair material, t_matglossPair layerMaterial, t_matglossPair veinMaterial);
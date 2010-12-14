#include "Block.h"
#include "common.h"
#include "GUI.h"
#include "GameBuildings.h"
#include "WorldSegment.h"
#include "BlockFactory.h"
#include "ContentLoader.h"
#include "SpriteColors.h"


int randomCube[RANDOM_CUBE][RANDOM_CUBE][RANDOM_CUBE];

void initRandomCube()
{
	for(int i = 0; i < RANDOM_CUBE; i++)
		for(int j = 0; j < RANDOM_CUBE; j++)
			for(int k = 0; k < RANDOM_CUBE; k++)
				randomCube[i][j][k] = rand();
}

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



Block::~Block(void)
{
}

void* Block::operator new (size_t size){
	return blockFactory.allocateBlock( );
}
void Block::operator delete (void *p){
	blockFactory.deleteBlock( (Block*)p );
}

void Block::Draw_pixel(bool * bitmask){
	if((material.type == INORGANIC) && (material.index == -1))
	{
		material.index = 0;
	}
	bool defaultSnow = 1;
	int sheetOffsetX, sheetOffsetY;
	t_SpriteWithOffset sprite;
	/*if(config.hide_outer_blocks){
	if(x == ownerSegment->x || x == ownerSegment->x + ownerSegment->sizex - 1) return;
	if(y == ownerSegment->y || y == ownerSegment->y + ownerSegment->sizey - 1) return;
	}*/
	int32_t drawx = x;
	int32_t drawy = y;
	int32_t drawz = z; //- ownerSegment->sizez + 1;


	correctBlockForSegmetOffset( drawx, drawy, drawz);
	correctBlockForRotation( drawx, drawy, drawz);
	int32_t viewx = drawx;
	int32_t viewy = drawy;
	int32_t viewz = drawz;
	pointToTile((int*)&drawx, (int*)&drawy, drawz);

	if(bitmask[drawx + (drawy * al_get_bitmap_width(al_get_target_bitmap()))] == 1)
		return;
	ALLEGRO_COLOR tileBorderColor = al_map_rgb(85,85,85);
	int rando = randomCube[x%RANDOM_CUBE][y%RANDOM_CUBE][z%RANDOM_CUBE];
	//Draw Block
	if(tileTypeTable[tileType].c != EMPTY)
	{
		ALLEGRO_COLOR color;
		if(tileTypeTable[tileType].m == GRASS)
			color = al_map_rgb(89,164,61);
		else if(tileTypeTable[tileType].m == GRASS2)
			color = al_map_rgb(62,137,64);
		else if(tileTypeTable[tileType].m == GRASS_DEAD)
			color = al_map_rgb(151,164,61);
		else if(tileTypeTable[tileType].m == GRASS_DRY)
			color = al_map_rgb(151,164,61);
		else if(water.index > 0)
		{
			if(water.type == 0)
				color = al_map_rgb(0,186,255);
			else
				color = al_map_rgb(255,44,0);

		}
		//probably all we need right now
		else
			color = lookupMaterialColor(material.type, material.index);

		if(tileTypeTable[tileType].c == BOULDER)
		{
			color.r *= 0.5;
			color.g *= 0.5;
			color.b *= 0.5;
		}
		else if(tileTypeTable[tileType].c == PEBBLES)
		{
			color.r *= 0.75;
			color.g *= 0.75;
			color.b *= 0.75;
		}

		if(!designation.bits.skyview)
		{
			if(openborders & 8)
			{
				color.r *= 0.5;
				color.g *= 0.5;
				color.b *= 0.5;
			}
			else
			{
				color.r *= 0.75;
				color.g *= 0.75;
				color.b *= 0.75;
			}
		}
		al_put_pixel(drawx, drawy, color);
		bitmask[drawx + (drawy * al_get_bitmap_width(al_get_target_bitmap()))] = true;
	}

	//vegetation
	if(tree.index > 0 || tree.type > 0)
	{
		ALLEGRO_COLOR color;
		color = al_map_rgb(18,72,21);
		al_put_pixel(drawx, drawy, color);
		//al_put_pixel(drawx, drawy-1, color);
		bitmask[drawx + (drawy * al_get_bitmap_width(al_get_target_bitmap()))] = true;
	}


	////draw surf
	//if(eff_oceanwave > 0)
	//{
	//	al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*al_map_rgba(255, 255, 255, (255*eff_oceanwave)/100));
	//	al_draw_tinted_bitmap(sprite_oceanwave, drawx, drawy - (WALLHEIGHT), 0);
	//	al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
	//}
	//if(eff_webing > 0)
	//{
	//	al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*al_map_rgba(255, 255, 255, (255*eff_webing)/100));
	//	al_draw_tinted_bitmap(sprite_webing, drawx, drawy - (WALLHEIGHT), 0);
	//	al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
	//}
	//Draw Ramp
	//if(ramp.type > 0){
	//	spriteobject = GetBlockSpriteMap(ramp.type, material, consForm);
	//	if (spriteobject->get_sheetindex() == UNCONFIGURED_INDEX)
	//	{
	//		spriteobject->set_sheetindex(0);
	//		spriteobject->set_fileindex(INVALID_INDEX);
	//		spriteobject->set_defaultsheet(IMGRampSheet);
	//	}
	//	if (spriteobject->get_sheetindex() != INVALID_INDEX)
	//	{
	//		spriteobject->set_size(SPRITEWIDTH, SPRITEHEIGHT);
	//		spriteobject->set_tile_layout(RAMPBOTTOMTILE);
	//		spriteobject->draw_world(x, y, z, (chopThisBlock && this->z == ownerSegment->z + ownerSegment->sizez -2));
	//	}
	//	spriteobject->set_tile_layout(BLOCKTILE);
	//}

	//drawFloorBlood ( this, drawx, drawy );
	////first part of snow
	//if(ramp.type == 0 && wallType == 0 && stairType == 0 && defaultSnow)
	//{
	//	if(snowlevel > 75)
	//	{
	//		DrawSpriteFromSheet( 20, IMGObjectSheet, al_map_rgb(255,255,255), drawx, drawy );
	//	}
	//	else if(snowlevel > 50)
	//	{
	//		DrawSpriteFromSheet( 21, IMGObjectSheet, al_map_rgb(255,255,255), drawx, drawy );
	//	}
	//	else if(snowlevel > 25)
	//	{
	//		DrawSpriteFromSheet( 22, IMGObjectSheet, al_map_rgb(255,255,255), drawx, drawy );
	//	}
	//	else if(snowlevel > 0)
	//	{
	//		DrawSpriteFromSheet( 23, IMGObjectSheet, al_map_rgb(255,255,255), drawx, drawy );
	//	}
	//}



	////shadow
	//if (shadow > 0)
	//{
	//	DrawSpriteFromSheet( BASE_SHADOW_TILE + shadow - 1, IMGObjectSheet, al_map_rgb(255,255,255), drawx, (ramp.type > 0)?(drawy - (WALLHEIGHT/2)):drawy );
	//}

	////Building
	//bool skipBuilding =
	//	(building.info.type == contentLoader.civzoneNum && !config.show_stockpiles) ||
	//	(building.info.type == contentLoader.stockpileNum && !config.show_zones);

	//if(building.info.type != BUILDINGTYPE_NA && !skipBuilding)
	//{
	//	int spriteNum =  SPRITEOBJECT_NA; //getBuildingSprite(this->building, mirroredBuilding);

	//	for(uint32_t i=0; i < building.sprites.size(); i++)
	//	{
	//		spriteobject = &building.sprites[i];
	//		spriteobject->draw_world(x, y, z);
	//	}
	//}




	////Draw Stairs
	//if(stairType > 0)
	//{
	//	bool mirrored = false;
	//	if(findWallCloseTo(ownerSegment, this) == eSimpleW)
	//		mirrored = true;

	//	//down part
	//	spriteobject = GetFloorSpriteMap(stairType, material, consForm);
	//	if(spriteobject->get_sheetindex() != INVALID_INDEX && spriteobject->get_sheetindex() != UNCONFIGURED_INDEX)
	//	{
	//		if (mirrored)
	//			spriteobject->draw_world_offset(x, y, z, 1);
	//		else
	//			spriteobject->draw_world(x, y, z);
	//	}

	//	//up part
	//	spriteobject = GetBlockSpriteMap(stairType, material, consForm);
	//	if(spriteobject->get_sheetindex() != INVALID_INDEX && spriteobject->get_sheetindex() != UNCONFIGURED_INDEX)
	//	{
	//		if (mirrored)
	//			spriteobject->draw_world_offset(x, y, z, 1);
	//		else
	//			spriteobject->draw_world(x, y, z);
	//	}
	//}

	//if(wallType > 0)
	//{
	//	//draw wall
	//	spriteobject =  GetBlockSpriteMap(wallType, material, consForm);
	//	int spriteOffset = 0;
	//	if (spriteobject->get_sheetindex() == UNCONFIGURED_INDEX)
	//	{
	//		spriteobject->set_sheetindex(SPRITEOBJECT_WALL_NA);
	//		spriteobject->set_fileindex(INVALID_INDEX);
	//			spriteobject->set_tile_layout(BLOCKTILE);
	//			spriteobject->set_defaultsheet(IMGObjectSheet);
	//	}
	//	if (spriteobject->get_sheetindex() == INVALID_INDEX)
	//	{
	//		//skip   
	//	}    
	//	else 
	//	{
	//		spriteobject->draw_world(x, y, z, (chopThisBlock && this->z == ownerSegment->z + ownerSegment->sizez -2));
	//	}
	//}


	//if(water.index > 0)
	//{
	//	//if(waterlevel == 7) waterlevel--;
	//	if(water.type == 0)
	//	{
	//		contentLoader.water[water.index-1].sprite.draw_world(x, y, z, (chopThisBlock && this->z == ownerSegment->z + ownerSegment->sizez -2));
	//	}
	//	else
	//	{
	//		contentLoader.lava[water.index-1].sprite.draw_world(x, y, z, (chopThisBlock && this->z == ownerSegment->z + ownerSegment->sizez -2));
	//	}
	//}

	//// creature
	//// ensure there is *some* creature according to the map data
	//// (no guarantee it is the right one)
	//if(creaturePresent)
	//{
	//	DrawCreature(drawx, drawy, creature);
	//}

	////second part of snow
	//if(wallType == 0 && stairType == 0 && defaultSnow)
	//{
	//	if(snowlevel > 75)
	//	{
	//		DrawSpriteFromSheet( 24, IMGObjectSheet, al_map_rgb(255,255,255), drawx, drawy );
	//	}
	//	else if(snowlevel > 50)
	//	{
	//		DrawSpriteFromSheet( 25, IMGObjectSheet, al_map_rgb(255,255,255), drawx, drawy );
	//	}
	//	else if(snowlevel > 25)
	//	{
	//		DrawSpriteFromSheet( 26, IMGObjectSheet, al_map_rgb(255,255,255), drawx, drawy );
	//	}
	//}
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


void drawFloorBlood ( Block *b, int32_t drawx, int32_t drawy )
{
	t_occupancy occ = b->occ;
	t_SpriteWithOffset sprite;
	int x = b->x, y = b->y, z = b->z;


	if( b->water.index < 1 && (b->bloodlevel))
	{
		sprite.fileIndex = INVALID_INDEX;

		// Spatter (should be blood, not blood2) swapped for testing
		if( b->bloodlevel <= config.poolcutoff )
			sprite.sheetIndex = 7;

		// Smear (should be blood2, not blood) swapped for testing
		else
		{
			// if there's no block in the respective direction it's false. if there's no blood in that direction it's false too. should also check to see if there's a ramp below, but since blood doesn't flow, that'd look wrong anyway.
			bool _N = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eUp ) != NULL ? (b->ownerSegment->getBlockRelativeTo( x, y, z, eUp )->bloodlevel > config.poolcutoff) : false ),
				_S = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eDown ) != NULL ? (b->ownerSegment->getBlockRelativeTo( x, y, z, eDown )->bloodlevel > config.poolcutoff) : false ),
				_E = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eRight ) != NULL ? (b->ownerSegment->getBlockRelativeTo( x, y, z, eRight )->bloodlevel > config.poolcutoff) : false ),
				_W = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eLeft ) != NULL ? (b->ownerSegment->getBlockRelativeTo( x, y, z, eLeft )->bloodlevel > config.poolcutoff) : false );

			// do rules-based puddling
			if( _N || _S || _E || _W )
			{
				if( _E )
				{
					if( _N && _S )
						sprite.sheetIndex = 5;
					else if( _S )
						sprite.sheetIndex = 3;
					else if( _W )
						sprite.sheetIndex = 1;
					else
						sprite.sheetIndex = 6;
				}
				else if( _W )
				{
					if( _S && _N)
						sprite.sheetIndex = 5;
					else if( _S )
						sprite.sheetIndex = 2;
					else
						sprite.sheetIndex = 0;
				}
				else if ( _N )
					sprite.sheetIndex = 4;
				else
					sprite.sheetIndex = 2;
			}
			else
				sprite.sheetIndex = 8;
		}

		int sheetOffsetX = TILEWIDTH * (sprite.sheetIndex % SHEET_OBJECTSWIDE),
			sheetOffsetY = 0;

		al_draw_tinted_bitmap_region( IMGBloodSheet, b->bloodcolor, sheetOffsetX, sheetOffsetY, TILEWIDTH, TILEHEIGHT+FLOORHEIGHT, drawx, drawy, 0);
		al_draw_tinted_bitmap_region( IMGBloodSheet, al_map_rgb(255,255,255), sheetOffsetX, sheetOffsetY+TILEHEIGHT+FLOORHEIGHT, TILEWIDTH, TILEHEIGHT+FLOORHEIGHT, drawx, drawy, 0);
	}
}


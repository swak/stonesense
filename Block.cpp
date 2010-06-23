#include "Block.h"
#include "common.h"
#include "GUI.h"
#include "SpriteMaps.h"
#include "GameBuildings.h"
#include "Creatures.h"
#include "WorldSegment.h"
#include "BlockFactory.h"
#include "ContentLoader.h"
#include "spriteColors.h"

#include "dfhack/include/DFTypes.h"



ALLEGRO_BITMAP *sprite_miasma = 0;
ALLEGRO_BITMAP *sprite_water = 0;
ALLEGRO_BITMAP *sprite_water2 = 0;
ALLEGRO_BITMAP *sprite_blood = 0;
ALLEGRO_BITMAP *sprite_dust = 0;
ALLEGRO_BITMAP *sprite_magma = 0;
ALLEGRO_BITMAP *sprite_smoke = 0;
ALLEGRO_BITMAP *sprite_dragonfire = 0;
ALLEGRO_BITMAP *sprite_fire = 0;
ALLEGRO_BITMAP *sprite_webing = 0;
ALLEGRO_BITMAP *sprite_boiling = 0;
ALLEGRO_BITMAP *sprite_oceanwave = 0;

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

inline ALLEGRO_BITMAP* imageSheet(t_SpriteWithOffset sprite, ALLEGRO_BITMAP* defaultBmp)
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

inline ALLEGRO_BITMAP* imageSheet(t_subSprite sprite, ALLEGRO_BITMAP* defaultBmp)
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

void Block::Draw(){
	bool defaultSnow = 1;
	int sheetOffsetX, sheetOffsetY;
	t_SpriteWithOffset sprite;
	/*if(config.hide_outer_blocks){
	if(x == ownerSegment->x || x == ownerSegment->x + ownerSegment->sizex - 1) return;
	if(y == ownerSegment->y || y == ownerSegment->y + ownerSegment->sizey - 1) return;
	}*/
	int op, src, dst, alpha_op, alpha_src, alpha_dst;
	ALLEGRO_COLOR color;
	al_get_separate_blender(&op, &src, &dst, &alpha_op, &alpha_src, &alpha_dst, &color);
	int32_t drawx = x;
	int32_t drawy = y;
	int32_t drawz = z; //- ownerSegment->sizez + 1;


	correctBlockForSegmetOffset( drawx, drawy, drawz);
	correctBlockForRotation( drawx, drawy, drawz);
	int32_t viewx = drawx;
	int32_t viewy = drawy;
	int32_t viewz = drawz;
	pointToScreen((int*)&drawx, (int*)&drawy, drawz);
	drawx -= TILEWIDTH>>1;

	ALLEGRO_COLOR tileBorderColor = al_map_rgb(85,85,85);
	int rando = randomCube[x%RANDOM_CUBE][y%RANDOM_CUBE][z%RANDOM_CUBE];
	//Draw Floor
	if(floorType > 0 || wallType > 0 || ramp.type > 0 || stairType > 0){

		//If tile has no floor, look for a Filler Floor from it's wall
		if (floorType > 0)
		{
			sprite = GetFloorSpriteMap(floorType, this->material, consForm);
		}
		else if (wallType > 0)
		{
			sprite = GetFloorSpriteMap(wallType, this->material, consForm);
		}
		else if (ramp.type > 0)
		{
			sprite = GetFloorSpriteMap(ramp.type, this->material, consForm);
		}
		else if (stairType > 0)
		{
			sprite = GetFloorSpriteMap(stairType, this->material, consForm);
		}

		if(sprite.sheetIndex != INVALID_INDEX)
		{
			int spriteOffset = 0;;
			if(sprite.numVariations)
				spriteOffset = rando % sprite.numVariations;
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite, this->material, this->layerMaterial, this->veinMaterial));


			//if floor is muddy, override regular floor
			if( mudlevel && water.index == 0)
			{
				sprite.sheetIndex = SPRITEFLOOR_WATERFLOOR;
				sprite.fileIndex = INVALID_INDEX;
				spriteOffset = 0;
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
			}
			////if floor is snowed down, override  regular floor
			//if( snowlevel )
			//{
			//	sprite.sheetIndex = SPRITEFLOOR_SNOW;
			//	sprite.fileIndex = INVALID_INDEX;
			//	spriteOffset = 0;
			//	al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
			//}

			if (sprite.sheetIndex == UNCONFIGURED_INDEX)
			{
				sprite.sheetIndex = SPRITEOBJECT_FLOOR_NA;
				sprite.fileIndex = INVALID_INDEX;
				spriteOffset = 0;
			}

			sheetOffsetX = TILEWIDTH * ((sprite.sheetIndex+spriteOffset) % SHEET_OBJECTSWIDE);
			sheetOffsetY = (TILEHEIGHT + FLOORHEIGHT) * ((sprite.sheetIndex+spriteOffset) / SHEET_OBJECTSWIDE);
			if (sprite.snowMin > 0) defaultSnow = 0;
			if((sprite.snowMin <= snowlevel) && (sprite.snowMax >= snowlevel))
				al_draw_bitmap_region(imageSheet(sprite,IMGObjectSheet), sheetOffsetX, sheetOffsetY,  TILEWIDTH, TILEHEIGHT + FLOORHEIGHT, drawx, drawy, 0);
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
			if(sprite.sheetIndex != SPRITEFLOOR_WATERFLOOR && sprite.sheetIndex != SPRITEFLOOR_SNOW)
			{
				if(!(sprite.subSprites.empty()))
				{
					for(int i = 0; i < sprite.subSprites.size(); i++)
					{
						if (sprite.subSprites[i].snowMin > 0) defaultSnow = 0;
						sheetOffsetX = TILEWIDTH * ((sprite.subSprites[i].sheetIndex+spriteOffset) % SHEET_OBJECTSWIDE);
						sheetOffsetY = (TILEHEIGHT + FLOORHEIGHT) * ((sprite.subSprites[i].sheetIndex+spriteOffset) / SHEET_OBJECTSWIDE);
						al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite.subSprites[i], this->material, this->layerMaterial, this->veinMaterial));
						if((sprite.subSprites[i].snowMin <= snowlevel) && (sprite.subSprites[i].snowMax >= snowlevel))
							al_draw_bitmap_region(imageSheet(sprite.subSprites[i],IMGObjectSheet), sheetOffsetX, sheetOffsetY,  TILEWIDTH, TILEHEIGHT + FLOORHEIGHT, drawx, drawy, 0);
						al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
					}
				}
			}
		}

		if(sprite.needOutline)
		{
			drawy += (WALLHEIGHT);
			//Northern border
			if(this->depthBorderNorth)
				DrawSpriteFromSheet(281, IMGObjectSheet, drawx, drawy );

			//Western border
			if(this->depthBorderWest)
				DrawSpriteFromSheet(280, IMGObjectSheet, drawx, drawy );

			drawy -= (WALLHEIGHT);
		}
	}

	//draw surf
	if(eff_oceanwave > 0)
	{
		al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*al_map_rgba(255, 255, 255, (255*eff_oceanwave)/100));
		al_draw_bitmap(sprite_oceanwave, drawx, drawy - (WALLHEIGHT), 0);
		al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
	}
	if(eff_webing > 0)
	{
		al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*al_map_rgba(255, 255, 255, (255*eff_webing)/100));
		al_draw_bitmap(sprite_webing, drawx, drawy - (WALLHEIGHT), 0);
		al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
	}
	//Draw Ramp
	if(ramp.type > 0){
		sprite = GetBlockSpriteMap(ramp.type, material, consForm);
		if (sprite.sheetIndex == UNCONFIGURED_INDEX)
		{
			sprite.sheetIndex = 0;
			sprite.fileIndex = INVALID_INDEX;
		}
		if (sprite.sheetIndex != INVALID_INDEX)
		{
			sheetOffsetX = SPRITEWIDTH * ramp.index;
			sheetOffsetY = ((TILEHEIGHT + FLOORHEIGHT + SPRITEHEIGHT) * sprite.sheetIndex)+(TILEHEIGHT + FLOORHEIGHT);
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite, this->material, this->layerMaterial, this->veinMaterial));
			if (sprite.snowMin > 0) defaultSnow = 0;
			if((sprite.snowMin <= snowlevel) && (sprite.snowMax >= snowlevel))
				al_draw_bitmap_region(imageSheet(sprite,IMGRampSheet), sheetOffsetX, sheetOffsetY, SPRITEWIDTH, SPRITEHEIGHT, drawx, drawy - (WALLHEIGHT), 0);
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
			if(!(sprite.subSprites.empty()))
			{
				for(int i = 0; i < sprite.subSprites.size(); i++)
				{
					sheetOffsetX = SPRITEWIDTH * ramp.index;
					sheetOffsetY = ((TILEHEIGHT + FLOORHEIGHT + SPRITEHEIGHT) * sprite.subSprites[i].sheetIndex)+(TILEHEIGHT + FLOORHEIGHT);
					al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite.subSprites[i], this->material, this->layerMaterial, this->veinMaterial));
					if((sprite.subSprites[i].snowMin <= snowlevel) && (sprite.subSprites[i].snowMax >= snowlevel))
						al_draw_bitmap_region(imageSheet(sprite.subSprites[i],IMGObjectSheet), sheetOffsetX, sheetOffsetY, SPRITEWIDTH, SPRITEHEIGHT, drawx, drawy - (WALLHEIGHT), 0);
					al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
				}
			}
		}
	}

	drawFloorBlood ( this, drawx, drawy );
	//first part of snow
	if(ramp.type == 0 && wallType == 0 && stairType == 0 && defaultSnow)
	{
		if(snowlevel > 75)
		{
			DrawSpriteFromSheet( 20, IMGObjectSheet, drawx, drawy );
		}
		else if(snowlevel > 50)
		{
			DrawSpriteFromSheet( 21, IMGObjectSheet, drawx, drawy );
		}
		else if(snowlevel > 25)
		{
			DrawSpriteFromSheet( 22, IMGObjectSheet, drawx, drawy );
		}
		else if(snowlevel > 0)
		{
			DrawSpriteFromSheet( 23, IMGObjectSheet, drawx, drawy );
		}
	}


	//vegetation
	if(tree.index > 0 || tree.type > 0){
		GetSpriteVegetation( (TileClass) getVegetationType( this->floorType ), tree.index ).draw_screen(drawx, drawy);
	}

	//shadow
	if (shadow > 0)
	{
		DrawSpriteFromSheet( BASE_SHADOW_TILE + shadow - 1, IMGObjectSheet, drawx, (ramp.type > 0)?(drawy - (WALLHEIGHT/2)):drawy );
	}

	//Building
	bool skipBuilding =
		(building.info.type == TranslateBuildingName("building_civzonest", contentLoader.classIdStrings ) && !config.show_stockpiles) ||
		(building.info.type == TranslateBuildingName("building_stockpilest", contentLoader.classIdStrings ) && !config.show_zones);

	if(building.info.type != BUILDINGTYPE_NA && !skipBuilding)
	{
		int spriteNum =  SPRITEOBJECT_NA; //getBuildingSprite(this->building, mirroredBuilding);

		for(uint32_t i=0; i < building.sprites.size(); i++){
			sprite = building.sprites[i];
			if(sprite.numVariations)
				sprite.sheetIndex += rando % sprite.numVariations;
			if (!(sprite.animFrames & (1 << currentAnimationFrame)))
				continue;
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite, building.info.material, this->layerMaterial, this->veinMaterial));

			DrawSpriteFromSheet(sprite.sheetIndex , imageSheet(sprite,IMGObjectSheet), 
				drawx + building.sprites[i].x,
				drawy + building.sprites[i].y);
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
		}
	}




	//Draw Stairs
	if(stairType > 0){
		//down part
		//skipping at the moment?
		//int spriteNum = GetFloorSpriteMap(stairType,material);
		//DrawSpriteFromSheet( spriteNum, target, IMGObjectSheet, drawx, drawy );

		//up part

		bool mirrored = false;
		if(findWallCloseTo(ownerSegment, this) == eSimpleW)
			mirrored = true;
		sprite = GetBlockSpriteMap(stairType, material, consForm);
		if(sprite.sheetIndex != INVALID_INDEX && sprite.sheetIndex != UNCONFIGURED_INDEX)
		{
			if (mirrored)
				sprite.sheetIndex += 1;
			if(bloodlevel)
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite, this->material, this->layerMaterial, this->veinMaterial)*bloodcolor);
			else
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite, this->material, this->layerMaterial, this->veinMaterial));
			DrawSpriteFromSheet( sprite.sheetIndex, imageSheet(sprite,IMGObjectSheet), drawx, drawy );
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
			if(!(sprite.subSprites.empty()))
			{
				for(int i = 0; i < sprite.subSprites.size(); i++)
				{
					if(bloodlevel)
						al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite.subSprites[i], this->material, this->layerMaterial, this->veinMaterial)*bloodcolor);
					else
						al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite.subSprites[i], this->material, this->layerMaterial, this->veinMaterial));
					DrawSpriteFromSheet( sprite.subSprites[i].sheetIndex, imageSheet(sprite.subSprites[i],IMGObjectSheet), drawx, drawy );
					al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
				}
			}
		}
	}

	//Draw Walls
	if(wallType > 0){
		//draw wall
		sprite =  GetBlockSpriteMap(wallType, material, consForm);
		int spriteOffset = 0;
		if(sprite.numVariations)
			spriteOffset = rando % sprite.numVariations;
		if (sprite.sheetIndex == UNCONFIGURED_INDEX)
		{
			sprite.sheetIndex = SPRITEOBJECT_WALL_NA;
			sprite.fileIndex = INVALID_INDEX;
			spriteOffset = 0;
		}
		if (sprite.sheetIndex == INVALID_INDEX)
		{
			//skip   
		}    
		else 
		{
			if( config.truncate_walls && this->z == ownerSegment->z + ownerSegment->sizez -2){
				int sheetx = (sprite.sheetIndex+spriteOffset) % SHEET_OBJECTSWIDE;
				int sheety = (sprite.sheetIndex+spriteOffset) / SHEET_OBJECTSWIDE;
				//draw a tiny bit of wall
				if(bloodlevel)
					al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite, this->material, this->layerMaterial, this->veinMaterial)*bloodcolor);
				else
					al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite, this->material, this->layerMaterial, this->veinMaterial));
				if((sprite.snowMin <= snowlevel) && (sprite.snowMax >= snowlevel))
					al_draw_bitmap_region(imageSheet(sprite,IMGObjectSheet),
					sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT+WALL_CUTOFF_HEIGHT,
					SPRITEWIDTH, SPRITEHEIGHT-WALL_CUTOFF_HEIGHT, drawx, drawy - (WALLHEIGHT)+WALL_CUTOFF_HEIGHT, 0);
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
				if(sprite.subSprites.size() > 0)
				{
					for(int i = 0; i < sprite.subSprites.size(); i++)
					{
						sheetx = (sprite.subSprites[i].sheetIndex+spriteOffset) % SHEET_OBJECTSWIDE;
						sheety = (sprite.subSprites[i].sheetIndex+spriteOffset) / SHEET_OBJECTSWIDE;
						if(bloodlevel)
							al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite.subSprites[i], this->material, this->layerMaterial, this->veinMaterial)*bloodcolor);
						else
							al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite.subSprites[i], this->material, this->layerMaterial, this->veinMaterial));
						if((sprite.subSprites[i].snowMin <= snowlevel) && (sprite.subSprites[i].snowMax >= snowlevel))
							al_draw_bitmap_region(imageSheet(sprite.subSprites[i],IMGObjectSheet),
							sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT+WALL_CUTOFF_HEIGHT,
							SPRITEWIDTH, SPRITEHEIGHT-WALL_CUTOFF_HEIGHT, drawx, drawy - (WALLHEIGHT)+WALL_CUTOFF_HEIGHT, 0);
						al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
					}
				}
				//draw cut-off floor thing
				al_draw_bitmap_region(IMGObjectSheet, 
					TILEWIDTH * SPRITEFLOOR_CUTOFF, 0,
					SPRITEWIDTH, SPRITEWIDTH, 
					drawx, drawy-(SPRITEHEIGHT-WALL_CUTOFF_HEIGHT)/2, 0);
			}
			else 
			{
				//al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, al_map_rgb(sprite.shadeRed, sprite.shadeGreen, sprite.shadeBlue));
				if(bloodlevel)
					al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite, this->material, this->layerMaterial, this->veinMaterial)*bloodcolor);
				else
					al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite, this->material, this->layerMaterial, this->veinMaterial));
				if((sprite.snowMin <= snowlevel) && (sprite.snowMax >= snowlevel))
					DrawSpriteFromSheet(sprite.sheetIndex+spriteOffset, imageSheet(sprite,IMGObjectSheet), drawx, drawy );
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
				if(sprite.subSprites.size() > 0)
				{
					for(int i = 0; i < sprite.subSprites.size(); i++)
					{
						if(bloodlevel)
							al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite.subSprites[i], this->material, this->layerMaterial, this->veinMaterial)*bloodcolor);
						else
							al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite.subSprites[i], this->material, this->layerMaterial, this->veinMaterial));
						if((sprite.subSprites[i].snowMin <= snowlevel) && (sprite.subSprites[i].snowMax >= snowlevel))
							DrawSpriteFromSheet(sprite.subSprites[i].sheetIndex+spriteOffset, imageSheet(sprite.subSprites[i],IMGObjectSheet), drawx, drawy );
						al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
					}
				}

				if(sprite.needOutline)
				{
					//drawy -= (WALLHEIGHT);
					//Northern border
					if(this->depthBorderNorth)
						DrawSpriteFromSheet(281, IMGObjectSheet, drawx, drawy );

					//Western border
					if(this->depthBorderWest)
						DrawSpriteFromSheet(280, IMGObjectSheet, drawx, drawy );

					//drawy += (WALLHEIGHT);
				}
			}
		}
	}

	//water
	if(water.index > 0)
	{
		int spriteNum = 0;
		int waterlevel = water.index;

		//if(waterlevel == 7) waterlevel--;

		if(water.type == 0)
		{
			if(bloodlevel == 0)
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*al_map_rgb(168,248,248));
			else if(bloodlevel <= 255)
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*partialBlend(al_map_rgb(168,248,248), bloodcolor, (bloodlevel*100/255)));
			else
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*bloodcolor);
			spriteNum = SPRITEOBJECT_WATERLEVEL1 + waterlevel - 1;
		}
		else
		{
			spriteNum = SPRITEOBJECT_WATERLEVEL1_LAVA + waterlevel - 1;
		}
		DrawSpriteFromSheet( spriteNum, IMGObjectSheet, drawx, drawy );
		al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
		}

		// creature
	// ensure there is *some* creature according to the map data
	// (no guarantee it is the right one)
	if(creaturePresent)
	{
		DrawCreature(drawx, drawy, creature);
	}

	//second part of snow
	if(wallType == 0 && stairType == 0 && defaultSnow)
	{
		if(snowlevel > 75)
		{
			DrawSpriteFromSheet( 24, IMGObjectSheet, drawx, drawy );
		}
		else if(snowlevel > 50)
		{
			DrawSpriteFromSheet( 25, IMGObjectSheet, drawx, drawy );
		}
		else if(snowlevel > 25)
		{
			DrawSpriteFromSheet( 26, IMGObjectSheet, drawx, drawy );
		}
	}

	//if(eff_miasma > 0)
	//{
	//	draw_particle_cloud(eff_miasma, drawx, drawy - (SPRITEHEIGHT/2), SPRITEWIDTH, SPRITEHEIGHT, sprite_miasma);
	//}
	//if(eff_water > 0)
	//{
	//	draw_particle_cloud(eff_water, drawx, drawy - (SPRITEHEIGHT/2), SPRITEWIDTH, SPRITEHEIGHT, sprite_water);
	//}
	//if(eff_water2 > 0)
	//{
	//	draw_particle_cloud(eff_water2, drawx, drawy - (SPRITEHEIGHT/2), SPRITEWIDTH, SPRITEHEIGHT, sprite_water2);
	//}
	//if(eff_blood > 0)
	//{
	//	draw_particle_cloud(eff_blood, drawx, drawy - (SPRITEHEIGHT/2), SPRITEWIDTH, SPRITEHEIGHT, sprite_blood);
	//}
	////if(eff_magma > 0)
	////{
	////	al_set_separate_blender(op, ALLEGRO_ONE, ALLEGRO_ONE, alpha_op, ALLEGRO_ONE, ALLEGRO_ONE, al_map_rgba(255, 255, 255, (255*eff_magma/100)));
	////	DrawSpriteFromSheet( 185, IMGObjectSheet, drawx, drawy );
	////	al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
	////}
	//if(eff_magma > 0)
	//{
	//	draw_particle_cloud(eff_magma, drawx, drawy - (SPRITEHEIGHT/2), SPRITEWIDTH, SPRITEHEIGHT, sprite_magma);
	//}
	//if(eff_dust > 0)
	//{
	//	draw_particle_cloud(eff_dust, drawx, drawy - (SPRITEHEIGHT/2), SPRITEWIDTH, SPRITEHEIGHT, sprite_dust);
	//}
	//if(eff_smoke > 0)
	//{
	//	draw_particle_cloud(eff_smoke, drawx, drawy - (SPRITEHEIGHT/2), SPRITEWIDTH, SPRITEHEIGHT, sprite_smoke);
	//}
	//if(eff_dragonfire > 0)
	//{
	//	draw_particle_cloud(eff_smoke, drawx, drawy - (SPRITEHEIGHT/2), SPRITEWIDTH, SPRITEHEIGHT, sprite_smoke);
	//}
	//if(eff_fire > 0)
	//{
	//	draw_particle_cloud(eff_smoke, drawx, drawy - (SPRITEHEIGHT/2), SPRITEWIDTH, SPRITEHEIGHT, sprite_smoke);
	//}
	//if(eff_boiling > 0)
	//{
	//	draw_particle_cloud(eff_smoke, drawx, drawy - (SPRITEHEIGHT/2), SPRITEWIDTH, SPRITEHEIGHT, sprite_smoke);
	//}
	al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
}

void Block::Drawcreaturetext(){
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
	pointToScreen((int*)&drawx, (int*)&drawy, drawz);
	drawx -= TILEWIDTH>>1;

	// creature
	// ensure there is *some* creature according to the map data
	// (no guarantee it is the right one)
	if(creaturePresent)
	{
		DrawCreatureText(drawx, drawy, creature);
	}

}

void Block::DrawRamptops(){
	if (ramp.type > 0)
	{
		int op, src, dst, alpha_op, alpha_src, alpha_dst;
		ALLEGRO_COLOR color;
		al_get_separate_blender(&op, &src, &dst, &alpha_op, &alpha_src, &alpha_dst, &color);
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

		t_SpriteWithOffset sprite = GetBlockSpriteMap(ramp.type,material, consForm);
		if (sprite.sheetIndex == UNCONFIGURED_INDEX)
		{
			sprite.sheetIndex = 0;
			sprite.fileIndex = INVALID_INDEX;
		}
		if (sprite.sheetIndex != INVALID_INDEX)
		{
			sheetOffsetX = SPRITEWIDTH * ramp.index;
			sheetOffsetY = (TILEHEIGHT + FLOORHEIGHT + SPRITEHEIGHT) * sprite.sheetIndex;
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color*getSpriteColor(sprite, this->material, this->layerMaterial, this->veinMaterial));
			al_draw_bitmap_region(imageSheet(sprite,IMGRampSheet), sheetOffsetX, sheetOffsetY, SPRITEWIDTH, TILEHEIGHT + FLOORHEIGHT, drawx, drawy, 0);
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
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

void createEffectSprites()
{
	sprite_miasma		= CreateSpriteFromSheet( 180, IMGObjectSheet);
	sprite_water		= CreateSpriteFromSheet( 181, IMGObjectSheet);
	sprite_water2		= CreateSpriteFromSheet( 182, IMGObjectSheet);
	sprite_blood		= CreateSpriteFromSheet( 183, IMGObjectSheet);
	sprite_dust			= CreateSpriteFromSheet( 184, IMGObjectSheet);
	sprite_magma		= CreateSpriteFromSheet( 185, IMGObjectSheet);
	sprite_smoke		= CreateSpriteFromSheet( 186, IMGObjectSheet);
	sprite_dragonfire	= CreateSpriteFromSheet( 187, IMGObjectSheet);
	sprite_fire			= CreateSpriteFromSheet( 188, IMGObjectSheet);
	sprite_webing		= CreateSpriteFromSheet( 189, IMGObjectSheet);
	sprite_boiling		= CreateSpriteFromSheet( 190, IMGObjectSheet);
	sprite_oceanwave	= CreateSpriteFromSheet( 191, IMGObjectSheet);
}

void destroyEffectSprites()
{
	al_destroy_bitmap(sprite_miasma);
	al_destroy_bitmap(sprite_water);
	al_destroy_bitmap(sprite_water2);
	al_destroy_bitmap(sprite_blood);
	al_destroy_bitmap(sprite_dust);
	al_destroy_bitmap(sprite_magma);
	al_destroy_bitmap(sprite_smoke);
	al_destroy_bitmap(sprite_dragonfire);
	al_destroy_bitmap(sprite_fire);
	al_destroy_bitmap(sprite_webing);
	al_destroy_bitmap(sprite_boiling);
	al_destroy_bitmap(sprite_oceanwave);
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
		if( b->bloodlevel <= 255 )
			sprite.sheetIndex = 7;

		// Smear (should be blood2, not blood) swapped for testing
		else
		{
			// if there's no block in the respective direction it's false. if there's no blood in that direction it's false too. should also check to see if there's a ramp below, but since blood doesn't flow, that'd look wrong anyway.
			bool _N = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eUp ) != NULL ? (b->ownerSegment->getBlockRelativeTo( x, y, z, eUp )->bloodlevel > 255) : false ),
				_S = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eDown ) != NULL ? (b->ownerSegment->getBlockRelativeTo( x, y, z, eDown )->bloodlevel > 255) : false ),
				_E = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eRight ) != NULL ? (b->ownerSegment->getBlockRelativeTo( x, y, z, eRight )->bloodlevel > 255) : false ),
				_W = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eLeft ) != NULL ? (b->ownerSegment->getBlockRelativeTo( x, y, z, eLeft )->bloodlevel > 255) : false );

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
		int op, src, dst, alpha_op, alpha_src, alpha_dst;
		ALLEGRO_COLOR color;
		al_get_separate_blender(&op, &src, &dst, &alpha_op, &alpha_src, &alpha_dst, &color);
		if(b->bloodlevel <=255)
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, al_map_rgba_f(b->bloodcolor.r, b->bloodcolor.g, b->bloodcolor.b, (b->bloodlevel/255.0f)));
		else
			al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, b->bloodcolor);

		int sheetOffsetX = TILEWIDTH * (sprite.sheetIndex % SHEET_OBJECTSWIDE),
			sheetOffsetY = 0;

		al_draw_bitmap_region( IMGBloodSheet, sheetOffsetX, sheetOffsetY, TILEWIDTH, TILEHEIGHT+FLOORHEIGHT, drawx, drawy, 0);
		al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
		al_draw_bitmap_region( IMGBloodSheet, sheetOffsetX, sheetOffsetY+TILEHEIGHT+FLOORHEIGHT, TILEWIDTH, TILEHEIGHT+FLOORHEIGHT, drawx, drawy, 0);
	}
}


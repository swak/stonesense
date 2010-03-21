#include "Block.h"
#include "common.h"
#include "GUI.h"
#include "SpriteMaps.h"
#include "GameBuildings.h"
#include "Creatures.h"
#include "WorldSegment.h"
#include "BlockFactory.h"

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

void Block::Draw(){
	int sheetOffsetX, sheetOffsetY;
	t_SpriteWithOffset sprite;
	/*if(config.hide_outer_blocks){
	if(x == ownerSegment->x || x == ownerSegment->x + ownerSegment->sizex - 1) return;
	if(y == ownerSegment->y || y == ownerSegment->y + ownerSegment->sizey - 1) return;
	}*/
	//debug overlays
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

	if(config.currentOverlay != none)
		switch(config.currentOverlay){
		case flow_size:
			break;
		case pile:
			break;
		case dig:
			break;
		case detail:
			break;
		case detail_event:
			break;
		case hidden:
			break;
		case geolayer_index:
			break;
		case light:
			break;
		case subterranean:
			break;
		case skyview:
			break;
		case biome:
			break;
		case liquid_type:
			break;
		case water_table:
			break;
		case rained:
			break;
		case traffic:
			break;
		case flow_forbid:
			break;
		case liquid_static:
			break;
		case moss:
			break;
		case feature_present:
			break;
		case liquid_character:
			if(designations.bits.liquid_character == 1)
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, al_map_rgb(255, 255, 0)); //yellow
			if(designations.bits.liquid_character == 2)
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, al_map_rgb(255, 0, 255)); //pink
			if(designations.bits.liquid_character == 3)
				al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, al_map_rgb(0, 255, 255)); //cyan
			break;
		//default:
	}
	//Draw Floor
	if(floorType > 0 || wallType > 0 || ramp.type > 0 || stairType > 0){

		//If tile has no floor, look for a Filler Floor from it's wall
		if (floorType > 0)
		{
			sprite = GetFloorSpriteMap(floorType, this->material);
		}
		else if (wallType > 0)
		{
			sprite = GetFloorSpriteMap(wallType, this->material);
		}
		else if (ramp.type > 0)
		{
			sprite = GetFloorSpriteMap(ramp.type, this->material);
		}
		else if (stairType > 0)
		{
			sprite = GetFloorSpriteMap(stairType, this->material);
		}

		if(sprite.sheetIndex != INVALID_INDEX)
		{

			//if floor is muddy, override regular floor
			if( occ.bits.mud && water.index == 0)
			{
				sprite.sheetIndex = SPRITEFLOOR_WATERFLOOR;
				sprite.fileIndex = INVALID_INDEX;
			}
			//if floor is snowed down, override  regular floor
			if( occ.bits.snow )
			{
				sprite.sheetIndex = SPRITEFLOOR_SNOW;
				sprite.fileIndex = INVALID_INDEX;
			}

			if (sprite.sheetIndex == UNCONFIGURED_INDEX)
			{
				sprite.sheetIndex = SPRITEOBJECT_FLOOR_NA;
				sprite.fileIndex = INVALID_INDEX;
			}

			sheetOffsetX = TILEWIDTH * (sprite.sheetIndex % SHEET_OBJECTSWIDE);
			sheetOffsetY = (TILEHEIGHT + FLOORHEIGHT) * (sprite.sheetIndex / SHEET_OBJECTSWIDE);
			al_draw_bitmap_region(imageSheet(sprite,IMGObjectSheet), sheetOffsetX, sheetOffsetY,  TILEWIDTH, TILEHEIGHT + FLOORHEIGHT, drawx, drawy, 0);
		}

			drawy += (WALLHEIGHT);
			//Northern border
			if(this->depthBorderNorth)
				DrawSpriteFromSheet(281, IMGObjectSheet, drawx, drawy );

			//Western border
			if(this->depthBorderWest)
				DrawSpriteFromSheet(280, IMGObjectSheet, drawx, drawy );

			drawy -= (WALLHEIGHT);
	}

	//Draw Ramp
	if(ramp.type > 0){
		sprite = GetBlockSpriteMap(ramp.type, material);
		if (sprite.sheetIndex == UNCONFIGURED_INDEX)
		{
			sprite.sheetIndex = 0;
			sprite.fileIndex = INVALID_INDEX;
		}
		if (sprite.sheetIndex != INVALID_INDEX)
		{
			sheetOffsetX = SPRITEWIDTH * ramp.index;
			sheetOffsetY = ((TILEHEIGHT + FLOORHEIGHT + SPRITEHEIGHT) * sprite.sheetIndex)+(TILEHEIGHT + FLOORHEIGHT);

			al_draw_bitmap_region(imageSheet(sprite,IMGRampSheet), sheetOffsetX, sheetOffsetY, SPRITEWIDTH, SPRITEHEIGHT, drawx, drawy - (WALLHEIGHT), 0);
		}
	}


	//vegetation
	if(tree.index > 0 || tree.type > 0){
		sprite =  GetSpriteVegetation( (TileClass) getVegetationType( this->floorType ), tree.index );
		DrawSpriteFromSheet( sprite.sheetIndex, imageSheet(sprite,IMGObjectSheet), drawx, drawy );
	}

	//shadow
	if (shadow > 0)
	{
		DrawSpriteFromSheet( BASE_SHADOW_TILE + shadow - 1, IMGObjectSheet, drawx, (ramp.type > 0)?(drawy - (WALLHEIGHT/2)):drawy );
	}

	//Building
	bool skipBuilding =
		(building.info.type == BUILDINGTYPE_STOCKPILE && !config.show_stockpiles) ||
		(building.info.type == BUILDINGTYPE_ZONE && !config.show_zones);

	if(building.info.type != BUILDINGTYPE_NA && !skipBuilding){

		int spriteNum =  SPRITEOBJECT_NA; //getBuildingSprite(this->building, mirroredBuilding);

		for(uint32_t i=0; i < building.sprites.size(); i++){
			sprite = building.sprites[i];
			if (!(sprite.animFrames & (1 << currentAnimationFrame)))
				continue;
			DrawSpriteFromSheet(sprite.sheetIndex , imageSheet(sprite,IMGObjectSheet), 

				drawx + building.sprites[i].x,
				drawy + building.sprites[i].y);
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
		sprite = GetBlockSpriteMap(stairType, material);
		if(sprite.sheetIndex != INVALID_INDEX && sprite.sheetIndex != UNCONFIGURED_INDEX)
		{
			if (mirrored)
				sprite.sheetIndex += 1;
			DrawSpriteFromSheet( sprite.sheetIndex, imageSheet(sprite,IMGObjectSheet), drawx, drawy );
		}
	}

	//Draw Walls
	if(wallType > 0){
		//draw wall
		sprite =  GetBlockSpriteMap(wallType, material);

		if (sprite.sheetIndex == UNCONFIGURED_INDEX)
		{
			sprite.sheetIndex = SPRITEOBJECT_WALL_NA;
			sprite.fileIndex = INVALID_INDEX;  
		}
		if (sprite.sheetIndex == INVALID_INDEX)
		{
			//skip   
		}    
		else if( config.truncate_walls && this->z == ownerSegment->z + ownerSegment->sizez -2){
			int sheetx = sprite.sheetIndex % SHEET_OBJECTSWIDE;
			int sheety = sprite.sheetIndex / SHEET_OBJECTSWIDE;
			//draw a tiny bit of wall
			al_draw_bitmap_region(imageSheet(sprite,IMGObjectSheet),
				sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT+WALL_CUTOFF_HEIGHT,
				SPRITEWIDTH, SPRITEHEIGHT-WALL_CUTOFF_HEIGHT, drawx, drawy - (WALLHEIGHT)+WALL_CUTOFF_HEIGHT, 0);
			//draw cut-off floor thing
			al_draw_bitmap_region(IMGObjectSheet, 
				TILEWIDTH * SPRITEFLOOR_CUTOFF, 0,
				SPRITEWIDTH, SPRITEWIDTH, 
				drawx, drawy-(SPRITEHEIGHT-WALL_CUTOFF_HEIGHT)/2, 0);
		}
		else {
			DrawSpriteFromSheet(sprite.sheetIndex, imageSheet(sprite,IMGObjectSheet), drawx, drawy );

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

	//water
	if(water.index > 0){
		int spriteNum = 0;
		int waterlevel = water.index;

		//if(waterlevel == 7) waterlevel--;

		if(water.type == 0)
			spriteNum = SPRITEOBJECT_WATERLEVEL1 + waterlevel - 1;
		if(water.type == 1)
			spriteNum = SPRITEOBJECT_WATERLEVEL1_LAVA + waterlevel - 1;
		DrawSpriteFromSheet( spriteNum, IMGObjectSheet, drawx, drawy );
	}

	// creature
	// ensure there is *some* creature according to the map data
	// (no guarantee it is the right one)
	if(creature != null && (occ.bits.unit || occ.bits.unit_grounded)){
		DrawCreature(drawx, drawy, creature);
	}
	al_set_separate_blender(op, src, dst, alpha_op, alpha_src, alpha_dst, color);
	if(config.currentOverlay != none)
		switch(config.currentOverlay){
		case flow_size:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.flow_size);
			break;
		case pile:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.pile);
			break;
		case dig:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.dig);
			break;
		case detail:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.detail);
			break;
		case detail_event:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.detail_event);
			break;
		case hidden:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.hidden);
			break;
		case geolayer_index:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.geolayer_index);
			break;
		case light:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.light);
			break;
		case subterranean:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.subterranean);
			break;
		case skyview:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.skyview);
			break;
		case biome:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.biome);
			break;
		case liquid_type:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.liquid_type);
			break;
		case water_table:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.water_table);
			break;
		case rained:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.rained);
			break;
		case traffic:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.traffic);
			break;
		case flow_forbid:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.flow_forbid);
			break;
		case liquid_static:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.liquid_static);
			break;
		case moss:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.moss);
			break;
		case feature_present:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.feature_present);
			break;
		case liquid_character:
			draw_textf_border(font, drawx + (SPRITEWIDTH/2), drawy-al_get_font_line_height(font), ALLEGRO_ALIGN_CENTRE,
				"%i", designations.bits.liquid_character);
			break;
			//default:
	}
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
	if(creature != null && (occ.bits.unit || occ.bits.unit_grounded)){
		DrawCreatureText(drawx, drawy, creature);
	}
}

void Block::DrawRamptops(){
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

			al_draw_bitmap_region(imageSheet(sprite,IMGRampSheet), sheetOffsetX, sheetOffsetY, SPRITEWIDTH, TILEHEIGHT + FLOORHEIGHT, drawx, drawy, 0);
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

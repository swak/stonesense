#include "newblockdraw.h"

std::map< t_cacheKey, BITMAP * > onDemandSpriteCache;

void onDemandSpriteFromSheet( Block *b, int spriteNum, BITMAP* target, BITMAP* spriteSheet, int x, int y, tileTypes type )
{
	BITMAP *t = null;
	int sheetx = spriteNum % SHEET_OBJECTSWIDE;
	int sheety = spriteNum / SHEET_OBJECTSWIDE;

	struct t_SpriteWithOffset s;
	s.sheetIndex = spriteNum;
	s.fileIndex = INVALID_INDEX;

	t = onDemandBlock ( b, &s, type );

	if(t)
		masked_blit(t, target,
		  0, 0,
		  x, y - (WALLHEIGHT), 
		  SPRITEHEIGHT, SPRITEHEIGHT);
	else
		DrawSpriteFromSheet( spriteNum, target, spriteSheet, x, y );
}

// Builds a shaded block on-demand, uses cached copy if one is available.
// call only if there are handled FX for this block or it'll just cache useless crap
BITMAP *onDemandBlock ( Block *b, t_SpriteWithOffset *sprite, tileTypes type )
{
	// IF t_occupancy IS CHANGED, THIS WILL BREAK UNTIL t_occupancy_bits IS CHANGED TO MATCH IT
	t_occupancy_bits o = *((t_occupancy_bits *)&b->occ.bits);
	t_cacheKey k;

	// Blank out all the building and item bits, we just don't care for tinting.
	o.building = o.item = o.unit = o.unit_grounded = false;

	// if there are no renderable FX on this tile, tell draw to use unmodified tile
	if( *((unsigned int *)&o) < 1 && b->Darken == false)
		return null;

	k.occupancy = *((unsigned int *)&o);
	k.sprite = *sprite;
	std::map< t_cacheKey, BITMAP * >::iterator v;

	// If we found a cached version of this colorized sprite, return that
	if( (v = onDemandSpriteCache.find(k) ) != onDemandSpriteCache.end() )
		return v->second;

	unsigned int 
		targetHeight = 0, 
		targetWidth = SPRITEWIDTH, 
		sheetOffsetY = 0, 
		sheetOffsetX = SPRITEWIDTH * (sprite->sheetIndex % SHEET_OBJECTSWIDE);

	switch( type )
	{
	case floor:
		targetHeight = TILEHEIGHT + FLOORHEIGHT;
		sheetOffsetY = (TILEHEIGHT + FLOORHEIGHT) * (sprite->sheetIndex / SHEET_OBJECTSWIDE);
		break;
	case wall:
	case water:
		targetHeight = SPRITEHEIGHT;
		sheetOffsetY = SPRITEHEIGHT * (sprite->sheetIndex / SHEET_OBJECTSWIDE);
		break;
	default:
		targetHeight = TILEHEIGHT;
		sheetOffsetY = 0;
		break;
	}

	// Create the base sprite to colorize
	BITMAP *tmpBmp = null;
	BITMAP *backBmp = create_bitmap( TILEWIDTH, targetHeight );

	clear_to_color( backBmp, makecol(255, 0, 255) );

	masked_blit(
		( sprite->fileIndex > -1 ? getImgFile( sprite->fileIndex ) : IMGObjectSheet ),
		backBmp, 
		sheetOffsetX, sheetOffsetY, 
		0, 0, 
		TILEWIDTH, targetHeight
		);

	if( o.snow && type != water )
	{
		tmpBmp = create_bitmap( backBmp->w, backBmp->h );
		clear_to_color( tmpBmp, makecol(255, 0, 255) );


		BITMAP *backBmp2 = create_bitmap( backBmp->w, backBmp->h );
		clear_to_color( backBmp2, makecol(255, 0, 255) );

		masked_blit( backBmp, tmpBmp, 0,0,0,0,TILEWIDTH, targetHeight);		
		// this will overdraw squares to the south and east (down left and down right) but they'll be overdrawn... will fuck up on edges, fixme
		masked_blit( IMGObjectSheet, backBmp2, SPRITEOBJECT_WHITEFILTER * TILEWIDTH, 0, 0, TILEWIDTH - targetHeight, TILEWIDTH, targetHeight);

		set_luminance_blender( 192, 192, 200, 0 );
		draw_lit_sprite( backBmp2, tmpBmp, 0, 0, 128 );

		set_dodge_blender( 255, 255, 255, 0 );
		draw_lit_sprite( tmpBmp, backBmp2, 0, 0, 255 );
		destroy_bitmap( backBmp2 );
	}

	// tint/shade everything but floors for now, floors use the nasty blobs
	if( floor != type )
	{
		if( o.vomit || o.blood || o.blood2 )
		{
			if( tmpBmp == null )
			{
				tmpBmp = create_bitmap( backBmp->w, backBmp->h );
				clear_to_color( tmpBmp, makecol(255, 0, 255) );
			}
		}

		if ( o.vomit && ( o.blood || o.blood2 ) )
		{
			set_trans_blender( 255, 255, 255, 0 );
			draw_lit_sprite( tmpBmp, backBmp, 0, 0, 230 );

			set_multiply_blender( 100, 120, 25, 0 );
			draw_lit_sprite( tmpBmp, backBmp, 0, 0, 230 );
		}
		else if( ( o.blood || o.blood2 ) )
		{
			//set_trans_blender( 255, 255, 255, 0 );
			//draw_lit_sprite( tmpBmp, backBmp, 0, 0, 230 );

			set_trans_blender( 220, 20, 25, 0 );
			draw_lit_sprite( tmpBmp, backBmp, 0, 0, 255 );
		}
		else if( o.vomit )
		{
			set_trans_blender( 255, 255, 255, 0 );
			draw_lit_sprite( tmpBmp, backBmp, 0, 0, 230 );

			set_multiply_blender( 75, 120, 25, 0 );
			draw_lit_sprite( tmpBmp, backBmp, 0, 0, 230 );
		}
	}

	// do floor lighting last
	if( type == floor && b->Darken )
	{
		tmpBmp = create_bitmap( TILEWIDTH, targetHeight );
		clear_to_color( tmpBmp, makecol( 255, 0, 255) );
		masked_blit( 
			( sprite->fileIndex != -1 ? getImgFile( sprite->fileIndex ) : IMGObjectSheet ),
			tmpBmp,
			TILEWIDTH * (sprite->sheetIndex % SPRITEWIDTH),
			FLOORHEIGHT * (sprite->sheetIndex / SPRITEHEIGHT),
			0,0,
			TILEWIDTH, targetHeight);

		clear_to_color( backBmp, makecol( 255, 0, 255 ) );
		masked_blit( IMGObjectSheet, backBmp, 6 * TILEWIDTH, 0, 0, 0, TILEWIDTH, SPRITEHEIGHT );

		set_multiply_blender( 255, 255, 255, 64 );
		draw_trans_sprite( tmpBmp, backBmp, 0, 0 );
	}

	destroy_bitmap(backBmp);

	if(tmpBmp == null)
		return null;

	return onDemandSpriteCache[k] = tmpBmp;
}

void drawFloor ( Block *b, BITMAP *target, int32_t drawx, int32_t drawy )
{
	t_SpriteWithOffset sprite;

	//Draw Floor
	if(b->floorType > 0 || b->wallType > 0 || b->ramp.type > 0 || b->stairType > 0){

		//If tile has no floor, look for a Filler Floor from it's wall
		if (b->floorType > 0)
			sprite = GetFloorSpriteMap(b->floorType, b->material);
		else if (b->wallType > 0)
			sprite = GetFloorSpriteMap(b->wallType, b->material);
		else if (b->ramp.type > 0)
			sprite = GetFloorSpriteMap(b->ramp.type, b->material);
		else if (b->stairType > 0)
			sprite = GetFloorSpriteMap(b->stairType, b->material);

		if(sprite.sheetIndex != INVALID_INDEX)
		{
			//if floor is muddy, override regular floor
			if( b->occ.bits.mud ) // && water.index == 0 ) // ???
			{
				sprite.sheetIndex = SPRITEFLOOR_WATERFLOOR;
				sprite.fileIndex = INVALID_INDEX;
			}
			if (sprite.sheetIndex == UNCONFIGURED_INDEX)
			{
				sprite.sheetIndex = SPRITEOBJECT_FLOOR_NA;
				sprite.fileIndex = INVALID_INDEX;
			}


			BITMAP *tmpBmp = NULL;

			// handle ondemand tile generation stuff for occupancy.
			// if no FX are required, it'll still do the fast draw
			if( *(((unsigned int *)&b->occ.bits)) > 0 || b->Darken )
				tmpBmp = onDemandBlock( b, &sprite, floor );

			if(tmpBmp != NULL)
			{
				masked_blit(tmpBmp, target, 0, 0, drawx, drawy, TILEWIDTH,TILEHEIGHT + FLOORHEIGHT);
			}
			else
			{
				int sheetOffsetX = TILEWIDTH * (sprite.sheetIndex % SHEET_OBJECTSWIDE),
					sheetOffsetY = (FLOORHEIGHT) * (sprite.sheetIndex / SHEET_OBJECTSWIDE);

				masked_blit( 
					(sprite.fileIndex == -1 ? IMGObjectSheet : getImgFile(sprite.fileIndex)), 
					target, 
					sheetOffsetX,sheetOffsetY, 
					drawx,drawy, 
					TILEWIDTH,TILEHEIGHT + FLOORHEIGHT
				);
			}
		}
	}
}

void drawFloorBlood ( Block *b, BITMAP *target, int32_t drawx, int32_t drawy )
{
	t_occupancy occ = b->occ;
	t_SpriteWithOffset sprite;

	int x = b->x, y = b->y, z = b->z;


	if( b->water.index < 1 && (occ.bits.blood || occ.bits.blood2 ))
	{
		sprite.fileIndex = INVALID_INDEX;

		// Spatter (should be blood, not blood2) swapped for testing
		if( occ.bits.blood2 )
			sprite.sheetIndex = 7;

		// Smear (should be blood2, not blood) swapped for testing
		else
		{
			// if there's no block in the respective direction it's false. if there's no blood in that direction it's false too. should also check to see if there's a ramp below, but since blood doesn't flow, that'd look wrong anyway.
			bool _N = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eUp ) != NULL ? b->ownerSegment->getBlockRelativeTo( x, y, z, eUp )->occ.bits.blood : false ),
				_S = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eDown ) != NULL ? b->ownerSegment->getBlockRelativeTo( x, y, z, eDown )->occ.bits.blood : false ),
				_E = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eRight ) != NULL ? b->ownerSegment->getBlockRelativeTo( x, y, z, eRight )->occ.bits.blood : false ),
				_W = ( b->ownerSegment->getBlockRelativeTo( x, y, z, eLeft ) != NULL ? b->ownerSegment->getBlockRelativeTo( x, y, z, eLeft )->occ.bits.blood : false );

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

		masked_blit( IMGBloodSheet, target, sheetOffsetX, sheetOffsetY, drawx, drawy, TILEWIDTH, TILEHEIGHT + FLOORHEIGHT);
	}
}

void drawRamps( Block *b, BITMAP *target, int32_t drawx, int32_t drawy )
{
	t_SpriteWithOffset sprite;

	//Draw Ramp
	if( b->ramp.type > 0 )
	{
		sprite = GetBlockSpriteMap( b->ramp.type, b->material );
		if ( sprite.sheetIndex == UNCONFIGURED_INDEX )
		{
			sprite.sheetIndex = 0;
			sprite.fileIndex = INVALID_INDEX;
		}
		if ( sprite.sheetIndex != INVALID_INDEX )
		{
			int
				sheetOffsetX = SPRITEWIDTH *  b->ramp.index,
				sheetOffsetY = ( (TILEHEIGHT + FLOORHEIGHT + SPRITEHEIGHT) * sprite.sheetIndex ) + TILEHEIGHT + FLOORHEIGHT;

			masked_blit(
				( sprite.fileIndex == -1 ? IMGRampSheet : getImgFile(sprite.fileIndex) ),
				target, sheetOffsetX,sheetOffsetY, drawx,drawy - (WALLHEIGHT), SPRITEWIDTH, SPRITEHEIGHT);
		}
	}
}

void drawWalls ( Block *b, BITMAP *target, int32_t drawx, int32_t drawy )
{
	t_SpriteWithOffset sprite;

	//Draw Walls
	if(b->wallType > 0){
		//draw wall
		sprite =  GetBlockSpriteMap(b->wallType, b->material);

		if (sprite.sheetIndex == UNCONFIGURED_INDEX)
		{
			sprite.sheetIndex = SPRITEOBJECT_WALL_NA;
			sprite.fileIndex = INVALID_INDEX;  
		}
		if (sprite.sheetIndex == INVALID_INDEX)
		{
			//skip   
		}    
		else if( config.truncate_walls && b->z == b->ownerSegment->z + b->ownerSegment->sizez -2){
			int sheetx = sprite.sheetIndex % SHEET_OBJECTSWIDE;
			int sheety = sprite.sheetIndex / SHEET_OBJECTSWIDE;
			//draw a tiny bit of wall
			masked_blit(
				( sprite.fileIndex == -1 ? IMGObjectSheet : getImgFile(sprite.fileIndex) ),
				target,
				sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT+WALL_CUTOFF_HEIGHT,
				drawx, drawy - (WALLHEIGHT)+WALL_CUTOFF_HEIGHT, SPRITEWIDTH, SPRITEHEIGHT-WALL_CUTOFF_HEIGHT);
			//draw cut-off floor thing
			masked_blit(IMGObjectSheet, target,
				TILEWIDTH * SPRITEFLOOR_CUTOFF, 0,
				drawx, drawy-(SPRITEHEIGHT-WALL_CUTOFF_HEIGHT)/2, SPRITEWIDTH, SPRITEWIDTH);
		}
		else {
			DrawSpriteFromSheet(sprite.sheetIndex, 
				target,
				( sprite.fileIndex == -1 ? IMGObjectSheet : getImgFile(sprite.fileIndex) ),
				drawx, drawy );
		}
	}
}

void drawStairs( Block *b, BITMAP *target, int32_t drawx, int32_t drawy )
{
	t_SpriteWithOffset sprite;

	//Draw Stairs
	if( b->stairType > 0 )
	{
		//down part
		//skipping at the moment?
		//int spriteNum = GetFloorSpriteMap(stairType,material);
		//DrawSpriteFromSheet( spriteNum, target, IMGObjectSheet, drawx, drawy );

		//up part
		bool mirrored = false;
		if( findWallCloseTo(b->ownerSegment, b) == eSimpleW )
			mirrored = true;
		sprite = GetBlockSpriteMap( b->stairType, b->material );
		if( sprite.sheetIndex != INVALID_INDEX && sprite.sheetIndex != UNCONFIGURED_INDEX )
		{
			if (mirrored)
				sprite.sheetIndex += 1;
			DrawSpriteFromSheet( sprite.sheetIndex, target, 
				( sprite.fileIndex == -1 ? IMGObjectSheet : getImgFile(sprite.fileIndex) ), drawx, drawy );
		}
	}
}

void drawWallBlood( Block *b, BITMAP *target, int32_t drawx, int32_t drawy )
{
	// until this is defined with spriting, this is handled by the ondemandblock function as tinting the wall red
}

void drawWallLighting( Block *b, BITMAP *target, int32_t drawx, int32_t drawy )
{
	int offset = -1;

	if( b->Darken )
	{
		Block *_E = b->ownerSegment->getBlockRelativeTo( b->x, b->y, b->z, eRight ),
			*_S = b->ownerSegment->getBlockRelativeTo( b->x, b->y, b->z, eDown );

		offset = 6;

		if( _E != null && _E->Darken )
		{
			if( _S != null && _S->Darken )
				offset = 5;
			else
				offset = 3;
		}
		else if( _S != null && _S->Darken )
		{
			offset = 4;
		}
	}
	else
	{
		Block *_E = b->ownerSegment->getBlock( b->x + 1, b->y, b->z ),
		*_S = b->ownerSegment->getBlock( b->x, b->y + 1, b->z );

		if( _E != null && _E->Darken )
		{
			if( _S != null && _S->Darken )
				offset = 2;
			else
				offset = 1;
		}
		else if( _S != null && _S->Darken )
		{
			offset = 0;
		}
	}
	if( offset > -1 )
	{
		BITMAP *backBmp = create_bitmap( TILEWIDTH, SPRITEHEIGHT );
		clear_to_color( backBmp, makecol(255, 0, 255) );
		masked_blit( IMGObjectSheet, backBmp, (offset * TILEWIDTH), SPRITEHEIGHT, 0, 0, TILEWIDTH, SPRITEHEIGHT );
		set_multiply_blender( 255, 255, 255, 64 );
		draw_trans_sprite( target, backBmp, drawx, drawy - WALLHEIGHT );
		destroy_bitmap( backBmp );
	//	DrawSpriteFromSheet( 20 + offset, target, IMGObjectSheet, drawx, drawy );
	}
}

void drawWater ( Block *b, BITMAP *target, int32_t drawx, int32_t drawy )
{
	//water
	if(b->water.index > 0){
		int spriteNum = 0;
		int waterlevel = b->water.index;

		//if(waterlevel == 7) waterlevel--;

		if(b->water.type == 0)
		{
			spriteNum = SPRITEOBJECT_WATERLEVEL1 + waterlevel - 1;

			// This automatically does water bloodying
			onDemandSpriteFromSheet( b, spriteNum, target, IMGObjectSheet, drawx, drawy, water );
			//DrawSpriteFromSheet( spriteNum, target, IMGObjectSheet, drawx, drawy);
		}
		if(b->water.type == 1)
		{
			spriteNum = SPRITEOBJECT_WATERLEVEL1_LAVA + waterlevel - 1;
			DrawSpriteFromSheet( spriteNum, target, IMGObjectSheet, drawx, drawy);
		}
	}
}


void Block::Draw(BITMAP *target)
{
	t_SpriteWithOffset sprite;

	int tileBorderColor = makecol( 85, 85, 85 );

	int32_t drawx = x,
		drawy = y,
		drawz = z;

	correctBlockForSegmetOffset ( drawx, drawy, drawz );
	correctBlockForRotation ( drawx, drawy, drawz );
	pointToScreen( &drawx, &drawy, drawz );

	drawx -= TILEWIDTH >> 1;

	// draw floor
	drawFloor ( this, target, drawx, drawy );

	// draw floor blood if there's nothing on top of the floor that makes it look funky
	if(  floorType > 0 && ! (wallType > 0 || this->ramp.type > 0 || this->stairType > 0 ) )
		drawFloorBlood ( this, target, drawx, drawy );

	//Northern frame
	if(this->depthBorderNorth)
		line(target, drawx + (TILEWIDTH>>1), drawy, drawx+TILEWIDTH-1, drawy+(TILEHEIGHT>>1)-1, tileBorderColor);

	//Western frame
	if(this->depthBorderWest)
		line(target, drawx, drawy+(TILEHEIGHT>>1)-1, drawx+(TILEWIDTH>>1)-1, drawy, tileBorderColor);

	drawRamps ( this, target, drawx, drawy );

	//vegetation
	if(tree.index > 0 || tree.type > 0){
		sprite =  GetSpriteVegetation( (TileClass) getVegetationType( this->floorType ), tree.index );
		DrawSpriteFromSheet( sprite.sheetIndex, target, 
			( sprite.fileIndex == -1 ? IMGObjectSheet : getImgFile(sprite.fileIndex) ), drawx, drawy );
	}

	//shadow
	if (shadow > 0)
	{
		DrawSpriteFromSheet( BASE_SHADOW_TILE + shadow - 1, target, IMGObjectSheet, drawx, (ramp.type > 0)?(drawy - (WALLHEIGHT/2)):drawy );
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
			DrawSpriteFromSheet(sprite.sheetIndex , target, 
				( sprite.fileIndex == -1 ? IMGObjectSheet : getImgFile(sprite.fileIndex) ), 
				drawx + building.sprites[i].x,
				drawy + building.sprites[i].y);
		}
	}

	// draw stairs
	drawStairs( this, target, drawx, drawy );

	// draw walls
	drawWalls( this, target, drawx, drawy );

	// Wall bloodying for fun
	drawWallBlood( this, target, drawx, drawy );

	// Wall shading for indoors
	if( wallType > 0 )
		drawWallLighting( this, target, drawx, drawy );

	// water
	drawWater ( this, target, drawx, drawy );

	// creature
	// ensure there is *some* creature according to the map data
	// (no guarantee it is the right one)
	if(creature != null && (occ.bits.unit || occ.bits.unit_grounded))
		DrawCreature( target, drawx, drawy, creature);
}

/* DEBRIS FLAGS!

kaypy + batcountry

wooden	<= d1: 0, d2: 1, d3: 1, d4: 0, d5: 1
wooden	>= d1: 0, d2: 1, d3: 1, d4: 0, d5: 0

bone	<= d1: 1, d2: 1, d3: 1, d4: 0, d5: 1
bone	>= d1: 1, d2: 1, d3: 1, d4: 0, d5: 0

iron	<= d1: 0, d2: 0, d3: 0, d4: 1, d5: 1
iron	>= d1: 0, d2: 0, d3: 0, d4: 1, d5: 0

debris1-3 == 1 2 4 bits for EGA/DF color palette
debris4 == brightness?
debris5 == GTE vs LTE icon in DF - facing

*/
#pragma once

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

enum tileTypes {
	floor,
	wall, 
	ramp,
	ramptop,
	water
};

// Builds a shaded block on-demand, uses cached copy if one is available.
// call only if there are handled FX for this block or it'll just cache useless crap
BITMAP *onDemandBlock ( Block *b, t_SpriteWithOffset *sprite, tileTypes type );
void drawFloor ( Block *b, BITMAP *target, int32_t drawx, int32_t drawy );
void drawFloorBlood ( Block *b, BITMAP *target, int32_t drawx, int32_t drawy );
void drawRamps( Block *b, BITMAP *target, int32_t drawx, int32_t drawy );
void drawWalls ( Block *b, BITMAP *target, int32_t drawx, int32_t drawy );
void drawStairs( Block *b, BITMAP *target, int32_t drawx, int32_t drawy );
void drawWallBlood( Block *b, BITMAP *target, int32_t drawx, int32_t drawy );
void drawWallLighting( Block *b, BITMAP *target, int32_t drawx, int32_t drawy );
void drawWater ( Block *b, BITMAP *target, int32_t drawx, int32_t drawy );

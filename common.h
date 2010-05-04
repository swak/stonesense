#pragma once
#pragma warning( disable : 4312 ) //64-bit portability issue
#pragma warning( disable : 4996 ) //'deprecated' function warning
#pragma warning( disable : 4251 ) //'identifier' : class 'type' needs to have dll-interface to be used by clients of class 'type2'
#define SKIP_DFHACK_STDINT

#include <assert.h>
#include <vector>
#include <map>
#include <allegro5/allegro.h>
#include <allegro5/allegro_primitives.h>
#include <allegro5/allegro_font.h>
#include <allegro5/allegro_ttf.h>
#include <allegro5/allegro_image.h>
#include <allegro5/allegro_native_dialog.h>
#include <allegro5/allegro_opengl.h>
#include <allegro5/utf8.h>

using namespace std;

#include <DFTypes.h>
#include <DFHackAPI.h>
#include <DFProcess.h>
#include <DFMemInfo.h>
#include <DFVector.h>
#include <modules/Vegetation.h>
#include <modules/Materials.h>
#include <modules/Position.h>
#include <modules/Maps.h>
#include <modules/Creatures.h>
#include <modules/Buildings.h>
#include <modules/Constructions.h>
#include <modules/Gui.h>
using namespace DFHack;

#include "commonTypes.h"
#include "Block.h"
#include <stdio.h>
#include <stdarg.h>

#define RELEASE

#define null 0

#define INVALID_INDEX -1
#define UNCONFIGURED_INDEX -2

#define ISOMETRIC
//#define CAVALIER

// TILEWIDTH: total size of sprite left to right
// TILEHEIGHT: floor part of sprite top to bottom
// WALLHEIGHT: height of a one pixel wide stripe of wall top to bottom
//		== amount top of wall is raised from where the floor would be
// FLOORHEIGHT: height of a one pixel stripe of the 'wall' of a floor tile

#ifdef CAVALIER
#define TILEWIDTH 46
#define TILEHEIGHT 46
#define WALLHEIGHT 24
#define FLOORHEIGHT 6
#endif

#ifdef ISOMETRIC
#define TILEWIDTH 32
#define TILEHEIGHT 16
#define WALLHEIGHT 16
#define FLOORHEIGHT 4
#endif

#ifdef DOUBLESIZE
#define TILEWIDTH 64
#define TILEHEIGHT 32
#define WALLHEIGHT 32
#define FLOORHEIGHT 8
#endif

#define GFXMODE GFX_AUTODETECT_WINDOWED
#define FULLSCREEN false 
#define RESOLUTION_WIDTH 800
#define RESOLUTION_HEIGHT 600
// Height of a one pixel stripe of the wall of an entire block,
//		including wall and floor tile
#define BLOCKHEIGHT (WALLHEIGHT + FLOORHEIGHT)
// Width of area copied from an image file
// may be different to tile dimensions to allow overlap later
#define SPRITEWIDTH TILEWIDTH
// Height of area copied from an image file
// may be different to tile dimensions to allow overlap later
#define SPRITEHEIGHT (TILEHEIGHT + WALLHEIGHT)
#define WALL_CUTOFF_HEIGHT 15

#define DEFAULT_SEGMENTSIZE 20
#define DEFAULT_SEGMENTSIZE_Z  6
#define MAPNAVIGATIONSTEP 1
#define MAPNAVIGATIONSTEPBIG 10

#define SHEET_OBJECTSWIDE 20

//do not alter, defined by DF
#define CELLEDGESIZE 16

#define COLOR_SEGMENTOUTLINE 0x112211

#define BASE_SHADOW_TILE 160
#define DEFAULT_SHADOW 4
#define MAX_SHADOW 7

#define RANDOM_CUBE 16

// this shouldn't change with mods, so should
// be know ahead of time (especially since we
// use it for fake terrains already)
#define MAX_BASE_TERRAIN 525
#define FAKE_TERRAIN_COUNT 0
// we only need as many as our translation system
// in ContentLoader/lookupMaterialType deals with
#define MAX_MATGLOSS 24

enum dirTypes{
  eSimpleInvalid = -1,
  eSimpleSingle,
  //-----START: Do NOT rearrange these, they're used to autoinsert building borders
  eSimpleN,
  eSimpleW,
  eSimpleS,
  eSimpleE,

  eSimpleNnW,
  eSimpleSnW,
  eSimpleSnE,
  eSimpleNnE,
  //-----END

  eSimpleNnS,
  eSimpleWnE,

  eSimpleNnEnS,
  eSimpleNnEnW,
  eSimpleNnWnS,
  eSimpleWnSnE,

  eSimpleNnWnSnE
};

enum dirRelative{
  eLeft,
  eDown,
  eRight,
  eUp,
  eAbove,
  eBelow,

  eUpLeft,
  eUpRight,
  eDownLeft,
  eDownRight,
};
//class WorldSegment;



//main.cpp
void correctBlockForSegmetOffset(int32_t& x, int32_t& y, int32_t& z);

void WriteErr(char* msg, ...);
void LogVerbose(char* msg, ...);
void DisplayErr(const char *format, ...);

extern GameConfiguration config;

extern uint32_t DebugInt1;
extern uint32_t ClockedTime;
extern uint32_t ClockedTime2;

extern bool timeToReloadSegment;
extern bool timeToReloadConfig;
extern char currentAnimationFrame;
extern bool animationFrameShown;

#define MAX_ANIMFRAME 6

// binary 00111111
#define ALL_FRAMES 63

#define TMR1_START (ClockedTime = clock())
#define TMR1_STOP  (ClockedTime = clock() - ClockedTime)
#define TMR2_START (ClockedTime2 = clock())
#define TMR2_STOP  (ClockedTime2 = clock() - ClockedTime2)

extern bool key[ALLEGRO_KEY_MAX];

void correctBlockForRotation(int32_t& x, int32_t& y, int32_t& z);

//from UserInput.cpp
void doKeys();
void doKeys(int key);
void initAutoReload();
void abortAutoReload();

//Config.cpp
bool loadConfigFile();


//xmlBuildingReader.cpp
class BuildingConfiguration;

#define FILENAME_BUFFERSIZE 50
// temp buffers are bigger because they deal with absolute paths
#define FILENAME_BUFFERSIZE_LOCAL 200

extern ALLEGRO_FONT *font;
extern ALLEGRO_DISPLAY     *display;
extern ALLEGRO_KEYBOARD_STATE keyboard;
extern ALLEGRO_TIMER *reloadtimer;
extern ALLEGRO_TIMER * animationtimer;
extern ALLEGRO_MOUSE_STATE mouse;

#define INORGANIC 0
#define AMBER 1
#define CORAL 2
#define GREEN_GLASS 3
#define CLEAR_GLASS 4
#define CRYSTAL_GLASS 5
#define ICE 6
#define COAL 7
#define POTASH 8
#define ASH 9
#define PEARLASH 10
#define LYE 11
#define MUD 12
#define VOMIT 13
#define SALT 14
#define FILTH 15
#define FILTH_FROZEN 16
#define UNKOWN_FROZEN 17
#define GRIME 18
#define LEATHER 37
#define WOOD 420
#define PLANTCLOTH 421

#define NUM_FORMS 5
#define FORM_BAR 1
#define FORM_BLOCK 2
#define FORM_BOULDER 3
#define FORM_LOG 4
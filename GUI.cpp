#include <assert.h>
#include <vector>

using namespace std;


#include "common.h"
#include "Block.h"
#include "GUI.h"
#include "WorldSegment.h"
#include "SpriteMaps.h"
#include "MapLoading.h"
#include "GameBuildings.h"
#include "Creatures.h"
#include "ContentLoader.h"
#include "BlockFactory.h"

#include "loadpng/loadpng.h"


WorldSegment* viewedSegment;
int DisplayedSegmentX;
int DisplayedSegmentY;
int DisplayedSegmentZ;
int DisplayedRotation = 0;
int MiniMapTopLeftX = 0;
int MiniMapTopLeftY = 0;
int MiniMapBottomRightX = 0;
int MiniMapBottomRightY = 0;
int MiniMapSegmentWidth =0;
int MiniMapSegmentHeight =0;
double oneBlockInPixels = 0;

BITMAP* IMGFloorSheet; 
BITMAP* IMGObjectSheet;
BITMAP* IMGCreatureSheet; 
BITMAP* IMGRampSheet; 
BITMAP* IMGRamptopSheet; 
BITMAP* buffer = 0;
vector<BITMAP*> IMGFilelist;
vector<string*> IMGFilenames;

Crd2D debugCursor;

void ScreenToPoint(int x,int y,int &x1, int &y1, int &z1)
{ //assume z of 0
    x-=TILEWIDTH/2;
    y+=TILEWIDTH/2;
    z1 = -1;
    y+= z1*BLOCKHEIGHT/2;
    //y-=BLOCKHEIGHT;
    x+=TILEWIDTH>>1;
    static int offx = config.screenWidth /2;
    static int offy = 50;
    if( config.lift_segment_offscreen ) offy = -200;
    y-=offy;
    x-=offx;
    y1=y*2-x;
    x1=x*2+y1;
    x1/=TILEWIDTH;
    y1/=TILEWIDTH;
 
}

void pointToScreen(int *inx, int *iny, int inz){
  static int offx = config.screenWidth / 2;
  static int offy = 50;
  if( config.lift_segment_offscreen ) offy = -200;
	int z=inz;
	int x = *inx-*iny;
	int y = *inx+*iny;
	x = x * TILEWIDTH / 2;
	y = y * TILEHEIGHT / 2;
	x+=offx;
	y+=offy;
	y-=z * BLOCKHEIGHT;
	*inx=x;*iny=y;
}

Crd2D WorldBlockToScreen(int32_t x, int32_t y, int32_t z){
	correctBlockForSegmetOffset( x, y, z);
	return LocalBlockToScreen(x, y, z);
}

Crd2D LocalBlockToScreen(int32_t x, int32_t y, int32_t z){
	pointToScreen((int*)&x, (int*)&y, z);
	Crd2D result;
	result.x = x;
	result.y = y;
	return result;
}

void DrawCurrentLevelOutline(BITMAP* target, bool backPart){
  int x = viewedSegment->x+1;
  int y = viewedSegment->y+1;
  int z = DisplayedSegmentZ;
  int sizex = config.segmentSize.x-2;
  int sizey = config.segmentSize.y-2;
	
  if(config.hide_outer_blocks){
    x++;y++;
    sizex -= 2;
    sizey -= 2;
  }

	Crd2D p1 = WorldBlockToScreen(x, y, z);
	Crd2D p2 = WorldBlockToScreen(x, y + sizey , z);
	Crd2D p3 = WorldBlockToScreen(x + sizex , y, z);
	Crd2D p4 = WorldBlockToScreen(x + sizex , y + sizey , z);
  p1.y += FLOORHEIGHT;
  p2.y += FLOORHEIGHT;
  p3.y += FLOORHEIGHT;
  p4.y += FLOORHEIGHT;
	if(backPart){
		line(target, p1.x, p1.y, p1.x, p1.y-BLOCKHEIGHT, COLOR_SEGMENTOUTLINE);
		line(target, p1.x, p1.y, p2.x, p2.y, COLOR_SEGMENTOUTLINE);
		line(target, p1.x, p1.y-BLOCKHEIGHT, p2.x, p2.y-BLOCKHEIGHT, COLOR_SEGMENTOUTLINE);
		line(target, p2.x, p2.y, p2.x, p2.y-BLOCKHEIGHT, COLOR_SEGMENTOUTLINE);

		line(target, p1.x, p1.y, p3.x, p3.y, COLOR_SEGMENTOUTLINE);
		line(target, p1.x, p1.y-BLOCKHEIGHT, p3.x, p3.y-BLOCKHEIGHT, COLOR_SEGMENTOUTLINE);
		line(target, p3.x, p3.y, p3.x, p3.y-BLOCKHEIGHT, COLOR_SEGMENTOUTLINE);
	}else{
		line(target, p4.x, p4.y, p4.x, p4.y-BLOCKHEIGHT, COLOR_SEGMENTOUTLINE);
		line(target, p4.x, p4.y, p2.x, p2.y, COLOR_SEGMENTOUTLINE);
		line(target, p4.x, p4.y-BLOCKHEIGHT, p2.x, p2.y-BLOCKHEIGHT, COLOR_SEGMENTOUTLINE);

		line(target, p4.x, p4.y, p3.x, p3.y, COLOR_SEGMENTOUTLINE);
		line(target, p4.x, p4.y-BLOCKHEIGHT, p3.x, p3.y-BLOCKHEIGHT, COLOR_SEGMENTOUTLINE);
	}
}

void drawDebugCursorAndInfo(BITMAP* target){
  Crd2D point = LocalBlockToScreen(debugCursor.x, debugCursor.y, 0);

  int spriteNum =  SPRITEOBJECT_CURSOR;
	int sheetx = spriteNum % SHEET_OBJECTSWIDE;
	int sheety = spriteNum / SHEET_OBJECTSWIDE;
  masked_blit(IMGObjectSheet, target,
    sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT,
    point.x - SPRITEWIDTH/2, point.y - (WALLHEIGHT), SPRITEWIDTH, SPRITEHEIGHT);

  //get block info
  Block* b = viewedSegment->getBlockLocal( debugCursor.x, debugCursor.y, viewedSegment->sizez - 1);
  int i = 10;
  textprintf(target, font, 2, config.screenHeight-20-(i--*10), 0xFFFFFF, "Block 0x%x", b);
  if(!b) return;

  textprintf(target, font, 2, config.screenHeight-20-(i--*10), 0xFFFFFF, 
    "Coord:(%i,%i,%i)", b->x,b->y,b->z);

  textprintf(target, font, 2, config.screenHeight-20-(i--*10), 0xFFFFFF, 
    "wall:%i floor:%i  Material:%s(%i)", b->wallType, b->floorType, 
    (b->materialIndex != INVALID_INDEX ? v_stonetypes[b->materialIndex].id: ""),
    b->materialIndex);
    
    textprintf(target, font, 2, config.screenHeight-20-(i--*10), 0xFFFFFF, 
    "Building Occ: %i Index: %i", b->occ.bits.building, b->building.index);

  if(b->water.index > 0 || b->tree.index != 0)
    textprintf(target, font, 2, config.screenHeight-20-(i--*10), 0xFFFFFF, 
      "tree:%i water:%i", b->tree.index, b->water.index);
  //building
  if(b->building.info.type != BUILDINGTYPE_NA && b->building.info.type != BUILDINGTYPE_BLACKBOX){
    textprintf(target, font, 2, config.screenHeight-20-(i--*10), 0xFFFFFF, 
      "Building: %s(%i) MatType:%i MatIndex:%i", 
      contentLoader.buildingNameStrings.at(b->building.info.type).c_str(),
      b->building.info.type, b->building.info.material.type, b->building.info.material.index);
  }
  //creatures
  if(b->creature != null){
    textprintf(target, font, 2, config.screenHeight-20-(i--*10), 0xFFFFFF, 
      "Creature:%s(%i) Job:%s", 
      contentLoader.creatureNameStrings.at(b->creature->type).id, b->creature->type, 
      dfMemoryInfo.getProfession( b->creature->profession ).c_str());
    
    char strCreature[150] = {0};
    generateCreatureDebugString( b->creature, strCreature );
    //memset(strCreature, -1, 50);
    textprintf(target, font, 2, config.screenHeight-20-(i--*10), 0xFFFFFF, 
      "flag1: %s ", strCreature );
  }
}

void DrawMinimap(BITMAP* target){
	int size = 100;
  //double oneBlockInPixels;
  int posx = config.screenWidth-size-10;
	int posy = 10;

  if(!viewedSegment || viewedSegment->regionSize.x == 0 || viewedSegment->regionSize.y == 0){
		textprintf(target, font, posx,posy, 0xFFFFFF, "No map loaded");
		return;
	}

  oneBlockInPixels = (double) size / viewedSegment->regionSize.x;
  //map outine
  int mapheight = (int)(viewedSegment->regionSize.y * oneBlockInPixels);
  rect(target, posx, posy, posx+size, posy+mapheight, 0);
  //current segment outline
  int x = (size * (viewedSegment->x+1)) / viewedSegment->regionSize.x;
  int y = (mapheight * (viewedSegment->y+1)) / viewedSegment->regionSize.y;
  MiniMapSegmentWidth = (viewedSegment->sizex-2) * oneBlockInPixels;
  MiniMapSegmentHeight = (viewedSegment->sizey-2) * oneBlockInPixels;
  rect(target, posx+x, posy+y, posx+x+MiniMapSegmentWidth, posy+y+MiniMapSegmentHeight,0);
  MiniMapTopLeftX = posx;
  MiniMapTopLeftY = posy;
  MiniMapBottomRightX = posx+size;
  MiniMapBottomRightY = posy+mapheight;
}

void DrawSpriteFromSheet( int spriteNum, BITMAP* target, BITMAP* spriteSheet, int x, int y){
    int sheetx = spriteNum % SHEET_OBJECTSWIDE;
		int sheety = spriteNum / SHEET_OBJECTSWIDE;
    //
    /*
    static BITMAP* tiny = null;
    if(!tiny)
      tiny = create_bitmap_ex(32, 32, 32);
   
    blit(spriteSheet, tiny, sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT, 0, 0, SPRITEWIDTH, SPRITEHEIGHT);

    blit(tiny, target,
      0,0,
      10, 60 , SPRITEWIDTH, SPRITEHEIGHT);
*/
    //draw_trans_sprite(target, tiny, x, y);
    
    masked_blit(spriteSheet, target,
      sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT,
      x, y - (WALLHEIGHT), SPRITEWIDTH, SPRITEHEIGHT);
}

void DrawSpriteIndexOverlay(int imageIndex){
  BITMAP* currentImage;
  if (imageIndex == -1)
  {
	  currentImage=IMGObjectSheet;
  }
  else
  {
    if( imageIndex >= (int)IMGFilelist.size()) 
      return;
	  currentImage=IMGFilelist[imageIndex];
  }
  clear_to_color(screen, makecol(255,0,255));
  draw_sprite(screen,currentImage,0,0);
  for(int i =0; i<= 20*SPRITEWIDTH; i+=SPRITEWIDTH)
    line(screen, i,0,i, config.screenHeight, 0);
  for(int i =0; i< config.screenHeight; i+=SPRITEHEIGHT)
    line(screen, 0,i, 20*SPRITEWIDTH,i, 0);

  for(int y = 0; y<20; y++){
    for(int x = 0; x<20; x+=5){
      int index = y * 20 + x;
      textprintf(screen, font,  x*SPRITEWIDTH+5, y* SPRITEHEIGHT+5, 0xFFffFF, "%i", index);
    }
  }	
  textprintf_right(screen, font, config.screenWidth-10, config.screenHeight -10, 0xFFffFF, 
    "%s (Press SPACE to return)",
    (imageIndex==-1?"objects.png":IMGFilenames[imageIndex]->c_str()));  
}


void DoSpriteIndexOverlay(){
  DrawSpriteIndexOverlay(-1);
  int index = 0;
  int max = (int)IMGFilenames.size();
  while(true)
  {
  	while(!key[KEY_SPACE] && !key[KEY_F10]) rest(50);
  	if (key[KEY_SPACE])
  	{
	  	break;
  	}
	DrawSpriteIndexOverlay(index);
	index++;
	if (index >= max)
		index = -1;
  }
  //redraw screen again
  paintboard();
}

void paintboard(){
  show_mouse(NULL);
	uint32_t starttime = clock();
	if(!buffer)
		buffer = create_bitmap(config.screenWidth, config.screenHeight);
  
  clear_to_color(buffer,makecol(95,95,160));
  //clear_to_color(buffer,makecol(12,7,49)); //this one is calm and nice
  
  if( viewedSegment == NULL ){
    textprintf_centre(screen, font, config.screenWidth/2, config.screenHeight/2, 0xFFffFF, "Could not find DF process");
    return;
  }

	
	

	if (config.show_osd) DrawCurrentLevelOutline(buffer, true);
  viewedSegment->drawAllBlocks(buffer);
	if (config.show_osd) DrawCurrentLevelOutline(buffer, false);

  DebugInt1 = viewedSegment->getNumBlocks();
	
  int DrawTime = clock() - starttime;
  
  if (config.show_osd)
  {
    textprintf_ex(buffer, font, 10,10, 0xFFFFFF,0, "%i,%i,%i, r%i", DisplayedSegmentX,DisplayedSegmentY,DisplayedSegmentZ, DisplayedRotation);
    
    if(config.debug_mode){
	    textprintf_ex(buffer, font, 10,20, 0xFFFFFF,0, "Timer1: %ims", ClockedTime);
      textprintf_ex(buffer, font, 10,30, 0xFFFFFF,0, "Timer2: %ims", ClockedTime2);
      textprintf_ex(buffer, font, 10,40, 0xFFFFFF,0, "Draw: %ims", DrawTime);
      textprintf_ex(buffer, font, 10,50, 0xFFFFFF,0, "D1: %i", blockFactory.getPoolSize());

      drawDebugCursorAndInfo(buffer);
    }

    if(config.follow_DFscreen)
      textprintf_centre_ex(buffer, font, config.screenWidth/2,10, 0xFFFFFF,0, "Locked on DF screen + (%d,%d,%d)",config.viewXoffset,config.viewYoffset,config.viewZoffset);
    if(config.single_layer_view)
      textprintf_centre_ex(buffer, font, config.screenWidth/2,20, 0xFFFFFF,0, "Single layer view");
    if(config.automatic_reload_time)
      textprintf_centre_ex(buffer, font, config.screenWidth/2,30, 0xFFFFFF,0, "Reloading every %0.1fs", (float)config.automatic_reload_time/1000);

    DrawMinimap(buffer);
  }
	acquire_screen();
  
	draw_sprite(screen,buffer,0,0);
  if( !config.Fullscreen )
    show_mouse(screen);
	release_screen();
}




BITMAP* load_bitmap_withWarning(char* path){
  BITMAP* img = 0;
  img = load_bitmap(path,0);
  if(!img){
    allegro_message("Unable to load image %s", path);
    exit(0);
  }
  return img;
}

void loadGraphicsFromDisk(){
  register_png_file_type();
	IMGObjectSheet = load_bitmap_withWarning("objects.png");
	IMGFloorSheet = load_bitmap_withWarning("floors.png");
	IMGCreatureSheet = load_bitmap_withWarning("creatures.png");
	IMGRampSheet = load_bitmap_withWarning("ramps.png");
	IMGRamptopSheet = load_bitmap_withWarning("ramptops.png");	
}
void destroyGraphics(){
  destroy_bitmap(IMGFloorSheet);
  destroy_bitmap(IMGObjectSheet);
  destroy_bitmap(IMGCreatureSheet);
  destroy_bitmap(IMGRampSheet);
  destroy_bitmap(IMGRamptopSheet);
}

//delete and clean out the image files
void flushImgFiles()
{
	//should be OK because we keep others from directly acccessing this stuff
	uint32_t numFiles = (uint32_t)IMGFilelist.size();
  assert( numFiles == IMGFilenames.size());
	for(uint32_t i = 0; i < numFiles; i++)
	{
		destroy_bitmap(IMGFilelist[i]);
		//should be same length, I hope
		delete(IMGFilenames[i]);
	}
	IMGFilelist.clear();
	IMGFilenames.clear();
}

BITMAP* getImgFile(int index)
{
	return IMGFilelist[index];	
}

int loadImgFile(char* filename)
{
	uint32_t numFiles = (uint32_t)IMGFilelist.size();
	for(uint32_t i = 0; i < numFiles; i++)
	{
		if (strcmp(filename, IMGFilenames[i]->c_str()) == 0)
			return i;
	}
	IMGFilelist.push_back(load_bitmap_withWarning(filename));
	IMGFilenames.push_back(new string(filename));
  return (int)IMGFilelist.size() - 1;
}


void saveScreenshot(){
  paintboard();
  //get filename
  char filename[20] ={0};
  FILE* fp;
  int index = 1;
  //search for the first screenshot# that does not exist already
  while(true){
    sprintf(filename, "screenshot%i.png", index);
    
    fp = fopen(filename, "r");
    if( fp != 0)
      fclose(fp);
    else
      //file does not exist, so exit loop
      break;
    index++;
  };
  //move image to 16 bits
  BITMAP* temp = create_bitmap_ex(16, buffer->w, buffer->h);
  blit(buffer, temp, 0, 0, 0,0, buffer->w,buffer->h);
  save_png(filename, temp, 0);
  destroy_bitmap(temp);
}

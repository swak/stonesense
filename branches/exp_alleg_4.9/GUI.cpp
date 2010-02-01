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

ALLEGRO_COLOR color_segmentoutline = al_map_rgb(17,34,17);

extern ALLEGRO_FONT *font;

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

ALLEGRO_BITMAP* IMGObjectSheet;
ALLEGRO_BITMAP* IMGCreatureSheet; 
ALLEGRO_BITMAP* IMGRampSheet; 
ALLEGRO_BITMAP* IMGFog;
vector<ALLEGRO_BITMAP*> IMGFilelist;
vector<string*> IMGFilenames;

Crd2D debugCursor;

void ScreenToPoint(int x,int y,int &x1, int &y1, int &z1)
{ //assume z of 0
	x-=TILEWIDTH/2;
	y+=TILEWIDTH/2;
	z1 = -3;
	y+= z1*BLOCKHEIGHT/2;
	//y-=BLOCKHEIGHT;
	x+=TILEWIDTH>>1;
	int offx = al_get_bitmap_width(al_get_target_bitmap()) /2;
	int offy = 50-(BLOCKHEIGHT * config.lift_segment_offscreen);
	y-=offy;
	x-=offx;
	y1=y*2-x;
	x1=x*2+y1;
	x1/=TILEWIDTH;
	y1/=TILEWIDTH;

}

void pointToScreen(int *inx, int *iny, int inz){
	int offx = al_get_bitmap_width(al_get_target_bitmap()) / 2;
	static int offy = 50-(BLOCKHEIGHT * config.lift_segment_offscreen);
	int z=inz-1;
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
	return LocalBlockToScreen(x, y, z-1);
}

Crd2D LocalBlockToScreen(int32_t x, int32_t y, int32_t z){
	pointToScreen((int*)&x, (int*)&y, z);
	Crd2D result;
	result.x = x;
	result.y = y;
	return result;
}

void DrawCurrentLevelOutline(ALLEGRO_BITMAP* target, bool backPart){
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
		al_draw_line(p1.x, p1.y, p1.x, p1.y-BLOCKHEIGHT, color_segmentoutline, 0);
		al_draw_line(p1.x, p1.y, p1.x, p1.y-BLOCKHEIGHT, color_segmentoutline, 0);
		al_draw_line(p1.x, p1.y, p2.x, p2.y, color_segmentoutline, 0);
		al_draw_line(p1.x, p1.y-BLOCKHEIGHT, p2.x, p2.y-BLOCKHEIGHT, color_segmentoutline, 0);
		al_draw_line(p2.x, p2.y, p2.x, p2.y-BLOCKHEIGHT, color_segmentoutline, 0);

		al_draw_line(p1.x, p1.y, p3.x, p3.y, color_segmentoutline, 0);
		al_draw_line(p1.x, p1.y-BLOCKHEIGHT, p3.x, p3.y-BLOCKHEIGHT, color_segmentoutline, 0);
		al_draw_line(p3.x, p3.y, p3.x, p3.y-BLOCKHEIGHT, color_segmentoutline, 0);
	}else{
		al_draw_line(p4.x, p4.y, p4.x, p4.y-BLOCKHEIGHT, color_segmentoutline, 0);
		al_draw_line(p4.x, p4.y, p2.x, p2.y, color_segmentoutline ,0);
		al_draw_line(p4.x, p4.y-BLOCKHEIGHT, p2.x, p2.y-BLOCKHEIGHT, color_segmentoutline ,0);

		al_draw_line(p4.x, p4.y, p3.x, p3.y, color_segmentoutline, 0);
		al_draw_line(p4.x, p4.y-BLOCKHEIGHT, p3.x, p3.y-BLOCKHEIGHT, color_segmentoutline, 0);
	}
}

void drawDebugCursorAndInfo(){
	Crd2D point = LocalBlockToScreen(debugCursor.x, debugCursor.y, 0);

	int spriteNum =  SPRITEOBJECT_CURSOR;
	int sheetx = spriteNum % SHEET_OBJECTSWIDE;
	int sheety = spriteNum / SHEET_OBJECTSWIDE;
	al_draw_bitmap_region(IMGObjectSheet, sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT, SPRITEWIDTH, SPRITEHEIGHT, point.x - SPRITEWIDTH/2, point.y - (WALLHEIGHT), 0);

	//get block info
	Block* b = viewedSegment->getBlockLocal( debugCursor.x, debugCursor.y, viewedSegment->sizez-2);
	int i = 10;
	al_draw_textf(font, 2, al_get_bitmap_height(al_get_target_bitmap())-20-(i--*10), 0, "Block 0x%x", b);
	if(!b) return;

	al_draw_textf(font, 2, al_get_bitmap_height(al_get_target_bitmap())-20-(i--*10), 0, 
		"Coord:(%i,%i,%i)", b->x,b->y,b->z);

	int ttype;
	char* tform = NULL;
	if (b->floorType>0)
	{
		ttype=b->floorType;	  
		tform="floor";
	}
	else if (b->wallType > 0)
	{
		ttype=b->wallType;	  
		tform="wall";	 
	}
	else if (b->ramp.type > 0)
	{
		ttype=b->ramp.type;	  
		tform="ramp";	 
	}
	else if (b->stairType > 0)
	{
		ttype=b->stairType;	  
		tform="stair";	 
	}

	if (tform != NULL)
	{
		const char* matName = lookupMaterialTypeName(b->material.type);
		const char* subMatName = lookupMaterialName(b->material.type,b->material.index);
		al_draw_textf(font, 2, config.screenHeight-20-(i--*10), 0, 
			"%s:%i Material:%s%s%s", tform, ttype, 
			matName?matName:"Unknown",subMatName?"/":"",subMatName?subMatName:"");
	} 
	al_draw_textf(font, 2, al_get_bitmap_height(al_get_target_bitmap())-20-(i--*10), 0, 
		"Building Occ: %i Index: %i", b->occ.bits.building, b->building.index);

	if(b->water.index > 0 || b->tree.index != 0)
		al_draw_textf(font, 2, al_get_bitmap_height(al_get_target_bitmap())-20-(i--*10), 0, 
		"tree:%i water:%i", b->tree.index, b->water.index);
	//building
	//if(b->building.info.type != BUILDINGTYPE_NA && b->building.info.type != BUILDINGTYPE_BLACKBOX){
	//	const char* matName = lookupMaterialTypeName(b->building.info.material.type);
	//	const char* subMatName = lookupMaterialName(b->building.info.material.type,b->building.info.material.index);
	//	al_draw_textf(font, 2, config.screenHeight-20-(i--*10), 0, 
	//		"Building: %s(%i) Material: %s%s%s", 
	//		contentLoader.buildingNameStrings.at(b->building.info.type).c_str(),
	//		b->building.info.type, matName?matName:"Unknown",subMatName?"/":"",subMatName?subMatName:"");
	//}
	//creatures
	if(b->creature != null){
		al_draw_textf(font, 2, al_get_bitmap_height(al_get_target_bitmap())-20-(i--*10), 0, 
			"Creature:%s(%i) Job:%s", 
			contentLoader.creatureNameStrings.at(b->creature->type).id, b->creature->type, 
			dfMemoryInfo.getProfession( b->creature->profession ).c_str());

		char strCreature[150] = {0};
		generateCreatureDebugString( b->creature, strCreature );
		//memset(strCreature, -1, 50);
		al_draw_textf(font, 2, al_get_bitmap_height(al_get_target_bitmap())-20-(i--*10), 0, 
			"flag1: %s ", strCreature );
	}
	//basecon
	//textprintf(target, font, 2, config.screenHeight-20-(i--*10), 0xFFFFFF, 
	//   "base: %d %d %d ", b->basetile, b->basecon.type, b->basecon.index );
}

void DrawMinimap(){
	int size = 100;
	//double oneBlockInPixels;
	int posx = al_get_bitmap_width(al_get_target_bitmap())-size-10;
	int posy = 10;

	if(!viewedSegment || viewedSegment->regionSize.x == 0 || viewedSegment->regionSize.y == 0){
		al_draw_textf(font, posx, posy, 0, "No map loaded");
		return;
	}

	oneBlockInPixels = (double) size / viewedSegment->regionSize.x;
	//map outine
	int mapheight = (int)(viewedSegment->regionSize.y * oneBlockInPixels);
	al_draw_rectangle(posx, posy, posx+size, posy+mapheight, al_map_rgb(0,0,0),0);
	//current segment outline
	int x = (size * (viewedSegment->x+1)) / viewedSegment->regionSize.x;
	int y = (mapheight * (viewedSegment->y+1)) / viewedSegment->regionSize.y;
	MiniMapSegmentWidth = (viewedSegment->sizex-2) * oneBlockInPixels;
	MiniMapSegmentHeight = (viewedSegment->sizey-2) * oneBlockInPixels;
	al_draw_rectangle(posx+x, posy+y, posx+x+MiniMapSegmentWidth, posy+y+MiniMapSegmentHeight,al_map_rgb(0,0,0),0);
	MiniMapTopLeftX = posx;
	MiniMapTopLeftY = posy;
	MiniMapBottomRightX = posx+size;
	MiniMapBottomRightY = posy+mapheight;
}

void DrawSpriteFromSheet( int spriteNum, ALLEGRO_BITMAP* spriteSheet, int x, int y){
	int sheetx = spriteNum % SHEET_OBJECTSWIDE;
	int sheety = spriteNum / SHEET_OBJECTSWIDE;
	//
	/*
	static ALLEGRO_BITMAP* tiny = null;
	if(!tiny)
	tiny = create_bitmap_ex(32, 32, 32);

	blit(spriteSheet, tiny, sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT, 0, 0, SPRITEWIDTH, SPRITEHEIGHT);

	blit(tiny, target,
	0,0,
	10, 60 , SPRITEWIDTH, SPRITEHEIGHT);
	*/
	//draw_trans_sprite(target, tiny, x, y);

	al_draw_bitmap_region(spriteSheet, sheetx * SPRITEWIDTH, sheety * SPRITEHEIGHT, SPRITEWIDTH, SPRITEHEIGHT, x, y - (WALLHEIGHT), 0);
}

void DrawSpriteIndexOverlay(int imageIndex){
	ALLEGRO_BITMAP* currentImage;
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
	al_clear_to_color(al_map_rgba(0, 0, 0, 0));
	al_draw_bitmap(currentImage,0,0,0);
	for(int i =0; i<= 20*SPRITEWIDTH; i+=SPRITEWIDTH)
		al_draw_line(i,0,i, al_get_bitmap_height(al_get_target_bitmap()), al_map_rgb(0,0,0), 0);
	for(int i =0; i< al_get_bitmap_height(al_get_target_bitmap()); i+=SPRITEHEIGHT)
		al_draw_line(0,i, 20*SPRITEWIDTH,i,al_map_rgb(0,0,0), 0);

	for(int y = 0; y<20; y++){
		for(int x = 0; x<20; x+=5){
			int index = y * 20 + x;
			al_draw_textf(font,  x*SPRITEWIDTH+5, y* SPRITEHEIGHT+5, 0, "%i", index);
		}
	}	
	al_draw_textf(font, al_get_bitmap_width(al_get_target_bitmap())-10, al_get_bitmap_height(al_get_target_bitmap()) -10, ALLEGRO_ALIGN_RIGHT, 
		"%s (Press SPACE to return)",
		(imageIndex==-1?"objects.png":IMGFilenames[imageIndex]->c_str()));  
}


void DoSpriteIndexOverlay(){
	DrawSpriteIndexOverlay(-1);
	int index = 0;
	int max = (int)IMGFilenames.size();
	//while(true)
	//{
	//	while(!key[ALLEGRO_KEY_SPACE] && !key[ALLEGRO_KEY_F10]) rest(50);
	//	if (key[ALLEGRO_KEY_SPACE])
	//	{
	//		break;
	//	}
	//	DrawSpriteIndexOverlay(index);
	//	index++;
	//	if (index >= max)
	//		index = -1;
	//	//debounce f10
	//	while(key[KEY_F10]) rest(50);
	//}
	//redraw screen again
	paintboard();
}

void paintboard(){
	uint32_t starttime = clock();
	al_set_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, al_map_rgba_f(1.0, 1.0, 1.0, 1.0));
	al_clear_to_color(al_map_rgb(config.backr,config.backg,config.backb));
	//clear_to_color(buffer,makecol(12,7,49)); //this one is calm and nice

	if( viewedSegment == NULL ){
		al_draw_textf(font, al_get_bitmap_width(al_get_target_bitmap())/2, al_get_bitmap_height(al_get_target_bitmap())/2, ALLEGRO_ALIGN_CENTRE, "Could not find DF process");
		return;
	}




	if (config.show_osd) DrawCurrentLevelOutline(al_get_target_bitmap(), true);
	viewedSegment->drawAllBlocks();
	if (config.show_osd) DrawCurrentLevelOutline(al_get_target_bitmap(), false);

	DebugInt1 = viewedSegment->getNumBlocks();

	uint32_t DrawTime = clock() - starttime;

	if (config.show_osd)
	{
		al_draw_textf(font, 10,10, 0, "%i,%i,%i, r%i", DisplayedSegmentX,DisplayedSegmentY,DisplayedSegmentZ, DisplayedRotation);

		if(config.debug_mode){
			al_draw_textf(font, 10, 20, 0, "Timer1: %ims", ClockedTime);
			al_draw_textf(font, 10, 30, 0, "Timer2: %ims", ClockedTime2);
			al_draw_textf(font, 10, 40, 0, "Draw: %ims", DrawTime);
			al_draw_textf(font, 10, 50, 0, "D1: %i", blockFactory.getPoolSize());
			drawDebugCursorAndInfo();
		}

		if(config.follow_DFscreen)
			al_draw_textf(font, al_get_bitmap_width(al_get_target_bitmap())/2,10, ALLEGRO_ALIGN_CENTRE, "Locked on DF screen + (%d,%d,%d)",config.viewXoffset,config.viewYoffset,config.viewZoffset);
		if(config.single_layer_view)
			al_draw_textf(font, al_get_bitmap_width(al_get_target_bitmap())/2,20, ALLEGRO_ALIGN_CENTRE, "Single layer view");
		if(config.automatic_reload_time)
			al_draw_textf(font, al_get_bitmap_width(al_get_target_bitmap())/2,30, ALLEGRO_ALIGN_CENTRE, "Reloading every %0.1fs", (float)config.automatic_reload_time/1000);

		DrawMinimap();
	}
	al_flip_display();
}




ALLEGRO_BITMAP* load_bitmap_withWarning(char* path){
	ALLEGRO_BITMAP* img = 0;
	img = al_load_bitmap(path);
	if(!img){
		al_show_native_message_box("Error", "ERROR", "Unable to load image", NULL, NULL);
		exit(0);
	}
	al_convert_mask_to_alpha(img, al_map_rgb(255, 0, 255));
	return img;
}

void loadGraphicsFromDisk(){
	IMGObjectSheet = load_bitmap_withWarning("objects.png");
	IMGCreatureSheet = load_bitmap_withWarning("creatures.png");
	IMGRampSheet = load_bitmap_withWarning("ramps.png");
	IMGFog = load_bitmap_withWarning("fog.png");
}
void destroyGraphics(){
	/* TODO these should really be merged in with the main imagefile reading routine */
	al_destroy_bitmap(IMGObjectSheet);
	al_destroy_bitmap(IMGCreatureSheet);
	al_destroy_bitmap(IMGRampSheet);
}

//delete and clean out the image files
void flushImgFiles()
{
	//should be OK because we keep others from directly acccessing this stuff
	uint32_t numFiles = (uint32_t)IMGFilelist.size();
	assert( numFiles == IMGFilenames.size());
	for(uint32_t i = 0; i < numFiles; i++)
	{
		al_destroy_bitmap(IMGFilelist[i]);
		//should be same length, I hope
		delete(IMGFilenames[i]);
	}
	IMGFilelist.clear();
	IMGFilenames.clear();
}

ALLEGRO_BITMAP* getImgFile(int index)
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
	LogVerbose("New image: %s\n",filename);
	return (int)IMGFilelist.size() - 1;
}
int loadImgFile(ALLEGRO_PATH* filepath)
{
	char *filename = strcpy(filename,al_path_cstr(filepath, ALLEGRO_NATIVE_PATH_SEP));
	uint32_t numFiles = (uint32_t)IMGFilelist.size();
	for(uint32_t i = 0; i < numFiles; i++)
	{
		if (strcmp(filename, IMGFilenames[i]->c_str()) == 0)
			return i;
	}
	IMGFilelist.push_back(load_bitmap_withWarning(filename));
	IMGFilenames.push_back(new string(filename));
	LogVerbose("New image: %s\n",filename);
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
	ALLEGRO_BITMAP* temp = al_create_bitmap(al_get_bitmap_width(al_get_target_bitmap()), al_get_bitmap_height(al_get_target_bitmap()));
	al_set_target_bitmap(temp);
	paintboard();
	al_save_bitmap(filename, temp);
	al_set_target_bitmap(al_get_backbuffer());
	al_destroy_bitmap(temp);
}

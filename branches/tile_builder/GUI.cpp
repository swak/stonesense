#include <assert.h>
#include <vector>

using namespace std;


#include "common.h"
#include "Block.h"
#include "GUI.h"
#include "WorldSegment.h"
#include "MapLoading.h"
#include "GameBuildings.h"
#include "ContentLoader.h"
#include "BlockFactory.h"
#include "Block.h"

#define color_segmentoutline al_map_rgb(0,0,0)

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
ALLEGRO_BITMAP* IMGStatusSheet; 
ALLEGRO_BITMAP* IMGBloodSheet; 
ALLEGRO_BITMAP* buffer = 0;
ALLEGRO_BITMAP* bigFile = 0;
vector<ALLEGRO_BITMAP*> IMGCache;
vector<ALLEGRO_BITMAP*> IMGFilelist;
vector<string*> IMGFilenames;
GLhandleARB tinter;
GLhandleARB tinter_shader;
Crd3D debugCursor;
void draw_diamond(float x, float y, ALLEGRO_COLOR color)
{
	al_draw_filled_triangle(x, y, x+4, y+4, x-4, y+4, color);
	al_draw_filled_triangle(x+4, y+4, x, y+8, x-4, y+4, color);
}
void draw_borders(float x, float y, uint8_t borders)
{
	if(borders & 1)
		draw_diamond(x, y, al_map_rgb(255,255,255));
	else
		draw_diamond(x, y, al_map_rgb(0,0,0));

	if(borders & 2)
		draw_diamond(x+4, y+4, al_map_rgb(255,255,255));
	else
		draw_diamond(x+4, y+4, al_map_rgb(0,0,0));

	if(borders & 4)
		draw_diamond(x+8, y+8, al_map_rgb(255,255,255));
	else
		draw_diamond(x+8, y+8, al_map_rgb(0,0,0));

	if(borders & 8)
		draw_diamond(x+4, y+12, al_map_rgb(255,255,255));
	else
		draw_diamond(x+4, y+12, al_map_rgb(0,0,0));

	if(borders & 16)
		draw_diamond(x, y+16, al_map_rgb(255,255,255));
	else
		draw_diamond(x, y+16, al_map_rgb(0,0,0));

	if(borders & 32)
		draw_diamond(x-4, y+12, al_map_rgb(255,255,255));
	else
		draw_diamond(x-4, y+12, al_map_rgb(0,0,0));

	if(borders & 64)
		draw_diamond(x-8, y+8, al_map_rgb(255,255,255));
	else
		draw_diamond(x-8, y+8, al_map_rgb(0,0,0));

	if(borders & 128)
		draw_diamond(x-4, y+4, al_map_rgb(255,255,255));
	else
		draw_diamond(x-4, y+4, al_map_rgb(0,0,0));

}

ALLEGRO_COLOR operator*(const ALLEGRO_COLOR &color1, const ALLEGRO_COLOR &color2)
{
	ALLEGRO_COLOR temp;
	temp.r=color1.r*color2.r;
	temp.g=color1.g*color2.g;
	temp.b=color1.b*color2.b;
	temp.a=color1.a*color2.a;
	return temp;
}

ALLEGRO_COLOR operator+(const ALLEGRO_COLOR &color1, const ALLEGRO_COLOR &color2)
{
	ALLEGRO_COLOR temp;
	temp.r=color1.r+(color2.r*(1-color1.r));
	temp.g=color1.g+(color2.g*(1-color1.g));
	temp.b=color1.b+(color2.b*(1-color1.b));
	temp.a=color1.a+(color2.a*(1-color1.a));
	return temp;
}

ALLEGRO_COLOR partialBlend(const ALLEGRO_COLOR & color2, const ALLEGRO_COLOR & color1, int percent)
{
	float blend = percent/100.0;
	ALLEGRO_COLOR result;
	result.r=(blend*color1.r)+((1.0-blend)*color2.r);
	result.g=(blend*color1.g)+((1.0-blend)*color2.g);
	result.b=(blend*color1.b)+((1.0-blend)*color2.b);
	if(color1.a > color2.a)
		result.a = color1.a;
	else result.a = color2.a;
	return result;
}

ALLEGRO_COLOR getDayShade(int hour, int tick)
{
	ALLEGRO_COLOR nightShade = al_map_rgb(158,155,255);
	ALLEGRO_COLOR dawnShade = al_map_rgb(254,172,142);

	if(hour < 6)
		return nightShade;
	else if((hour < 7) && (tick < 25))
		return partialBlend(nightShade, dawnShade, (tick * 4));
	else if(hour < 7)
		return partialBlend(dawnShade, al_map_rgb(255,255,255), ((tick-25) * 4));
	else if((hour > 20) && (hour <= 21) && (tick < 25))
		return partialBlend(al_map_rgb(255,255,255), dawnShade, (tick * 4));
	else if((hour > 20) && (hour <= 21))
		return partialBlend(dawnShade, nightShade, ((tick-25) * 4));
	else if(hour > 21)
		return nightShade;
	return al_map_rgb(255,255,255);
}


void ScreenToPoint(int x,int y,int &x1, int &y1, int &z1)
{ //assume z of 0
	x-=TILEWIDTH/2;
	y+=TILEWIDTH/2;
	z1 = -3;
	y+= z1*BLOCKHEIGHT/2;
	//y-=BLOCKHEIGHT;
	x+=TILEWIDTH>>1;
	int offx = al_get_bitmap_width(al_get_target_bitmap()) /2;
	int offy = (-20)-(BLOCKHEIGHT * config.lift_segment_offscreen);
	y-=offy;
	x-=offx;
	y1=y*2-x;
	x1=x*2+y1;
	x1/=TILEWIDTH;
	y1/=TILEWIDTH;

}

int get_textf_width(const ALLEGRO_FONT *font, const char *format, ...)
{
	ALLEGRO_USTR *buf;
	va_list ap;
	const char *s;

	int width;
	/* Fast path for common case. */
	if (0 == strcmp(format, "%s")) {
		va_start(ap, format);
		s = va_arg(ap, const char *);
		width = al_get_text_width(font, s);
		va_end(ap);
		return width;
	}

	va_start(ap, format);
	buf = al_ustr_new("");
	al_ustr_vappendf(buf, format, ap);
	va_end(ap);

	width = al_get_text_width(font, al_cstr(buf));

	al_ustr_free(buf);
	return width;
}

void draw_textf_border(const ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, int flags, const char *format, ...)
{
	ALLEGRO_USTR *buf;
	va_list arglist;
	const char *s;

	/* Fast path for common case. */
	if (0 == strcmp(format, "%s")) {
		va_start(arglist, format);
		s = va_arg(arglist, const char *);
		al_draw_text(font, al_map_rgb(0, 0, 0), x-1, y-1, flags, s);
		al_draw_text(font, al_map_rgb(0, 0, 0), x-1, y+1, flags, s);
		al_draw_text(font, al_map_rgb(0, 0, 0), x+1, y+1, flags, s);
		al_draw_text(font, al_map_rgb(0, 0, 0), x+1, y-1, flags, s);
		al_draw_text(font, color, x, y, flags, s);
		va_end(arglist);
		return;
	}


	va_start(arglist, format);
	buf = al_ustr_new("");
	al_ustr_vappendf(buf, format, arglist);
	va_end(arglist);
	al_draw_text(font, al_map_rgb(0, 0, 0), x-1, y-1, flags, al_cstr(buf));
	al_draw_text(font, al_map_rgb(0, 0, 0), x-1, y+1, flags, al_cstr(buf));
	al_draw_text(font, al_map_rgb(0, 0, 0), x+1, y+1, flags, al_cstr(buf));
	al_draw_text(font, al_map_rgb(0, 0, 0), x+1, y-1, flags, al_cstr(buf));
	al_draw_text(font, al_map_rgb(0, 0, 0), x-1, y, flags, al_cstr(buf));
	al_draw_text(font, al_map_rgb(0, 0, 0), x, y+1, flags, al_cstr(buf));
	al_draw_text(font, al_map_rgb(0, 0, 0), x+1, y, flags, al_cstr(buf));
	al_draw_text(font, al_map_rgb(0, 0, 0), x, y-1, flags, al_cstr(buf));
	al_draw_text(font, color, x, y, flags, al_cstr(buf));
	al_ustr_free(buf);
}
void draw_text_border(const ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, int flags, const char *ustr)
{
	al_draw_text(font, al_map_rgb(0, 0, 0), x-1, y-1, flags, ustr);
	al_draw_text(font, al_map_rgb(0, 0, 0), x-1, y+1, flags, ustr);
	al_draw_text(font, al_map_rgb(0, 0, 0), x+1, y+1, flags, ustr);
	al_draw_text(font, al_map_rgb(0, 0, 0), x+1, y-1, flags, ustr);

	al_draw_text(font, al_map_rgb(0, 0, 0), x-1, y, flags, ustr);
	al_draw_text(font, al_map_rgb(0, 0, 0), x, y+1, flags, ustr);
	al_draw_text(font, al_map_rgb(0, 0, 0), x+1, y, flags, ustr);
	al_draw_text(font, al_map_rgb(0, 0, 0), x, y-1, flags, ustr);
	al_draw_text(font, color, x, y, flags, ustr);
}
void draw_ustr_border(const ALLEGRO_FONT *font, ALLEGRO_COLOR color, float x, float y, int flags, const ALLEGRO_USTR *ustr)
{
	al_draw_ustr(font, al_map_rgb(0, 0, 0), x-1, y-1, flags, ustr);
	al_draw_ustr(font, al_map_rgb(0, 0, 0), x-1, y+1, flags, ustr);
	al_draw_ustr(font, al_map_rgb(0, 0, 0), x+1, y+1, flags, ustr);
	al_draw_ustr(font, al_map_rgb(0, 0, 0), x+1, y-1, flags, ustr);
	al_draw_ustr(font, al_map_rgb(0, 0, 0), x-1, y, flags, ustr);
	al_draw_ustr(font, al_map_rgb(0, 0, 0), x, y+1, flags, ustr);
	al_draw_ustr(font, al_map_rgb(0, 0, 0), x+1, y, flags, ustr);
	al_draw_ustr(font, al_map_rgb(0, 0, 0), x, y-1, flags, ustr);
	al_draw_ustr(font, color, x, y, flags, ustr);
}
void pointToScreen(int *inx, int *iny, int inz){
	int offx = al_get_bitmap_width(al_get_target_bitmap()) / 2;
	int offy = (-20)-(BLOCKHEIGHT * config.lift_segment_offscreen);
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

void pointToTile(int *inx, int *iny, int inz){
	int offx = al_get_bitmap_width(al_get_target_bitmap()) / 2;
	int offy = 0;
	int z=inz-1;
	int x = *inx-*iny;
	int y = *inx+*iny;
	x = x * (TARGET_TILE_WIDTH / 2) / TARGET_REGION_WIDTH;
	y = y * (TARGET_TILE_HEIGHT / 2) / TARGET_REGION_WIDTH;
	x+=offx;
	y+=offy;
	y-=z * TARGET_Z_HEIGHT;
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

void DrawCurrentLevelOutline(bool backPart){
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

void paintboard(){
	uint32_t starttime = clock();
	//al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_NO_ALPHA);
	//if(!buffer)
	//	buffer = al_create_bitmap(al_get_display_width(al_get_current_display()), al_get_display_height(al_get_current_display()));
	//if(al_get_bitmap_width(buffer) != al_get_display_width(al_get_current_display()) || al_get_bitmap_height(buffer) != al_get_display_height(al_get_current_display()))
	//{
	//	al_destroy_bitmap(buffer);
	//	buffer = al_create_bitmap(al_get_display_width(al_get_current_display()), al_get_display_height(al_get_current_display()));
	//}
	//ALLEGRO_BITMAP * backup = al_get_target_bitmap();
	//al_set_target_bitmap(buffer);
	//al_set_separate_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ONE, ALLEGRO_ONE);
	//al_set_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, al_map_rgba(255, 255, 255, 255));
	if(!config.transparentScreenshots)
		al_clear_to_color(al_map_rgb(config.backr,config.backg,config.backb));
	//clear_to_color(buffer,makecol(12,7,49)); //this one is calm and nice

	if( viewedSegment == NULL ){
		draw_textf_border(font, al_map_rgb(255,255,255), al_get_bitmap_width(al_get_target_bitmap())/2, al_get_bitmap_height(al_get_target_bitmap())/2, ALLEGRO_ALIGN_CENTRE, "Could not find DF process");
		return;
	}



	//viewedSegment->drawAllBlocks();
	viewedSegment->drawTile();
	if (config.show_osd) DrawCurrentLevelOutline(false);

	DebugInt1 = viewedSegment->getNumBlocks();

	uint32_t DrawTime = clock() - starttime;

	//teh drawtime indicator is too jumpy, so I'm averaging it out over 10 frames.
	static uint32_t DrawTimes[10];
	static int ind = 0;
	if(ind >= 10)
		ind = 0;
	DrawTimes[ind] = DrawTime;
	ind++;
	DrawTime = 0;
	for(int i = 0; i<10; i++)
		DrawTime += DrawTimes[i];
	DrawTime = DrawTime / 10;
	if (config.show_osd)
	{
		al_hold_bitmap_drawing(true);
		draw_textf_border(font, al_map_rgb(255,255,255), 10,al_get_font_line_height(font), 0, "%i,%i,%i, r%i", DisplayedSegmentX,DisplayedSegmentY,DisplayedSegmentZ, DisplayedRotation);

		if(config.debug_mode){
			draw_textf_border(font, al_map_rgb(255,255,255), 10, 2*al_get_font_line_height(font), 0, "Timer1: %ims", ClockedTime);
			draw_textf_border(font, al_map_rgb(255,255,255), 10, 3*al_get_font_line_height(font), 0, "Timer2: %ims", ClockedTime2);
			draw_textf_border(font, al_map_rgb(255,255,255), 10, 4*al_get_font_line_height(font), 0, "Draw: %ims", DrawTime);
			draw_textf_border(font, al_map_rgb(255,255,255), 10, 5*al_get_font_line_height(font), 0, "D1: %i", blockFactory.getPoolSize());
			draw_textf_border(font, al_map_rgb(255,255,255), 10, 6*al_get_font_line_height(font), 0, "%i/%i/%i, %i:%i", contentLoader.currentDay+1, contentLoader.currentMonth+1, contentLoader.currentYear, contentLoader.currentHour, (contentLoader.currentTickRel*60)/50);
			draw_textf_border(font, al_map_rgb(255,255,255), 10, 7*al_get_font_line_height(font), 0, "Coords: %i,%i,%i", config.world_offsetX, config.world_offsetY, config.world_offsetZ);
		}
		int top = 0;
		if(config.single_layer_view)
		{
			top += al_get_font_line_height(font);
			draw_textf_border(font, al_map_rgb(255,255,255), al_get_bitmap_width(al_get_target_bitmap())/2,top, ALLEGRO_ALIGN_CENTRE, "Single layer view");
		}
		if(config.automatic_reload_time)
		{
			top += al_get_font_line_height(font);
			draw_textf_border(font, al_map_rgb(255,255,255), al_get_bitmap_width(al_get_target_bitmap())/2,top, ALLEGRO_ALIGN_CENTRE, "Reloading every %0.1fs", (float)config.automatic_reload_time/1000);
		}
		al_hold_bitmap_drawing(false);
	}
	//al_set_target_bitmap(backup);
	//al_draw_bitmap(buffer, 0, 0, 0);
	al_flip_display();
}




ALLEGRO_BITMAP* load_bitmap_withWarning(char* path){
	ALLEGRO_BITMAP* img = 0;
	img = al_load_bitmap(path);
	if(!img){
		DisplayErr("Cannot load image: %s", path);
		exit(0);
	}
	al_convert_mask_to_alpha(img, al_map_rgb(255, 0, 255));
	return img;
}
//delete and clean out the image files
inline int returnGreater(int a, int b)
{
	if(a>b)
		return a;
	else return b;
}

void saveScreenshot(){
	al_clear_to_color(al_map_rgb(config.backr,config.backg,config.backb));
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
	//al_set_new_bitmap_format(ALLEGRO_PIXEL_FORMAT_ANY_NO_ALPHA);
	al_save_bitmap(filename, al_get_target_bitmap());
}
void saveImage(ALLEGRO_BITMAP* image){
	//get filename
	char filename[20] ={0};
	FILE* fp;
	int index = 1;
	//search for the first screenshot# that does not exist already
	while(true){
		sprintf(filename, "Image%i.png", index);

		fp = fopen(filename, "r");
		if( fp != 0)
			fclose(fp);
		else
			//file does not exist, so exit loop
			break;
		index++;
	};
	al_save_bitmap(filename, image);
}

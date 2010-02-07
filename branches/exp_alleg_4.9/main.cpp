#include <assert.h>
#include <vector>


using namespace std;

#include "common.h"
#include "Block.h"
#include "GUI.h"
#include "SpriteMaps.h"
#include "GameBuildings.h"
#include "Constructions.h"
#include "MapLoading.h"
#include "WorldSegment.h"
#include "Creatures.h"
#include "GroundMaterialConfiguration.h"
#include "ContentLoader.h"

#ifdef LINUX_BUILD
#include "stonesense.xpm"
extern void *allegro_icon;
#endif


uint32_t ClockedTime = 0;
uint32_t ClockedTime2 = 0;
uint32_t DebugInt1;

int keyoffset=0;

GameConfiguration config;
bool timeToReloadSegment;
bool timeToReloadConfig;
char currentAnimationFrame;
bool animationFrameShown;

vector<t_matgloss> v_stonetypes;

ALLEGRO_FONT * font;

ALLEGRO_DISPLAY * display;

ALLEGRO_KEYBOARD_STATE * keyboard;

ALLEGRO_TIMER * reloadtimer;

ALLEGRO_EVENT_QUEUE *queue;

ALLEGRO_EVENT event;

int mouse_x, mouse_y, mouse_z;
unsigned int mouse_b;
bool key[ALLEGRO_KEY_MAX];

/*int32_t viewx = 0;
int32_t viewy = 0;
int32_t viewz = 0;
bool followmode = true;*/
volatile int close_button_pressed = FALSE;

void WriteErr(char* msg, ...){
	va_list arglist;
	va_start(arglist, msg);
	//  char buf[200] = {0};
	//  vsprintf(buf, msg, arglist);
	FILE* fp = fopen( "Stonesense.log", "a");
	if(fp)
		vfprintf( fp, msg, arglist );
	va_end(arglist);
	fclose(fp);
}

void LogVerbose(char* msg, ...){
	if (!config.verbose_logging)
		return;
	va_list arglist;
	va_start(arglist, msg);
	//  char buf[200] = {0};
	//  vsprintf(buf, msg, arglist);
	FILE* fp = fopen( "Stonesense.log", "a");
	if(fp)
		vfprintf( fp, msg, arglist );
	va_end(arglist);
	fclose(fp);
}

void correctBlockForSegmetOffset(int32_t& x, int32_t& y, int32_t& z){
	x -= viewedSegment->x;
	y -= viewedSegment->y; //DisplayedSegmentY;
	z -= viewedSegment->z + viewedSegment->sizez - 2; // loading one above the top of the displayed segment for block rules
}

void loadfont()
{
	font = al_load_font("a4_font.tga", 0, 0);
	if (!font) {
		al_show_native_message_box("Error", "Cannot load font:", "a4_font.tga", NULL, 0);
		exit(1);
	}
}

void benchmark(){
	DisplayedSegmentX = DisplayedSegmentY = 0;
	DisplayedSegmentX = 110; DisplayedSegmentY = 110;DisplayedSegmentZ = 18;
	uint32_t startTime = clock();
	int i = 20;
	while(i--)
		reloadDisplayedSegment();

	FILE* fp = fopen("benchmark.txt", "w" );
	if(!fp) return;
	fprintf( fp, "%lims", clock() - startTime);
	fclose(fp);
}

void animUpdateProc()
{
	if (animationFrameShown)
	{
		// check before setting, or threadsafety will be borked
		if (currentAnimationFrame >= (MAX_ANIMFRAME-1)) // ie ends up [0 .. MAX_ANIMFRAME)
			currentAnimationFrame = 0;
		else
			currentAnimationFrame = currentAnimationFrame + 1;
		animationFrameShown = false;
	}
}

int main(void)
{
#ifdef LINUX_BUILD
	allegro_icon = stonesense_xpm;
#endif
	al_init();
	al_init_image_addon();
	al_init_font_addon();
	al_install_keyboard();
	al_install_mouse();
	al_show_mouse_cursor();
	al_set_system_mouse_cursor(ALLEGRO_SYSTEM_MOUSE_CURSOR_DEFAULT);
	WriteErr("\nStonesense launched\n");

	config.debug_mode = false;
	config.hide_outer_blocks = false;
	config.shade_hidden_blocks = true;
	config.load_ground_materials = true;
	config.automatic_reload_time = 0;
	config.automatic_reload_step = 500;
	config.lift_segment_offscreen = 0;
	config.Fullscreen = FULLSCREEN;
	config.screenHeight = RESOLUTION_HEIGHT;
	config.screenWidth = RESOLUTION_WIDTH;
	config.segmentSize.x = DEFAULT_SEGMENTSIZE;
	config.segmentSize.y = DEFAULT_SEGMENTSIZE;
	config.segmentSize.z = DEFAULT_SEGMENTSIZE_Z;
	config.show_creature_names = true;
	config.show_osd = true;
	config.show_intro = true;
	config.track_center = false;
	config.animation_step = 300;
	config.follow_DFscreen = false;
	timeToReloadConfig = true;
	config.fogr = 255;
	config.fogg = 255;
	config.fogb = 255;
	config.foga = 255;
	config.backr = 95;
	config.backg = 95;
	config.backb = 160;
	config.fogenable = true;
	loadfont();

	loadConfigFile();
	//set debug cursor
	debugCursor.x = config.segmentSize.x / 2;
	debugCursor.y = config.segmentSize.y / 2;

	

	int gfxMode = config.Fullscreen ? ALLEGRO_FULLSCREEN : ALLEGRO_WINDOWED;
	al_set_new_display_flags(gfxMode|ALLEGRO_RESIZABLE);
	display = al_create_display(config.screenWidth, config.screenHeight);
	if (!al_install_keyboard()) {
		al_show_native_message_box("Error", "Error", "al_install_keyboard failed.", NULL, ALLEGRO_MESSAGEBOX_ERROR);
		exit(1);
		return 1;
	}
	//al_set_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, al_map_rgba_f(1.0, 1.0, 1.0, 1.0));
	al_set_separate_blender(ALLEGRO_ALPHA, ALLEGRO_INVERSE_ALPHA, ALLEGRO_ALPHA, ALLEGRO_ONE, al_map_rgba(255, 255, 255, 255));
	loadGraphicsFromDisk();
	reloadtimer = al_install_timer(ALLEGRO_MSECS_TO_SECS(config.automatic_reload_time));

	// Start the event queue to handle keyboard input
	queue = al_create_event_queue();
	al_register_event_source(queue, al_get_keyboard_event_source());
	al_register_event_source(queue, al_get_display_event_source(display));
	al_register_event_source(queue, al_get_mouse_event_source());
	al_register_event_source(queue, al_get_timer_event_source(reloadtimer));

	//upper left corners
	DisplayedSegmentX = DisplayedSegmentY = DisplayedSegmentZ = 0;

	//ramps
	//DisplayedSegmentX = 238; DisplayedSegmentY = 220;DisplayedSegmentZ = 23;

	//ford. Main hall
	DisplayedSegmentX = 172; DisplayedSegmentY = 195;DisplayedSegmentZ = 15;

	//ford. desert map
	//sDisplayedSegmentX = 78; DisplayedSegmentY = 123;DisplayedSegmentZ = 15;

	DisplayedSegmentX = 125; DisplayedSegmentY = 125;DisplayedSegmentZ = 18;

	//DisplayedSegmentX = 242; DisplayedSegmentY = 345;DisplayedSegmentZ = 15;


	//while(1)
	//reloadDisplayedSegment();
	//if(!viewedSegment) return 1;

	// we should have a dfhack attached now, load the config
	/*LoadBuildingConfiguration( &buildingTypes );
	LoadCreatureConfiguration( &creatureTypes );
	LoadGroundMaterialConfiguration( );
	*/


	// reload now we have config
	//reloadDisplayedSegment();

#ifdef BENCHMARK
	benchmark();
#endif
	//install_int( animUpdateProc, config.animation_step );
	initAutoReload();


	bool redraw = true;
	while (true) {
		if ((redraw || timeToReloadSegment) && al_event_queue_is_empty(queue)) {
			if( config.show_intro )
			{
				al_clear_to_color(al_map_rgb(0,0,0));
				//centred splash image
				{
					ALLEGRO_BITMAP* SplashImage = load_bitmap_withWarning("splash.png");
					al_draw_bitmap_region(SplashImage, 0, 0,
						al_get_bitmap_width(SplashImage), al_get_bitmap_height(SplashImage),
						(al_get_bitmap_width(al_get_backbuffer()) - al_get_bitmap_width(SplashImage))/2,
						(al_get_bitmap_height(al_get_backbuffer()) - al_get_bitmap_height(SplashImage))/2, 0);
					al_destroy_bitmap(SplashImage);
				}
				int src, dst, alpha_src, alpha_dst;
				ALLEGRO_COLOR color;
				al_get_separate_blender(&src, &dst, &alpha_src, &alpha_dst, &color);
				al_set_separate_blender(src, dst, alpha_src, alpha_dst, al_map_rgb(255, 255, 0));
				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, 50, ALLEGRO_ALIGN_CENTRE, "Welcome to Stonesense!");
				al_set_separate_blender(src, dst, alpha_src, alpha_dst, color);
				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, 60, ALLEGRO_ALIGN_CENTRE, "Stonesense is an isometric viewer for Dwarf Fortress.");

				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, 80, ALLEGRO_ALIGN_CENTRE, "Programming, Jonas Ask and Kris Parker");
				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, 90, ALLEGRO_ALIGN_CENTRE, "Lead graphics designer, Dale Holdampf");

				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, al_get_bitmap_height(al_get_backbuffer())-130, ALLEGRO_ALIGN_CENTRE, "Contributors:");
				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, al_get_bitmap_height(al_get_backbuffer())-120, ALLEGRO_ALIGN_CENTRE, "7c Nickel, Belal, DeKaFu, Dante, Deon, dyze, Errol, fifth angel,");
				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, al_get_bitmap_height(al_get_backbuffer())-110, ALLEGRO_ALIGN_CENTRE, "frumpton, IDreamOfGiniCoeff, Impaler, Japa, jarathor, ");
				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, al_get_bitmap_height(al_get_backbuffer())-100, ALLEGRO_ALIGN_CENTRE, "Jiri Petru, Lord Nightmare, McMe, Mike Mayday, Nexii ");
				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, al_get_bitmap_height(al_get_backbuffer())-90, ALLEGRO_ALIGN_CENTRE, "Malthus, peterix, Seuss, Talvara, winner, and Xandrin.");

				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, al_get_bitmap_height(al_get_backbuffer())-70, ALLEGRO_ALIGN_CENTRE, "With special thanks to peterix for making dfHack");

				//"The program is in a very early alpha, we're only showcasing it to get ideas and feedback, so use it at your own risk."
				al_draw_text(font, al_get_bitmap_width(al_get_backbuffer())/2, al_get_bitmap_height(al_get_backbuffer())-40, ALLEGRO_ALIGN_CENTRE, "Press F9 to continue");

				// Make the backbuffer visible
				al_flip_display();
				doKeys();
			}
			else{
				if( timeToReloadSegment ){
					reloadDisplayedSegment();
					paintboard();
					timeToReloadSegment = false;
					animationFrameShown = true;
				}
				else if (animationFrameShown == false)
				{
					paintboard();
					animationFrameShown = true;
				}
				//doKeys();
			}
			redraw = false;
		}

		al_wait_for_event(queue, &event);
		if (event.type == ALLEGRO_EVENT_DISPLAY_RESIZE) {
			al_acknowledge_resize(event.display.source);
			redraw = true;
			timeToReloadSegment = true;
#if 1
			{
				/* XXX the opengl drivers currently don't resize the backbuffer */
				ALLEGRO_BITMAP *bb = al_get_backbuffer();
				int w = al_get_bitmap_width(bb);
				int h = al_get_bitmap_height(bb);
				printf("backbuffer w, h: %d, %d\n", w, h);
			}
#endif
		}
		if (event.type == ALLEGRO_EVENT_DISPLAY_EXPOSE) {
			timeToReloadSegment = true;
			redraw = true;
		}
		if (event.type == ALLEGRO_EVENT_KEY_DOWN &&
			event.keyboard.keycode == ALLEGRO_KEY_ESCAPE) {
				break;
		}
		if ((event.type == ALLEGRO_EVENT_KEY_DOWN) &&
			(event.keyboard.keycode != ALLEGRO_KEY_ESCAPE)) {
				key[event.keyboard.keycode] = true;
				//doKeys();
				redraw = true;
		}
		if ((event.type == ALLEGRO_EVENT_KEY_UP) &&
			(event.keyboard.keycode != ALLEGRO_KEY_ESCAPE)) {
				key[event.keyboard.keycode] = false;
				doKeys();
				redraw = true;
		}
		if (event.type == ALLEGRO_EVENT_MOUSE_AXES) {
				mouse_x = event.mouse.x;
				mouse_y = event.mouse.y;
				mouse_z = event.mouse.z;
				//doKeys();
				//redraw = true;
		}
		if (event.type == ALLEGRO_EVENT_MOUSE_BUTTON_DOWN || ALLEGRO_EVENT_MOUSE_BUTTON_UP) {
				mouse_x = event.mouse.x;
				mouse_y = event.mouse.y;
				mouse_b = event.mouse.button;
				doKeys();
				redraw = true;
		}
		if (event.type == ALLEGRO_EVENT_DISPLAY_CLOSE) {
			break;
		}
		if (event.type == ALLEGRO_EVENT_TIMER){
			doKeys();
			redraw = true;
			timeToReloadSegment = true;
		}
	}
	destroyGraphics();
	flushImgFiles();
	DisconnectFromDF();

	//dispose old segment
	if(viewedSegment){
		viewedSegment->Dispose();
		delete(viewedSegment);
	}

	return 0;
}
END_OF_MAIN()



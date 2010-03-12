#pragma once



struct t_SpriteWithOffset{
	int32_t sheetIndex;
	int16_t x;
	int16_t y;
	int32_t fileIndex;
	char animFrames;
} ;


typedef struct Crd2D {
	int32_t x,y;
}Crd2D;
typedef struct Crd3D {
	int32_t x,y,z;
}Crd3D;

enum overlayEnum
	{
		none,
		flow_size,
		pile,
		dig,
		detail,
		detail_event,
		hidden,
		geolayer_index,
		light,
		subterranean,
		skyview,
		biome,
		liquid_type,
		water_table,
		rained,
		traffic,
		flow_forbid,
		liquid_static,
		moss,
		feature_present,
		liquid_character
	};


typedef struct {
	bool show_zones;
	bool show_stockpiles;
	bool show_osd;
	bool single_layer_view;
	bool shade_hidden_blocks;
	bool show_hidden_blocks;
	bool show_creature_names;
	bool names_use_nick;
	bool names_use_species;
	bool show_all_creatures;
	bool load_ground_materials;
	bool hide_outer_blocks;
	bool debug_mode;
	bool track_center;
	int lift_segment_offscreen;
	bool truncate_walls;
	bool follow_DFscreen;
	bool verbose_logging;
	int viewXoffset;
	int viewYoffset;
	int viewZoffset;
	int automatic_reload_time;
	int automatic_reload_step;
	int animation_step;
	int fontsize;
	ALLEGRO_PATH * font;
	int screenWidth;
	int screenHeight;
	bool Fullscreen;
	bool show_intro;
	int fogr;
	int fogg;
	int fogb;
	int foga;
	int backr;
	int backg;
	int backb;
	bool fogenable;
	Crd3D segmentSize;
	
	bool follow_DFcursor;
	int dfCursorX;
	int dfCursorY;
	int dfCursorZ;
	unsigned int cellDimX;
	unsigned int cellDimY;
	unsigned int cellDimZ;

	bool saveImageCache;
	int imageCacheSize;
	overlayEnum currentOverlay;
} GameConfiguration;


enum enumCreatureSex{
	eCreatureSex_NA,
	eCreatureSex_Male,
	eCreatureSex_Female
};
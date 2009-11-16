#pragma once



struct t_SpriteWithOffset{
  int32_t sheetIndex;
  int16_t x;
  int16_t y;
  int32_t fileIndex;
} ;


typedef struct Crd2D {
	int32_t x,y;
}Crd2D;
typedef struct Crd3D {
	int32_t x,y,z;
}Crd3D;




typedef struct {
  bool show_zones;
  bool show_stockpiles;
  bool show_osd;
  bool single_layer_view;
  bool shade_hidden_blocks;
  bool show_hidden_blocks;
  bool show_all_creatures;
  bool load_ground_materials;
  bool hide_outer_blocks;
  bool debug_mode;
  bool lift_segment_offscreen;
  bool truncate_walls;
  int automatic_reload_time;
  int automatic_reload_step;
  int animation_step;
  
  int screenWidth;
  int screenHeight;
  bool Fullscreen;

  Crd3D segmentSize;

} GameConfiguration;

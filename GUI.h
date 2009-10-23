#pragma once

#include "common.h"



void pointToScreen(int *inx, int *iny, int inz);
Crd2D WorldBlockToScreen(int32_t x, int32_t y, int32_t z);
Crd2D LocalBlockToScreen(int32_t x, int32_t y, int32_t z);
void DrawMinimap(BITMAP* target);
void paintboard();


BITMAP* load_bitmap_withWarning(char* path);
void loadGraphicsFromDisk();
void destroyGraphics();
void saveScreenshot();

extern WorldSegment* viewedSegment;//current, loaded
extern int DisplayedSegmentX;
extern int DisplayedSegmentY;
extern int DisplayedSegmentZ;


extern BITMAP* IMGFloorSheet; 
extern BITMAP* IMGObjectSheet; 
extern BITMAP* IMGCreatureSheet; 
extern BITMAP* IMGRampSheet; 

extern Crd2D debugCursor;
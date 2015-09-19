# Sprites #
Understanding how Stonesense deals with sprites is central to anyone who wishes to modify the content. The scheme is not very complicated, and this guide will give a short introduction to how they work.

The sprites are stored in four bitmaps; _floors.png, ramps.png, creatures.png_ and _objects.png_. All found in the root of your Stonesense directory.

The way sprites are loaded from these files varies a bit.

## In general ##
Stonesense reads the world as a series of blocks.
These blocks are defined to be 32x32 pixels in size. Hence **every sprite is 32x32 pixels** big. There are no alpha values yet, so partial transparency is not supported. RGB 255,0,255 (Hot pink) is considered fully transparent. Floors are smaller, only 32x16. The sprites are organized into _sheets_.

Here's an example of a typical Stonesense sprite:

![http://stonesense.googlecode.com/svn/wiki/img/tiletree.png](http://stonesense.googlecode.com/svn/wiki/img/tiletree.png)

Note that, in order not to conflict with neighboring sprites, a sprite must actually be within a smaller area than its 32x32 block. A template for the area used by most sprites is:

![http://stonesense.googlecode.com/svn/wiki/img/box.png](http://stonesense.googlecode.com/svn/wiki/img/box.png)

The solid area is the floor space taken up by a sprite, while the dotted box indicates the volume above this area corresponding to one z-level.

### Sprite Index (Sheet Index) ###
Sprite Index is a concept for referring to a specific sprite on a sheet. The index starts with the upper left sprite which has index zero. It then increments to the right. Stonesense is hardcoded to 20 sprites wide sheets, this means that the last sprite to the right in the first row has Sprite Index 19. The first sprite on the second row has index 20. This boundary is hardcoded and changing the size of the sheet will not affect it.

**NOTE: Pressing F10 in Stonesense will display the objects sheet with sprite indexes.**

This image shows how sprites are indexed. Note: Grid added for readability.
![http://stonesense.googlecode.com/svn/wiki/img/sheets_index_objects.png](http://stonesense.googlecode.com/svn/wiki/img/sheets_index_objects.png)


## objects.png ##
This file contains most of the visualizer's artwork, anything that does not fit on the other sheets goes here. Typically buildings, vegetation, walls and liquids. Each sprite occupies a 32x32 pixel area, and the sheet is 20 sprites wide (640 pixels).

## creatures.png ##
Holds all the creatures, and just like the objects sheet, it also wraps its Sprite Index at 20.

## floors.png ##
Holds all the floor sprites. Note that these are **32x16** pixels in size. The Sprite Index for floors is **not** wrapped.

## Ramps.png ##
Holds all the various ramp sprites. Each row of sprites represent a different ramp _material_, eg grass, stone or dirt.
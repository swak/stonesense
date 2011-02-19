#include "common.h"
#include "WorldSegment.h"
#include "GUI.h"
#include "ContentLoader.h"


ALLEGRO_BITMAP * level = 0;
ALLEGRO_BITMAP * fog = 0;

Block* WorldSegment::getBlock(int32_t x, int32_t y, int32_t z){
	if(x < this->x || x >= this->x + this->sizex) return 0;
	if(y < this->y || y >= this->y + this->sizey) return 0;
	if(z < this->z || z >= this->z + this->sizez) return 0;
	/*for(uint32_t i=0; i<this->blocks.size(); i++){
	Block* b = this->blocks[i];
	if(x == b->x && y == b->y && z == b->z)
	return b;
	}
	return 0;*/

	uint32_t lx = x;
	uint32_t ly = y;
	uint32_t lz = z;
	//make local
	lx -= this->x;
	ly -= this->y;
	lz -= this->z;

	correctBlockForRotation( (int32_t&)lx,(int32_t&)ly,(int32_t&)lz );

	uint32_t index = lx + (ly * this->sizex) + ((lz) * this->sizex * this->sizey);
	return blocksAsPointerVolume[index];
}

Block* WorldSegment::getBlockRelativeTo(uint32_t x, uint32_t y, uint32_t z,  dirRelative direction){
	int32_t lx = x;
	int32_t ly = y;
	int32_t lz = z;
	//make local
	lx -= this->x;
	ly -= this->y;
	lz -= this->z;

	correctBlockForRotation( (int32_t&)lx,(int32_t&)ly,(int32_t&)lz );
	switch (direction){
	case eUp:
		ly--; break;
	case eDown:
		ly++; break;
	case eLeft:
		lx--; break;
	case eRight:
		lx++; break;
	case eAbove:
		lz++; break;
	case eBelow:
		lz--; break;
	case eUpLeft:
		ly--; lx--; break;
	case eUpRight:
		ly--; lx++; break;
	case eDownLeft:
		ly++; lx--; break;
	case eDownRight:
		ly++; lx++; break;
	}

	if((int)lx < 0 || lx >= this->sizex) return 0;
	if((int)ly < 0 || ly >= this->sizey) return 0;
	if((int)lz < 0 || lz >= this->sizez) return 0;

	uint32_t index = lx + (ly * this->sizex) + ((lz) * this->sizex * this->sizey);
	return blocksAsPointerVolume[index];
}

Block* WorldSegment::getBlockLocal(uint32_t x, uint32_t y, uint32_t z){
	if((int)x < 0 || x >= (uint32_t)this->sizex) return 0;
	if((int)y < 0 || y >= (uint32_t)this->sizey) return 0;
	if((int)z < 0 || z >= (uint32_t)this->sizez) return 0;

	uint32_t index = x + (y * this->sizex) + ((z) * this->sizex * this->sizey);
	return blocksAsPointerVolume[index];
}

Block* WorldSegment::getBlock(uint32_t index){
	if(index<0 || index >= blocks.size() )
		return 0;
	return blocks[index];
}

void correctBlockForRotation(int32_t& x, int32_t& y, int32_t& z){
	int32_t oldx = x;
	int32_t oldy = y;
	int w = config.segmentSize.x;
	int h = config.segmentSize.y;

	if(DisplayedRotation == 1){
		x = h - oldy -1;
		y = oldx;
	}
	if(DisplayedRotation == 2){
		x = w - oldx -1;
		y = h - oldy -1;
	}
	if(DisplayedRotation == 3){
		x = oldy;
		y = w - oldx -1;
	}
}

void WorldSegment::addBlock(Block* b){
	this->blocks.push_back(b);
	//b = &(this->blocks[ blocks.size() - 1]);

	uint32_t x = b->x;
	uint32_t y = b->y;
	uint32_t z = b->z;
	//make local
	x -= this->x;
	y -= this->y;
	z -= this->z;

	//rotate
	correctBlockForRotation( (int32_t&)x, (int32_t&)y, (int32_t&)z );

	uint32_t index = x + (y * this->sizex) + ((z) * this->sizex * this->sizey);
	//assert( x < sizex && x >=0);
	//assert( y < sizey && y >=0);
	//assert( z < sizez && z >=0);
	//assure not overwriting
	//if(blocksAsPointerVolume[index] != 0)
	//  assert(blocksAsPointerVolume[index] == 0);

	blocksAsPointerVolume[index] = b;
	/*Block* test = getBlock(b->x, b->y, b->z);
	if(test->x != b->x || test->y != b->y || test->z != b->z){
	test = getBlock(b->x, b->y, b->z);
	assert (test == b);
	}*/

}

void WorldSegment::drawTile(){

	// x,y,z print pricess
	ALLEGRO_BITMAP * temp = al_get_target_bitmap();
	int32_t vsxmax = viewedSegment->sizex-1;
	int32_t vsymax = viewedSegment->sizey-1;
	int32_t vszmax = viewedSegment->sizez-1; // grabbing one tile +z more than we should for tile rules
	//al_hold_bitmap_drawing(true);
	int op, src, dst, alpha_op, alpha_src, alpha_dst;
	ALLEGRO_COLOR color;
	al_get_separate_blender(&op, &src, &dst, &alpha_op, &alpha_src, &alpha_dst);

	int size = al_get_bitmap_height(al_get_target_bitmap()) * 
		al_get_bitmap_width(al_get_target_bitmap());
	if(!(mask_size == size))
	{
		delete [mask_size] bitmask;
		bitmask = new bool[size];
		mask_size = size;
	}
	for(int i = 0; i < mask_size; i++)
		bitmask[i] = 0;
	al_lock_bitmap(al_get_target_bitmap(),al_get_bitmap_format(al_get_target_bitmap()), ALLEGRO_LOCK_WRITEONLY);
	for(int32_t vsz=0; vsz < vszmax; vsz++)
	{
		for(int32_t vsx=1; vsx < vsxmax; vsx++)
		{
			for(int32_t vsy=1; vsy < vsymax; vsy++)
			{
				Block *b = getBlockLocal(vsx,vsy,vsz);
				if (b)
				{
					b->Draw_pixel(bitmask);
					//while(!key[KEY_SPACE]) ;
					//rest(100);
				}
			}
		}
	}
	al_unlock_bitmap(al_get_target_bitmap());
}
bool WorldSegment::CoordinateInsideSegment(uint32_t x, uint32_t y, uint32_t z){
	/*if( x < 0 || (int32_t)x >= this->regionSize.x) return false;
	if( y < 0 || (int32_t)y >= this->regionSize.y) return false;
	if( z < 0 || (int32_t)z >= this->regionSize.z) return false;*/
	if( (int32_t)x < this->x || (int32_t)x >= this->x + this->sizex) return false;
	if( (int32_t)y < this->y || (int32_t)y >= this->y + this->sizey) return false;
	if( (int32_t)z < this->z || (int32_t)z >= this->z + this->sizez) return false;
	return true;
}

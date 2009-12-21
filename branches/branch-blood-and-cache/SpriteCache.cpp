#include "common.h"
#include "SpriteCache.h"

bool t_cacheKey::operator< ( const t_cacheKey &B ) const
{
	if( sprite.fileIndex < B.sprite.fileIndex )
		return true;
	else if(sprite.sheetIndex < B.sprite.sheetIndex && sprite.fileIndex == B.sprite.fileIndex )
		return true;
	else if( occupancy < B.occupancy && sprite.fileIndex == B.sprite.fileIndex && sprite.sheetIndex == B.sprite.sheetIndex )
		return true;
	else return false;
}

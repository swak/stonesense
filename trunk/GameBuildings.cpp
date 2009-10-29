#include "common.h"
#include "WorldSegment.h"
#include "SpriteMaps.h"
#include "GameBuildings.h"
#include "BuildingConfiguration.h"

vector<BuildingConfiguration> buildingTypes;
vector <string> v_buildingtypes;//should be empty for all buildings


void loadBuildingSprites( Block* b );

int BlockNeighbourhoodType_simple(WorldSegment* segment, Block* b, bool validationFuctionProc(Block*) ){
  uint32_t x,y,z;
  x = b->x; y = b->y; z = b->z;

	bool n = validationFuctionProc( segment->getBlock( x, y-1, z) );
	bool s = validationFuctionProc( segment->getBlock( x, y+1, z) );
	bool e = validationFuctionProc( segment->getBlock( x+1, y, z) );
	bool w = validationFuctionProc( segment->getBlock( x-1, y, z) );
	/*bool nw = validationFuctionProc( segment->getBlock(, x-1, y-1, z) );
	bool ne = validationFuctionProc( segment->getBlock(, x+1, y-1, z) );
	bool SW = validationFuctionProc( segment->getBlock(, x-1, y+1, z) );
	bool se = validationFuctionProc( segment->getBlock(, x+1, y+1, z) );*/
  
  if(!n && !s && !w && !e) return eSimpleSingle;
  if( n && !s && !w && !e) return eSimpleN;
  if(!n && !s &&  w && !e) return eSimpleW;
  if(!n &&  s && !w && !e) return eSimpleS;
  if(!n && !s && !w &&  e) return eSimpleE;
  
  if( n &&  s && !w && !e) return eSimpleNnS;
  if(!n && !s &&  w &&  e) return eSimpleWnE;

  if( n && !s &&  w && !e) return eSimpleNnW;
  if(!n &&  s &&  w && !e) return eSimpleSnW;
  if(!n &&  s && !w &&  e) return eSimpleSnE;
  if( n && !s && !w &&  e) return eSimpleNnE;
  
  //....

  return eSimpleSingle;
}

bool blockHasBridge(Block* b){
  if(!b) return 0;
  return b->building.info.type == BUILDINGTYPE_BRIDGE;
}

dirTypes findWallCloseTo(WorldSegment* segment, Block* b){
  uint32_t x,y,z;
  x = b->x; y = b->y; z = b->z;
  bool n = hasWall( segment->getBlock( x, y-1, z) );
	bool s = hasWall( segment->getBlock( x, y+1, z) );
	bool e = hasWall( segment->getBlock( x+1, y, z) );
	bool w = hasWall( segment->getBlock( x-1, y, z) );
  
  if(w) return eSimpleW;
  if(n) return eSimpleN;
  if(s) return eSimpleS;
  if(e) return eSimpleE;
  
  return eSimpleSingle;
}

void ReadBuildings(DFHackAPI& DF, vector<t_building>* buildingHolder){
  if(!buildingHolder) return;

  v_buildingtypes.clear();
	uint32_t numbuildings = DF.InitReadBuildings(v_buildingtypes);
	t_building tempbuilding;

  if( !BuildingNamesTranslatedFromGame )
    TranslateBuildingNames();

	uint32_t index = 0;
	while(index < numbuildings){
		DF.ReadBuilding(index, tempbuilding);
    buildingHolder->push_back(tempbuilding);
    index++;
  }
}


void MergeBuildingsToSegment(vector<t_building>* buildings, WorldSegment* segment){
	t_building tempbuilding;

	uint32_t index = 0;
  uint32_t numBuildings = (uint32_t)buildings->size();
	for(uint32_t i=0; i < numBuildings; i++){
    tempbuilding = (*buildings)[i];
		
		//int bheight = tempbuilding.y2 - tempbuilding.y1;
		for(uint32_t yy = tempbuilding.y1; yy <= tempbuilding.y2; yy++)
		for(uint32_t xx = tempbuilding.x1; xx <= tempbuilding.x2; xx++){
			Block* b;
      //want hashtable :(
			if( b = segment->getBlock( xx, yy, tempbuilding.z) ){
        //handle special case where zones and stockpiles overlap buildings, and try to replace them
        if(b->building.info.type != BUILDINGTYPE_NA && tempbuilding.type == BUILDINGTYPE_ZONE )
          continue;
        if(b->building.info.type != BUILDINGTYPE_NA && tempbuilding.type == BUILDINGTYPE_STOCKPILE )
          continue; 

				b->building.info = tempbuilding;
      }
    }
		index++;
	}

  //all blocks in the segment now have their building info loaded.
  //now set their sprites
  for(uint32_t i=0; i < segment->getNumBlocks(); i++){
    Block* b = segment->getBlock( i );
    if( b->building.info.type != BUILDINGTYPE_NA && b->building.info.type != BUILDINGTYPE_BLACKBOX )
      loadBuildingSprites( b );
  }

	
}


void loadBuildingSprites ( Block* b){
  uint32_t i,j;
  bool foundBlockBuildingInfo = false;
  uint32_t numBuildings = (uint32_t)buildingTypes.size();
  for(i = 0; i < numBuildings; i++){
    BuildingConfiguration& conf = buildingTypes[i];
    if(b->building.info.type != conf.gameID) continue;

    //check all sprites for one that matches all conditions
    uint32_t numSprites = (uint32_t)conf.sprites.size();
    for(j = 0; j < numSprites; j++){
      if(conf.sprites[j].BlockMatches(b)){
        b->building.sprites = conf.sprites[j].sprites;
        foundBlockBuildingInfo = true;
        break;
      }
    }
    break;
  }
  //add yellow box, if needed. But only if the building was not found (this way we can have blank slots in buildings)
  if(b->building.sprites.size() == 0 && foundBlockBuildingInfo == false){
    t_SpriteWithOffset unknownBuildingSprite = {SPRITEOBJECT_NA, 0, 0};
    b->building.sprites.push_back( unknownBuildingSprite );
  }
}

/*TODO: this function takes a massive amount of work, looping all buildings for every block*/
bool BlockHasSuspendedBuilding(vector<t_building>* buildingList, Block* b){
  uint32_t num = (uint32_t)buildingList->size();
  for(uint32_t i=0; i < num; i++){
    t_building* building = &(*buildingList)[i];

    //boundry check
    if(b->z != building->z) continue;
    if(b->x < building->x1  ||   b->x > building->x2) continue;
    if(b->y < building->y1  ||   b->y > building->y2) continue;

    if(building->type == BUILDINGTYPE_BRIDGE){
        return true;
    }
    if(building->type == BUILDINGTYPE_ZONE)
      return true;
  }
  return false;
}
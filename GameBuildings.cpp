#include "common.h"
#include "WorldSegment.h"
#include "SpriteMaps.h"
#include "GameBuildings.h"
#include "BuildingConfiguration.h"
#include "ContentLoader.h"
#include "GUI.h"

//vector<BuildingConfiguration> buildingTypes;
//vector <string> v_buildingtypes;//should be empty for all buildings
/*
int BlockNeighbourhoodType_simple(WorldSegment* segment, Block* b, bool validationFuctionProc(Block*) ){
uint32_t x,y,z;
x = b->x; y = b->y; z = b->z;

bool n = validationFuctionProc( segment->getBlock( x, y-1, z, true) );
bool s = validationFuctionProc( segment->getBlock( x, y+1, z, true) );
bool e = validationFuctionProc( segment->getBlock( x+1, y, z, true) );
bool w = validationFuctionProc( segment->getBlock( x-1, y, z, true) );
//bool nw = validationFuctionProc( segment->getBlock(, x-1, y-1, z) );
//bool ne = validationFuctionProc( segment->getBlock(, x+1, y-1, z) );
//bool SW = validationFuctionProc( segment->getBlock(, x-1, y+1, z) );
//bool se = validationFuctionProc( segment->getBlock(, x+1, y+1, z) );

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
}*/

bool blockHasBridge(Block* b){
	if(!b) return 0;
	return b->building.info.type == BUILDINGTYPE_BRIDGE;
}

dirTypes findWallCloseTo(WorldSegment* segment, Block* b){
	uint32_t x,y,z;
	x = b->x; y = b->y; z = b->z;
	bool n = hasWall( segment->getBlockRelativeTo( x, y, z, eUp) );
	bool s = hasWall( segment->getBlockRelativeTo( x, y, z, eDown) );
	bool w = hasWall( segment->getBlockRelativeTo( x, y, z, eLeft) );
	bool e = hasWall( segment->getBlockRelativeTo( x, y, z, eRight) );

	if(w) return eSimpleW;
	if(n) return eSimpleN;
	if(s) return eSimpleS;
	if(e) return eSimpleE;

	return eSimpleSingle;
}

void ReadBuildings(DFHack::Context& DF, vector<t_building>* buildingHolder)
{
	if(config.skipBuildings)
		return;
	if(!buildingHolder) return;

	DFHack::Buildings * Bld;
	try
	{
		Bld = DF.getBuildings();
	}
	catch (exception &e)
	{
		WriteErr("DFhack exeption: %s\n", e.what());
		config.skipBuildings = true;
		return;
	}

	vector<string> dummy;
	uint32_t numbuildings = 0;
	try
	{
		if (!Bld->Start(numbuildings)) return;
	}
	catch (exception &e)
	{
		WriteErr("DFhack exeption: %s\n", e.what());
		config.skipBuildings = true;
		return;
	}
	t_building tempbuilding;

	uint32_t index = 0;
	while(index < numbuildings){
		try
		{
			Bld->Read(index, tempbuilding);
		}
		catch (exception &e)
		{
			WriteErr("DFhack exeption: %s\n", e.what());
			config.skipBuildings = true;
			return;
		}
		buildingHolder->push_back(tempbuilding);
		index++;
	}
	Bld->Finish();
}


void MergeBuildingsToSegment(vector<t_building>* buildings, WorldSegment* segment){
	t_building tempbuilding;
	uint32_t numBuildings = (uint32_t)buildings->size();
	for(uint32_t i=0; i < numBuildings; i++){
		tempbuilding = (*buildings)[i];

		//int bheight = tempbuilding.y2 - tempbuilding.y1;
		for(uint32_t yy = tempbuilding.y1; yy <= tempbuilding.y2; yy++)
			for(uint32_t xx = tempbuilding.x1; xx <= tempbuilding.x2; xx++){
				Block* b;
				bool inside = segment->CoordinateInsideSegment(xx,yy, tempbuilding.z);
				if(inside){
					//want hashtable :(
					// still need to test for b, because of ramp/building overlap
					b = segment->getBlock( xx, yy, tempbuilding.z);

					if(!b){
						//inside segment, but no block to represent it
						b = new Block(segment);
						b->x = xx;
						b->y = yy;
						b->z = tempbuilding.z;
						segment->addBlock( b );
					}

					if( b ){
						//handle special case where zones and stockpiles overlap buildings, and try to replace them
						if(b->building.info.type != BUILDINGTYPE_NA && tempbuilding.type == TranslateBuildingName("building_civzonest", contentLoader.classIdStrings ) )
							continue;
						if(b->building.info.type != BUILDINGTYPE_NA && tempbuilding.type == TranslateBuildingName("building_stockpilest", contentLoader.classIdStrings ) )
							continue; 
						b->building.index = i;
						b->building.info = tempbuilding;
						b->building.custom_building_type = contentLoader.Bld->GetCustomWorkshopType(tempbuilding);
					}
				}
			}
	}

	//all blocks in the segment now have their building info loaded.
	//now set their sprites
	/*
	for(uint32_t i=0; i < segment->getNumBlocks(); i++){
	Block* b = segment->getBlock( i );
	if( b->building.info.type != BUILDINGTYPE_NA && b->building.info.type != BUILDINGTYPE_BLACKBOX )
	loadBuildingSprites( b );
	}
	*/

}


void loadBuildingSprites ( Block* b){
	bool foundBlockBuildingInfo = false;
	if (b == NULL)
	{
		WriteErr("Null Block skipped in loadBuildingSprites\n");
		return;
	}
	if(b->building.custom_building_type == -1)
	{
		uint32_t numBuildings = (uint32_t)contentLoader.buildingConfigs.size();
		for(uint32_t i = 0; i < numBuildings; i++){
			BuildingConfiguration& conf = contentLoader.buildingConfigs[i];
			if(b->building.info.type != conf.gameID) continue;

			//check all sprites for one that matches all conditions
			if (conf.sprites != NULL && conf.sprites->BlockMatches(b))
			{
				foundBlockBuildingInfo = true;
			}
			break;
		}
	}
	else
	{
		uint32_t numCustBuildings = (uint32_t)contentLoader.customBuildingConfigs.size();
		for(uint32_t i = 0; i < numCustBuildings; i++){
			BuildingConfiguration& cust = contentLoader.customBuildingConfigs[i];
			if(b->building.custom_building_type != cust.gameID) continue;

			//check all sprites for one that matches all conditions
			if (cust.sprites != NULL && cust.sprites->BlockMatches(b))
			{
				foundBlockBuildingInfo = true;
			}
			break;
		}
	}

	//add yellow box, if needed. But only if the building was not found (this way we can have blank slots in buildings)
	if(b->building.sprites.size() == 0 && foundBlockBuildingInfo == false){
		c_sprite unknownBuildingSprite;
		unknownBuildingSprite.set_defaultsheet(IMGObjectSheet);
		unknownBuildingSprite.set_fileindex(-1);
		unknownBuildingSprite.set_sheetindex(0);
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

		if(building->type == TranslateBuildingName("building_bridgest", contentLoader.classIdStrings )){
			return true;
		}
		if(building->type == TranslateBuildingName("building_civzonest", contentLoader.classIdStrings ))
			return true;
	}
	return false;
}

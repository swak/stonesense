#include "common.h"
#include "WorldSegment.h"
#include "GameBuildings.h"
#include "ContentLoader.h"

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
						b->building.index = i;
						b->building.info = tempbuilding;
					}
				}
			}
	}
}
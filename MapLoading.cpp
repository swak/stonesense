#include "common.h"
#include "GUI.h"
#include "MapLoading.h"
#include "WorldSegment.h"
#include "SpriteMaps.h"
#include "Constructions.h"
#include "GameBuildings.h"
#include "Creatures.h"
#include "ContentLoader.h"

static DFHack::Context* pDFApiHandle = 0;
static DFHack::ContextManager* DFMgr = 0;
const VersionInfo *dfMemoryInfo;
bool memInfoHasBeenRead;
bool connected = 0;
bool threadrunnng = 0;
struct segParams
{
	int x;
	int y;
	int z;
	int sizex;
	int sizey;
	int sizez;
} parms;

inline bool IDisWall(int in){
	//if not a custom type, do a lookup in dfHack's interface
	return isWallTerrain( in );
}

inline bool IDisFloor(int in){
	//if not a custom type, do a lookup in dfHack's interface
	return isFloorTerrain( in );;
}

//big look up table
char rampblut[] =
// generated by blutmaker.py
{
	1 ,  2 ,  8 ,  2 ,  4 , 12 ,  4 , 12 ,  9 ,  2 , 21 ,  2 ,  4 , 12 ,  4 , 12 ,
	5 , 16 ,  5 , 16 , 13 , 13 , 13 , 12 ,  5 , 16 ,  5 , 16 , 13 , 13 , 13 , 16 ,
	7 ,  2 , 14 ,  2 ,  4 , 12 ,  4 , 12 , 20 , 26 , 25 , 26 ,  4 , 12 ,  4 , 12 ,
	5 , 16 ,  5 , 16 , 13 , 16 , 13 , 16 ,  5 , 16 ,  5 , 16 , 13 , 16 , 13 , 16 ,
	3 , 10 ,  3 , 10 , 17 , 12 , 17 , 12 ,  3 , 10 , 26 , 10 , 17 , 17 , 17 , 17 ,
	11 , 10 , 11 , 16 , 11 , 26 , 17 , 12 , 11 , 16 , 11 , 16 , 13 , 13 , 17 , 16 ,
	3 , 10 ,  3 , 10 , 17 , 17 , 17 , 17 ,  3 , 10 , 26 , 10 , 17 , 17 , 17 , 17 ,
	11 , 11 , 11 , 16 , 11 , 11 , 17 , 14 , 11 , 16 , 11 , 16 , 17 , 17 , 17 , 13 ,
	6 ,  2 , 19 ,  2 ,  4 , 12 ,  4 , 12 , 15 ,  2 , 24 ,  2 ,  4 , 12 ,  4 , 12 ,
	5 , 16 , 26 , 16 , 13 , 16 , 13 , 16 ,  5 , 16 , 26 , 16 , 13 , 16 , 13 , 16 ,
	18 ,  2 , 22 ,  2 , 26 , 12 , 26 , 12 , 23 , 26 , 26 , 26 , 26 , 12 , 26 , 12 ,
	5 , 16 , 26 , 16 , 13 , 16 , 13 , 16 ,  5 , 16 , 26 , 16 , 13 , 16 , 13 , 16 ,
	3 , 10 ,  3 , 10 , 17 , 10 , 17 , 17 ,  3 , 10 , 26 , 10 , 17 , 17 , 17 , 17 ,
	11 , 10 , 11 , 16 , 17 , 10 , 17 , 17 , 11 , 16 , 11 , 16 , 17 , 15 , 17 , 12 ,
	3 , 10 ,  3 , 10 , 17 , 17 , 17 , 17 ,  3 , 10 , 26 , 10 , 17 , 17 , 17 , 17 ,
	11 , 16 , 11 , 16 , 17 , 16 , 17 , 10 , 11 , 16 , 11 , 16 , 17 , 11 , 17 , 26
};

inline bool isBlockHighRampEnd(uint32_t x, uint32_t y, uint32_t z, WorldSegment* segment, dirRelative dir)
{
	Block* block = segment->getBlockRelativeTo( x, y, z, dir);
	if(!block) return false;
	if(block->wallType == 0) return false;
	return IDisWall( block->wallType );
}

inline int blockWaterDepth(uint32_t x, uint32_t y, uint32_t z, WorldSegment* segment, dirRelative dir)
{
	Block* block = segment->getBlockRelativeTo( x, y, z, dir);
	if(!block) return false;
	if(block->water.index == 0 || block->water.type == 1) return false;
	return block->water.index;
}

inline bool isBlockHighRampTop(uint32_t x, uint32_t y, uint32_t z, WorldSegment* segment, dirRelative dir)
{
	Block* block = segment->getBlockRelativeTo( x, y, z, dir);
	if(!block) return false;
	if(block->floorType == 0 && block->ramp.type == 0 && block->stairType == 0) return false;
	if(block->wallType == 0) return true;
	return !IDisWall( block->wallType );
}

int CalculateRampType(uint32_t x, uint32_t y, uint32_t z, WorldSegment* segment){
	int ramplookup = 0;
	if (isBlockHighRampEnd(x, y, z, segment, eUp) && isBlockHighRampTop(x, y, z+1, segment, eUp))
		ramplookup ^= 1;
	if (isBlockHighRampEnd(x, y, z, segment, eUpRight) && isBlockHighRampTop(x, y, z+1, segment, eUpRight))
		ramplookup ^= 2;
	if (isBlockHighRampEnd(x, y, z, segment, eRight) && isBlockHighRampTop(x, y, z+1, segment, eRight))
		ramplookup ^= 4;
	if (isBlockHighRampEnd(x, y, z, segment, eDownRight) && isBlockHighRampTop(x, y, z+1, segment, eDownRight))
		ramplookup ^= 8;
	if (isBlockHighRampEnd(x, y, z, segment, eDown) && isBlockHighRampTop(x, y, z+1, segment, eDown))
		ramplookup ^= 16;
	if (isBlockHighRampEnd(x, y, z, segment, eDownLeft) && isBlockHighRampTop(x, y, z+1, segment, eDownLeft))
		ramplookup ^= 32;
	if (isBlockHighRampEnd(x, y, z, segment, eLeft) && isBlockHighRampTop(x, y, z+1, segment, eLeft))
		ramplookup ^= 64;
	if (isBlockHighRampEnd(x, y, z, segment, eUpLeft) && isBlockHighRampTop(x, y, z+1, segment, eUpLeft))
		ramplookup ^= 128;

	// creation should ensure in range
	if (ramplookup > 0)
	{
		return rampblut[ramplookup];
	}

	if (isBlockHighRampEnd(x, y, z, segment, eUp))
		ramplookup ^= 1;
	if (isBlockHighRampEnd(x, y, z, segment, eUpRight))
		ramplookup ^= 2;
	if (isBlockHighRampEnd(x, y, z, segment, eRight))
		ramplookup ^= 4;
	if (isBlockHighRampEnd(x, y, z, segment, eDownRight))
		ramplookup ^= 8;
	if (isBlockHighRampEnd(x, y, z, segment, eDown))
		ramplookup ^= 16;
	if (isBlockHighRampEnd(x, y, z, segment, eDownLeft))
		ramplookup ^= 32;
	if (isBlockHighRampEnd(x, y, z, segment, eLeft))
		ramplookup ^= 64;
	if (isBlockHighRampEnd(x, y, z, segment, eUpLeft))
		ramplookup ^= 128;

	// creation should ensure in range
	return rampblut[ramplookup];
}

bool isBlockOnVisibleEdgeOfSegment(WorldSegment* segment, Block* b)
{
	if(b->z == segment->z + segment->sizez - 2) 
		return true;

	if (DisplayedRotation == 0 && 
		(
		b->x == segment->x + segment->sizex - 2
		|| b->y == segment->y + segment->sizey - 2
		|| b->x == segment->regionSize.x - 1
		|| b->y == segment->regionSize.y - 1
		))
	{
		return true;
	}
	else if (DisplayedRotation == 1 &&
		(
		b->x == segment->x + segment->sizex - 2
		|| b->y == segment->y + 1
		|| b->x == segment->regionSize.x - 1
		|| b->y == 0
		))
	{
		return true;
	}
	else if (DisplayedRotation == 2 &&
		(
		b->x == segment->x + 1
		|| b->y == segment->y + 1
		|| b->x == 0
		|| b->y == 0
		))
	{
		return true;
	}
	else if (DisplayedRotation == 3 && 
		(
		b->x == segment->x + 1
		|| b->y == segment->y + segment->sizey - 2
		|| b->x == 0
		|| b->y == segment->regionSize.y - 1
		))
	{
		return true;
	}

	return false;
}

bool areNeighborsVisible(t_designation designations[16][16],int  x,int y)
{
	if(designations[x-1][y-1].bits.hidden == false)
		return true;
	if(designations[x-1][y].bits.hidden == false)
		return true;
	if(designations[x-1][y+1].bits.hidden == false)
		return true;
	if(designations[x][y-1].bits.hidden == false)
		return true;
	if(designations[x][y+1].bits.hidden == false)
		return true;
	if(designations[x+1][y-1].bits.hidden == false)
		return true;
	if(designations[x+1][y].bits.hidden == false)
		return true;
	if(designations[x+1][y+1].bits.hidden == false)
		return true;
	return false;
}

void ReadCellToSegment(DFHack::Context& DF, WorldSegment& segment, int CellX, int CellY, int CellZ,
					   uint32_t BoundrySX, uint32_t BoundrySY,
					   uint32_t BoundryEX, uint32_t BoundryEY, 
					   uint16_t Flags/*not in use*/, 
					   vector<t_building>* allBuildings, 
					   vector<t_construction>* allConstructions,
					   vector< vector <uint16_t> >* allLayers,
					   vector<DFHack::t_feature> * global_features,
					   std::map <DFHack::planecoord, std::vector<DFHack::t_feature *> > *local_features,
					   DFHack::Maps *Maps)
{
	if(config.skipMaps)
		return;

	//boundry check
	int celldimX, celldimY, celldimZ;
	Maps->getSize((unsigned int &)celldimX, (unsigned int &)celldimY, (unsigned int &)celldimZ);
	if( CellX < 0 || CellX >= celldimX ||
		CellY < 0 || CellY >= celldimY ||
		CellZ < 0 || CellZ >= celldimZ ) 
		return;
	if(!(DF.isSuspended()))
		DisplayErr("DF isn't suspended! something is very very wrong!");
	if(!Maps->isValidBlock(CellX, CellY, CellZ))
		return;


	//make boundries local
	BoundrySX -= CellX * CELLEDGESIZE;
	BoundryEX -= CellX * CELLEDGESIZE;
	BoundrySY -= CellY * CELLEDGESIZE;
	BoundryEY -= CellY * CELLEDGESIZE;


	//read cell data
	uint16_t tiletypes[16][16];
	t_designation designations[16][16];
	t_occupancy occupancies[16][16];
	uint8_t regionoffsets[16];
	t_temperatures temp1, temp2;
	DFHack::mapblock40d mapBlock;
	Maps->ReadTileTypes(CellX, CellY, CellZ, (tiletypes40d *) tiletypes);
	Maps->ReadDesignations(CellX, CellY, CellZ, (designations40d *) designations);
	Maps->ReadOccupancy(CellX, CellY, CellZ, (occupancies40d *) occupancies);
	Maps->ReadRegionOffsets(CellX,CellY,CellZ, (biome_indices40d *)regionoffsets);
	Maps->ReadTemperatures(CellX, CellY, CellZ, &temp1, &temp2);
	Maps->ReadBlock40d(CellX, CellY, CellZ, &mapBlock);
	//read local vein data
	vector <t_vein> veins;
	vector <t_frozenliquidvein> ices;
	vector <t_spattervein> splatter;

	Maps->ReadVeins(CellX,CellY,CellZ,&veins,&ices,&splatter);
	uint32_t numVeins = (uint32_t)veins.size();

	//parse cell
	for(uint32_t ly = BoundrySY; ly <= BoundryEY; ly++){
		for(uint32_t lx = BoundrySX; lx <= BoundryEX; lx++){
			uint32_t gx = lx + (CellX * CELLEDGESIZE);
			uint32_t gy = ly + (CellY * CELLEDGESIZE);
			if( !segment.CoordinateInsideSegment( gx, gy, CellZ) )
			{ 
				continue;
			}
			bool createdBlock = false;
			Block* b = segment.getBlock( gx, gy, CellZ);

			if (!b)
			{
				createdBlock = true;
				b = new Block ( &segment );
				b->x = gx;
				b->y = gy;
				b->z = CellZ;
			}

			b->occ = occupancies[lx][ly];
			b->designation = designations[lx][ly];
			b->mudlevel = 0;
			b->snowlevel = 0;
			b->bloodlevel = 0;
			if(1)
			{
				long red=0;
				long green=0;
				long blue=0;
				for(int i = 0; i < splatter.size(); i++)
				{
					if(splatter[i].mat1 == MUD)
					{
						b->mudlevel = splatter[i].intensity[lx][ly];
					}
					else if(splatter[i].mat1 == ICE)
					{
						b->snowlevel = splatter[i].intensity[lx][ly];
					}
					else if(splatter[i].mat1 == VOMIT)
					{
						b->bloodlevel += splatter[i].intensity[lx][ly];
						red += (127 * splatter[i].intensity[lx][ly]);
						green += (196 * splatter[i].intensity[lx][ly]);
						blue += (28 *splatter[i].intensity[lx][ly]);
					}
					else if(splatter[i].mat1 == ICHOR)
					{
						b->bloodlevel += splatter[i].intensity[lx][ly];
						red += (255 * splatter[i].intensity[lx][ly]);
						green += (255 * splatter[i].intensity[lx][ly]);
						blue += (255 * splatter[i].intensity[lx][ly]);
					}
					else if(splatter[i].mat1 == BLOOD_NAMED)
					{
						b->bloodlevel += splatter[i].intensity[lx][ly];
						red += (150 * splatter[i].intensity[lx][ly]);
						//green += (0 * splatter[i].intensity[lx][ly]);
						blue += (24 * splatter[i].intensity[lx][ly]);
					}
					else if(splatter[i].mat1 == BLOOD_1
						|| splatter[i].mat1 == BLOOD_2
						|| splatter[i].mat1 == BLOOD_3
						|| splatter[i].mat1 == BLOOD_4
						|| splatter[i].mat1 == BLOOD_5
						|| splatter[i].mat1 == BLOOD_6)
					{
						b->bloodlevel += splatter[i].intensity[lx][ly];
						if(splatter[i].mat2 == 206) //troll
						{
							//red += (0 * splatter[i].intensity[lx][ly]);
							green += (255 * splatter[i].intensity[lx][ly]);
							blue += (255 * splatter[i].intensity[lx][ly]);
						}
						else if(splatter[i].mat2 == 242) //imp
						{
							//red += (0 * splatter[i].intensity[lx][ly]);
							//green += (0 * splatter[i].intensity[lx][ly]);
							//blue += (0 * splatter[i].intensity[lx][ly]);
						}
						else
						{
							red += (150 * splatter[i].intensity[lx][ly]);
							//green += (0 * splatter[i].intensity[lx][ly]);
							blue += (24 * splatter[i].intensity[lx][ly]);
						}
					}
				}
				if(b->bloodlevel)
				{
				b->bloodcolor = al_map_rgba(red/b->bloodlevel, green/b->bloodlevel, blue/b->bloodlevel, (b->bloodlevel > config.bloodcutoff) ? 255 : b->bloodlevel*255/config.bloodcutoff);
				}
				else
					b->bloodcolor = al_map_rgba(0,0,0,0);
			}
			else
			{
				b->bloodcolor = al_map_rgb(150, 0, 24);
			}
			//temperatures

			b->temp1 = temp1[lx][ly];
			b->temp2 = temp2[lx][ly];
			//liquids
			if(designations[lx][ly].bits.flow_size > 0){
				b->water.type  = designations[lx][ly].bits.liquid_type;
				b->water.index = designations[lx][ly].bits.flow_size;
			}

			//read tiletype
			int t = tiletypes[lx][ly];
			if(IDisWall(t)) 
				b->wallType = t;
			if(IDisFloor(t))
				b->floorType = t;
			if(isStairTerrain(t))
				b->stairType = t;
			if(isRampTerrain(t))
				b->ramp.type = t;
			b->tileType = t;

			//142,136,15
			//if(b->x == 142 && b->y == 136 && b->z == 15)
			//  int j = 10;

			//save in segment
			bool isHidden = designations[lx][ly].bits.hidden;
			//option for including hidden blocks
			isHidden &= !config.show_hidden_blocks;
			bool shouldBeIncluded = (!isOpenTerrain(t) || b->water.index) && !isHidden;
			//include hidden blocks as shaded black 
			if(config.shade_hidden_blocks && isHidden && (isBlockOnVisibleEdgeOfSegment(&segment, b) || areNeighborsVisible(designations, lx, ly)))
			{
				b->wallType = 0;
				b->floorType = 0;
				b->stairType = 0;
				b->ramp.type = 0;
				b->water.index = 0;
				b->building.info.type = BUILDINGTYPE_BLACKBOX;
				static c_sprite sprite;
				sprite.set_sheetindex(SPRITEOBJECT_BLACK);
				sprite.set_defaultsheet(IMGObjectSheet);
				sprite.set_offset(0, 4);
				b->building.sprites.push_back( sprite );
				sprite.set_offset(0, 0);
				b->building.sprites.push_back( sprite );
				shouldBeIncluded= true;
			}

			if( shouldBeIncluded )
			{
				//this only needs to be done for included blocks

				//determine rock/soil type
				int rockIndex = -1;
				if(regionoffsets[designations[lx][ly].bits.biome] < (*allLayers).size())
					if(designations[lx][ly].bits.geolayer_index < (*allLayers).at(regionoffsets[designations[lx][ly].bits.biome]).size())
						rockIndex = (*allLayers).at(regionoffsets[designations[lx][ly].bits.biome]).at(designations[lx][ly].bits.geolayer_index);
				b->layerMaterial.type = INORGANIC;
				b->layerMaterial.index = rockIndex;
				//check veins
				//if there's no veins, the vein material should just be the layer material.
				b->veinMaterial.type = INORGANIC;
				b->veinMaterial.index = rockIndex;
				for(uint32_t i=0; i<numVeins; i++)
				{
					//TODO: This will be fixed in dfHack at some point, but right now objects that arnt veins pass through as. So we filter on vtable

					//if((uint32_t)veins[i].type >= groundTypes.size())
					//continue;

					// DANGER: THIS CODE MAY BE BUGGY
					// This was apparently causing a crash in previous version
					// But works fine for me
					uint16_t row = veins[i].assignment[ly];
					bool set = (row & (1 << lx)) != 0;
					if(set){
						rockIndex = veins[i].type;
						b->veinMaterial.type = INORGANIC;
						b->veinMaterial.index = veins[i].type;
						b->hasVein = 1;
					}
					else
					{
						b->veinMaterial.type = INORGANIC;
						b->veinMaterial.index = rockIndex;
					}
				}
				b->material.type = INORGANIC;
				b->material.index = b->veinMaterial.index;

				//read global features
				int16_t idx = mapBlock.global_feature;
				if( idx != -1 && (uint16_t)idx < global_features->size() && global_features->at(idx).main_material != -1)
				{
					if(designations[lx][ly].bits.feature_global)
					{
						//if(global_features->at(idx).main_material == INORGANIC) // stone
						//{
						//there may be other features.
						b->layerMaterial.type = global_features->at(idx).main_material;
						b->layerMaterial.index = global_features->at(idx).sub_material;
						b->material.type = global_features->at(idx).main_material;
						b->material.index = global_features->at(idx).sub_material;
						b->hasVein = 0;
						//}
					}
				}

				//read local features
				idx = mapBlock.local_feature;
				if( idx != -1 )
				{
					DFHack::planecoord pc;
					pc.dim.x = CellX;
					pc.dim.y = CellY;
					std::map <DFHack::planecoord, std::vector<DFHack::t_feature *> >::iterator it;
					it = local_features->find(pc);
					if(it != local_features->end())
					{
						std::vector<DFHack::t_feature *>& vectr = (*it).second;
						if((uint16_t)idx < vectr.size() && vectr[idx]->main_material != -1)
						{
							if(mapBlock.designation[lx][ly].bits.feature_local)
							{
								//if(vectr[idx]->main_material == INORGANIC) // stone
								//{
								//We can probably get away with this.
								b->veinMaterial.type = vectr[idx]->main_material;
								b->veinMaterial.index = vectr[idx]->sub_material;
								b->material.type = vectr[idx]->main_material;
								b->material.index = vectr[idx]->sub_material;
								b->hasVein = 1;
								//}
							}
						}
					}
				}

				if(tileTypeTable[b->tileType].m == OBSIDIAN)
				{
					b->material.type = INORGANIC;
					b->material.index = contentLoader.obsidian;
				}


				//string name = v_stonetypes[j].id;
				if (createdBlock)
				{
					segment.addBlock(b);
				}
			}else if (createdBlock){
				delete(b);
			}

		}
	}
}


bool checkFloorBorderRequirement(WorldSegment* segment, int x, int y, int z, dirRelative offset)
{
	Block* bHigh = segment->getBlockRelativeTo(x, y, z, offset);
	if (bHigh && (bHigh->floorType > 0 || bHigh->ramp.type > 0 || bHigh->wallType > 0))
	{
		return false;
	}
	Block* bLow = segment->getBlockRelativeTo(x, y, z-1, offset);
	if (bLow == NULL || bLow->ramp.type == 0)
	{
		return true;
	}
	return false;
}


WorldSegment* ReadMapSegment(DFHack::Context &DF, int x, int y, int z, int sizex, int sizey, int sizez){
	uint32_t index;
	TMR2_START;
	DFHack::Maps *Maps;
	if(!config.skipMaps)
	{
		try
		{
			Maps = DF.getMaps();
		}
		catch (exception &e)
		{
			WriteErr("%DFhack exeption: s\n", e.what());
		}
	}
	DFHack::Materials *Mats;
	try
	{
		Mats = DF.getMaterials();
	}
	catch (exception &e)
	{
		WriteErr("DFhack exeption: %s\n", e.what());
	}
	DFHack::Position *Pos;
	try
	{
		Pos = DF.getPosition();
	}
	catch (exception &e)
	{
		WriteErr("DFhack exeption: %s\n", e.what());
	}
	DFHack::Vegetation *Veg;
	if(!config.skipVegetation)
	{
		try
		{
			Veg = DF.getVegetation();
		}
		catch (exception &e)
		{
			WriteErr("DFhack exeption: %s\n", e.what());
			config.skipVegetation = true;
		}
	}
	DFHack::Constructions *Cons;
	if(!config.skipConstructions)
	{
		try
		{
			Cons = DF.getConstructions();
		}
		catch (exception &e)
		{
			WriteErr("DFhack exeption: %s\n", e.what());
			config.skipConstructions = true;
		}
	}
	DFHack::World *Wold;
	if(!config.skipWorld)
	{
		try
		{
			Wold = DF.getWorld();
		}
		catch (exception &e)
		{
			WriteErr("DFhack exeption: %s\n", e.what());
			config.skipWorld = true;
		}
	}

	if(!config.skipMaps)
	{
		if(!Maps->Start())
		{
			WriteErr("Can't init map.");
			DisconnectFromDF();
			//return new blank segment
			return new WorldSegment(x,y,z + 1,sizex,sizey,sizez + 1);
		}
	}
	else
	{
		//return new blank segment
		return new WorldSegment(x,y,z + 1,sizex,sizey,sizez + 1);
	}
	if( IsConnectedToDF() == false){
		DisconnectFromDF();
		//return new blank segment
		return new WorldSegment(x,y,z + 1,sizex,sizey,sizez + 1);
	}

	//read memory info
	if( memInfoHasBeenRead == false){
		dfMemoryInfo = DF.getMemoryInfo();
		memInfoHasBeenRead = true;
	}

	if (timeToReloadConfig)
	{
		contentLoader.Load(DF);
		timeToReloadConfig = false;
	}

	//read date
	if(!config.skipWorld)
	{
		contentLoader.currentYear = Wold->ReadCurrentYear();
		contentLoader.currentTick = Wold->ReadCurrentTick();
		contentLoader.currentMonth = (contentLoader.currentTick+9)/33600;
		contentLoader.currentDay = ((contentLoader.currentTick+9)%33600)/1200;
		contentLoader.currentHour = ((contentLoader.currentTick+9)-(((contentLoader.currentMonth*28)+contentLoader.currentDay)*1200))/50;
		contentLoader.currentTickRel = (contentLoader.currentTick+9)-(((((contentLoader.currentMonth*28)+contentLoader.currentDay)*24)+contentLoader.currentHour)*50);
	}

	//Read Number of cells
	int celldimX, celldimY, celldimZ;
	Maps->getSize((unsigned int &)celldimX, (unsigned int &)celldimY, (unsigned int &)celldimZ);
	//Store these
	config.cellDimX = celldimX * 16;
	config.cellDimY = celldimY * 16;
	config.cellDimZ = celldimZ;
	//bound view to world
	if(x > celldimX * CELLEDGESIZE -sizex/2) DisplayedSegmentX = x = celldimX * CELLEDGESIZE -sizex/2;
	if(y > celldimY * CELLEDGESIZE -sizey/2) DisplayedSegmentY = y = celldimY * CELLEDGESIZE -sizey/2;
	if(x < -sizex/2) DisplayedSegmentX = x = -sizex/2;
	if(y < -sizey/2) DisplayedSegmentY = y = -sizey/2;

	//setup new world segment
	WorldSegment* segment = new WorldSegment(x,y,z,sizex,sizey,sizez);
	segment->regionSize.x = celldimX * CELLEDGESIZE;
	segment->regionSize.y = celldimY * CELLEDGESIZE;
	segment->regionSize.z = celldimZ;
	segment->rotation = DisplayedRotation;

	//read world wide buildings
	vector<t_building> allBuildings;
	if(!config.skipBuildings)
		ReadBuildings(DF, &allBuildings);

	/*if(GroundMaterialNamesTranslatedFromGame == false)
	TranslateGroundMaterialNames();*/

	// get region geology
	vector< vector <uint16_t> > layers;
	if(!Maps->ReadGeology( layers ))
	{
		WriteErr("Can't get region geology.\n");
	}

	//read cursor
	Pos->getCursorCoords(config.dfCursorX, config.dfCursorY, config.dfCursorZ);

	// read constructions
	vector<t_construction> allConstructions;
	uint32_t numconstructions = 0;

	if(!config.skipConstructions)
	{
		try
		{
			if (Cons->Start(numconstructions))
			{
				t_construction tempcon;
				index = 0;
				while(index < numconstructions)
				{
					Cons->Read(index, tempcon);
					if(segment->CoordinateInsideSegment(tempcon.x, tempcon.y, tempcon.z))
						allConstructions.push_back(tempcon);
					index++;
				}
				Cons->Finish();
			}
		}
		catch(exception &e)
		{
			WriteErr("DFhack exception: %s\n", e.what());
			config.skipConstructions = true;
		}
	}

	//merge buildings with segment
	if(!config.skipBuildings)
		MergeBuildingsToSegment(&allBuildings, segment);

	//figure out what cells to read
	int32_t firstTileToReadX = x;
	if( firstTileToReadX < 0 ) firstTileToReadX = 0;

	//read global features
	vector<DFHack::t_feature> global_features;
	Maps->ReadGlobalFeatures(global_features);

	//read local features
	std::map <DFHack::planecoord, std::vector<DFHack::t_feature *> > local_features;
	Maps->ReadLocalFeatures(local_features);
	while(firstTileToReadX < x + sizex){
		int cellx = firstTileToReadX / CELLEDGESIZE;
		int32_t lastTileInCellX = (cellx+1) * CELLEDGESIZE - 1;
		int32_t lastTileToReadX = min<int32_t>(lastTileInCellX, x+sizex-1);

		int32_t firstTileToReadY = y;
		if( firstTileToReadY < 0 ) firstTileToReadY = 0;

		while(firstTileToReadY < y + sizey){
			int celly = firstTileToReadY / CELLEDGESIZE;
			int32_t lastTileInCellY = (celly+1) * CELLEDGESIZE - 1;
			int32_t lastTileToReadY = min<uint32_t>(lastTileInCellY, y+sizey-1);

			for(int lz=z-sizez; lz <= z; lz++){
				//load the blcoks from this cell to the map segment
				ReadCellToSegment(DF, *segment, cellx, celly, lz, 
					firstTileToReadX, firstTileToReadY, lastTileToReadX, lastTileToReadY,
					0, &allBuildings, &allConstructions, &layers, &global_features, &local_features, Maps);

			}
			firstTileToReadY = lastTileToReadY + 1;
		}
		firstTileToReadX = lastTileToReadX + 1;
	}

	//translate constructions
	changeConstructionMaterials(segment, &allConstructions);


	//Read Vegetation
	uint32_t numtrees;
	if(!config.skipVegetation)
	{
		try
		{
			if (Veg->Start(numtrees))
			{
				t_tree temptree;
				index = 0;
				while(index < numtrees )
				{
					Veg->Read(index, temptree);
					//want hashtable :(
					Block* b;
					if( b = segment->getBlock( temptree.x, temptree.y, temptree.z) )
					{
						b->tree.type = temptree.type;
						b->tree.index = temptree.material;
					}
					index ++;
				}
				Veg->Finish();
			}
		}
		catch(exception &err)
		{
			WriteErr("DFhack exeption: %s\n", err.what());
			config.skipVegetation = true;
		}
	}

	////Read Effects
	//uint32_t numeffects;
	//if (DF.InitReadEffects(numeffects))
	//{
	//	t_effect_df40d tempeffect;
	//	index = 0;
	//	while(index < numeffects )
	//	{
	//		DF.ReadEffect(index, tempeffect);
	//		//want hashtable :(
	//		Block* b;
	//		if( b = segment->getBlock( tempeffect.x, tempeffect.y, tempeffect.z) )
	//			if(!(tempeffect.isHidden))
	//			{
	//				b->blockeffects.type = tempeffect.type;
	//				b->blockeffects.canCreateNew = tempeffect.canCreateNew;
	//				b->blockeffects.lifetime = tempeffect.lifetime;
	//				b->blockeffects.material = tempeffect.material;
	//				b->blockeffects.x_direction = tempeffect.x_direction;
	//				b->blockeffects.y_direction = tempeffect.y_direction;
	//				b->blockeffects.count +=1;
	//				if(tempeffect.type == 0)
	//					b->eff_miasma = tempeffect.lifetime;
	//				if(tempeffect.type == 1)
	//					b->eff_water = tempeffect.lifetime;
	//				if(tempeffect.type == 2)
	//					b->eff_water2 = tempeffect.lifetime;
	//				if(tempeffect.type == 3)
	//					b->eff_blood = tempeffect.lifetime;
	//				if(tempeffect.type == 4)
	//					b->eff_dust = tempeffect.lifetime;
	//				if(tempeffect.type == 5)
	//					b->eff_magma = tempeffect.lifetime;
	//				if(tempeffect.type == 6)
	//					b->eff_smoke = tempeffect.lifetime;
	//				if(tempeffect.type == 7)
	//					b->eff_dragonfire = tempeffect.lifetime;
	//				if(tempeffect.type == 8)
	//					b->eff_fire = tempeffect.lifetime;
	//				if(tempeffect.type == 9)
	//					b->eff_webing = tempeffect.lifetime;
	//				if(tempeffect.type == 10)
	//					b->eff_boiling = tempeffect.lifetime;
	//				if(tempeffect.type == 11)
	//					b->eff_oceanwave = tempeffect.lifetime;
	//			}
	//			index ++;
	//	}
	//	DF.FinishReadEffects();
	//}
	//Read Creatures
	if(!config.skipCreatures)
		ReadCreaturesToSegment( DF, segment );

	//do misc beautification
	uint32_t numblocks = segment->getNumBlocks();
	for(uint32_t i=0; i < numblocks; i++){
		Block* b = segment->getBlock(i);
		//setup building sprites
		if( b->building.info.type != BUILDINGTYPE_NA && b->building.info.type != BUILDINGTYPE_BLACKBOX )
			loadBuildingSprites( b, DF );

		//setup deep water
		if( b->water.index == 7 && b->water.type == 0)
		{
			int topdepth = blockWaterDepth(b->x, b->y, b->z, segment, eAbove);
			if(topdepth)
				b->water.index = 8;
		}


		//setup ramps
		if(b->ramp.type > 0) 
			b->ramp.index = CalculateRampType(b->x, b->y, b->z, segment);
		//add edges to blocks and floors  

		Block * dir1 = segment->getBlockRelativeTo(b->x, b->y, b->z, eUpLeft);
		Block * dir2 = segment->getBlockRelativeTo(b->x, b->y, b->z, eUp);
		Block * dir3 = segment->getBlockRelativeTo(b->x, b->y, b->z, eUpRight);
		Block * dir4 = segment->getBlockRelativeTo(b->x, b->y, b->z, eRight);
		Block * dir5 = segment->getBlockRelativeTo(b->x, b->y, b->z, eDownRight);
		Block * dir6 = segment->getBlockRelativeTo(b->x, b->y, b->z, eDown);
		Block * dir7 = segment->getBlockRelativeTo(b->x, b->y, b->z, eDownLeft);
		Block * dir8 = segment->getBlockRelativeTo(b->x, b->y, b->z, eLeft);

		b->obscuringBuilding=0;
		b->obscuringCreature=0;

		if(dir1) if(dir1->creaturePresent) b->obscuringCreature = 1;
		if(dir2) if(dir2->creaturePresent) b->obscuringCreature = 1;
		if(dir8) if(dir8->creaturePresent) b->obscuringCreature = 1;

		if(dir1) if(dir1->building.info.type != BUILDINGTYPE_NA && dir1->building.info.type != BUILDINGTYPE_BLACKBOX && dir1->building.info.type != contentLoader.civzoneNum && dir1->building.info.type != contentLoader.stockpileNum) b->obscuringBuilding = 1;
		if(dir2) if(dir2->building.info.type != BUILDINGTYPE_NA && dir2->building.info.type != BUILDINGTYPE_BLACKBOX && dir2->building.info.type != contentLoader.civzoneNum && dir2->building.info.type != contentLoader.stockpileNum) b->obscuringBuilding = 1;
		if(dir8) if(dir8->building.info.type != BUILDINGTYPE_NA && dir8->building.info.type != BUILDINGTYPE_BLACKBOX && dir8->building.info.type != contentLoader.civzoneNum && dir8->building.info.type != contentLoader.stockpileNum) b->obscuringBuilding = 1;

		if( b->floorType > 0 )
		{
			b->depthBorderWest = checkFloorBorderRequirement(segment, b->x, b->y, b->z, eLeft);
			b->depthBorderNorth = checkFloorBorderRequirement(segment, b->x, b->y, b->z, eUp);

			Block* belowBlock = segment->getBlockRelativeTo(b->x, b->y, b->z, eBelow);
			if(!belowBlock || (!belowBlock->wallType && !belowBlock->ramp.type)) 
				b->depthBorderDown = true;
		}
		else if( b->wallType > 0 && wallShouldNotHaveBorders( b->wallType ) == false )
		{
			Block* leftBlock = segment->getBlockRelativeTo(b->x, b->y, b->z, eLeft);
			Block* upBlock = segment->getBlockRelativeTo(b->x, b->y, b->z, eUp);
			if(!leftBlock || (!leftBlock->wallType && !leftBlock->ramp.type)) 
				b->depthBorderWest = true;
			if(!upBlock || (!upBlock->wallType && !upBlock->ramp.type))
				b->depthBorderNorth = true;
			Block* belowBlock = segment->getBlockRelativeTo(b->x, b->y, b->z, eBelow);
			if(!belowBlock || (!belowBlock->wallType && !belowBlock->ramp.type)) 
				b->depthBorderDown = true;
		}
		b->wallborders = 0;
		if(dir1) if(dir1->wallType) b->wallborders |= 1;
		if(dir2) if(dir2->wallType) b->wallborders |= 2;
		if(dir3) if(dir3->wallType) b->wallborders |= 4;
		if(dir4) if(dir4->wallType) b->wallborders |= 8;
		if(dir5) if(dir5->wallType) b->wallborders |= 16;
		if(dir6) if(dir6->wallType) b->wallborders |= 32;
		if(dir7) if(dir7->wallType) b->wallborders |= 64;
		if(dir8) if(dir8->wallType) b->wallborders |= 128;

		b->rampborders = 0;
		if(dir1) if(dir1->ramp.type) b->wallborders |= 1;
		if(dir2) if(dir2->ramp.type) b->wallborders |= 2;
		if(dir3) if(dir3->ramp.type) b->wallborders |= 4;
		if(dir4) if(dir4->ramp.type) b->wallborders |= 8;
		if(dir5) if(dir5->ramp.type) b->wallborders |= 16;
		if(dir6) if(dir6->ramp.type) b->wallborders |= 32;
		if(dir7) if(dir7->ramp.type) b->wallborders |= 64;
		if(dir8) if(dir8->ramp.type) b->wallborders |= 128;

		b->upstairborders = 0;
		b->downstairborders = 0;
		if(dir1) if(dir1->stairType == STAIR_UP) b->upstairborders |= 1;
		if(dir2) if(dir2->stairType == STAIR_UP) b->upstairborders |= 2;
		if(dir3) if(dir3->stairType == STAIR_UP) b->upstairborders |= 4;
		if(dir4) if(dir4->stairType == STAIR_UP) b->upstairborders |= 8;
		if(dir5) if(dir5->stairType == STAIR_UP) b->upstairborders |= 16;
		if(dir6) if(dir6->stairType == STAIR_UP) b->upstairborders |= 32;
		if(dir7) if(dir7->stairType == STAIR_UP) b->upstairborders |= 64;
		if(dir8) if(dir8->stairType == STAIR_UP) b->upstairborders |= 128;

		if(dir1) if(dir1->stairType == STAIR_UPDOWN) b->upstairborders |= 1;
		if(dir2) if(dir2->stairType == STAIR_UPDOWN) b->upstairborders |= 2;
		if(dir3) if(dir3->stairType == STAIR_UPDOWN) b->upstairborders |= 4;
		if(dir4) if(dir4->stairType == STAIR_UPDOWN) b->upstairborders |= 8;
		if(dir5) if(dir5->stairType == STAIR_UPDOWN) b->upstairborders |= 16;
		if(dir6) if(dir6->stairType == STAIR_UPDOWN) b->upstairborders |= 32;
		if(dir7) if(dir7->stairType == STAIR_UPDOWN) b->upstairborders |= 64;
		if(dir8) if(dir8->stairType == STAIR_UPDOWN) b->upstairborders |= 128;

		if(dir1) if(dir1->stairType == STAIR_UPDOWN) b->downstairborders |= 1;
		if(dir2) if(dir2->stairType == STAIR_UPDOWN) b->downstairborders |= 2;
		if(dir3) if(dir3->stairType == STAIR_UPDOWN) b->downstairborders |= 4;
		if(dir4) if(dir4->stairType == STAIR_UPDOWN) b->downstairborders |= 8;
		if(dir5) if(dir5->stairType == STAIR_UPDOWN) b->downstairborders |= 16;
		if(dir6) if(dir6->stairType == STAIR_UPDOWN) b->downstairborders |= 32;
		if(dir7) if(dir7->stairType == STAIR_UPDOWN) b->downstairborders |= 64;
		if(dir8) if(dir8->stairType == STAIR_UPDOWN) b->downstairborders |= 128;

		if(dir1) if(dir1->stairType == STAIR_DOWN) b->downstairborders |= 1;
		if(dir2) if(dir2->stairType == STAIR_DOWN) b->downstairborders |= 2;
		if(dir3) if(dir3->stairType == STAIR_DOWN) b->downstairborders |= 4;
		if(dir4) if(dir4->stairType == STAIR_DOWN) b->downstairborders |= 8;
		if(dir5) if(dir5->stairType == STAIR_DOWN) b->downstairborders |= 16;
		if(dir6) if(dir6->stairType == STAIR_DOWN) b->downstairborders |= 32;
		if(dir7) if(dir7->stairType == STAIR_DOWN) b->downstairborders |= 64;
		if(dir8) if(dir8->stairType == STAIR_DOWN) b->downstairborders |= 128;

		b->floorborders = 0;
		if(dir1) if(dir1->floorType) b->floorborders |= 1;
		if(dir2) if(dir2->floorType) b->floorborders |= 2;
		if(dir3) if(dir3->floorType) b->floorborders |= 4;
		if(dir4) if(dir4->floorType) b->floorborders |= 8;
		if(dir5) if(dir5->floorType) b->floorborders |= 16;
		if(dir6) if(dir6->floorType) b->floorborders |= 32;
		if(dir7) if(dir7->floorType) b->floorborders |= 64;
		if(dir8) if(dir8->floorType) b->floorborders |= 128;

		b->openborders = ~(b->floorborders|b->rampborders|b->wallborders|b->downstairborders|b->upstairborders);
	}

	Maps->Finish();
	segment->loaded = 1;
	TMR2_STOP;

	return segment;
}


bool ConnectDFAPI(DFHack::Context* pDF){
	try
	{
		pDF->Attach();
	}
	catch(exception & err)
	{
		WriteErr("DFhack exeption: %s \n", err.what());
		return false;
	}
	catch(...)
	{
		return false;
	}
	//in case DF has locked up, force it's thread to resume
	pDF->ForceResume();
	return pDF->isAttached();
}

void DisconnectFromDF(){
	if(pDFApiHandle){
		//pDFApiHandle->getMaps()->DestroyMap();
		//in case DF has locked up, force it's thread to resume
		pDFApiHandle->ForceResume();
		pDFApiHandle->Detach();
		delete pDFApiHandle;
		pDFApiHandle = 0;
	}
}

bool IsConnectedToDF(){
	if(!pDFApiHandle) return false;
	return pDFApiHandle->isAttached();
}

void FollowCurrentDFWindow( )
{
	int32_t newviewx;
	int32_t newviewy;
	int32_t viewsizex;
	int32_t viewsizey;
	int32_t newviewz;
	int32_t mapx, mapy, mapz;
	DFHack::Position *Pos =pDFApiHandle->getPosition();
	try
	{
		if (Pos)
		{
			// we take the rectangle you'd get if you scrolled the DF view closely around
			// map edges with a pen pierced through the center,
			// compute the scaling factor between this rectangle and the map bounds and then scale
			// the coords with this scaling factor
			/**

			+---+
			|W+-++----------+
			+-+-+---------+ |
			| |         | |
			| | inner   | |
			| |   rect. | |
			| |         | |
			| |         | |--- map boundary
			| +---------+ |
			+-------------+  W - corrected view

			*/
			pDFApiHandle->getMaps()->getSize((uint32_t &)mapx, (uint32_t &)mapy, (uint32_t &)mapz);
			mapx *= 16;
			mapy *= 16;

			Pos->getWindowSize(viewsizex,viewsizey);
			float scalex = float (mapx) / float (mapx - viewsizex);
			float scaley = float (mapy) / float (mapy - viewsizey);

			Pos->getViewCoords(newviewx,newviewy,newviewz);
			newviewx = newviewx + (viewsizex / 2) - mapx / 2;
			newviewy = newviewy + (viewsizey / 2) - mapy / 2;

			DisplayedSegmentX = float (newviewx) * scalex - (config.segmentSize.x / 2) + config.viewXoffset + mapx / 2;
			DisplayedSegmentY = float (newviewy) * scaley - (config.segmentSize.y / 2) + config.viewYoffset + mapy / 2;
			DisplayedSegmentZ = newviewz + config.viewZoffset + 1;

		}
		else
		{
			//fail
			config.follow_DFscreen = false;
		}
	}
	catch(exception &err)
	{
		WriteErr("DFhack exeption: %s \n", err.what());
		config.follow_DFscreen = false;
	}
}

void FollowCurrentDFCenter( )
{
	int32_t newviewx;
	int32_t newviewy;
	int32_t viewsizex;
	int32_t viewsizey;
	int32_t newviewz;
	DFHack::Position *Pos =pDFApiHandle->getPosition();
	try
	{
		if (Pos)
		{
			Pos->getWindowSize(viewsizex,viewsizey); 
			Pos->getViewCoords(newviewx,newviewy,newviewz);

			DisplayedSegmentX = newviewx + (viewsizex/2) - (config.segmentSize.x / 2) + config.viewXoffset;
			DisplayedSegmentY = newviewy + (viewsizey/2) - (config.segmentSize.y / 2) + config.viewYoffset;
			DisplayedSegmentZ = newviewz + config.viewZoffset + 1;       
		}
		else
		{
			//fail
			config.follow_DFscreen = false;
		}
	}
	catch(exception &err)
	{
		WriteErr("DFhack exeption: %s \n", err.what());
		config.follow_DFscreen = false;
	}
}

static void * threadedSegment(ALLEGRO_THREAD *thread, void *arg)
{
	while(!al_get_thread_should_stop(thread))
	{
		al_lock_mutex(config.readMutex);
		al_wait_cond(config.readCond, config.readMutex);
		config.threadstarted = 1;
		pDFApiHandle->Suspend();
		if(altSegment)
		{
			al_lock_mutex(altSegment->mutie);
			al_unlock_mutex(altSegment->mutie);
			altSegment->Dispose();
			delete(altSegment);
		}
		altSegment = ReadMapSegment(*pDFApiHandle, parms.x, parms.y, parms.z,
			parms.sizex, parms.sizey, parms.sizez);
		config.threadstarted = 0;
		pDFApiHandle->Resume();
		if(viewedSegment)
			al_lock_mutex(viewedSegment->mutie);
		swapSegments();
		if(altSegment)
		al_unlock_mutex(altSegment->mutie);
		al_unlock_mutex(config.readMutex);
		al_rest(config.automatic_reload_time/1000.0);
	}
	return 0;
}

void reloadDisplayedSegment(){
	//create handle to dfHack API
	bool firstLoad = (pDFApiHandle == 0);
	if(pDFApiHandle == 0){
		al_clear_to_color(al_map_rgb(0,0,0));
		draw_textf_border(font, al_map_rgb(255,255,255),
			al_get_bitmap_width(al_get_target_bitmap())/2,
			al_get_bitmap_height(al_get_target_bitmap())/2,
			ALLEGRO_ALIGN_CENTRE, "Connecting to DF...");
		al_flip_display();
		memInfoHasBeenRead = false;
		DFMgr = new ContextManager("Memory.xml");
		try
		{
			pDFApiHandle = DFMgr->getSingleContext();
			pDFApiHandle->Attach();
			pDFApiHandle->Detach();
		}
		catch (exception& e)
		{
			WriteErr("No Dwarf Fortress executable found\n");
			WriteErr("DFhack exeption: %s\n", e.what());
			delete( pDFApiHandle );
			pDFApiHandle = 0;
			return ;
		}
	}
	DFHack::Context& DF = *pDFApiHandle;
	if( !connected)
	{
		DF.Attach();
		connected = 1;
	}
	TMR1_START;

#ifndef RELEASE
	firstLoad=false;
#endif

	if (timeToReloadConfig)
	{
		DF.Suspend();
		contentLoader.Load(DF);
		timeToReloadConfig = false;
	}

	//G->Start();
	//G->ReadViewScreen(config.viewscreen);
	//config.menustate = G->ReadMenuState();
	//G->Finish();
	//dispose old segment

	if (firstLoad || config.follow_DFscreen)
	{
		if (config.track_center)
		{
			FollowCurrentDFCenter();
		}
		else
		{
			FollowCurrentDFWindow();
		}
	}

	int segmentHeight = config.single_layer_view ? 2 : config.segmentSize.z;
	//load segment
	if(!config.threadmade)
	{
		config.readThread = al_create_thread(threadedSegment, NULL);
		config.threadmade = 1;
	}
	//if(config.threadstarted)
	//	al_join_thread(config.readThread, NULL);

	parms.x = DisplayedSegmentX;
	parms.y = DisplayedSegmentY;
	parms.z = DisplayedSegmentZ;
	parms.sizex = config.segmentSize.x;
	parms.sizey = config.segmentSize.y;
	parms.sizez = segmentHeight;
	al_start_thread(config.readThread);
	al_broadcast_cond(config.readCond);
	//if(!viewedSegment || viewedSegment->regionSize.x == 0 || viewedSegment->regionSize.y == 0)
	//{
	//	abortAutoReload();
	//	timeToReloadConfig = true;
	//}
	TMR1_STOP;
}

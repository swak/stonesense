/*
www.sourceforge.net/projects/dfhack
Copyright (c) 2009 Petr Mrázek (peterix), Kenneth Ferland (Impaler[WrG]), dorf

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any
damages arising from the use of this software.

Permission is granted to anyone to use this software for any
purpose, including commercial applications, and to alter it and
redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must
not claim that you wrote the original software. If you use this
software in a product, an acknowledgment in the product documentation
would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and
must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source
distribution.
*/

#include "DFCommonInternal.h"
using namespace DFHack;

class API::Private
{
    public:
        Private()
        : block(NULL)
        , pm(NULL), p(NULL), dm(NULL), offset_descriptor(NULL)
        , p_cons(NULL), p_bld(NULL), p_veg(NULL)
        {}
        uint32_t * block;
        uint32_t x_block_count, y_block_count, z_block_count;
        uint32_t regionX, regionY, regionZ;
        uint32_t worldSizeX, worldSizeY;
        
        uint32_t tile_type_offset;
        uint32_t designation_offset;
        uint32_t occupancy_offset;
        
        uint32_t window_x_offset;
        uint32_t window_y_offset;
        uint32_t window_z_offset;
        uint32_t cursor_xyz_offset;
        
        uint32_t creature_pos_offset;
        uint32_t creature_type_offset;
        uint32_t creature_flags1_offset;
        uint32_t creature_flags2_offset;
        uint32_t creature_first_name_offset;
        uint32_t creature_nick_name_offset;
        uint32_t creature_last_name_offset;
        
        uint32_t creature_custom_profession_offset;
        uint32_t creature_profession_offset;
        uint32_t creature_sex_offset;
        uint32_t creature_id_offset;
        uint32_t creature_squad_name_offset;
        uint32_t creature_squad_leader_id_offset;
        uint32_t creature_money_offset;
        uint32_t creature_current_job_offset;
        uint32_t creature_current_job_id_offset;
        uint32_t creature_strength_offset;
        uint32_t creature_agility_offset;
        uint32_t creature_toughness_offset;
        uint32_t creature_skills_offset;
        uint32_t creature_labors_offset;
        uint32_t creature_happiness_offset;
        uint32_t creature_traits_offset;
        
        uint32_t dwarf_lang_table_offset;
        
        ProcessManager* pm;
        Process* p;
        DataModel* dm;
        memory_info* offset_descriptor;
        vector<uint16_t> v_geology[eBiomeCount];
        string xml;
        
        bool constructionsInited;
        bool buildingsInited;
        bool vegetationInited;
        bool creaturesInited;
        bool cursorWindowInited;
        
        bool nameTablesInited;
        
        uint32_t tree_offset;
        DfVector *p_cre;
        DfVector *p_cons;
        DfVector *p_bld;
        DfVector *p_veg;
        
        DfVector *p_trans;
        DfVector *p_generic;
        DfVector *p_dwarf_names;
        
        string getLastNameByAddress(const uint32_t &address, bool use_generic=false);
        string getSquadNameByAddress(const uint32_t &address, bool use_generic=false);
        string getProfessionByAddress(const uint32_t &address);
        string getCurrentJobByAddress(const uint32_t &address);
        void getSkillsByAddress(const uint32_t &address, vector<t_skill> &);
        void getTraitsByAddress(const uint32_t &address, vector<t_trait> &);
        void getLaborsByAddress(const uint32_t &address, vector<t_labor> &);
};

API::API(const string path_to_xml)
: d(new Private())
{
    d->xml = QUOT(MEMXML_DATA_PATH);
    d->xml += "/";
    d->xml += path_to_xml;
    d->constructionsInited = false;
    d->creaturesInited = false;
    d->buildingsInited = false;
    d->vegetationInited = false;
    d->cursorWindowInited = false;
    d->pm = NULL;
}

API::~API()
{
    delete d;
}

/*-----------------------------------*
 *  Init the mapblock pointer array   *
 *-----------------------------------*/
bool API::InitMap()
{
    uint32_t    map_loc,   // location of the X array
                temp_loc,  // block location
                temp_locx, // iterator for the X array
                temp_locy, // iterator for the Y array
                temp_locz; // iterator for the Z array
    uint32_t map_offset = d->offset_descriptor->getAddress("map_data");
    uint32_t x_count_offset = d->offset_descriptor->getAddress("x_count");
    uint32_t y_count_offset = d->offset_descriptor->getAddress("y_count");
    uint32_t z_count_offset = d->offset_descriptor->getAddress("z_count");

    // get the offsets once here
    d->tile_type_offset = d->offset_descriptor->getOffset("type");
    d->designation_offset = d->offset_descriptor->getOffset("designation");
    d->occupancy_offset = d->offset_descriptor->getOffset("occupancy");

    // get the map pointer
    map_loc = MreadDWord(map_offset);
    //FIXME: very inadequate
    if (!map_loc)
    {
        // bad stuffz happend
        return false;
    }

    // get the size
    d->x_block_count = MreadDWord(x_count_offset);
    d->y_block_count = MreadDWord(y_count_offset);
    d->z_block_count = MreadDWord(z_count_offset);

    // alloc array for pinters to all blocks
    d->block = new uint32_t[d->x_block_count*d->y_block_count*d->z_block_count];

    //read the memory from the map blocks - x -> map slice
    for(uint32_t x = 0; x < d->x_block_count; x++)
    {
        temp_locx = map_loc + ( 4 * x );
        temp_locy = MreadDWord(temp_locx);

        // y -> map column
        for(uint32_t y = 0; y < d->y_block_count; y++)
        {
            temp_locz = MreadDWord(temp_locy);
            temp_locy += 4;

            // z -> map block (16x16)
            for(uint32_t z = 0; z < d->z_block_count; z++)
            {
                temp_loc = MreadDWord(temp_locz);
                temp_locz += 4;
                d->block[x*d->y_block_count*d->z_block_count + y*d->z_block_count + z] = temp_loc;
            }
        }
    }
    return true;
}

bool API::DestroyMap()
{
    if (d->block != NULL)
    {
        delete [] d->block;
        d->block = NULL;
    }

    return true;
}

bool API::isValidBlock(uint32_t x, uint32_t y, uint32_t z)
{
    return d->block[x*d->y_block_count*d->z_block_count + y*d->z_block_count + z] != NULL;
}

// 256 * sizeof(uint16_t)
bool API::ReadTileTypes(uint32_t x, uint32_t y, uint32_t z, uint16_t *buffer)
{
    uint32_t addr = d->block[x*d->y_block_count*d->z_block_count + y*d->z_block_count + z];
    if (addr!=NULL)
    {
        Mread(addr+d->tile_type_offset, 256 * sizeof(uint16_t), (uint8_t *)buffer);
        return true;
    }
    return false;
}


// 256 * sizeof(uint32_t)
bool API::ReadDesignations(uint32_t x, uint32_t y, uint32_t z, uint32_t *buffer)
{
    uint32_t addr = d->block[x*d->y_block_count*d->z_block_count + y*d->z_block_count + z];
    if (addr!=NULL)
    {
        Mread(addr+d->designation_offset, 256 * sizeof(uint32_t), (uint8_t *)buffer);
        return true;
    }
    return false;
}


// 256 * sizeof(uint32_t)
bool API::ReadOccupancy(uint32_t x, uint32_t y, uint32_t z, uint32_t *buffer)
{
    uint32_t addr = d->block[x*d->y_block_count*d->z_block_count + y*d->z_block_count + z];
    if (addr!=NULL)
    {
        Mread(addr+d->occupancy_offset, 256 * sizeof(uint32_t), (uint8_t *)buffer);
        return true;
    }
    return false;
}


// 256 * sizeof(uint16_t)
bool API::WriteTileTypes(uint32_t x, uint32_t y, uint32_t z, uint16_t *buffer)
{
    uint32_t addr = d->block[x*d->y_block_count*d->z_block_count + y*d->z_block_count + z];
    if (addr!=NULL)
    {
        Mwrite(addr+d->tile_type_offset, 256 * sizeof(uint16_t), (uint8_t *)buffer);
        return true;
    }
    return false;
}


// 256 * sizeof(uint32_t)
bool API::WriteDesignations(uint32_t x, uint32_t y, uint32_t z, uint32_t *buffer)
{
    uint32_t addr = d->block[x*d->y_block_count*d->z_block_count + y*d->z_block_count + z];
    if (addr!=NULL)
    {
        Mwrite(addr+d->designation_offset, 256 * sizeof(uint32_t), (uint8_t *)buffer);
        return true;
    }
    return false;
}


// 256 * sizeof(uint32_t)
bool API::WriteOccupancy(uint32_t x, uint32_t y, uint32_t z, uint32_t *buffer)
{
    uint32_t addr = d->block[x*d->y_block_count*d->z_block_count + y*d->z_block_count + z];
    if (addr!=NULL)
    {
        Mwrite(addr+d->occupancy_offset, 256 * sizeof(uint32_t), (uint8_t *)buffer);
        return true;
    }
    return false;
}


//16 of them? IDK... there's probably just 7. Reading more doesn't cause errors as it's an array nested inside a block
// 16 * sizeof(uint8_t)
bool API::ReadRegionOffsets(uint32_t x, uint32_t y, uint32_t z, uint8_t *buffer)
{
    uint32_t biome_stuffs = d->offset_descriptor->getOffset("biome_stuffs");
    uint32_t addr = d->block[x*d->y_block_count*d->z_block_count + y*d->z_block_count + z];
    if (addr!=NULL)
    {
        Mread(addr+biome_stuffs, 16 * sizeof(uint8_t), buffer);
        return true;
    }
    return false;
}


// veins of a block, expects empty vein vector
bool API::ReadVeins(uint32_t x, uint32_t y, uint32_t z, vector <t_vein> & veins)
{
    uint32_t addr = d->block[x*d->y_block_count*d->z_block_count + y*d->z_block_count + z];
    int veinvector = d->offset_descriptor->getOffset("v_vein");
    int veinsize = d->offset_descriptor->getHexValue("v_vein_size");
    veins.clear();
    if(addr!=NULL && veinvector && veinsize)
    {
        assert(sizeof(t_vein) == veinsize);
        // veins are stored as a vector of pointers to veins
        /*pointer is 4 bytes! we work with a 32bit program here, no matter what architecture we compile khazad for*/
        DfVector p_veins = d->dm->readVector(addr + veinvector, 4);

        // read all veins
        for (uint32_t i = 0; i< p_veins.getSize();i++)
        {
            t_vein v;
            uint32_t temp;
            // read the vein pointer from the vector
            p_veins.read((uint32_t)i,(uint8_t *)&temp);
            // read the vein data (dereference pointer)
            Mread(temp, veinsize, (uint8_t *)&v);
            // store it in the vector
            veins.push_back(v);
        }
        return true;
    }
    return false;
}


// getter for map size
void API::getSize(uint32_t& x, uint32_t& y, uint32_t& z)
{
    x = d->x_block_count;
    y = d->y_block_count;
    z = d->z_block_count;
}

bool API::ReadWoodMatgloss(vector<t_matgloss> & woods)
{
    int matgloss_address = d->offset_descriptor->getAddress("matgloss");
    // TODO: find flag for autumnal coloring?
    DfVector p_matgloss = d->dm->readVector(matgloss_address, 4);
    
    woods.clear();
    
    t_matgloss mat;
    // TODO: use brown?
    mat.fore = 7;
    mat.back = 0;
    mat.bright = 0;
    for (uint32_t i = 0; i< p_matgloss.getSize();i++)
    {
        uint32_t temp;
        
        // read the matgloss pointer from the vector into temp
        p_matgloss.read((uint32_t)i,(uint8_t *)&temp);
        
        // read the string pointed at by
        fill_char_buf(mat.id, d->dm->readSTLString(temp)); // reads a C string given an address
        woods.push_back(mat);
    }
    return true;
}

bool API::ReadStoneMatgloss(vector<t_matgloss> & stones)
{
    memory_info * minfo = d->offset_descriptor;
    int matgloss_address = minfo->getAddress("matgloss");
    int matgloss_offset = minfo->getHexValue("matgloss_skip");
    int matgloss_colors = minfo->getOffset("matgloss_stone_color");
    DfVector p_matgloss = d->dm->readVector(matgloss_address + matgloss_offset, 4);

    int size = p_matgloss.getSize();
    stones.resize(0);
    stones.reserve(size);
    for (uint32_t i = 0; i< size;i++)
    {
        uint32_t temp;
        // read the matgloss pointer from the vector into temp
        p_matgloss.read((uint32_t)i,(uint8_t *)&temp);
        // read the string pointed at by
        t_matgloss mat;
        fill_char_buf(mat.id, d->dm->readSTLString(temp)); // reads a C string given an address
        mat.fore = (uint8_t)MreadWord(temp + matgloss_colors);
        mat.back = (uint8_t)MreadWord(temp + matgloss_colors + 2);
        mat.bright = (uint8_t)MreadWord(temp + matgloss_colors + 4);
        stones.push_back(mat);
    }
    return true;
}


bool API::ReadMetalMatgloss(vector<t_matgloss> & metals)
{
    memory_info * minfo = d->offset_descriptor;
    int matgloss_address = minfo->getAddress("matgloss");
    int matgloss_offset = minfo->getHexValue("matgloss_skip");
    int matgloss_colors = minfo->getOffset("matgloss_metal_color");
    DfVector p_matgloss = d->dm->readVector(matgloss_address + matgloss_offset*3, 4);

    metals.clear();

    for (uint32_t i = 0; i< p_matgloss.getSize();i++)
    {
        uint32_t temp;

        // read the matgloss pointer from the vector into temp
        p_matgloss.read((uint32_t)i,(uint8_t *)&temp);

        // read the string pointed at by
        t_matgloss mat;
        fill_char_buf(mat.id, d->dm->readSTLString(temp)); // reads a C string given an address
        mat.fore = (uint8_t)MreadWord(temp + matgloss_colors);
        mat.back = (uint8_t)MreadWord(temp + matgloss_colors + 2);
        mat.bright = (uint8_t)MreadWord(temp + matgloss_colors + 4);
        metals.push_back(mat);
    }
    return true;
}

bool API::ReadPlantMatgloss(vector<t_matgloss> & plants)
{
    memory_info * minfo = d->offset_descriptor;
    int matgloss_address = minfo->getAddress("matgloss");
    int matgloss_offset = minfo->getHexValue("matgloss_skip");
    DfVector p_matgloss = d->dm->readVector(matgloss_address + matgloss_offset*2, 4);

    plants.clear();

    // TODO: use green?
    t_matgloss mat;
    mat.fore = 7;
    mat.back = 0;
    mat.bright = 0;
    for (uint32_t i = 0; i< p_matgloss.getSize();i++)
    {
        uint32_t temp;

        // read the matgloss pointer from the vector into temp
        p_matgloss.read((uint32_t)i,(uint8_t *)&temp);

        // read the string pointed at by
        fill_char_buf(mat.id, d->dm->readSTLString(temp)); // reads a C string given an address
        plants.push_back(mat);
    }
    return true;
}

bool API::ReadCreatureMatgloss(vector<t_matgloss> & creatures)
{
    memory_info * minfo = d->offset_descriptor;
    int matgloss_address = minfo->getAddress("matgloss");
    int matgloss_offset = minfo->getHexValue("matgloss_skip");
    DfVector p_matgloss = d->dm->readVector(matgloss_address + matgloss_offset*6, 4);
    
    creatures.clear();
    
    // TODO: use green?
    t_matgloss mat;
    mat.fore = 7;
    mat.back = 0;
    mat.bright = 0;
    for (uint32_t i = 0; i< p_matgloss.getSize();i++)
    {
        uint32_t temp;
        
        // read the matgloss pointer from the vector into temp
        p_matgloss.read((uint32_t)i,(uint8_t *)&temp);
        
        // read the string pointed at by
        fill_char_buf(mat.id, d->dm->readSTLString(temp)); // reads a C string given an address
        creatures.push_back(mat);
    }
    return true;
}


//vector<uint16_t> v_geology[eBiomeCount];
bool API::ReadGeology( vector < vector <uint16_t> >& assign )
{
    memory_info * minfo = d->offset_descriptor;
    // get needed addresses and offsets
    int region_x_offset = minfo->getAddress("region_x");
    int region_y_offset = minfo->getAddress("region_y");
    int region_z_offset =  minfo->getAddress("region_z");
    int world_offset =  minfo->getAddress("world");
    int world_regions_offset =  minfo->getOffset("w_regions_arr");
    int region_size =  minfo->getHexValue("region_size");
    int region_geo_index_offset =  minfo->getOffset("region_geo_index_off");
    int world_geoblocks_offset =  minfo->getOffset("w_geoblocks");
    int world_size_x = minfo->getOffset("world_size_x");
    int world_size_y = minfo->getOffset("world_size_y");
    int geolayer_geoblock_offset = minfo->getOffset("geolayer_geoblock_offset");

    uint32_t regionX, regionY, regionZ;
    uint16_t worldSizeX, worldSizeY;

    // check if we have 'em all
    if(
        !(
        region_x_offset && region_y_offset && region_z_offset && world_size_x && world_size_y
        && world_offset && world_regions_offset && world_geoblocks_offset && region_size
        && region_geo_index_offset && geolayer_geoblock_offset
        )
    )
    {
        // fail if we don't have them
        return false;
    }

    // read position of the region inside DF world
    MreadDWord(region_x_offset, regionX);
    MreadDWord(region_y_offset, regionY);
    MreadDWord(region_z_offset, regionZ);

    // get world size
    MreadWord(world_offset + world_size_x, worldSizeX);
    MreadWord(world_offset + world_size_y, worldSizeY);

    // get pointer to first part of 2d array of regions
    uint32_t regions = MreadDWord(world_offset + world_regions_offset);

    // read the geoblock vector
    DfVector geoblocks = d->dm->readVector(world_offset + world_geoblocks_offset,4);

    // iterate over 8 surrounding regions + local region
    for(int i = eNorthWest; i< eBiomeCount; i++)
    {
        // check bounds, fix them if needed
        int bioRX = regionX / 16 + (i%3) - 1;
        if( bioRX < 0) bioRX = 0;
        if( bioRX >= worldSizeX) bioRX = worldSizeX - 1;
        int bioRY = regionY / 16 + (i/3) - 1;
        if( bioRY < 0) bioRY = 0;
        if( bioRY >= worldSizeY) bioRY = worldSizeY - 1;

        // get pointer to column of regions
        uint32_t geoX;
        MreadDWord(regions + bioRX*4, geoX);

        // get index into geoblock vector
        uint16_t geoindex;
        MreadWord(geoX + bioRY*region_size + region_geo_index_offset, geoindex);

        // get the geoblock from the geoblock vector using the geoindex
        uint32_t geoblock_off;
        geoblocks.read(geoindex,(uint8_t *) &geoblock_off);

        // get the vector with pointer to layers
        DfVector geolayers = d->dm->readVector(geoblock_off + geolayer_geoblock_offset , 4); // let's hope
        // make sure we don't load crap
        assert(geolayers.getSize() > 0 && geolayers.getSize() <= 16);

        // finally, read the layer matgloss
        for(uint32_t j = 0;j< geolayers.getSize();j++)
        {
            int geol_offset;
            // read pointer to a layer
            geolayers.read(j, (uint8_t *) & geol_offset);
            // read word at pointer + 2, store in our geology vectors
            d->v_geology[i].push_back(MreadWord(geol_offset + 2));
        }
    }
    assign.clear();
    // TODO: clean this up
    for(int i = 0; i< eBiomeCount;i++)
    {
        assign.push_back(d->v_geology[i]);
    }
    return true;
}


// returns number of buildings, expects v_buildingtypes that will later map t_building.type to its name
uint32_t API::InitReadBuildings(vector <string> &v_buildingtypes)
{
    d->buildingsInited = true;
    int buildings = d->offset_descriptor->getAddress("buildings");
    assert(buildings);
    d->p_bld = new DfVector( d->dm->readVector(buildings,4));
    d->offset_descriptor->copyBuildings(v_buildingtypes);
    return d->p_bld->getSize();
}


// read one building
bool API::ReadBuilding(const uint32_t &index, t_building & building)
{
    assert(d->buildingsInited);
    uint32_t temp;
    t_building_df40d bld_40d;

    // read pointer from vector at position
    d->p_bld->read(index,(uint8_t *)&temp);

    //read building from memory
    Mread(temp, sizeof(t_building_df40d), (uint8_t *)&bld_40d);

    // transform
    int32_t type = -1;
    d->offset_descriptor->resolveClassId(temp, type);
    building.origin = temp;
    building.vtable = bld_40d.vtable;
    building.x1 = bld_40d.x1;
    building.x2 = bld_40d.x2;
    building.y1 = bld_40d.y1;
    building.y2 = bld_40d.y2;
    building.z = bld_40d.z;
    building.material = bld_40d.material;
    building.type = type;

    return true;
}


void API::FinishReadBuildings()
{
    delete d->p_bld;
    d->p_bld = NULL;
    d->buildingsInited = false;
}


//TODO: maybe do construction reading differently - this could go slow with many of them.
// returns number of constructions, prepares a vector, returns total number of constructions
uint32_t API::InitReadConstructions()
{
    d->constructionsInited = true;
    int constructions = d->offset_descriptor->getAddress("constructions");
    assert(constructions);

    d->p_cons = new DfVector(d->dm->readVector(constructions,4));
    return d->p_cons->getSize();
}


bool API::ReadConstruction(const uint32_t &index, t_construction & construction)
{
    assert(d->constructionsInited);
    t_construction_df40d c_40d;
    uint32_t temp;

    // read pointer from vector at position
    d->p_cons->read((uint32_t)index,(uint8_t *)&temp);

    //read construction from memory
    Mread(temp, sizeof(t_construction_df40d), (uint8_t *)&c_40d);

    // transform
    construction.x = c_40d.x;
    construction.y = c_40d.y;
    construction.z = c_40d.z;
    construction.material = c_40d.material;

	return true;
}


void API::FinishReadConstructions()
{
    delete d->p_cons;
    d->p_cons = NULL;
    d->constructionsInited = false;
}


uint32_t API::InitReadVegetation()
{
    d->vegetationInited = true;
    int vegetation = d->offset_descriptor->getAddress("vegetation");
    d->tree_offset = d->offset_descriptor->getOffset("tree_desc_offset");
    assert(vegetation && d->tree_offset);
    d->p_veg = new DfVector(d->dm->readVector(vegetation,4));
    return d->p_veg->getSize();
}


bool API::ReadVegetation(const uint32_t &index, t_tree_desc & shrubbery)
{
    assert(d->vegetationInited);
    uint32_t temp;
    // read pointer from vector at position
    d->p_veg->read(index,(uint8_t *)&temp);
    //read construction from memory
    Mread(temp + d->tree_offset, sizeof(t_tree_desc), (uint8_t *) &shrubbery);
    // FIXME: this is completely wrong. type isn't just tree/shrub but also different kinds of trees. stuff that grows around ponds has its own type ID
    if(shrubbery.material.type == 3) shrubbery.material.type = 2;
    return true;
}


void API::FinishReadVegetation()
{
    delete d->p_veg;
    d->p_veg = NULL;
    d->vegetationInited = false;
}


uint32_t API::InitReadCreatures()
{
    memory_info * minfo = d->offset_descriptor;
    int creatures = d->offset_descriptor->getAddress("creatures");
    d->creature_pos_offset = minfo->getOffset("creature_position");
    d->creature_type_offset = minfo->getOffset("creature_race");
    d->creature_flags1_offset = minfo->getOffset("creature_flags1");
    d->creature_flags2_offset = minfo->getOffset("creature_flags2");
    d->creature_first_name_offset = minfo->getOffset("creature_first_name");
    d->creature_nick_name_offset = minfo->getOffset("creature_nick_name");
    d->creature_last_name_offset = minfo->getOffset("creature_last_name");
    d->creature_custom_profession_offset = minfo->getOffset("creature_custom_profession");
    d->creature_profession_offset = minfo->getOffset("creature_profession");
    d->creature_sex_offset = minfo->getOffset("creature_sex");
    d->creature_id_offset = minfo->getOffset("creature_id");
    d->creature_squad_name_offset = minfo->getOffset("creature_squad_name");
    d->creature_squad_leader_id_offset = minfo->getOffset("creature_squad_leader_id");
    d->creature_money_offset = minfo->getOffset("creature_money");
    d->creature_current_job_offset = minfo->getOffset("creature_current_job");
    d->creature_current_job_id_offset = minfo->getOffset("current_job_id");
    d->creature_strength_offset = minfo->getOffset("creature_strength");
    d->creature_agility_offset = minfo->getOffset("creature_agility");
    d->creature_toughness_offset = minfo->getOffset("creature_toughness");
    d->creature_skills_offset = minfo->getOffset("creature_skills");
    d->creature_labors_offset = minfo->getOffset("creature_labors");
    d->creature_happiness_offset = minfo->getOffset("creature_happiness");
    d->creature_traits_offset = minfo->getOffset("creature_traits");
    if( creatures
        && d->creature_pos_offset
        && d->creature_type_offset
        && d->creature_flags1_offset
        && d->creature_flags2_offset
        && d->creature_nick_name_offset
        && d->creature_custom_profession_offset
        && d->creature_profession_offset
        && d->creature_sex_offset
        && d->creature_id_offset
        && d->creature_squad_name_offset
        && d->creature_squad_leader_id_offset
        && d->creature_money_offset
        && d->creature_current_job_offset
        && d->creature_strength_offset
        && d->creature_agility_offset
        && d->creature_toughness_offset
        && d->creature_skills_offset
        && d->creature_labors_offset
        && d->creature_happiness_offset
        && d->creature_traits_offset
    )
    {
        d->p_cre = new DfVector(d->dm->readVector(creatures, 4));
        InitReadNameTables();
        //if(d->InitReadNameTables())
        //{
            d->creaturesInited = true;
            return d->p_cre->getSize();
        //}
        //else
        //{
        //    return false;
        //}
    }
    else
    {
        return false;
    }
}

//This code was mostly adapted fromh dwarftherapist by chmod
string API::Private::getLastNameByAddress(const uint32_t &address, bool use_generic)
{
    string out; 
    uint32_t wordIndex;
    for (int i = 0; i<7;i++)
    {
        MreadDWord(address+i*4, wordIndex);
        if(wordIndex == 0xFFFFFFFF)
        {
            break;
        }
        if(use_generic)
        {
            uint32_t genericPtr;
            p_generic->read(wordIndex,(uint8_t *)&genericPtr);
            out.append(dm->readSTLString(genericPtr));
        }
        else
        {
            uint32_t transPtr;
            p_dwarf_names->read(wordIndex,(uint8_t *)&transPtr);
            out.append(dm->readSTLString(transPtr));
        }
    }
    return out;
}

string API::Private::getSquadNameByAddress(const uint32_t &address, bool use_generic)
{
    string out; 
    uint32_t wordIndex;
    for (int i = 0; i<6;i++)
    {
        MreadDWord(address+i*4, wordIndex);
        if(wordIndex == 0xFFFFFFFF)
        {
            continue;
        }
        if(wordIndex == 0)
        {
            break;
        }
        if(use_generic)
        {
            uint32_t genericPtr;
            p_generic->read(wordIndex,(uint8_t *)&genericPtr);
            out.append(dm->readSTLString(genericPtr));
        }
        else
        {
            if(i == 4) // There will be a space in game if there is a name in the last 
            {
                out.append(" ");
            }
            uint32_t transPtr;
            p_dwarf_names->read(wordIndex,(uint8_t *)&transPtr);
            out.append(dm->readSTLString(transPtr));
        }
    }
    return out;
}

string API::Private::getProfessionByAddress(const uint32_t &address)
{
    string profession;
    uint8_t profId = MreadByte(address);
    profession = offset_descriptor->getProfession(profId);
    return profession;
}

string API::Private::getCurrentJobByAddress(const uint32_t &address)
{
    string job;
    uint32_t jobIdAddr = MreadDWord(address);
    if(jobIdAddr != 0)
    {
        uint8_t jobId = MreadByte(jobIdAddr+creature_current_job_id_offset);
        job = offset_descriptor->getJob(jobId);
    }
    else
    {
        job ="No Job";
    }
    return job;
}

string API::getLastName(const uint32_t &index, bool use_generic=false)
{
    assert(d->creaturesInited);
    uint32_t temp;
    // read pointer from vector at position
    d->p_cre->read(index,(uint8_t *)&temp);
    return(d->getLastNameByAddress(temp+d->creature_last_name_offset,use_generic));
}
string API::getSquadName(const uint32_t &index, bool use_generic=false)
{
    assert(d->creaturesInited);
    uint32_t temp;
    // read pointer from vector at position
    d->p_cre->read(index,(uint8_t *)&temp);
    return(d->getSquadNameByAddress(temp+d->creature_squad_name_offset,use_generic));
}
string API::getProfession(const uint32_t &index)
{
    assert(d->creaturesInited);
    uint32_t temp;
    // read pointer from vector at position
    d->p_cre->read(index,(uint8_t *)&temp);
    return(d->getProfessionByAddress(temp+d->creature_profession_offset));
}
string API::getCurrentJob(const uint32_t &index)
{
    assert(d->creaturesInited);
    uint32_t temp;
    // read pointer from vector at position
    d->p_cre->read(index,(uint8_t *)&temp);
    return(d->getCurrentJobByAddress(temp+d->creature_current_job_offset));
}
vector<t_skill> API::getSkills(const uint32_t &index)
{
    assert(d->creaturesInited);
    uint32_t temp;
    // read pointer from vector at position
    d->p_cre->read(index,(uint8_t *)&temp);
    vector<t_skill> tempSkillVec;
    d->getSkillsByAddress(temp+d->creature_last_name_offset,tempSkillVec);
    return(tempSkillVec);
}

vector<t_trait> API::getTraits(const uint32_t &index)
{
    assert(d->creaturesInited);
    uint32_t temp;
    // read pointer from vector at position
    d->p_cre->read(index,(uint8_t *)&temp);
    vector<t_trait> tempTraitVec;
    d->getTraitsByAddress(temp+d->creature_traits_offset,tempTraitVec);
    return(tempTraitVec);
}

void API::Private::getSkillsByAddress(const uint32_t &address, vector<t_skill> & skills)
{
    DfVector* skillVector = new DfVector(dm->readVector(address,4));
    for(uint32_t i = 0; i<skillVector->getSize();i++)
    {
        uint32_t temp;
        skillVector->read(i, (uint8_t *) &temp);
        t_skill tempSkill;
        tempSkill.id= MreadByte(temp);
        tempSkill.name = offset_descriptor->getSkill(tempSkill.id);
        tempSkill.experience = MreadWord(temp+8);
        tempSkill.rating = MreadByte(temp+4);
        /*
        add up all the experience per level
        for (int j = 0; j < tempSkill.rating; ++j) 
        { 
            tempSkill.experience += 500 + (j * 100);
        }
        */
        skills.push_back(tempSkill);
    }
}

void API::Private::getTraitsByAddress(const uint32_t &address, vector<t_trait> & traits)
{
    for(int i = 0; i < 30; i++)
    {
        t_trait tempTrait;
        tempTrait.value =MreadWord(address+i*2);
        tempTrait.displayTxt = offset_descriptor->getTrait(i,tempTrait.value);
        tempTrait.name = offset_descriptor->getTraitName(i);
        traits.push_back(tempTrait);
    }
}

void API::Private::getLaborsByAddress(const uint32_t &address, vector<t_labor> & labors)
{
    uint8_t laborArray[102] = {0};
    Mread(address, 102, laborArray);
    for(int i = 0;i<102; i++)
    {
        t_labor tempLabor;
        tempLabor.name = offset_descriptor->getLabor(i);
        tempLabor.value = laborArray[i];
        labors.push_back(tempLabor);
    }
}
bool API::ReadCreature(const uint32_t &index, t_creature & furball)
{
    assert(d->creaturesInited);
    uint32_t temp;
    // read pointer from vector at position
    d->p_cre->read(index,(uint8_t *)&temp);
    //read creature from memory
    Mread(temp + d->creature_pos_offset, 3 * sizeof(uint16_t), (uint8_t *) &(furball.x)); // xyz really
    MreadDWord(temp + d->creature_type_offset, furball.type);
    MreadDWord(temp + d->creature_flags1_offset, furball.flags1.whole);
    MreadDWord(temp + d->creature_flags2_offset, furball.flags2.whole);
    // names
    furball.first_name = d->dm->readSTLString(temp+d->creature_first_name_offset);
    furball.nick_name  = d->dm->readSTLString(temp+d->creature_nick_name_offset);
    furball.trans_name = d->getLastNameByAddress(temp+d->creature_last_name_offset);
    furball.generic_name = d->getLastNameByAddress(temp+d->creature_last_name_offset,true);
    furball.generic_squad_name = d->getSquadNameByAddress(temp+d->creature_squad_name_offset, true);
    furball.trans_squad_name = d->getSquadNameByAddress(temp+d->creature_squad_name_offset, false);
    furball.custom_profession = d->dm->readSTLString(temp+d->creature_custom_profession_offset);
    furball.profession = d->getProfessionByAddress(temp+d->creature_profession_offset);
    furball.current_job = d->getCurrentJobByAddress(temp+d->creature_current_job_offset);
    d->getSkillsByAddress(temp+d->creature_skills_offset,furball.skills);
    d->getTraitsByAddress(temp+d->creature_traits_offset,furball.traits);
    d->getLaborsByAddress(temp+d->creature_labors_offset,furball.labors);

    MreadDWord(temp + d->creature_happiness_offset, furball.happiness);
    MreadDWord(temp + d->creature_id_offset, furball.id);
    MreadDWord(temp + d->creature_agility_offset, furball.agility);
    MreadDWord(temp + d->creature_strength_offset, furball.strength);
    MreadDWord(temp + d->creature_toughness_offset, furball.toughness);
    MreadDWord(temp + d->creature_money_offset, furball.money);
    furball.squad_leader_id = int32_t(MreadDWord(temp + d->creature_squad_leader_id_offset));
    MreadByte(temp + d->creature_sex_offset, furball.sex);
    return true;
}

void API::InitReadNameTables()
{
    int genericAddress = d->offset_descriptor->getAddress("language_vector");
    int transAddress = d->offset_descriptor->getAddress("translation_vector");
    int word_table_offset = d->offset_descriptor->getOffset("word_table");
    
    d->p_generic = new DfVector(d->dm->readVector(genericAddress, 4));
    d->p_trans = new DfVector(d->dm->readVector(transAddress, 4));
    uint32_t dwarf_entry = 0;
    
    for(uint32_t i = 0; i < d->p_trans->getSize();i++)
    {
        uint32_t namePtr;
        d->p_trans->read(i,(uint8_t *)&namePtr);
        string raceName = d->dm->readSTLString(namePtr);
        
        if(raceName == "DWARF")
        {
            dwarf_entry = namePtr;
        }
    }
    
    d->dwarf_lang_table_offset = dwarf_entry + word_table_offset;
    d->p_dwarf_names = new DfVector(d->dm->readVector(d->dwarf_lang_table_offset,4));
    d->nameTablesInited = true;
}

void API::FinishReadNameTables()
{
    delete d->p_trans;
    delete d->p_generic;
    d->p_trans=d->p_generic=NULL;
    d->nameTablesInited=false;
}

void API::FinishReadCreatures()
{
    delete d->p_cre;
    d->p_cre = NULL;
    d->creaturesInited = false;
    FinishReadNameTables();
}

bool API::Attach()
{
    // detach all processes, destroy manager
    if(d->pm == NULL)
    {
        d->pm = new ProcessManager(d->xml); // FIXME: handle bad XML better
    }
    
    // find a process (ProcessManager can find multiple when used properly)
    if(!d->pm->findProcessess())
    {
        return false;
    }
    d->p = (*d->pm)[0];
    if(!d->p->attach())
    {
        return false; // couldn't attach to process, no go
    }
    d->offset_descriptor = d->p->getDescriptor();
    d->dm = d->p->getDataModel();
    // process is attached, everything went just fine... hopefully
    return true;
}


bool API::Detach()
{
    if (!d->p->detach())
    {
        return false;
    }
    if(d->pm != NULL)
    {
        delete d->pm;
    }
    d->pm = NULL;
    d->p = NULL;
    d->offset_descriptor = NULL;
    d->dm = NULL;
    return true;
}

bool API::isAttached()
{
    return d->dm != NULL;
}

void API::ReadRaw (const uint32_t &offset, const uint32_t &size, uint8_t *target)
{
    Mread(offset, size, target);
}

void API::WriteRaw (const uint32_t &offset, const uint32_t &size, uint8_t *source)
{
    Mwrite(offset, size, source);
}

bool API::InitViewAndCursor()
{
    
    d->window_x_offset = d->offset_descriptor->getAddress("window_x");
    d->window_y_offset = d->offset_descriptor->getAddress("window_y");
    d->window_z_offset = d->offset_descriptor->getAddress("window_z");
    d->cursor_xyz_offset = d->offset_descriptor->getAddress("cursor_xyz");
    if(d->window_x_offset && d->window_y_offset && d->window_z_offset)
    {
        d->cursorWindowInited = true;
        return true;
    }
    else
    {
        return false;
    }
}

bool API::getViewCoords (int32_t &x, int32_t &y, int32_t &z)
{
    assert(d->cursorWindowInited);
    MreadDWord(d->window_x_offset,(uint32_t &)x);
    MreadDWord(d->window_y_offset,(uint32_t &)y);
    MreadDWord(d->window_z_offset,(uint32_t &)z);
    return true;
}
//FIXME: confine writing of coords to map bounds?
bool API::setViewCoords (const int32_t &x, const int32_t &y, const int32_t &z)
{
    assert(d->cursorWindowInited);
    MwriteDWord(d->window_x_offset,(uint32_t &)x);
    MwriteDWord(d->window_y_offset,(uint32_t &)y);
    MwriteDWord(d->window_z_offset,(uint32_t &)z);
    return true;
}

bool API::getCursorCoords (int32_t &x, int32_t &y, int32_t &z)
{
    assert(d->cursorWindowInited);
    int32_t coords[3];
    Mread(d->cursor_xyz_offset,3*sizeof(int32_t),(uint8_t *)coords);
    x = coords[0];
    y = coords[1];
    z = coords[2];
    if(x == -30000) return false;
    return true;
}
//FIXME: confine writing of coords to map bounds?
bool API::setCursorCoords (const int32_t &x, const int32_t &y, const int32_t &z)
{
    assert(d->cursorWindowInited);
    int32_t coords[3] = {x,y,z};
    Mwrite(d->cursor_xyz_offset,3*sizeof(int32_t),(uint8_t *)coords);
    return true;
}
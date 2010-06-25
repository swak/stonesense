#include "common.h"
#include "SpriteMaps.h"
#include "GroundMaterialConfiguration.h"
#include "ContentLoader.h"


c_sprite *  GetTerrainSpriteMap(int in, t_matglossPair material, vector<TerrainConfiguration*>& configTable, uint16_t form)
{
	int tempform;
	if(form == constr_bar)
		tempform = FORM_BAR;
	if(form == constr_block)
		tempform = FORM_BLOCK;
	if(form == constr_boulder)
		tempform = FORM_BOULDER;
	if(form == constr_logs)
		tempform = FORM_LOG;
	// in case we need to return nothing
	static c_sprite * defaultSprite = new c_sprite;
	defaultSprite->set_sheetindex(UNCONFIGURED_INDEX);
	defaultSprite->set_fileindex(INVALID_INDEX);

	// first check the input is sane
	if( in < 0 || in >= (int)configTable.size() )
	{
		return defaultSprite;
	}
	// find a matching terrainConfig
	TerrainConfiguration* terrain = configTable[in];
	if (terrain == NULL)
	{
		return defaultSprite;
	}
	// check material sanity
	if (material.type<0 || material.type >= (int16_t)terrain->terrainMaterials.size())
	{
		if(terrain->defaultSprite[tempform].get_sheetindex() == UNCONFIGURED_INDEX)
			return &(terrain->defaultSprite[0]);
		else return &(terrain->defaultSprite[tempform]);
	}
	// find mat config
	TerrainMaterialConfiguration* terrainMat = terrain->terrainMaterials[material.type];
	if (terrainMat == NULL)
	{
		if(terrain->defaultSprite[tempform].get_sheetindex() == UNCONFIGURED_INDEX)
			return &(terrain->defaultSprite[0]);
		else return &(terrain->defaultSprite[tempform]);
	}
	if(material.index == -1)
	{
		if(terrainMat->defaultSprite[tempform].get_sheetindex() == UNCONFIGURED_INDEX)
			return &(terrainMat->defaultSprite[0]);
		else return &(terrainMat->defaultSprite[tempform]);
	}
	// return subtype, type default or terrain default as available
	// do map lookup
	map<int,c_sprite>::iterator it = terrainMat->overridingMaterials[tempform].find(material.index);
	if (it != terrainMat->overridingMaterials[tempform].end())
	{
		return &(it->second);
	}
	if (terrainMat->defaultSprite[tempform].get_sheetindex() != UNCONFIGURED_INDEX)
	{
		return &(terrainMat->defaultSprite[tempform]);
	}
	it = terrainMat->overridingMaterials[0].find(material.index);
	if (it != terrainMat->overridingMaterials[0].end())
	{
		return &(it->second);
	}
	if (terrainMat->defaultSprite[0].get_sheetindex() != UNCONFIGURED_INDEX)
	{
		return &(terrainMat->defaultSprite[0]);
	}
	return &(terrain->defaultSprite[0]);
}

c_sprite * GetFloorSpriteMap(int in, t_matglossPair material, uint16_t form){
	return GetTerrainSpriteMap(in, material, contentLoader.terrainFloorConfigs, form);
}

c_sprite * GetBlockSpriteMap(int in, t_matglossPair material, uint16_t form){
	return GetTerrainSpriteMap(in, material, contentLoader.terrainBlockConfigs, form);
}

c_sprite * GetSpriteVegetation( TileClass type, int index)
{
	static c_sprite * defaultSprite = new c_sprite;
	int base_sprite = SPRITEOBJECT_BLUEPRINT;
	vector<VegetationConfiguration>* graphicSet;
	bool live=true;
	bool grown=true;
	switch(type)
	{
	case TREE_DEAD:
		base_sprite = SPRITEOBJECT_TREE_DEAD;
		graphicSet = &(contentLoader.treeConfigs);
		live = false;
		break;
	case TREE_OK:
		base_sprite = SPRITEOBJECT_TREE_OK;
		graphicSet = &(contentLoader.treeConfigs);
		break;
	case SAPLING_DEAD:
		base_sprite = SPRITEOBJECT_SAPLING_DEAD;
		live = false;
		grown = false;
		graphicSet = &(contentLoader.treeConfigs);
		break;
	case SAPLING_OK: 
		base_sprite = SPRITEOBJECT_SAPLING_OK;
		grown = false;
		graphicSet = &(contentLoader.treeConfigs);
		break;
	case SHRUB_DEAD:
		base_sprite = SPRITEOBJECT_SHRUB_DEAD;
		live = false;
		graphicSet = &(contentLoader.shrubConfigs);
		break;
	case SHRUB_OK: 
		base_sprite = SPRITEOBJECT_SHRUB_OK;
		graphicSet = &(contentLoader.shrubConfigs);
		break;
	default:
		defaultSprite->set_sheetindex(SPRITEOBJECT_BLANK);
		return defaultSprite;
	}  	
  	
	c_sprite * configuredSprite = getVegetationSprite(*graphicSet,index,live,grown);
	if (configuredSprite->get_sheetindex() == -1)
	{
		configuredSprite->set_fileindex(-1); // should be set already, but...
		configuredSprite->set_sheetindex(base_sprite);
	}
	return configuredSprite;
}


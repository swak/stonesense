#include "common.h"
#include "SpriteMaps.h"
#include "GroundMaterialConfiguration.h"
#include "ContentLoader.h"


t_SpriteWithOffset GetTerrainSpriteMap(int in, t_matglossPair material, vector<int>& lookupTable)
{
	t_SpriteWithOffset defaultSprite = {-1,0,0,-1,ALL_FRAMES};
	if( in >= (int)lookupTable.size() ) 
		return defaultSprite;
	TerrainConfiguration* terrain = contentLoader.terrainConfigs[in];
	if (terrain == NULL)
		return defaultSprite;
	if (material.type > terrain->terrainMaterials.size())
		return terrain->defaultSprite;
	TerrainMaterialConfiguration* terrainMat = terrain->terrainMaterials[material.type];
	if (terrainMat == NULL)
		return terrain->defaultSprite;
	int numMat = (int)terrainMat->overridingMaterials.size();
	for(int i=0; i<numMat; i++)
	{
		if(terrainMat->overridingMaterials[i].gameID == material.index)
		{
			return terrainMat->overridingMaterials[i].sprite;
		}
	}
	return terrainMat->defaultSprite;
}

t_SpriteWithOffset GetFloorSpriteMap(int in, t_matglossPair material){
	return GetTerrainSpriteMap(in, material, contentLoader.terrainFloorLookup);
}

t_SpriteWithOffset GetBlockSpriteMap(int in, t_matglossPair material){
	return GetTerrainSpriteMap(in, material, contentLoader.terrainBlockLookup);
}

t_SpriteWithOffset GetSpriteVegetation( TileClass type, int index)
{
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
	}  	
  	
	t_SpriteWithOffset configuredSprite = getVegetationSprite(*graphicSet,index,live,grown);
	if (configuredSprite.sheetIndex == -1)
	{
		configuredSprite.fileIndex = -1; // should be set already, but...
		configuredSprite.sheetIndex = base_sprite;
	}
	return configuredSprite;
}


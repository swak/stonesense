#include "common.h"
#include "SpriteMaps.h"
#include "GroundMaterialConfiguration.h"
#include "ContentLoader.h"


t_SpriteWithOffset GetTerrainSpriteMap(int in, t_matglossPair material, vector<int>& lookupTable)
{
	//WriteErr("%d %d %d: test\n",in, material.type,material.index); 
	t_SpriteWithOffset defaultSprite = {-1,0,0,-1,ALL_FRAMES};
	if( in >= (int)lookupTable.size() )
	{
		//WriteErr("%d %d %d (%d): No lookup\n",in, material.type,material.index,lookupTable.size()); 
		return defaultSprite;
	}
	if (lookupTable[in] == INVALID_INDEX)
	{
		//WriteErr("%d %d %d (%d): No terrain\n",in, material.type,material.index,lookupTable.size()); 
		return defaultSprite;
	}
	//WriteErr("lookup %d\n",lookupTable[in]);
	TerrainConfiguration* terrain = contentLoader.terrainConfigs[lookupTable[in]];
	if (terrain == NULL)
	{
		//WriteErr("%d %d %d: Null terrain\n",in, material.type,material.index); 
		return defaultSprite;
	}
	//WriteErr("terrain\n");
	if (material.type >= terrain->terrainMaterials.size())
	{
		//WriteErr("%d %d %d: No material\n",in, material.type,material.index); 
		return terrain->defaultSprite;
	}
	//WriteErr("->\n");
	TerrainMaterialConfiguration* terrainMat = terrain->terrainMaterials[material.type];
	//WriteErr("->\n");
	//WriteErr("terrain: %d\n",terrainMat);
	if (terrainMat == NULL)
	{
		//WriteErr("%d %d %d: Null material\n",in, material.type,material.index); 
		return terrain->defaultSprite;
	}
	int numMat = (int)terrainMat->overridingMaterials.size();
	for(int i=0; i<numMat; i++)
	{
		if(terrainMat->overridingMaterials[i].gameID == material.index)
		{
			//WriteErr("%d %d %d: Got material\n",in, material.type,material.index);
			return terrainMat->overridingMaterials[i].sprite;
		}
	}
	//WriteErr("%d %d %d: Def material\n",in, material.type,material.index); 
	return terrainMat->defaultSprite;
}

t_SpriteWithOffset GetFloorSpriteMap(int in, t_matglossPair material){
	//WriteErr("gfsm+\n");
	t_SpriteWithOffset temp = GetTerrainSpriteMap(in, material, contentLoader.terrainFloorLookup);
	//WriteErr("gfsm-\n");
	return temp;
}

t_SpriteWithOffset GetBlockSpriteMap(int in, t_matglossPair material){
	//WriteErr("gbsm+\n");
	t_SpriteWithOffset temp = GetTerrainSpriteMap(in, material, contentLoader.terrainBlockLookup);
	//WriteErr("gfsm-\n");
	return temp;
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


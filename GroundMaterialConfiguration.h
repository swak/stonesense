#pragma once

#include "dfhack/library/tinyxml/tinyxml.h"

typedef struct OverridingMaterial{
	int gameID;
	t_SpriteWithOffset sprite;
}OverridingMaterial;

class TerrainMaterialConfiguration
{
public:
	t_SpriteWithOffset defaultSprite;
	int gameID;
	vector<OverridingMaterial> overridingMaterials;
	
	TerrainMaterialConfiguration();
	~TerrainMaterialConfiguration(){}
};

class TerrainConfiguration
{
public:
	vector<TerrainMaterialConfiguration*> terrainMaterials;
	t_SpriteWithOffset defaultSprite;
	TerrainConfiguration();
	~TerrainConfiguration();	
};

bool addSingleTerrainConfig( TiXmlElement* elemRoot);
//void LoadGroundMaterialConfiguration( );
//void TranslateGroundMaterialNames();

//extern bool GroundMaterialNamesTranslatedFromGame;
//extern vector<GroundMaterialConfiguration*> groundTypes;

void flushTerrainConfig(vector<TerrainConfiguration*>& config);

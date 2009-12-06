#include "common.h"
#include "SpriteMaps.h"
#include "GroundMaterialConfiguration.h"
#include <set>
#include "dfhack/library/tinyxml/tinyxml.h"
#include "GUI.h"
#include "ContentLoader.h"

TerrainMaterialConfiguration::TerrainMaterialConfiguration()
{
	defaultSprite.fileIndex=INVALID_INDEX;
	defaultSprite.sheetIndex=INVALID_INDEX;
	//dont really care about the rest of the sprite right now.
	
}

TerrainConfiguration::TerrainConfiguration()
{
	defaultSprite.fileIndex=INVALID_INDEX;
	defaultSprite.sheetIndex=INVALID_INDEX;
	//dont really care about the rest of the sprite right now.
}

TerrainConfiguration::~TerrainConfiguration()
{
	uint32_t currentsize=terrainMaterials.size();
	for (uint32_t i=0;i<currentsize;i++)
	{
		if (terrainMaterials[i] != NULL)
		{
			delete(terrainMaterials[i]);
		}
	}
}

void DumpGroundMaterialNamesToDisk(){
  FILE* fp = fopen("dump.txt", "w");
  if(!fp) return;
  for(uint32_t j=0; j < v_stonetypes.size(); j++){
    fprintf(fp, "%i:%s\n",j, v_stonetypes[j].id);
  }
  fclose(fp);
}

void parseWallFloorSpriteElement( TiXmlElement* elemWallFloorSprite, vector<int>& lookupTable ,int basefile)
{
	contentError("start",elemWallFloorSprite);
	const char* spriteIndexStr = elemWallFloorSprite->Attribute("sprite");
	if (spriteIndexStr == NULL || spriteIndexStr[0] == 0)
	{
		contentError("Invalid or missing sprite attribute",elemWallFloorSprite);
		return; //nothing to work with
	}
	
	t_SpriteWithOffset sprite;
	sprite.sheetIndex=atoi(spriteIndexStr);
	sprite.fileIndex=basefile;
	sprite.x=0;
	sprite.y=0;
	sprite.animFrames=ALL_FRAMES; //augh! no animated terrains! please!
	const char* filename = elemWallFloorSprite->Attribute("file");
	if (filename != NULL && filename[0] != 0)
	{
		sprite.fileIndex = loadImgFile((char*)filename);
	}
	
	set<int> lookupKeys;
	
	int newLookup=contentLoader.terrainConfigs.size();
	TiXmlElement* elemTerrain = elemWallFloorSprite->FirstChildElement("terrain");
	for(TiXmlElement* elemTerrain = elemWallFloorSprite->FirstChildElement("terrain");
		 elemTerrain;
		 elemTerrain = elemTerrain->NextSiblingElement("terrain"))
	{
		int targetElem=INVALID_INDEX;
		const char* gameIDstr = elemTerrain->Attribute("value");
		if (gameIDstr == NULL || gameIDstr[0] == 0)
		{
			contentError("Invalid or missing value attribute",elemTerrain);
			continue;
		}
		targetElem = atoi (gameIDstr);
		if (lookupTable.size() <= targetElem)
		{
			lookupTable.resize(targetElem+1,INVALID_INDEX);
		}
		if (lookupTable[targetElem]!=INVALID_INDEX)
		{
			WriteErr("got %d: %d\n",targetElem,lookupTable[targetElem]);
			lookupKeys.insert(lookupTable[targetElem]);
		}
		else
		{
			WriteErr("new %d: %d\n",targetElem,newLookup);
			lookupKeys.insert(newLookup);
			lookupTable[targetElem]=newLookup;
			contentLoader.terrainConfigs.push_back(new TerrainConfiguration());
		}
	}
	if (lookupKeys.size() == 0)
	return; //nothing to link to
	
	TiXmlElement* elemMaterial = elemWallFloorSprite->FirstChildElement("material");
	if (elemMaterial == NULL)
	{
		//set default terrain sprites
		for (set<int>::iterator it=lookupKeys.begin() ; it != lookupKeys.end(); it++ )
		{
			int index = *it;
			TerrainConfiguration *tConfig = contentLoader.terrainConfigs[index];
			//if that was null we have *really* screwed up earlier
			//only update if not set earlier
			if (tConfig->defaultSprite.sheetIndex == INVALID_INDEX)
			{
				tConfig->defaultSprite = sprite;
			}
		}
	}
	for( ;elemMaterial;elemMaterial = elemMaterial->NextSiblingElement("material"))
	{
		WriteErr("mat\n");
		int elemIndex = lookupMaterialType(elemMaterial->Attribute("value"));
		if (elemIndex == INVALID_INDEX)
		{
			contentError("Invalid or missing value attribute",elemMaterial);
			continue;				
		}
		WriteErr("ei %d\n",elemIndex);
		/* TODO handle sub material types */
		//set default material sprites
		for (set<int>::iterator it=lookupKeys.begin() ; it != lookupKeys.end(); it++ )
		{
			int index = *it;
			WriteErr("->%d\n",index);
			TerrainConfiguration *tConfig = contentLoader.terrainConfigs[index];
			//if that was null we have *really* screwed up earlier
			//create a new TerrainMaterialConfiguration if required
				//make sure we have room for it first
			if (tConfig->terrainMaterials.size() < elemIndex)
			{
				//dont make a full size vector in advance- we wont need it except
				//for those who insist on Soap Fortresses
				tConfig->terrainMaterials.resize(elemIndex+1,NULL);
			}
			if (tConfig->terrainMaterials[elemIndex] == NULL)
			{
				//TODO cleanup for this
				tConfig->terrainMaterials[elemIndex] = new TerrainMaterialConfiguration();
			}
			//only update if not set earlier
			if (tConfig->terrainMaterials[elemIndex]->defaultSprite.sheetIndex == INVALID_INDEX)
			{
				tConfig->terrainMaterials[elemIndex]->defaultSprite = sprite;
			}
		} 	
	}
}

bool addSingleTerrainConfig( TiXmlElement* elemRoot){
	int basefile = INVALID_INDEX;
  const char* filename = elemRoot->Attribute("file");
	if (filename != NULL && filename[0] != 0)
	{
	  	basefile = loadImgFile((char*)filename);
	}
	
  string elementType = elemRoot->Value();
  if(elementType.compare( "floors" ) == 0){
	  WriteErr("floor?\n");
    //parse floors
    TiXmlElement* elemFloor = elemRoot->FirstChildElement("floor");
    while( elemFloor ){
      parseWallFloorSpriteElement( elemFloor, contentLoader.terrainFloorLookup, basefile );
      elemFloor = elemFloor->NextSiblingElement("floor");
    }
  }
  if(elementType.compare( "blocks" ) == 0){
	  WriteErr("block?\n");
    //parse walls
    TiXmlElement* elemWall = elemRoot->FirstChildElement("block");
    while( elemWall ){
      parseWallFloorSpriteElement( elemWall, contentLoader.terrainBlockLookup, basefile );
      elemWall = elemWall->NextSiblingElement("block");
    }
  }
  return true;
}

void flushTerrainConfig(vector<TerrainConfiguration*>& config)
{
	uint32_t currentsize=config.size();
	for (uint32_t i=0;i<currentsize;i++)
	{
		if (config[i] != NULL)
		{
			delete(config[i]);
		}
	}
	
	config.clear();
	config.resize(currentsize,NULL);
}

void flushTerrainLookup(vector<int>& lookup)
{
	uint32_t currentsize=lookup.size();
	lookup.clear();	
	if (currentsize < MAX_BASE_TERRAIN + FAKE_TERRAIN_COUNT)
		currentsize = MAX_BASE_TERRAIN + FAKE_TERRAIN_COUNT;
	lookup.resize(currentsize,INVALID_INDEX);
}

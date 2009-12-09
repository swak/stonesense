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
  for(uint32_t j=0; j < contentLoader.stoneNameStrings.size(); j++){
    fprintf(fp, "%i:%s\n",j, contentLoader.stoneNameStrings[j].id);
  }
  fclose(fp);
}

void parseWallFloorSpriteElement( TiXmlElement* elemWallFloorSprite, vector<TerrainConfiguration*>& configTable ,int basefile)
{
	//contentError("start",elemWallFloorSprite);
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
	
	vector<int> lookupKeys;
	
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
		lookupKeys.push_back(targetElem);
		if (configTable.size() <= targetElem)
		{
			configTable.resize(targetElem+1,NULL);
		}
		if (configTable[targetElem]==NULL)
		{
			// cleaned up in flushTerrainConfig
			configTable[targetElem] = new TerrainConfiguration();
		}
	}
	int elems = lookupKeys.size();
	//WriteErr("elems: %d\n",elems);
	if (elems == 0)
		return; //nothing to link to
	
	TiXmlElement* elemMaterial = elemWallFloorSprite->FirstChildElement("material");
	if (elemMaterial == NULL)
	{
		//set default terrain sprites
		for (int i=0 ; i < elems; i++ )
		{
			//WriteErr("index: %d / %d\n", index,contentLoader.terrainConfigs.size());
			TerrainConfiguration *tConfig = configTable[lookupKeys[i]];
			//if that was null we have *really* screwed up earlier
			//only update if not set earlier
			if (tConfig->defaultSprite.sheetIndex == INVALID_INDEX)
			{
				tConfig->defaultSprite = sprite;
				//WriteErr("def\n");	
			}
			else
			{
				//WriteErr("nodef\n");	
			}
		}
	}
	for( ;elemMaterial;elemMaterial = elemMaterial->NextSiblingElement("material"))
	{
		//WriteErr("mat\n");
		int elemIndex = lookupMaterialType(elemMaterial->Attribute("value"));
		if (elemIndex == INVALID_INDEX)
		{
			contentError("Invalid or missing value attribute",elemMaterial);
			continue;				
		}
		//WriteErr("mat: %d\n", elemIndex);
		/* TODO handle sub material types */
		//set default material sprites
		TiXmlElement* elemSubtype = elemMaterial->FirstChildElement("subtype");
		if (elemSubtype == NULL)
		{
			//WriteErr("nosub: %d\n", elemIndex);
			for (int i=0 ; i < elems; i++ )
			{
				//WriteErr("index: %d / %d\n", index,contentLoader.terrainConfigs.size());
				TerrainConfiguration *tConfig = configTable[lookupKeys[i]];
				//WriteErr("tc\n");
				//WriteErr("tc: %d\n", tConfig);
				//WriteErr("tcs: %d\n", tConfig->terrainMaterials.size());
				//if that was null we have *really* screwed up earlier
				//create a new TerrainMaterialConfiguration if required
					//make sure we have room for it first
				if (tConfig->terrainMaterials.size() <= elemIndex)
				{
					//dont make a full size vector in advance- we wont need it except
					//for those who insist on Soap Fortresses
					tConfig->terrainMaterials.resize(elemIndex+1,NULL);
				}
				//WriteErr("tc e\n");
				if (tConfig->terrainMaterials[elemIndex] == NULL)
				{
					tConfig->terrainMaterials[elemIndex] = new TerrainMaterialConfiguration();
				}
				//WriteErr("tc i\n");
				//only update if not set earlier
				if (tConfig->terrainMaterials[elemIndex]->defaultSprite.sheetIndex == INVALID_INDEX)
				{
					tConfig->terrainMaterials[elemIndex]->defaultSprite = sprite;
				}
			} 	
		}
		for (;elemSubtype; elemSubtype = elemSubtype ->NextSiblingElement("subtype"))
		{
			//WriteErr("nosub\n");
			int subtypeId = lookupMaterialIndex(elemIndex,elemSubtype->Attribute("value"));
			if (subtypeId == INVALID_INDEX)
			{
				contentError("Invalid or missing value attribute",elemSubtype);
				continue;				
			}
			//WriteErr("nosub: %d\n", subtypeId);
			for (int i=0 ; i < elems; i++ )
			{
				//WriteErr("index: %d / %d\n", index,contentLoader.terrainConfigs.size());
				TerrainConfiguration *tConfig = configTable[lookupKeys[i]];
				//if that was null we have *really* screwed up earlier
				//create a new TerrainMaterialConfiguration if required
					//make sure we have room for it first
				if (tConfig->terrainMaterials.size() <= elemIndex)
				{
					//dont make a full size vector in advance- we wont need it except
					//for those who insist on Soap Fortresses
					tConfig->terrainMaterials.resize(elemIndex+1,NULL);
				}
				if (tConfig->terrainMaterials[elemIndex] == NULL)
				{
					tConfig->terrainMaterials[elemIndex] = new TerrainMaterialConfiguration();
				}
				// add to list (if not already present)
				map<int,t_SpriteWithOffset>::iterator it = tConfig->terrainMaterials[elemIndex]->overridingMaterials.find(subtypeId);
				if (it == tConfig->terrainMaterials[elemIndex]->overridingMaterials.end())
				{
					tConfig->terrainMaterials[elemIndex]->overridingMaterials[subtypeId]=sprite;
				}			
			} 			
		}
	}
}

bool addSingleTerrainConfig( TiXmlElement* elemRoot){
	//WriteErr("astc+\n");
	int basefile = INVALID_INDEX;
  const char* filename = elemRoot->Attribute("file");
	if (filename != NULL && filename[0] != 0)
	{
	  	basefile = loadImgFile((char*)filename);
	}
	
  string elementType = elemRoot->Value();
  if(elementType.compare( "floors" ) == 0){
    //parse floors
    TiXmlElement* elemFloor = elemRoot->FirstChildElement("floor");
    while( elemFloor ){
      parseWallFloorSpriteElement( elemFloor, contentLoader.terrainFloorConfigs, basefile );
      elemFloor = elemFloor->NextSiblingElement("floor");
    }
  }
  if(elementType.compare( "blocks" ) == 0){
    //parse walls
    TiXmlElement* elemWall = elemRoot->FirstChildElement("block");
    while( elemWall ){
      parseWallFloorSpriteElement( elemWall, contentLoader.terrainBlockConfigs, basefile );
      elemWall = elemWall->NextSiblingElement("block");
    }
  }
  //WriteErr("astc-\n");
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
	if (currentsize < MAX_BASE_TERRAIN + FAKE_TERRAIN_COUNT)
		currentsize = MAX_BASE_TERRAIN + FAKE_TERRAIN_COUNT;
	config.resize(currentsize,NULL);
}

#include "common.h"
#include "CreatureConfiguration.h"
#include "Creatures.h"
#include "MapLoading.h"
#include "GUI.h"
#include "ContentLoader.h"
#include "spriteObjects.h"

#include "dfhack/depends/tinyxml/tinyxml.h"


VegetationConfiguration::VegetationConfiguration(int gameID, c_sprite &sprite, bool live, bool grown)
{
	memset(this, 0, sizeof(VegetationConfiguration) );
	this->sprite = sprite;
	this->gameID = gameID;
	this->live = live;
	this->grown = grown;
}

VegetationConfiguration::~VegetationConfiguration(void)
{
}

bool addSingleVegetationConfig( TiXmlElement* elemRoot,  vector<VegetationConfiguration>* vegetationConfigs, vector<t_matgloss>& plantNames )
{
	const char* sheetIndexStr;
	c_sprite sprite;
	int basefile = -1;

	const char* filename = elemRoot->Attribute("file");
	if (filename != NULL && filename[0] != 0)
	{
		basefile = loadConfigImgFile((char*)filename, elemRoot);
	}

	TiXmlElement* elemTree;
	for (elemTree = elemRoot->FirstChildElement("plant");
		elemTree; elemTree = elemTree->NextSiblingElement("plant") )
	{
		const char* idstr = elemTree->Attribute("gameID");
		int gameID = INVALID_INDEX;
		if (idstr && idstr[0])
		{
			gameID = lookupIndexedType(idstr,plantNames);
			if (gameID == INVALID_INDEX)
			{
				contentError("No matching plant type",elemTree);
				continue;
			}
		}
		const char* deadstr = elemTree->Attribute("dead");
		bool dead = (deadstr && deadstr[0]);
		const char* saplingstr = elemTree->Attribute("sapling");
		bool sapling = (saplingstr && saplingstr[0]);

		sprite.set_by_xml(elemTree, basefile);
		VegetationConfiguration vegetationConfiguration(gameID, sprite, !dead, !sapling);
		//add a copy to known creatures
		vegetationConfigs->push_back( vegetationConfiguration );
	}

	return true;
}

c_sprite * getVegetationSprite(vector<VegetationConfiguration>& vegetationConfigs,int index,bool live,bool grown)
{
	int vcmax = (int)vegetationConfigs.size();
	for (int i=0;i<vcmax;i++)
	{
		VegetationConfiguration* current = &(vegetationConfigs[i]);
		if (current->gameID != INVALID_INDEX && current->gameID != index) continue;
		if (current->live != live) continue;
		if (current->grown != grown) continue;
		return &(current->sprite);
	}
	static c_sprite* sprite = new c_sprite;
	sprite->set_sheetindex(-1);
	return sprite;
}


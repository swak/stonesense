#include "ColorConfiguration.h"
#include "ContentLoader.h"
#include <set>

ColorMaterialConfiguration::ColorMaterialConfiguration()
{
	color = al_map_rgb(255,255,255);
	colorSet = 0;
}

ColorConfiguration::~ColorConfiguration()
{
	colorMaterials.clear();
}

void parseColorElement( TiXmlElement* elemColor, vector<ColorConfiguration> & configTable)
{
	const char* colorRedStr = elemColor->Attribute("red");
	if(colorRedStr == NULL || colorRedStr[0] == 0)
	{
		contentError("Invalid or missing color attribute",elemColor);
		return; //nothing to work with
	}
	const char* colorGreenStr = elemColor->Attribute("green");
	if(colorGreenStr == NULL || colorGreenStr[0] == 0)
	{
		contentError("Invalid or missing color attribute",elemColor);
		return; //nothing to work with
	}
	const char* colorBlueStr = elemColor->Attribute("blue");
	if(colorBlueStr == NULL || colorBlueStr[0] == 0)
	{
		contentError("Invalid or missing color attribute",elemColor);
		return; //nothing to work with
	}
	int red = atoi(colorRedStr);
	int green = atoi(colorGreenStr);
	int blue = atoi(colorBlueStr);
	ALLEGRO_COLOR color = al_map_rgb(red, green, blue);

	

	//parse material elements
	TiXmlElement* elemMaterial = elemColor->FirstChildElement("material");
	if(elemMaterial == NULL)
	{
		//if none, there's nothing to be done with this color.
		contentError("Invalid or missing material attribute",elemColor);
		return;
	}
	for( ;elemMaterial;elemMaterial = elemMaterial->NextSiblingElement("material"))
	{
		// get material type
		int elemIndex = lookupMaterialType(elemMaterial->Attribute("value"));
		if (elemIndex == INVALID_INDEX)
		{
			contentError("Invalid or missing value attribute",elemMaterial);
			continue;				
		}

		// parse subtype elements
		TiXmlElement* elemSubtype = elemMaterial->FirstChildElement("subtype");
		if (elemSubtype == NULL)
		{
			// add the configurations
			if (configTable.size() <= (uint32_t)elemIndex)
			{
				//increase size if needed
				configTable.resize(elemIndex+1);
			}
			if(configTable.at(elemIndex).colorSet == false)
			{
			configTable.at(elemIndex).color = color;
			configTable.at(elemIndex).colorSet = true;
			}
			return;
		}
		for ( ;elemSubtype; elemSubtype = elemSubtype ->NextSiblingElement("subtype"))
		{
			// get subtype
			int subtypeId = lookupMaterialIndex(elemIndex,elemSubtype->Attribute("value"));
			if (subtypeId == INVALID_INDEX)
			{
				contentError("Invalid or missing value attribute",elemSubtype);
				continue;				
			}
			
			// add the configurations
			if (configTable.size() <= (uint32_t)elemIndex)
			{
				//increase size if needed
				configTable.resize(elemIndex+1);
			}

			if (configTable.at(elemIndex).colorMaterials.size() <= (uint32_t)subtypeId)
			{
				//increase size if needed
				configTable.at(elemIndex).colorMaterials.resize(subtypeId+1);
			}
			if (configTable.at(elemIndex).colorMaterials.at(subtypeId).colorSet == false)
			{
				configTable.at(elemIndex).colorMaterials.at(subtypeId).color = color;
				configTable.at(elemIndex).colorMaterials.at(subtypeId).colorSet = true;
			}
		}
	}
}

void flushColorConfig(vector<ColorConfiguration>& config)
{
	config.clear();
}

bool addSingleColorConfig( TiXmlElement* elemRoot){
	string elementType = elemRoot->Value();
	if(elementType.compare( "colors" ) == 0){
		//parse colors
		TiXmlElement* elemColor = elemRoot->FirstChildElement("color");
		while( elemColor ){
			parseColorElement( elemColor, contentLoader.colorConfigs);
			elemColor = elemColor->NextSiblingElement("color");
		}
	}
	return true;
}
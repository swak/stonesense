#include <iostream>
#include <fstream>
#include <string>

#include "common.h"
#include "BuildingConfiguration.h"
#include "BlockCondition.h"
#include "dfhack/library/tinyxml/tinyxml.h"


void parseConditionToSprite(ConditionalSprite& sprite, TiXmlElement* elemCondition){
   const char* strType = elemCondition->Attribute("type");
   BlockCondition* cond = NULL;
	
  if( strcmp(strType, "NeighbourWall") == 0){
     cond = new NeighbourWallCondition( elemCondition->Attribute("dir") );
     		// for some reason I dont seem to need to delete these myself
     		// must investigate further
  }
  
  else if( strcmp(strType, "PositionIndex") == 0){
     cond = new PositionIndexCondition( elemCondition->Attribute("value") );
  } 
  
  else if( strcmp(strType, "MaterialType") == 0){
     cond = new MaterialTypeCondition( elemCondition->Attribute("value") );
  }   
  
  else if( strcmp(strType, "BuildingOccupancy") == 0){
     cond = new BuildingOccupancyCondition( elemCondition->Attribute("value") );
  } 
  
  else if( strcmp(strType, "NeighbourSameBuilding") == 0){
    cond = new NeighbourSameBuildingCondition( elemCondition->Attribute("dir") );
  }     
  
  else if( strcmp(strType, "NeighbourSameType") == 0){
    cond = new NeighbourSameTypeCondition( elemCondition->Attribute("dir") );
  }
  
  else if( strcmp(strType, "NeighbourOfType") == 0){
    cond = new NeighbourOfTypeCondition( elemCondition->Attribute("dir") , elemCondition->Attribute("value") );
  }
  
  else if( strcmp(strType, "NeighbourIdentical") == 0){
    cond = new NeighbourIdenticalCondition( elemCondition->Attribute("dir") );
  }   
  
  if (cond != NULL)
  {
	  sprite.conditions.push_back( cond );
  }
}

bool addSingleConfig( const char* filename,  vector<BuildingConfiguration>* knownBuildings ){
  TiXmlDocument doc( filename );
  bool loadOkay = doc.LoadFile();
  TiXmlHandle hDoc(&doc);
  TiXmlElement* elemBuilding;
  if(!loadOkay)
    return false;
  

  elemBuilding = hDoc.FirstChildElement("Building").Element();
  if( elemBuilding == 0)
    return false;

  
  const char* strName = elemBuilding->Attribute("name");
  const char* strGameID = elemBuilding->Attribute("gameID");
  
  BuildingConfiguration building(strName, (char*) strGameID );

  //for every Tile this building has
  TiXmlElement* elemTile =  elemBuilding->FirstChildElement("Tile");
  while ( elemTile ){
    
    ConditionalSprite tile;
    //Add all the sprites that compose this tile
    TiXmlElement* elemSprite =  elemTile->FirstChildElement("Sprite");
    while( elemSprite ){
      const char* strSheetIndex = elemSprite->Attribute("sheetIndex");
      const char* strOffsetX = elemSprite->Attribute("offsetx");
      const char* strOffsetY = elemSprite->Attribute("offsety");
      int sheetIndex = (strSheetIndex != 0 ? atoi(strSheetIndex) : 0);
      int offsetX    = (strOffsetX    != 0 ? atoi(strOffsetX)    : 0);
      int offsetY    = (strOffsetY    != 0 ? atoi(strOffsetY)    : 0);

      t_SpriteWithOffset sprite = { sheetIndex, offsetX, offsetY };
      tile.sprites.push_back( sprite );
      elemSprite = elemSprite->NextSiblingElement("Sprite");
    }

    const char* strCont = elemTile->Attribute("continue");
    if (strCont != NULL && strCont[0] != 0) //quick nonempty check
    {
	    tile.continuesearch=true;
    }
    
    //load conditions
    TiXmlElement* elemCondition = elemTile->FirstChildElement("Condition");
    while( elemCondition ){
      parseConditionToSprite( tile, elemCondition );
      elemCondition = elemCondition->NextSiblingElement("Condition");
    }
    //add copy of sprite to building
    building.sprites.push_back( tile );

    elemTile = elemTile->NextSiblingElement("Tile");
  }

  //add a copy of 'building' to known buildings
  knownBuildings->push_back( building );
  return true;
}


bool LoadBuildingConfiguration( vector<BuildingConfiguration>* knownBuildings ){
  string line;
  ifstream myfile ("buildings/index.txt");
  if (myfile.is_open() == false){
    WriteErr("Unable to load building config index file!\n");
    return false;
  }

  knownBuildings->clear();

  while ( !myfile.eof() )
  {
    char filepath[50] = {0};
    getline (myfile,line);
    
    //some systems don't remove the \r char as a part of the line change:
    if(line.size() > 0 &&  line[line.size() -1 ] == '\r' )
      line.resize(line.size() -1);

    if(line.size() > 0){
      sprintf(filepath, "buildings/%s", line.c_str() );
      bool result = addSingleConfig( filepath, knownBuildings );
      if( !result )
        WriteErr("Unable to load building config %s\n", filepath);
    }
  }
  myfile.close();

  BuildingNamesTranslatedFromGame = false;

  return true;
}


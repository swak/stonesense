#include "SpriteMaps.h"
#include "BuildingConfiguration.h"
#include "GameBuildings.h"
#include "ContentLoader.h"

bool BuildingNamesTranslatedFromGame = false;

BuildingConfiguration::BuildingConfiguration(string name, int gameID)
{  
  this->name = name;
  this->gameID = gameID;
  this->width = this->height = 1;
  this->canBeFloating = false;
  this->canBeAnySize = false;
  this->sprites = NULL;
  
  /*int len = (int) strlen(IDstring);
  if(len > 100) len = 100;
  memcpy(this->gameIDstr, IDstring, len);
  this->gameIDstr[len] = 0;*/
}
BuildingConfiguration::BuildingConfiguration()
{  
  this->name = "";
  this->gameID = BUILDINGTYPE_NA;
  this->width = this->height = 1;
  this->canBeFloating = false;
  this->canBeAnySize = false;
  this->sprites = NULL;
  
  /*int len = (int) strlen(IDstring);
  if(len > 100) len = 100;
  memcpy(this->gameIDstr, IDstring, len);
  this->gameIDstr[len] = 0;*/
}
BuildingConfiguration::~BuildingConfiguration(void)
{
	//cant delete bc.sprites here- screws up BCs copy semantics
}


void DumpBuildingNamesToDisk(){
  FILE* fp = fopen("buildingdump.txt", "w");
  if(!fp) return;
  for(uint32_t j=0; j < contentLoader.classIdStrings.size(); j++){
    fprintf(fp, "%i:%s\n",j, contentLoader.classIdStrings[j].c_str());
  }
  fclose(fp);
}

/*void TranslateBuildingNames( vector<BuildingConfiguration>& configs, vector<string>& buildingNames ){
  
  //for each config, find it's integer ID
  for(uint32_t i=0; i < configs.size(); i++){
    char* ptr = configs[i].gameIDstr;
    uint32_t j;
    uint32_t num =  (uint32_t)buildingNames.size();
    for(j=0; j < num; j++){
      if( strcmp( ptr, buildingNames[j].c_str()) == 0){
        //assign ID
        configs[i].gameID = j; 
        //jump to next buildingType
        break;
      }
    }
    if(j >= buildingNames.size())
      WriteErr("Unable to match building '%s' to anything in-game\n", ptr);
  }

  BuildingNamesTranslatedFromGame = true;
}*/

int TranslateBuildingName( const char* currentName, vector<string>& buildingNames )
{
  if (currentName == NULL || currentName[0]==0)
	return INVALID_INDEX;

    uint32_t j;
    uint32_t num =  (uint32_t)buildingNames.size();
    for(j=0; j < num; j++){
      if( strcmp( currentName, buildingNames[j].c_str()) == 0){
        //assign ID
        return (j);
      }
    }
    
    WriteErr("Unable to match building '%s' to anything in-game\n", currentName);
	return INVALID_INDEX;
}

int TranslateBuildingName( const char* currentName, map<uint32_t, std::string>& buildingNames )
{
  if (currentName == NULL || currentName[0]==0)
	return INVALID_INDEX;

    uint32_t j;
    uint32_t num =  (uint32_t)buildingNames.size();
    for(j=0; j < num; j++){
      if( strcmp( currentName, buildingNames[j].c_str()) == 0){
        //assign ID
        return (j);
      }
    }
    
    WriteErr("Unable to match building '%s' to anything in-game\n", currentName);
	return INVALID_INDEX;
}

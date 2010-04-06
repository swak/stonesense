#include "common.h"
#include "CreatureConfiguration.h"
#include "Creatures.h"
#include "MapLoading.h"
#include "GUI.h"
#include "ContentLoader.h"

#include "dfhack/library/tinyxml/tinyxml.h"


CreatureConfiguration::CreatureConfiguration(int professionID, const char* professionStr, enumCreatureSex sex, enumCreatureSpecialCases special, t_SpriteWithOffset &sprite, int shadow)
{
  memset(this, 0, sizeof(CreatureConfiguration) );
  this->sprite = sprite;
  this->professionID = professionID;
  this->sex = sex;
  this->shadow = shadow;
  this->special = special;
  
  if(professionStr){
    int len = (int) strlen(professionStr);
    if(len > CREATURESTRLENGTH) len = CREATURESTRLENGTH;
    memcpy(this->professionstr, professionStr, len);
    this->professionstr[CREATURESTRLENGTH-1]=0;
  }
  //WriteErr("CC %d %d %d %d %s\n",this->gameID,this->professionID,this->sprite.fileIndex,this->sprite.sheetIndex,(this->professionstr[0]?this->professionstr:"-"));
}

CreatureConfiguration::~CreatureConfiguration(void)
{
}

void DumpCreatureNamesToDisk(){
  /*FILE* fp = fopen("dump.txt", "w");
  if(!fp) return;
  for(uint32_t j=0; j < v_creatureNames.size(); j++){
    fprintf(fp, "%i:%s\n",j, v_creatureNames[j].id);
  }
  fclose(fp);*/
}
void DumpProfessionsToDisk(){
  FILE* fp = fopen("dump.txt", "w");
  if(!fp) return;
  string proffStr;
  for(int j=0; (proffStr = dfMemoryInfo.getProfession(j)) != "" ; j++){
    fprintf(fp, "%i:%s\n",j, proffStr.c_str());
  }
  fclose(fp);
}

int translateProfession(const char* currentProf)
{
	if (currentProf == NULL || currentProf[0]==0)
		return INVALID_INDEX;
    uint32_t j;
    string proffStr;
    for(j=0; (proffStr = dfMemoryInfo.getProfession(j)) != "" ; j++)
    {   
      if( proffStr.compare( currentProf ) == 0)
      {
        //assign ID
        return j;
      }
    }
	WriteErr("Unable to match profession '%s' to anything in-game\n", currentProf);
	return INT_MAX; //if it is left at INVALID_INDEX, the condition is ignored entierly.
}

void pushCreatureConfig( vector<vector<CreatureConfiguration>*>& knownCreatures, int gameID, CreatureConfiguration& cre)
{
	vector<CreatureConfiguration>* creatureList;
	if (knownCreatures.size() <= gameID)
	{
		//resize using hint from creature name list
		int newsize = gameID +1;
		if (newsize <= contentLoader.creatureNameStrings.size())
		{
			newsize = contentLoader.creatureNameStrings.size() + 1;
		}
		knownCreatures.resize(newsize);
	}
	creatureList = knownCreatures[gameID];
	if (creatureList == NULL)
	{
		creatureList = new vector<CreatureConfiguration>();
		knownCreatures[gameID]=creatureList;
	}
	creatureList->push_back(cre);
}

bool addSingleCreatureConfig( TiXmlElement* elemCreature, vector<vector<CreatureConfiguration>*>& knownCreatures, int basefile ){
  int gameID = lookupIndexedType(elemCreature->Attribute("gameID"),contentLoader.creatureNameStrings);
  if (gameID == INVALID_INDEX)
  	return false;
  const char* sheetIndexStr;
  t_SpriteWithOffset sprite;
  sprite.fileIndex=basefile;
  sprite.x=0;
  sprite.y=0;
  sprite.animFrames=ALL_FRAMES;
  int baseShadow = DEFAULT_SHADOW;
  const char* shadowStr = elemCreature->Attribute("shadow");
  if (shadowStr != NULL && shadowStr[0] != 0)
  {
	baseShadow = atoi( shadowStr );	  
  }
  if (baseShadow < 0 || baseShadow > MAX_SHADOW)
  	baseShadow = DEFAULT_SHADOW;
  const char* filename = elemCreature->Attribute("file");
	if (filename != NULL && filename[0] != 0)
	{
	  	sprite.fileIndex = loadConfigImgFile((char*)filename,elemCreature);
	}
  TiXmlElement* elemVariant = elemCreature->FirstChildElement("variant");
  while( elemVariant ){
	int professionID = INVALID_INDEX;
    const char* profStr = elemVariant->Attribute("prof");
    if (profStr == NULL || profStr[0] == 0)
    {
	    profStr = elemVariant->Attribute("profession");
    }
   	professionID = translateProfession(profStr);

    const char* customStr = elemVariant->Attribute("custom");
    if (customStr != NULL && customStr[0] == 0)
    {
	    customStr = NULL;
    } 
      
	if (customStr != NULL)
	{
		WriteErr("custom: %s\n",customStr);	
	}
    
    const char* sexstr = elemVariant->Attribute("sex");
    sheetIndexStr = elemVariant->Attribute("sheetIndex");
    enumCreatureSex cresex = eCreatureSex_NA;
    if(sexstr){
      if(strcmp( sexstr, "M" ) == 0) cresex = eCreatureSex_Male;
      if(strcmp( sexstr, "F" ) == 0) cresex = eCreatureSex_Female;
    }
    const char* specstr = elemVariant->Attribute("special");
    enumCreatureSpecialCases crespec = eCSC_Any;
    if (specstr)
    {
      if(strcmp( specstr, "Normal" ) == 0) crespec = eCSC_Normal;
      if(strcmp( specstr, "Zombie" ) == 0) crespec = eCSC_Zombie;	      
      if(strcmp( specstr, "Skeleton" ) == 0) crespec = eCSC_Skeleton;	      
    }
    sprite.animFrames = getAnimFrames(elemVariant->Attribute("frames"));
	if (sprite.animFrames == 0)
		sprite.animFrames = ALL_FRAMES;

	int shadow = baseShadow;
	const char* shadowStr = elemVariant->Attribute("shadow");
	if (shadowStr != NULL && shadowStr[0] != 0)
	{
		shadow = atoi( shadowStr );	  
	}
	if (shadow < 0 || shadow > MAX_SHADOW)
		shadow = baseShadow;
		    
    //create profession config
    sprite.sheetIndex=atoi(sheetIndexStr);
    CreatureConfiguration cre( professionID, customStr , cresex, crespec, sprite, shadow);
    //add a copy to known creatures
    pushCreatureConfig(knownCreatures, gameID, cre);

    elemVariant = elemVariant->NextSiblingElement("variant");
  }

  //create default config
  baseShadow;
  sheetIndexStr = elemCreature->Attribute("sheetIndex");
  sprite.animFrames = ALL_FRAMES;
  if (sheetIndexStr)
  {
	sprite.sheetIndex = atoi( sheetIndexStr );
    CreatureConfiguration cre( INVALID_INDEX, NULL, eCreatureSex_NA, eCSC_Any, sprite, baseShadow);
  	//add a copy to known creatures
    pushCreatureConfig(knownCreatures, gameID, cre);
  }
  return true;
}

bool addCreaturesConfig( TiXmlElement* elemRoot, vector<vector<CreatureConfiguration>*>& knownCreatures ){
  int basefile = -1;
  const char* filename = elemRoot->Attribute("file");
  if (filename != NULL && filename[0] != 0)
  {
	basefile = loadConfigImgFile((char*)filename,elemRoot);
  } 
  TiXmlElement* elemCreature = elemRoot->FirstChildElement("creature");
  if (elemCreature == NULL)
  {
     contentError("No creatures found",elemRoot);
     return false;
  }
  while( elemCreature ){
	addSingleCreatureConfig(elemCreature,knownCreatures,basefile );
	elemCreature = elemCreature->NextSiblingElement("creature");
  }
  return true;
}
	
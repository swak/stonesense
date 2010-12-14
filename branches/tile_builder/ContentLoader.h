#pragma once
#include "dfhack/depends/tinyxml/tinyxml.h"
#include "ColorConfiguration.h"
#include "commonTypes.h"

class ContentLoader{
private:
	bool parseContentIndexFile( char* filepath );
	bool parseContentXMLFile( char* filepath );
	bool parseColorContent( TiXmlElement* elemRoot );

	bool translationComplete;
public:
	ContentLoader(void);
	~ContentLoader(void);

	bool Load( DFHack::Context& DF);

	vector<ColorConfiguration> colorConfigs;

	vector<string> classIdStrings;
	vector<string> professionStrings;
	VersionInfo *MemInfo;
	DFHack::Buildings * Bld;
	DFHack::Materials * Mats;

	uint32_t currentTick;
	uint32_t currentYear;
	uint8_t currentMonth;
	uint8_t currentDay;
	uint8_t currentHour;
	uint8_t currentTickRel;

	int obsidian;
};

//singleton instance
extern ContentLoader contentLoader;

extern const char* getDocument(TiXmlNode* element);
extern void contentError(const char* message, TiXmlNode* element);
extern int loadConfigImgFile(const char* filename, TiXmlElement* referrer);
int lookupMaterialType(const char* strValue);
int lookupMaterialIndex(int matType, const char* strValue);
int lookupIndexedType(const char* indexName, vector<t_matgloss>& typeVector);
const char *lookupMaterialTypeName(int matType);
const char *lookupMaterialName(int matType,int matIndex);
uint8_t lookupMaterialFore(int matType,int matIndex);
uint8_t lookupMaterialBack(int matType,int matIndex);
uint8_t lookupMaterialBright(int matType,int matIndex);
const char *lookupTreeName(int matIndex);
ALLEGRO_COLOR lookupMaterialColor(int matType,int matIndex);
const char * lookupFormName(int formType);
ShadeBy getShadeType(const char* Input);

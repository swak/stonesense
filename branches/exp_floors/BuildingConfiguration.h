#pragma once
#include "common.h"
#include "ConditionalSprite.h"


class BuildingConfiguration
{
public:
  char gameIDstr[100];
  uint32_t gameID;
  uint32_t width, height;
  string name;
  bool canBeFloating;
  bool canBeAnySize;
  vector<ConditionalSprite> sprites;

  BuildingConfiguration(string name, char* IDstring);
  ~BuildingConfiguration(void);
};


extern bool BuildingNamesTranslatedFromGame;

void TranslateBuildingNames();
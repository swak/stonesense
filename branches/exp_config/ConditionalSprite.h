#pragma once

#include "BlockCondition.h"

class ConditionalSprite : public ConditionalNode
{
private:

public:
  vector<t_SpriteWithOffset> sprites;
  BlockCondition* conditions;
  bool continuesearch;
  
  ConditionalSprite(void);
  ~ConditionalSprite(void){};

  bool BlockMatches(Block* b);
  void addChild(BlockCondition* cond);
};

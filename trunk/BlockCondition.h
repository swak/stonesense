#pragma once
#include "common.h"
#include "Block.h"

enum BlockConditionTypes{
  Cond_MaterialType,
  Cond_MaterialIndex,
  Cond_NeighbourWall,
  Cond_NeighbourSameBuilding,
  Cond_PositionIndex,
  Cond_NeighbourIdentical,
  
};

class BlockCondition
{
public:
  BlockConditionTypes type;
  int value;

  BlockCondition(BlockConditionTypes type);
  ~BlockCondition(void){}

  bool Matches(Block* b);
};

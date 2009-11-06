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
  Cond_BuildingOcc,
  Cond_NeighbourSameIndex,
    
};

// abstract base class
class BlockCondition
{
public:
  BlockCondition(){};
  ~BlockCondition(void){};

  virtual bool Matches(Block* b) = 0;
};


class NeighbourWallCondition : public BlockCondition
{
  public:
  NeighbourWallCondition(const char* strValue);
  ~NeighbourWallCondition(void){};

  int value;
  bool Matches(Block* b);
};


class PositionIndexCondition : public BlockCondition
{
  public:
  PositionIndexCondition(const char* strValue);
  ~PositionIndexCondition(void){};

  int value;
  bool Matches(Block* b);
};


class MaterialTypeCondition : public BlockCondition
{
  public:
  MaterialTypeCondition(const char* strValue);
  ~MaterialTypeCondition(void){};

  int value;
  bool Matches(Block* b);
};


class BuildingOccupancyCondition : public BlockCondition
{
  public:
  BuildingOccupancyCondition(const char* strValue);
  ~BuildingOccupancyCondition(void){};

  int value;
  bool Matches(Block* b);
};


class NeighbourSameBuildingCondition : public BlockCondition
{
  public:
  NeighbourSameBuildingCondition(const char* strValue);
  ~NeighbourSameBuildingCondition(void){};

  int value;
  bool Matches(Block* b);
};


class NeighbourIdenticalCondition : public BlockCondition
{
  public:
  NeighbourIdenticalCondition(const char* strValue);
  ~NeighbourIdenticalCondition(void){};

  int value;
  bool Matches(Block* b);
};


class NeighbourOfTypeCondition : public BlockCondition
{
  public:
  NeighbourOfTypeCondition(const char* strDir, const char* strValue);
  ~NeighbourOfTypeCondition(void){};

  int value;
  int direction;
  bool Matches(Block* b);
};

class NeighbourSameTypeCondition : public BlockCondition
{
  public:
  NeighbourSameTypeCondition(const char* strDir);
  ~NeighbourSameTypeCondition(void){};

  int direction;
  bool Matches(Block* b);
};


#include "BlockCondition.h"
#include "GameBuildings.h"
#include "WorldSegment.h"
#include <iostream>

int getDirectionFromString(const char* strDir)
{
	if (strDir == NULL)
		return INVALID_INDEX;
	if( strcmp(strDir, "None") == 0)
	  return eSimpleSingle;
	if( strcmp(strDir, "North") == 0)
	  return eSimpleN;
	if( strcmp(strDir, "South") == 0)
	  return eSimpleS;
	if( strcmp(strDir, "West") == 0)
	  return eSimpleW;
	if( strcmp(strDir, "East") == 0)
	  return eSimpleE;
	return INVALID_INDEX;	
}

int getBuildingFromString(const char* strType)
{
	for (uint32_t i=0; i<v_buildingtypes.size(); i++){
		cout << v_buildingtypes[i] << " <- " << i << endl;
		if (v_buildingtypes[i].compare(strType) == 0)
		{
			return i;
		}
	}
	return INVALID_INDEX;	
}

NeighbourWallCondition::NeighbourWallCondition(const char* strDir)
	: BlockCondition()
{
	cout << "nwall " << strDir << endl;
	this->value = getDirectionFromString(strDir);
}

bool NeighbourWallCondition::Matches(Block* b)
{
cout << "a" << endl;	
	bool n = hasWall( b->ownerSegment->getBlock( b->x, b->y - 1, b->z ) );
    bool s = hasWall( b->ownerSegment->getBlock( b->x, b->y + 1, b->z ) );
    bool w = hasWall( b->ownerSegment->getBlock( b->x - 1, b->y, b->z ) );
    bool e = hasWall( b->ownerSegment->getBlock( b->x + 1, b->y, b->z ) );
    cout << value << " to  : " << " " << n << " " << s << " " << e << " " << w << endl;	
    
    if( value == eSimpleN && n) return true;
    if( value == eSimpleS && s) return true;
    if( value == eSimpleW && w) return true;
    if( value == eSimpleE && e) return true;
    
    if( value == eSimpleSingle && !n && !s && !w && !e) return true;

    return false;
}


PositionIndexCondition::PositionIndexCondition(const char* strValue)
	: BlockCondition()
{
    this->value = atoi( strValue );
}

bool PositionIndexCondition::Matches(Block* b)
{
cout << "b" << endl;	
    int x = b->x - b->building.info.x1;
    int y = b->y - b->building.info.y1;
    int w = b->building.info.x2 - b->building.info.x1 + 1 ;
    int pos = y * w + x;

    return pos == this->value;
}




MaterialTypeCondition::MaterialTypeCondition(const char* strValue)
	: BlockCondition()
{
    this->value = atoi( strValue );
}

bool MaterialTypeCondition::Matches(Block* b)
{
cout << "c" << endl;
    return b->building.info.material.type == this->value;
}



BuildingOccupancyCondition::BuildingOccupancyCondition(const char* strValue)
	: BlockCondition()
{
    this->value = atoi( strValue );
}

bool BuildingOccupancyCondition::Matches(Block* b)
{
cout << "d" << endl;	
    return b->occ.bits.building == this->value;
}



NeighbourSameBuildingCondition::NeighbourSameBuildingCondition(const char* strDir)
	: BlockCondition()
{
	this->value = getDirectionFromString(strDir);
}

bool NeighbourSameBuildingCondition::Matches(Block* b)
{
cout << "e" << endl;	
    int blocksBuildingIndex = b->building.index;

    bool n = hasBuildingOfIndex( b->ownerSegment->getBlock( b->x, b->y - 1, b->z ), blocksBuildingIndex );
    bool s = hasBuildingOfIndex( b->ownerSegment->getBlock( b->x, b->y + 1, b->z ), blocksBuildingIndex );
    bool w = hasBuildingOfIndex( b->ownerSegment->getBlock( b->x - 1, b->y, b->z ), blocksBuildingIndex );
    bool e = hasBuildingOfIndex( b->ownerSegment->getBlock( b->x + 1, b->y, b->z ), blocksBuildingIndex );

    if( value == eSimpleN && n) return true;
    if( value == eSimpleS && s) return true;
    if( value == eSimpleW && w) return true;
    if( value == eSimpleE && e) return true;
    
    if( value == eSimpleSingle && !n && !s && !w && !e) return true;
    
    return false;
}



NeighbourIdenticalCondition::NeighbourIdenticalCondition(const char* strDir)
	: BlockCondition()
{
	this->value = getDirectionFromString(strDir);
}

bool NeighbourIdenticalCondition::Matches(Block* b)
{
cout << "f" << endl;	
    int blocksBuildingIndex = b->building.index;
    int blocksBuildingOcc = b->occ.bits.building;

    bool n = hasBuildingIdentity( b->ownerSegment->getBlock( b->x, b->y - 1, b->z ), blocksBuildingIndex, blocksBuildingOcc );
    bool s = hasBuildingIdentity( b->ownerSegment->getBlock( b->x, b->y + 1, b->z ), blocksBuildingIndex, blocksBuildingOcc );
    bool w = hasBuildingIdentity( b->ownerSegment->getBlock( b->x - 1, b->y, b->z ), blocksBuildingIndex, blocksBuildingOcc );
    bool e = hasBuildingIdentity( b->ownerSegment->getBlock( b->x + 1, b->y, b->z ), blocksBuildingIndex, blocksBuildingOcc );

    if( value == eSimpleN && n) return true;
    if( value == eSimpleS && s) return true;
    if( value == eSimpleW && w) return true;
    if( value == eSimpleE && e) return true;
    
    if( value == eSimpleSingle && !n && !s && !w && !e) return true;
    
    return false;
}


NeighbourOfTypeCondition::NeighbourOfTypeCondition(const char* strDir, const char* strType)
	: BlockCondition()
{
	this->direction = getDirectionFromString(strDir);
	this->value = getBuildingFromString(strType);
}

bool NeighbourOfTypeCondition::Matches(Block* b)
{
 cout << "g" << endl;	
   bool n = hasBuildingOfID( b->ownerSegment->getBlock( b->x, b->y - 1, b->z ), value );
    bool s = hasBuildingOfID( b->ownerSegment->getBlock( b->x, b->y + 1, b->z ), value );
    bool w = hasBuildingOfID( b->ownerSegment->getBlock( b->x - 1, b->y, b->z ), value );
    bool e = hasBuildingOfID( b->ownerSegment->getBlock( b->x + 1, b->y, b->z ), value );
    
    if( direction == eSimpleN && n) return true;
    if( direction == eSimpleS && s) return true;
    if( direction == eSimpleW && w) return true;
    if( direction == eSimpleE && e) return true;
    
    if( direction == eSimpleSingle && !n && !s && !w && !e) return true;
    
    return false;
}

NeighbourSameTypeCondition::NeighbourSameTypeCondition(const char* strDir)
	: BlockCondition()
{
	this->direction = getDirectionFromString(strDir);
}

bool NeighbourSameTypeCondition::Matches(Block* b)
{
cout << "h" << endl;	
	int value = b->building.info.type;
	
    bool n = hasBuildingOfID( b->ownerSegment->getBlock( b->x, b->y - 1, b->z ), value );
    bool s = hasBuildingOfID( b->ownerSegment->getBlock( b->x, b->y + 1, b->z ), value );
    bool w = hasBuildingOfID( b->ownerSegment->getBlock( b->x - 1, b->y, b->z ), value );
    bool e = hasBuildingOfID( b->ownerSegment->getBlock( b->x + 1, b->y, b->z ), value );
    
    if( direction == eSimpleN && n) return true;
    if( direction == eSimpleS && s) return true;
    if( direction == eSimpleW && w) return true;
    if( direction == eSimpleE && e) return true;
    
    if( direction == eSimpleSingle && !n && !s && !w && !e) return true;
    
    return false;
}

bool AndConditionalNode::Matches(Block* b)
{
cout << "i" << endl;	
	int max = children.size();
	for(uint32_t i=0; i<max; i++)
	{
		cout << "i:" << i << " of " << max << endl;
		if (!children[i]->Matches( b ))
			return false;
	}
	cout << "i ok" << endl;
	return true;
}

void AndConditionalNode::addChild(BlockCondition* cond)
{
	children.push_back(cond);
}

bool OrConditionalNode::Matches(Block* b)
{
cout << "j" << endl;	
	int max = children.size();
	for(uint32_t i=0; i<max; i++)
	{
		cout << "j:" << i << " of " << max << endl;
		if (children[i]->Matches( b ))
			return true;
	}
	cout << "j ok" << endl;
	return false;
}

void OrConditionalNode::addChild(BlockCondition* cond)
{
	children.push_back(cond);
}

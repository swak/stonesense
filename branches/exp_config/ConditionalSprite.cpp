#include "common.h"
#include "ConditionalSprite.h"
#include "Block.h"
#include "GameBuildings.h"

ConditionalSprite::ConditionalSprite(void)
{
  continuesearch = false;
  conditions = NULL;
}

bool ConditionalSprite::BlockMatches(Block* b)
{
	//cout << "cs.bm +" << endl;
	if (conditions == NULL)
	{
		//cout << "cs.bm - !c" << endl;	
		return true;	
	}
	if (b == NULL)
	{
		//cout << "cs.bm - !b" << endl;
		return false;	  	
	}	 
	
	//cout << "cs.bm .." << endl;	 
	bool t = conditions->Matches( b );
	cout << "cs.bm - " << t << endl;	 
	return t;
}

void ConditionalSprite::addChild(BlockCondition* cond){
	conditions = cond;
}
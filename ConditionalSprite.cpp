#include "common.h"
#include "ConditionalSprite.h"
#include "Block.h"
#include "GameBuildings.h"


SpriteBlock::SpriteBlock()
	: ConditionalNode(), SpriteNode()
{
	conditions = NULL;
	elsenode = NULL;
}

bool SpriteBlock::BlockMatches(Block* b)
{
	bool condMatch = false;
	if (conditions == NULL)
	{
		condMatch = true;	
	}
	else
	{
		condMatch = conditions->Matches( b );
	}
	
	bool haveMatch=false;
	if (condMatch)
	{
		uint32_t max = children.size();
		for(uint32_t i=0; i<max; i++)
		{
			if (children[i]->BlockMatches(b))
			{
				haveMatch = true;	
			}
		}
	}
	else if (elsenode != NULL)
	{
		haveMatch = elsenode->BlockMatches(b);
	}
	return haveMatch;
}

void SpriteBlock::addCondition(BlockCondition* cond){
	conditions = cond;
}
void SpriteBlock::addChild(SpriteNode* child){
	children.push_back(child);
}
void SpriteBlock::addElse(SpriteNode* child){
	elsenode = child;
}

bool RootBlock::BlockMatches(Block* b)
{
	bool haveMatch = false;
	uint32_t max = children.size();
	
	for(uint32_t i=0; i<max; i++)
	{
		if (children[i]->BlockMatches(b))
		{
			haveMatch = true;	
		}
	}
	return haveMatch;
}
void RootBlock::addChild(SpriteNode* child){
	children.push_back(child);
}


SpriteElement::SpriteElement()
	: SpriteNode()
{
	sprite.sheetIndex = -1;
	sprite.x = 0;
	sprite.y = 0;
}

bool SpriteElement::BlockMatches(Block* b)
{
	if (sprite.sheetIndex > -1)
	{
		b->building.sprites.push_back(sprite);
	}
	return true;
}

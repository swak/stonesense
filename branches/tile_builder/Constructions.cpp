#include "common.h"
#include "Constructions.h"
#include "WorldSegment.h"

void changeConstructionMaterials(WorldSegment* segment, vector<t_construction>* allConstructions){
	//find construction
	int32_t i;
	Block* b;
	t_construction* construct = 0;
	i = (uint32_t) allConstructions->size();
	if(i <= 0) return;
	while(--i >= 0){
		construct = &(*allConstructions)[i];	
		b = segment->getBlock(construct->x, construct->y, construct->z);
		if( !b ) continue;
		//don't assign invalid material indexes
		//if(construct->mat_idx != -1){
		//on second thought, invalid indices are needed.
		b->material.type = construct->mat_type;
		b->material.index = construct->mat_idx;
		//}
		b->consForm = construct->form;
	}
}
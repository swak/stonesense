#include "common.h"
#include "Creatures.h"
#include "WorldSegment.h"
#include "CreatureConfiguration.h"
#include "ContentLoader.h"
#include "GUI.h"

extern ALLEGRO_FONT *font;

//vector<t_matgloss> v_creatureNames;
//vector<CreatureConfiguration> creatureTypes;



bool IsCreatureVisible( t_creature* c ){
  if( config.show_all_creatures ) return true;

  if( c->flags1.bits.dead )
    return false;
  if( c->flags1.bits.caged )
    return false;
  if( c->flags1.bits.hidden_in_ambush )
    return false;
  return true;
}

void DrawCreature(int drawx, int drawy, t_creature* creature ){
	t_SpriteWithOffset sprite = GetCreatureSpriteMap( creature );
	//if(creature->x == 151 && creature->y == 145)
	//  int j = 10;

	ALLEGRO_BITMAP* creatureSheet;
	if (sprite.fileIndex == -1)
	{
		creatureSheet = IMGCreatureSheet;
	}
	else
	{
		creatureSheet = getImgFile(sprite.fileIndex);
	}    	
	DrawSpriteFromSheet( sprite.sheetIndex, creatureSheet, drawx, drawy );
}
void DrawCreatureText(int drawx, int drawy, t_creature* creature ){
	//if(creature->x == 151 && creature->y == 145)
	//  int j = 10;
	if( config.show_creature_names ){
		if (creature->nick_name[0] && config.names_use_nick)
		{
			al_draw_textf(font, drawx, drawy-20, 0, "%s", creature->nick_name);
		}
		else if (creature->first_name[0])
		{
			char buffer[128];
			strncpy(buffer,creature->first_name,127);
			buffer[127]=0;
			if (buffer[0]>90)
				buffer[0] -= 32;
			al_draw_textf(font, drawx, drawy-20, 0, "%s", buffer );
		}
		else if (config.names_use_species)
		{
			al_draw_textf(font, drawx, drawy-20, 0, "[%s]", contentLoader.creatureNameStrings.at(creature->type).id);
		}
	}
}
//t_creature* global = 0;

void ReadCreaturesToSegment(API& DF, WorldSegment* segment)
{
  int x1 = segment->x;
  int x2 = segment->x + segment->sizex;
  int y1 = segment->y;
  int y2 = segment->y + segment->sizey;
  int z1 = segment->z;
  int z2 = segment->z + segment->sizez;
	uint32_t numcreatures = DF.InitReadCreatures();
	
  if(x1<0) x1=0;
  if(y1<0) y1=0;
  if(z1<0) z1=0;
  if(x2<0) x2=0;
  if(y2<0) y2=0;
  if(z2<0) z2=0;

	t_creature *tempcreature = new t_creature();
	/*for (uint32_t index = 0; index < numcreatures ; index++)
	{
		DF.ReadCreature( index, *tempcreature );*/
  uint32_t index = 0;
	while((index = DF.ReadCreatureInBox( index, *tempcreature, x1,y1,z1,x2,y2,z2)) != -1 )
  {
    index++;
		if( IsCreatureVisible( tempcreature ) )
		{
			Block* b = segment->getBlock (tempcreature->x, tempcreature->y, tempcreature->z );
			if(!b)
			{
				//inside segment, but no block to represent it
				b = new Block(segment);
				b->x = tempcreature->x;
				b->y = tempcreature->y;
				b->z = tempcreature->z;
				// fake block occupancy where needed. This is starting to get hacky...
				b->occ.bits.unit=1;
				segment->addBlock( b );
			}
			if (!b->creature)
			{
				b->creature = tempcreature;
				// add shadow to nearest floor block
				for (int bz = tempcreature->z;bz>=z1;bz--)
				{
					b = segment->getBlock (tempcreature->x, tempcreature->y, bz );
					if (!b) continue;
					if (b->floorType > 0 || b->wallType > 0 || b->ramp.type > 0)
					{
						// todo figure out appropriate shadow size
						int tempShadow = GetCreatureShadowMap( tempcreature );
						if (b->shadow < tempShadow)
							b->shadow=tempShadow;
						break;	
					}
				}
				// need a new tempcreature now
				// old tempcreature should be deleted when b is
				tempcreature = new t_creature();
			}
		}
	}
	delete(tempcreature); // there will be one left over
	DF.FinishReadCreatures();
}


CreatureConfiguration *GetCreatureConfig( t_creature* c ){
	//find list for creature type
	vector<CreatureConfiguration>* creatureData;
	uint32_t num = (uint32_t)contentLoader.creatureConfigs.size();
	if (c->type >= num)
	{
		return NULL;	
	}
	creatureData = contentLoader.creatureConfigs[c->type];
	if (creatureData == NULL)
	{
		return NULL;
	}
	
	//search list for given creature variant
	int offsetAnimFrame = (currentAnimationFrame + c->id) % MAX_ANIMFRAME;
	num = (uint32_t)creatureData->size();
	for(uint32_t i=0; i < num; i++)
	{
		CreatureConfiguration *testConfig = &((*creatureData)[i]);
		     
		bool creatureMatchesJob = true;
		if( testConfig->professionID != INVALID_INDEX )
		{
			creatureMatchesJob = testConfig->professionID == c->profession;
		}
		if(!creatureMatchesJob) continue;
		
		bool creatureMatchesSex = true;
		if( testConfig->sex != eCreatureSex_NA )
		{
			creatureMatchesSex = 
				(c->sex == 0 &&  testConfig->sex == eCreatureSex_Female) ||
				(c->sex == 1 &&  testConfig->sex == eCreatureSex_Male);
		}
		if(!creatureMatchesSex) continue;
		
		bool creatureMatchesSpecial = true;
		if (testConfig->special != eCSC_Any)
		{
		 	if (testConfig->special == eCSC_Zombie && !c->flags1.bits.zombie) creatureMatchesSpecial = false;
		 	if (testConfig->special == eCSC_Skeleton && !c->flags1.bits.skeleton) creatureMatchesSpecial = false;
		 	if (testConfig->special == eCSC_Normal && (c->flags1.bits.zombie || c->flags1.bits.skeleton)) creatureMatchesSpecial = false;
		}
		if(!creatureMatchesSpecial) continue;
			
		if (!(testConfig->sprite.animFrames & (1 << offsetAnimFrame)))
			continue;
		
		// dont try to match strings until other tests pass
		if( testConfig->professionstr[0])
		{ //cant be NULL, so check has length
			creatureMatchesJob = (strcmp(testConfig->professionstr,c->custom_profession)==0);
		}
		if(!creatureMatchesJob) continue;
			
		return testConfig;
	}
	return NULL;
}


t_SpriteWithOffset GetCreatureSpriteMap( t_creature* c )
{
	CreatureConfiguration *testConfig = GetCreatureConfig( c );
	if (testConfig == NULL)
		return spriteCre_NA;
	return testConfig->sprite;
}

int GetCreatureShadowMap( t_creature* c )
{
	CreatureConfiguration *testConfig = GetCreatureConfig( c );
	if (testConfig == NULL)
		return 4;
	return testConfig->shadow;
}
void generateCreatureDebugString2( t_creature* c, char* strbuffer){
	if(c->flags2.bits.swimming)
		strcat(strbuffer, "swimming ");
	if(c->flags2.bits.sparring)
		strcat(strbuffer, "sparring ");
	if(c->flags2.bits.no_notify) // Do not notify about level gains (for embark etc)
		strcat(strbuffer, "no_notify ");
	if(c->flags2.bits.unused)
		strcat(strbuffer, "unused ");

	if(c->flags2.bits.calculated_nerves)
		strcat(strbuffer, "calculated_nerves ");
	if(c->flags2.bits.calculated_bodyparts)
		strcat(strbuffer, "calculated_bodyparts ");
	if(c->flags2.bits.important_historical_figure) // slight variation
		strcat(strbuffer, "Historical ");
	if(c->flags2.bits.killed) // killed by kill() function
		strcat(strbuffer, "killed ");

	if(c->flags2.bits.cleanup_1) // Must be forgotten by forget function (just cleanup)
		strcat(strbuffer, "cleanup_1 ");
	if(c->flags2.bits.cleanup_2) // Must be deleted (cleanup)
		strcat(strbuffer, "cleanup_2 ");
	if(c->flags2.bits.cleanup_3) // Recently forgotten (cleanup)
		strcat(strbuffer, "cleanup_3 ");
	if(c->flags2.bits.for_trade) // Offered for trade
		strcat(strbuffer, "for_trade ");

	if(c->flags2.bits.trade_resolved)
		strcat(strbuffer, "trade_resolved ");
	if(c->flags2.bits.has_breaks)
		strcat(strbuffer, "has_breaks ");
	if(c->flags2.bits.gutted)
		strcat(strbuffer, "gutted ");
	if(c->flags2.bits.circulatory_spray)
		strcat(strbuffer, "circulatory_spray ");

	if(c->flags2.bits.locked_in_for_trading)
		strcat(strbuffer, "locked_in_for_trading ");
	if(c->flags2.bits.slaughter) // marked for slaughter
		strcat(strbuffer, "slaughter ");
	if(c->flags2.bits.underworld) // Underworld creature
		strcat(strbuffer, "underworld ");
	if(c->flags2.bits.resident) // Current resident
		strcat(strbuffer, "resident ");

	if(c->flags2.bits.cleanup_4) // Marked for special cleanup as unused load from unit block on disk
		strcat(strbuffer, "cleanup_4 ");
	if(c->flags2.bits.calculated_insulation) // Insulation from clothing calculated
		strcat(strbuffer, "calculated_insulation ");
	if(c->flags2.bits.visitor_uninvited) // Uninvited guest
		strcat(strbuffer, "visitor_uninvited ");
	if(c->flags2.bits.visitor) // visitor
		strcat(strbuffer, "visitor ");

	if(c->flags2.bits.calculated_inventory) // Inventory order calculated
		strcat(strbuffer, "calculated_inventory ");
	if(c->flags2.bits.vision_good) // Vision -- have good part
		strcat(strbuffer, "goodEyes ");
	if(c->flags2.bits.vision_damaged) // Vision -- have damaged part
		strcat(strbuffer, "damagedEyes ");
	if(c->flags2.bits.vision_missing) // Vision -- have missing part
		strcat(strbuffer, "noEyes ");

	if(c->flags2.bits.breathing_good) // Breathing -- have good part
		strcat(strbuffer, "canBreathe ");
	if(c->flags2.bits.breathing_problem) // Breathing -- having a problem
		strcat(strbuffer, "can'tBreathe ");
	if(c->flags2.bits.roaming_wilderness_population_source)
		strcat(strbuffer, "roaming_wilderness_population_source ");
	if(c->flags2.bits.roaming_wilderness_population_source_not_a_map_feature)
		strcat(strbuffer, "roaming_wilderness_population_source_not_a_map_feature ");
}

void generateCreatureDebugString( t_creature* c, char* strbuffer){
  if(c->flags1.bits.active_invader)
    strcat(strbuffer, "activeInvader ");
  if(c->flags1.bits.caged)
    strcat(strbuffer, "Caged ");
  if(c->flags1.bits.chained)
    strcat(strbuffer, "chained ");
  if(c->flags1.bits.coward)
    strcat(strbuffer, "coward ");
   if(c->flags1.bits.dead)
    strcat(strbuffer, "Dead ");
  if(c->flags1.bits.diplomat)
    strcat(strbuffer, "Diplomat ");
  if(c->flags1.bits.drowning)
    strcat(strbuffer, "drowning ");
  if(c->flags1.bits.forest)
    strcat(strbuffer, "lostLeaving ");
  if(c->flags1.bits.fortress_guard)
    strcat(strbuffer, "FortGuard ");
  if(c->flags1.bits.had_mood)
    strcat(strbuffer, "HadMood ");
  if(c->flags1.bits.has_mood)
    strcat(strbuffer, "Mood ");
  if(c->flags1.bits.hidden_ambusher)
    strcat(strbuffer, "hiddenAmbush ");
  if(c->flags1.bits.hidden_in_ambush)
    strcat(strbuffer, "hiddenInAmbush ");
  if(c->flags1.bits.important_historical_figure)
    strcat(strbuffer, "Historical ");
  if(c->flags1.bits.incoming)
    strcat(strbuffer, "Incoming ");
  if(c->flags1.bits.invades)
    strcat(strbuffer, "invading ");
  if(c->flags1.bits.marauder)
    strcat(strbuffer, "marauder ");
  if(c->flags1.bits.merchant)
    strcat(strbuffer, "merchant ");
  if(c->flags1.bits.on_ground)
    strcat(strbuffer, "onGround ");
  if(c->flags1.bits.projectile)
    strcat(strbuffer, "projectile ");
  if(c->flags1.bits.ridden)
    strcat(strbuffer, "ridden ");
  if(c->flags1.bits.royal_guard)
    strcat(strbuffer, "RoyGuard ");
  if(c->flags1.bits.skeleton)
    strcat(strbuffer, "Skeleton ");
  if(c->flags1.bits.tame)
    strcat(strbuffer, "Tame ");
  if(c->flags1.bits.zombie)
    strcat(strbuffer, "Zombie ");

  if(c->flags2.bits.slaughter)
    strcat(strbuffer, "ReadyToSlaughter ");
  if(c->flags2.bits.resident)
    strcat(strbuffer, "Resident ");
  if(c->flags2.bits.sparring)
    strcat(strbuffer, "Sparring ");
  if(c->flags2.bits.swimming)
    strcat(strbuffer, "Swimming ");
  if(c->flags2.bits.underworld)
    strcat(strbuffer, "Underworld ");

  //if(c->flags1.bits.can_swap)
  //  strcat(strbuffer, "canSwap ");
  //if(c->flags1.bits.check_flows)
  //  strcat(strbuffer, "checFlows ");
  //if(c->flags1.bits.invader_origin)
  //  strcat(strbuffer, "invader_origin ");
  
}

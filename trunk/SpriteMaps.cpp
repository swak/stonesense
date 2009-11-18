#include "common.h"
#include "SpriteMaps.h"
#include "GroundMaterialConfiguration.h"




int getFloorSpriteWithDefault( int defaultSprite, int materialIndex ){
  //do a LOOKUP. not a search! 
  /*if( materialIndex != INVALID_INDEX ){
    uint32_t spriteIndex = groundTypes[materialIndex].floorSheetIndex;
    if(spriteIndex != INVALID_INDEX) 
      return spriteIndex;
  }*/
  return defaultSprite;
}


int getWallSpriteWithDefault( int defaultSprite, int materialIndex ){
  //do a LOOKUP. not a search! 
  /*if( materialIndex != INVALID_INDEX ){
    uint32_t spriteIndex = groundTypes[materialIndex].wallSheetIndex;
    if(spriteIndex != INVALID_INDEX) 
      return spriteIndex; 
  }*/
  return defaultSprite;
}

int GetWallSpriteMap(int in, int matIndex, bool getFillerFloor)
{
  if( in >= (int)groundTypes.size() ) 
    return (!getFillerFloor ? SPRITEOBJECT_WALL_NA : SPRITEFLOOR_NA);
  GroundMaterialConfiguration* wall = groundTypes[ in ];
	if(wall == NULL) 
    return (!getFillerFloor ? SPRITEOBJECT_WALL_NA : SPRITEFLOOR_NA);
  int numMat = (int)wall->overridingMaterials.size();
	for(int i=0; i<numMat; i++){
    if(wall->overridingMaterials[i].gameID == matIndex){
      if(getFillerFloor == false)
			  return wall->overridingMaterials[i].spriteIndex;
      else
        return wall->overridingMaterials[i].fillerFloorSpriteIndex;
		}
	}
  if(getFillerFloor == false)
	  return wall->defaultSprite;
  else
    return wall->fillerFloorSpriteIndex;

  int matIndexResult = -1;
    switch (in)
    {
        case 83: //frozen liquid pillar
        case 176: //stone wall worn1 (most worn)
        case 177: //stone wall worn2 (sorta worn)
        case 178: //stone wall worn3 (least worn)
        
        

        case 328: //lavastone wall worn1 (most worn)
        case 329: //lavastone wall worn2 (middle worn)
        case 330: //lavastone wall worn3 (least worn)
        
        case 332: //featstone wall worn1 (most worn)
        case 333: //featstone wall worn2 (middle worn)
        case 334: //featstone wall worn3 (least worn)

        case 360: //frozen liquid fortification
        case 361: //frozen liquid wall worn1 (most worn)
        case 362: //frozen liquid wall worn2 (middle worn)
        case 363: //frozen liquid wall worn3 (least worn)
        

        case 437: //minstone wall worn1
        case 438: //minstone wall worn2
        case 439: //minstone wall worn3
        
        case 450: //frozen liquid wall rd2
        case 451: //frozen liquid wall r2d
        case 452: //frozen liquid wall r2u
        case 453: //frozen liquid wall ru2
        case 454: //frozen liquid wall l2u
        case 455: //frozen liquid wall lu2
        case 456: //frozen liquid wall l2d
        case 457: //frozen liquid wall ld2
        case 458: //frozen liquid wall lrud
        case 459: //frozen liquid wall rud
        case 460: //frozen liquid wall lrd
        case 461: //frozen liquid wall lru
        case 462: //frozen liquid wall lud
        case 463: //frozen liquid wall rd
        case 464: //frozen liquid wall ru
        case 465: //frozen liquid wall lu
        case 466: //frozen liquid wall ld
        case 467: //frozen liquid wall ud
        case 468: //frozen liquid wall lr
        
            return SPRITEOBJECT_WALL_NA;
            break;

        case 364: //frozen liquid wall
          return SPRITEOBJECT_WALL_FROZEN;
          break;

        case 219: //stone wall (not worn)
          return getWallSpriteWithDefault( SPRITEOBJECT_WALL_STONE_ROUGH, matIndex );
          break;

        case 440: //minstone wall worn4
          return getWallSpriteWithDefault( SPRITEOBJECT_WALL_MINSTONE_ROUGH, matIndex );
          break;

        case 331: //lavastone wall
          return SPRITEOBJECT_WALL_LAVASTONE_ROUGH;
          break; 

        case 335: //featstone wall
          return SPRITEOBJECT_WALL_FEATSTONE_ROUGH;
          break; 

        case 307: //stone wall rd2
        case 308: //stone wall r2d
        case 309: //stone wall r2u
        case 310: //stone wall ru2
        case 311: //stone wall l2u
        case 312: //stone wall lu2
        case 313: //stone wall l2d
        case 314: //stone wall ld2
        case 315: //stone wall lrud
        case 316: //stone wall rud
        case 317: //stone wall lrd
        case 318: //stone wall lru
        case 319: //stone wall lud
        case 320: //stone wall rd
        case 321: //stone wall ru
        case 322: //stone wall lu
        case 323: //stone wall ld
        case 324: //stone wall ud
        case 325: //stone wall lr
        case 417: //minstone wall rd2
        case 418: //minstone wall r2d
        case 419: //minstone wall r2u
        case 420: //minstone wall ru2
        case 421: //minstone wall l2u
        case 422: //minstone wall lu2
        case 423: //minstone wall l2d
        case 424: //minstone wall ld2
        case 425: //minstone wall lrud
        case 426: //minstone wall rud
        case 427: //minstone wall lrd
        case 428: //minstone wall lru
        case 429: //minstone wall lud
        case 430: //minstone wall rd
        case 431: //minstone wall ru
        case 432: //minstone wall lu
        case 433: //minstone wall ld
        case 434: //minstone wall ud
        case 435: //minstone wall lr
        case 269: //lavastone wall rd2
        case 270: //lavastone wall r2d
        case 271: //lavastone wall r2u
        case 272: //lavastone wall ru2
        case 273: //lavastone wall l2u
        case 274: //lavastone wall lu2
        case 275: //lavastone wall l2d
        case 276: //lavastone wall ld2
        case 277: //lavastone wall lrud
        case 278: //lavastone wall rud
        case 279: //lavastone wall lrd
        case 280: //lavastone wall lru
        case 281: //lavastone wall lud
        case 282: //lavastone wall rd
        case 283: //lavastone wall ru
        case 284: //lavastone wall lu
        case 285: //lavastone wall ld
        case 286: //lavastone wall ud
        case 287: //lavastone wall lr
        case 288: //featstone wall rd2
        case 289: //featstone wall r2d
        case 290: //featstone wall r2u
        case 291: //featstone wall ru2
        case 292: //featstone wall l2u
        case 293: //featstone wall lu2
        case 294: //featstone wall l2d
        case 295: //featstone wall ld2
        case 296: //featstone wall lrud
        case 297: //featstone wall rud
        case 298: //featstone wall lrd
        case 299: //featstone wall lru
        case 300: //featstone wall lud
        case 301: //featstone wall rd
        case 382: //featstone wall ru
        case 303: //featstone wall lu
        case 304: //featstone wall ld
        case 305: //featstone wall ud
        case 306: //featstone wall lr
          return SPRITEOBJECT_WALL_STONE_SMOOTH;
          break;


        case 265: //soil wall
          return getWallSpriteWithDefault( SPRITEOBJECT_WALL_SOIL, matIndex );
          break;


        case 496: //constructed wall rd2
        case 497: //constructed wall r2d
        case 498: //constructed wall r2u
        case 499: //constructed wall ru2
        case 500: //constructed wall l2u
        case 501: //constructed wall lu2
        case 502: //constructed wall l2d
        case 503: //constructed wall ld2
        case 504: //constructed wall lrud
        case 505: //constructed wall rud
        case 506: //constructed wall lrd
        case 507: //constructed wall lru
        case 508: //constructed wall lud
        case 509: //constructed wall rd
        case 510: //constructed wall ru
        case 511: //constructed wall lu
        case 512: //constructed wall ld
        case 513: //constructed wall ud
        case 514: //constructed wall lr
          return SPRITEOBJECT_WALL_CONSTR;
          break;

        case 79: //stone pillar
        case 80: //lavastone pillar
        case 81: //featstone pillar
        case 82: //minstone pillar        
        case 495: //constructed pillar
          return SPRITEOBJECT_WALL_PILLAR;
          break;

        case 65: //stone fortification
        case 436: //minstone fortification
        case 326: //lavastone fortification
        case 327: //featstone fortification
          return SPRITEOBJECT_WALL_FORTIFICATION;
          break;

        case 494: //constructed fortification
          return SPRITEOBJECT_WALL_CONSTR_FORTIFICATION;
          break;

				case ID_WOODWALL:
					return SPRITEOBJECT_WALL_WOOD;
					break;

				case ID_WOODFORTIFICATION:
					return SPRITEOBJECT_WALL_WOOD_FORTIFICATION;

        case ID_METALWALL:
          return SPRITEOBJECT_WALL_METAL;
          break;
    }

    return SPRITEOBJECT_WALL_NA;
}
int GetFloorSpriteMap(int in, int matIndex){
  //TODO: groundTypes.size can be cahced
  if( in >= (int)groundTypes.size() ) 
    return SPRITEFLOOR_NA;
  GroundMaterialConfiguration* floor = groundTypes[ in ];
	if(floor == NULL) 
		return SPRITEFLOOR_NA;

  int numMat = (int)floor->overridingMaterials.size();
	for(int i=0; i<numMat; i++){
    if(floor->overridingMaterials[i].gameID == matIndex){
			return floor->overridingMaterials[i].spriteIndex;
		}
	}

	int spriteindex = floor->defaultSprite;

	return spriteindex;


   switch (in)
    {
			

			case 19: //driftwood stack
			// case 27: //up stair frozen liquid
			case 35: //Chasm
			// case 38: //up stair lavastone
			// case 41: //up stair soil
			case 42: //eerie pit

			case 47: //frozen liquid floor detailed
			/*
			case 51: //up stair grass1 [muddy?]
			case 54: //up stair grass2
			case 57: //up stair stone
			case 60: //up stair minstone
			case 63: //up stair featstone
			*/
			case 67: //campfire
			case 70: //fire
			/*
			case 79: //stone pillar
			case 80: //lavastone pillar
			case 81: //featstone pillar
			case 82: //minstone pillar
			case 83: //frozen liquid pillar
			*/
			case 89: //waterfall landing
			case 90: //river source

			/*
			case 233: //ramp grass dry
			case 234: //ramp grass dead
			case 235: //ramp grass1 [muddy?]
			case 236: //ramp grass2
			case 237: //ramp stone
			case 238: //ramp lavastone
			case 239: //ramp featstone
			case 240: //ramp minstone
			case 241: //ramp soil
			*/
			case 242: //ash1
			case 243: //ash2
			case 244: //ash3
			// frozen floors / ramps
			case 245: //ramp frozen liquid

			
			//        case 262: //Ice floor
			case 264: //Lava bottom of map

			case 344: //featstone floor 1 (raw stone)
			case 345: //featstone floor 2 (raw stone)
			case 346: //featstone floor 3 (raw stone)
			case 347: //featstone floor 4 (raw stone)


			case 356: //soil floor 1 wet (raw) [red sand?]
			case 357: //soil floor 2 wet (raw) [red sand?]
			case 358: //soil floor 3 wet (raw) [red sand?]
			case 359: //soil floor 4 wet (raw) [red sand?]



			case 409: //lavastone pebbles 1
			case 410: //lavastone pebbles 2
			case 411: //lavastone pebbles 3
			case 412: //lavastone pebbles 4

			case 413: //featstone pebbles 1
			case 414: //featstone pebbles 2
			case 415: //featstone pebbles 3
			case 416: //featstone pebbles 4


			case 445: //minstone boulder
			case 446: //minstone pebbles 1
			case 447: //minstone pebbles 2
			case 448: //minstone pebbles 3
			case 449: //minstone pebbles 4
			
			//case 495: //constructed pillar
			
			//case 518: //ramp constructed
				return SPRITEFLOOR_NA;

      //case 517: //stair up constructed
			case 43: //stone floor detailed
			case 44: //lavastone floor detailed
			case 45: //featstone? floor detailed
			case 46: //minstone? floor detailed [calcite]
			case 493: //constructed floor detailed
				return SPRITEFLOOR_DETAILED;

      case 2:   //murky pool
			case 365: //river n
			case 366: //river s
			case 367: //river e
			case 368: //river w
			case 369: //river nw
			case 370: //river ne
			case 371: //river sw
			case 372: //river se

			case 373: //stream wall n (below)
			case 374: //stream wall s (below)
			case 375: //stream wall e (below)
			case 376: //stream wall w (below)
			case 377: //stream wall nw (below)
			case 378: //stream wall ne (below)
			case 379: //stream wall sw (below)
			case 380: //stream wall se (below)
        return SPRITEFLOOR_WATERFLOOR;

      case 258: //frozen liquid 1
			case 259: //frozen liquid 2
			case 260: //frozen liquid 3
			case 262: //frozen liquid 0
      return SPRITEFLOOR_SNOW;

			case 348: //grass floor 1 (raw)
			case 349: //grass floor 2 (raw)
			case 350: //grass floor 3 (raw)
			case 351: //grass floor 4 (raw)
			case 398: //grass floor1b
			case 399: //grass floor2b
			case 400: //grass floor3b
			case 401: //grass floor4b
				return SPRITEFLOOR_GRASS; 

      case 340: //lavastone floor 1 (raw stone)
			case 341: //lavastone floor 2 (raw stone)
			case 342: //lavastone floor 3 (raw stone)
			case 343: //lavastone floor 4 (raw stone)
        return SPRITEFLOOR_LAVASTONE;


			case 336: //stone floor 1 (raw stone)
			case 337: //stone floor 2 (raw stone)
			case 338: //stone floor 3 (raw stone)
			case 339: //stone floor 4 (raw stone)
			case 441: //minstone floor 1 (cavern raw)
			case 442: //minstone floor 2 (cavern raw)
			case 443: //minstone floor 3 (cavern raw)
			case 444: //minstone floor 4 (cavern raw)
				return getFloorSpriteWithDefault( SPRITEFLOOR_STONE, matIndex );


			case 352: //soil floor 1 (raw)
			case 353: //soil floor 2 (raw)
			case 354: //soil floor 3 (raw)
			case 355: //soil floor 4 (raw)
				return getFloorSpriteWithDefault( SPRITEFLOOR_DIRT, matIndex );


			case 405: //stone pebbles 1
			case 406: //stone pebbles 2
			case 407: //stone pebbles 3
			case 408: //stone pebbles 4
				return SPRITEFLOOR_PEBBLES;

			case 402: //stone boulder
			case 403: //lavastone boulder
			case 404: //featstone boulder
				return SPRITEFLOOR_BOULDER;

			case 24: //tree
			case 34: //shrub
			case 231: //sapling
				return SPRITEFLOOR_VEGETATION;

			case 261: //furrowed soil [road?] JONAS thinks plowed field
				return SPRITEFLOOR_PLOWED;


			case 387: //dry grass floor1
			case 388: //dry grass floor2
			case 389: //dry grass floor3
			case 390: //dry grass floor4
			case 391: //dead tree
			case 392: //dead sapling
			case 393: //dead shrub
			case 394: //dead grass floor1
			case 395: //dead grass floor2
			case 396: //dead grass floor3
			case 397: //dead grass floor4
				return SPRITEFLOOR_DRYGRASS;
				break;
				
			case ID_WOODFLOOR: //Constructed wooden floor
				return SPRITEFLOOR_WOOD;

      case ID_METALFLOOR:
        return SPRITEFLOOR_METAL;
			
    }

		return SPRITEFLOOR_NA;


}

int GetDownStairTypeMap(int in){

	switch (in)
	{
		case 25: //up-down stair frozen liquid
		case 26: //down stair frozen liquid

		case 36: //up-down stair lavastone
		case 37: //down stair lavastone

		case 39: //up-down stair soil
		case 40: //down stair soil

		case 49: //up-down stair grass1 [muddy?]
		case 50: //down stair grass1 [muddy?]


		case 52: //up-down stair grass2
		case 53: //down stair grass2

		case 55: //up-down stair stone
		case 56: //down stair stone

		case 58: //up-down stair minstone
		case 59: //down stair minstone

		case 61: //up-down stair featstone
		case 62: //down stair featstone

		case 515: //stair up-down constructed
		case 516: //stair down constructed
			return SPRITEOBJECT_STAIRDOWN;

    case ID_WOODFLOOR_STAIR_DOWN:
      return SPRITEOBJECT_STAIRDOWN_WOOD;
				
	}
	return SPRITEOBJECT_NA;
}

int GetUpStairTypeMap(int in, bool mirrored){

	switch (in)
	{
		case 25: //up-down stair frozen liquid
		case 27: //up stair frozen liquid


		case 36: //up-down stair lavastone
		case 38: //up stair lavastone

		case 39: //up-down stair soil
		case 41: //up stair soil

		case 49: //up-down stair grass1 [muddy?]
		case 51: //up stair grass1 [muddy?]


		case 52: //up-down stair grass2
		case 54: //up stair grass2

		case 55: //up-down stair stone
		case 57: //up stair stone

		case 58: //up-down stair minstone
		case 60: //up stair minstone

		case 61: //up-down stair featstone
		case 63: //up stair featstone

		case 515: //stair up-down constructed
		case 517: //stair up constructed
      
      return mirrored ? SPRITEOBJECT_STAIRUP_MIR : SPRITEOBJECT_STAIRUP;
      break;
    case ID_WOODFLOOR_STAIR_UP:
    case ID_WOODFLOOR_STAIR_UPDOWN:
      return mirrored ? SPRITEOBJECT_STAIRUP_WOOD_MIR : SPRITEOBJECT_STAIRUP_WOOD;
      break;
				
	}
	return SPRITEOBJECT_NA;
}


int GetRampMaterialTypeMap(int in){

	switch (in)
	{
    case 233: //ramp grass dry
    case 234: //ramp grass dead
    case 235: //ramp grass1 [muddy?]
    case 236: //ramp grass2
      return SPRITERAMP_GRASS;
			break;
    case 237: //ramp stone
    case 238: //ramp lavastone
    case 239: //ramp featstone
    case 240: //ramp minstone
    case 241: //ramp soil
    case 245: //ramp frozen liquid
    case 518: //ramp constructed
		  return SPRITERAMP_GENERIC;
			break;	
	}
	return SPRITERAMP_GENERIC;
}


int GetSpriteVegitation( VegetationType type, int index){
  switch(type){
  case TREE_OK:
    switch(index){
      case 1://SAGUARO
        return SPRITEOBJECT_CACTUS;
        break;

      //case 3://??
      case 4://OAK
      //case 5://??
      //case 6://??
      //case 7://??
      
      
      //case 11://??
      case 12://FEATHERTREE
      case 13://HIGHWOOD
      case 15://CHESTNUT
      case 16://ALDER
      
      case 18://ASH
      //case 19://??
      //case 20://??
        return SPRITEOBJECT_TREE_BROADLEAF;
        break;

      case 9://WILLOW
        return SPRITEOBJECT_TREE_WILLOW;
        break;
      case 8://MAPLE
        return SPRITEOBJECT_TREE_MAPLE;
        break;
      
      case 10://Tower-Cap
        return SPRITEOBJECT_TREE_TOWERCAP;
        break;

      case 2://PINE
      case 14://LARCH
        return SPRITEOBJECT_TREE_CONIFER;
        break;

      case 17://BIRTCH
        return SPRITEOBJECT_TREE_BIRCH;
        break;
      default:
        return SPRITEOBJECT_TREE_BROADLEAF;
    }
    break;
  case TREE_DEAD:
    return SPRITEOBJECT_DEAD_TREE;
    break;
  case SHRUB_DEAD:
  case SHRUB_OK:
    /*switch( index ){
    case 0:
      return SPRITEOBJECT_SHRUB_PLUMP;
      break;
      
    };
    return SPRITEOBJECT_SHRUB_DEFAULT;*/
    return SPRITEOBJECT_SHRUB_PLUMP + index;
    break;
  case SAPLING_DEAD:
  case SAPLING_OK:
    return SPRITEOBJECT_SAPLING;
    break;

  }
  return SPRITEOBJECT_BLUEPRINT;
}


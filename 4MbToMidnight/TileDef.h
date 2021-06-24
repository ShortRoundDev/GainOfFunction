#pragma once

#include "Wall.hpp"

#define BLANK_WALL	0
#define NO_DRAW		3

//doors
#define RED_DOOR	100
#define BLUE_DOOR	101
#define YELLOW_DOOR	102
#define NORMAL_DOOR	104

//specials

#define WALL_SIGN	105
#define ELEVATOR	115
#define TV_SCREEN	120
#define SWITCH_OFF	124
#define SWITCH_ON	125

#define WALL_IS_KIND(x, y) (((x) != NULL) && ((x)->wallTexture == (y)))
#define TILE_IS_DOOR(x) (((x) >= RED_DOOR) && ((x) <= NORMAL_DOOR))
#define WALL_IS_DOOR(x) (((x) != NULL) && (TILE_IS_DOOR((x)->wallTexture)))
#define WALL_HAS_MESSAGE(x) (((x) != NULL) && TILE_HAS_MESSAGE((x)->wallTexture))
#define TILE_HAS_MESSAGE(x) (((x) == WALL_SIGN) || ((x) == ELEVATOR) || ((x) == TV_SCREEN))
#define COLLIDEABLE(x) ((x) != NULL && (!(x)->isDoor ^ ((x)->isDoor && !(x)->isOpen)))
#define COLLIDEABLE_ALL(x) (!WALL_IS_KIND((x), BLANK_WALL) && !WALL_IS_KIND((x), WALL_SIGN) && !WALL_IS_KIND((x), TV_SCREEN) && !WALL_IS_KIND((x), SWITCH_ON) && !WALL_IS_KIND((x), SWITCH_OFF) && COLLIDEABLE((x)))
#pragma once

// Entities at 1000 level
#define RED_KEY     1000
#define BLUE_KEY    1001
#define YELLOW_KEY  1002

#define ZOMBIE      1003
#define COLLEEN		1004

#define AMMO        1005
#define RIFLEAMMO   1006

#define PISTOL      1010
#define RIFLE       1011

#define SYRINGE		1012
#define BIGSYRINGE	1013

#define BEACON		1200

// Only spawn at runtime:
#define ZOMBIEGIB	1300
#define BULLETHOLE	1301

#define THROWER		1050
#define FIREBALL	1051

#ifndef ENT_CONST_ONLY
#include "RedKey.hpp"
#include "BlueKey.hpp"
#include "YellowKey.hpp"

#include "Zombie.hpp"
#include "Colleen.hpp"

#include "Ammo.hpp"
#include "RifleAmmo.hpp"

#include "Pistol.hpp"
#include "Rifle.hpp"

#include "Syringe.hpp"

#include "Beacon.hpp"

#include "Thrower.hpp"
#endif
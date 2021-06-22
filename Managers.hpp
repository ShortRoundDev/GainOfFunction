#pragma once
#include "glm/glm.hpp"

#include "GameManager.hpp"
#include "GraphicsManager.hpp"
#include "SoundManager.hpp"
#include "Platform.h"

#define CAMERA (GameManager::instance->camera)
#define WALLS (GameManager::instance->currentLevel->walls)
#define TRY_WALL(x, y) (LEVEL->tryWall((x), (y)))
#define LEVEL (GameManager::instance->currentLevel)
#define PLAYER (GameManager::instance->player)


#define SHADERS (GraphicsManager::shaders)
#define SCREEN_X(x) (GraphicsManager::px2scrnX(x))
#define SCREEN_Y(y) (GraphicsManager::px2scrnY(y))
#define SCREEN_W(w) (GraphicsManager::scrnscaleX(w))
#define SCREEN_H(h) (GraphicsManager::scrnscaleY(h))

#define COORDS(x, y) ((x) + ((y) * LEVEL->width))

#define IS_DOOR(t) ((t).isDoor)
#define NOT_SOLID(t) ((!(t).isSolid) || (t).wallTexture == 0 || (IS_DOOR(t) && (t).isOpen))
#define SOLID(t) (!NOT_SOLID(t))
#define IN_BOUNDS(x, y) ((x) >= 0 && (x) < LEVEL->width && (y) >= 0 && (y) < LEVEL->height)
#define NOT_IN_BOUNDS(x, y) (!IN_BOUNDS(x, y))

#define PLAY_S(s, p) (SoundManager::instance->playSound((s), (p), 1.0f))
#define PLAY_I(i, p) (SoundManager::instance->playSound((i), (p), 1.0f))

#define PRINT(str, x, y, siz) (GameManager::instance->print((str), (x), (y), (siz)))

#define PACK_COORDS(x, y) ((((uint32_t)(x)) << 16) | (y))
#define UNPACK_X(x) (((x) & 0xffff0000) >> 16)
#define UNPACK_Y(y) ((y) & 0x0000ffff)

#define DIST_2(a, b) (glm::length(glm::vec2((a).x, (a).z) - glm::vec2((b).x, (b).z)))

#define PRINT_VEC3(v) (v.x) << ", " << (v.y) << ", " << (v.z)


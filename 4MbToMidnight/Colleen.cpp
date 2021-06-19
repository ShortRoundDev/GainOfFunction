#include "Colleen.hpp"

#include <iostream>

#include "Managers.hpp"
#include "glm/gtx/rotate_vector.hpp"

#define ENT_CONST_ONLY
#include "EntDef.h"

Colleen::Colleen(glm::vec3 pos)
	: Entity(
		pos,
		"Resources/colleen.png",
		glm::vec2(1.0f, 1.0f),
		glm::vec2(0.3f, 0.3f),
		1200
	)
{
	currentAnimation = "float";
	animations["float"] = { 0, 1, 0 };
	totalAngles = 4;
	front = glm::vec3(0, 0, -1);
	auto loopBuffer = SoundManager::instance->sounds.find("Resources/Audio/WeirdNoises.ogg");
	if (loopBuffer != SoundManager::instance->sounds.end()) {
		loopSource = SoundManager::instance->playLoop(loopBuffer->second, pos, 2.0f);
	}
	beaconHistory.push_back(-1);
	beaconHistory.push_back(-1);
	beaconHistory.push_back(-1);
}

Colleen::~Colleen() {

}

void Colleen::update() {
	alSource3f(loopSource, AL_POSITION, position.x, position.y, position.z);

	/*if (canSeePlayer()) {
		state = SEES_PLAYER;
		if (!goals.empty()) {
			std::queue<glm::vec3> empty;
			std::swap(goals, empty);
		}
		if (!path.empty())
			path.clear();
	}*/

	// check transitions
	switch (state) {
	case IDLE:
		idleCheckTransitions();
		break;
	case SEES_PLAYER:
		attackingCheckTransitions();
		break;
	case HUNTING_PLAYER:
		pursuingCheckTransitions();
		break;
	case PATROLING:
		patrollingCheckTransitions();
		break;
	}

	// update
	switch (state) {
	case IDLE:
		idleUpdate();
		break;
	case SEES_PLAYER:
		attackingUpdate();
		break;
	case HUNTING_PLAYER:
		pursuingUpdate();
		break;
	case PATROLING:
		patrollingUpdate();
		break;
	}
}

void Colleen::idleCheckTransitions() {
	//for (Entity* e : LEVEL->entities) {
		//if(e.)
	//}
}

void Colleen::attackingCheckTransitions() {

}

void Colleen::pursuingCheckTransitions() {

}

void Colleen::patrollingCheckTransitions() {

}

void Colleen::idleUpdate() {

}

void Colleen::attackingUpdate() {

}

void Colleen::pursuingUpdate() {

}

void Colleen::patrollingUpdate() {

}


bool Colleen::canSeePlayer() {
	if (PLAYER.health <= 0)
		return false;
	int x, y;
	bool seen = GameManager::instance->dda(
		position.x, position.z,
		PLAYER.pos.x, PLAYER.pos.z,
		&x, &y
	);
	return seen;
}

Beacon* Colleen::findNearestBeacon() {
	float dist = 99999;
	Beacon* currentClosest = NULL;
	for (Entity* e : LEVEL->entities) {
		if (e->entityType == BEACON) {
			float eDist = glm::length(e->position - position);
			if (eDist < dist || currentClosest == NULL) {
				Beacon* b = (Beacon*)e;
				if (std::find(beaconHistory.begin(), beaconHistory.end(), b->id) != beaconHistory.end())
					continue;
				dist = eDist;
				currentClosest = b;
			}
		}
	}
	return currentClosest;
}

void Colleen::findPathToBeacon(Beacon* beacon) {
	if (beacon == NULL) {
		return;
	}
	bool found = GameManager::instance->bfs(
		(int16_t)position.x, (int16_t)position.z,
		(int16_t)beacon->position.x, (int16_t)beacon->position.z,
		path
	);

	if (goals.empty() && found) {

	}

}
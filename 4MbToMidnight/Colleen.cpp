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
		COLLEEN
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
	beaconHistory.push_back(-1);
}

Colleen::~Colleen() {

}

void Colleen::update() {
	alSource3f(loopSource, AL_POSITION, position.x, position.y, position.z);


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
	case PATROLLING:
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
	case PATROLLING:
		patrollingUpdate();
		break;
	}

	move();
}

void Colleen::idleCheckTransitions() {
	if (idleLastCheckedtransition-- > 0) {
		return;
	}

	//Check for newly available beacons
	idleLastCheckedtransition = 100;
	Beacon* b = findFarthestBeacon();
	if (!path.empty())
		path.clear();
	bool found = findPathToEntity(b, &path, &goals);
	if (found) {
		if (b != NULL) {
			beaconHistory.push_back(b->id);
			beaconHistory.pop_front();
		}
		state = PATROLLING;
	}
}

void Colleen::attackingCheckTransitions() {

}

void Colleen::pursuingCheckTransitions() {

}

void Colleen::patrollingCheckTransitions() {
	if (goals.empty()) {
		Beacon* b = findFarthestBeacon();
		if (!path.empty())
			path.clear();
		bool found = findPathToEntity(b, &path, &goals);
		if (!found) {
			state = IDLE;
		}
		else {
			beaconHistory.push_back(b->id);
			beaconHistory.pop_front();
		}
	}
}

void Colleen::idleUpdate() {
	currentGoal.x = -1;
	currentGoal.z = -1; // do nothing, kill goals
}

void Colleen::attackingUpdate() {

}

void Colleen::pursuingUpdate() {

}

void Colleen::patrollingUpdate() {
	auto dist = glm::distance(position, currentGoal);
	if (dist < 0.3 || currentGoal.x == -1 && currentGoal.z == -1) {
		if (!goals.empty()) {
			currentGoal = goals.front();
			goals.pop();
		}
		else {
			// out of goals
			currentGoal = glm::vec3(-1, 0, -1);
		}
	}
}

void Colleen::move() {
	if (currentGoal.x != -1 && currentGoal.z != -1) {
		auto moveDir = glm::normalize(currentGoal - position);
		front = moveDir;
		moveVec += front * 0.01f;
		moveVec = glm::normalize(moveVec) * 0.020f;
	}
	else {
		moveVec *= 0.98f;
	}
	position = pushWall(position + moveVec);
	// detect if she's stuck here?
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

Beacon* Colleen::findFarthestBeacon() {
	float dist = -1;
	Beacon* currentFarthest = NULL;
	std::map<uint32_t, uint32_t> tmpPath;
	for (Entity* e : LEVEL->entities) {
		if (e->entityType == BEACON) {
			float eDist = glm::length(e->position - position);
			if (eDist > dist || currentFarthest == NULL) {
				Beacon* b = (Beacon*)e;
				if (std::find(beaconHistory.begin(), beaconHistory.end(), b->id) != beaconHistory.end())
					continue;
				tmpPath.clear();
				// check if 
				if (GameManager::instance->bfs(
					position.x, position.z,
					b->position.x, b->position.z,
					tmpPath
				)) {
					dist = eDist;
					currentFarthest = b;
				}
			}
		}
	}
	return currentFarthest;
}
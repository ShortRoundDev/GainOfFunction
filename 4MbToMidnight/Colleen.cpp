#include "Colleen.hpp"

#include <iostream>

#include "Managers.hpp"
#include "glm/gtx/rotate_vector.hpp"
#include "Fireball.hpp"

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
	animations["dead"] = { 1, 5, 0 };
	
	totalFrames = 6;
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

	healthbarTex = GraphicsManager::instance->loadTex("Resources/healthbar.png", GL_BGRA);
	health = 40;
}

Colleen::~Colleen() {
	if(loopSource != -1)
		alSourceStop(loopSource);
	loopSource = -1;
	LEVEL->colleen = NULL;
}

void Colleen::update() {
	alSource3f(loopSource, AL_POSITION, position.x, position.y, position.z);

	if (currentAnimation == "dead") {
		if (!dead) {
			animations[currentAnimation].iterate(0.03f);
			if (animations[currentAnimation].checkLooped()) {
				dead = true;
				animations[currentAnimation].currentFrame = 4;
				if(loopSource != -1)
					alSourceStop(loopSource);
				loopSource = -1;
			}
		}
		else if(GameManager::maxBright > 0.0f){
			GameManager::maxBright -= 0.01f;
			if (GameManager::maxBright < 0.0f) {
				GameManager::maxBright == 0.0f;
				// start credit roll
			}
		}
		return;
	}

	shootable = (LEVEL->switchesOff == 0);
	if (shootable && healthbarWidth < 1024) {
		healthbarWidth += 10;
		if (healthbarWidth > 1024)
			healthbarWidth = 1024;
	}

	// check transitions
	switch (state) {
	case ColleenState::IDLE:
		idleCheckTransitions();
		break;
	case ColleenState::ATTACKING:
		attackingCheckTransitions();
		break;
	case ColleenState::PURSUING:
		pursuingCheckTransitions();
		break;
	case ColleenState::PATROLLING:
		patrollingCheckTransitions();
		break;
	}

	// update
	switch (state) {
	case ColleenState::IDLE:
		idleUpdate();
		break;
	case ColleenState::ATTACKING:
		attackingUpdate();
		break;
	case ColleenState::PURSUING:
		pursuingUpdate();
		break;
	case ColleenState::PATROLLING:
		patrollingUpdate();
		break;
	}

	move();
}

void Colleen::idleCheckTransitions() {
	if (idleLastCheckedtransition-- > 0) {
		return;
	}

	if (canSeePlayer()) {
		state = ColleenState::ATTACKING;
		return;
	}

	//Check for newly available beacons
	idleLastCheckedtransition = 100;
	Beacon* b = findFarthestBeacon();
	if (!path.empty())
		path.clear();
	if (b != NULL && findPathToEntity(b, &path, &goals)) {
		if (b != NULL) {
			beaconHistory.push_back(b->id);
			beaconHistory.pop_front();
		}
		state = ColleenState::PATROLLING;
	}
}

void Colleen::attackingCheckTransitions() {
	if (PLAYER.health <= 0) {
		state = ColleenState::PATROLLING;
		return;
	}
	else if (!canSeePlayer()) {
		(&PLAYER)->zonesCrossed = 0;
		state = ColleenState::PURSUING;
	}
}

void Colleen::pursuingCheckTransitions() {
	if (canSeePlayer()) {
		pathRecalculationTimer = 0;
		state = ColleenState::ATTACKING;
	}
	else if (PLAYER.zonesCrossed == 3 || PLAYER.actualZone == 8) {
		pathRecalculationTimer = 0;
		state = ColleenState::PATROLLING;

		//get new route
		std::queue<glm::vec3> empty;
		std::swap(goals, empty);

		Beacon* b = findFarthestBeacon();
		if (!path.empty())
			path.clear();
		if (b == NULL || !findPathToEntity(b, &path, &goals)) {
			state = ColleenState::IDLE;
		}
		else {
			beaconHistory.push_back(b->id);
			beaconHistory.pop_front();
		}

	}
}

void Colleen::patrollingCheckTransitions() {
	if (canSeePlayer()) {
		state = ColleenState::ATTACKING;
	}
	else if (goals.empty()) {
		Beacon* b = findFarthestBeacon();
		if (!path.empty())
			path.clear();
		if (b == NULL || !findPathToEntity(b, &path, &goals)) {
			state = ColleenState::IDLE;
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
	currentGoal = glm::vec3(PLAYER.pos.x, position.y, PLAYER.pos.z);
	if(shootable && volleyCooldown <= 0 && fireballCooldown <= 0){
		GameManager::addEntity(new Fireball(position + glm::vec3(0, 0.4f, 0), front));
		fireballCooldown = 10;
		fireballs--;
		if (fireballs <= 0) {
			fireballs = 3;
			volleyCooldown = 100;
		}
	}
	else {
		volleyCooldown--;
		fireballCooldown--;
	}
	if (DIST_2(PLAYER.pos, position) < 0.5f) {
		// kill player
		moveVec = glm::vec3(0);
		currentGoal = glm::vec3(-1);
		if(PLAYER.colleenKillingCooldown == -1)
			(&PLAYER)->colleenKillingCooldown = 100;
	}
}

void Colleen::pursuingUpdate() {
	if (pathRecalculationTimer == 0) {
		if (!path.empty())
			path.clear();
		if (!goals.empty()) {
			std::queue<glm::vec3> empty;
			std::swap(goals, empty);
		}
		if (!Entity::findPathToSpot((int)PLAYER.pos.x, (int)PLAYER.pos.z, &path, &goals)) { // shouldn't happen
			pathRecalculationTimer = 0;
			state = ColleenState::PATROLLING;
			return;
		}
		currentGoal = glm::vec3(-1);
		pathRecalculationTimer = 100;
	}
	if ((currentGoal.x == -1 && currentGoal.z == -1) || DIST_2(currentGoal, position) < 0.3 && !goals.empty()) {
		currentGoal = goals.front();
		goals.pop();
	}
	pathRecalculationTimer--;

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
		auto _moveVec = front * 0.01f;
		if (_moveVec != glm::vec3(0)) {
			if ((state == ColleenState::ATTACKING || state == ColleenState::PURSUING) && !shootable)
				moveVec = glm::normalize(_moveVec) * 0.02f;
			else if (shootable)
				moveVec = glm::normalize(_moveVec) * 0.017f;
			else
				moveVec = glm::normalize(_moveVec) * 0.01f;
		}
		else {
			moveVec *= 0.98f;
		}
	}
	else {
		moveVec *= 0.98f;
	}
	position = pushWall(position + moveVec);
	// detect if she's stuck here?
}


bool Colleen::canSeePlayer() {

#ifdef PLAYER_INVISIBLE
	return false;
#endif

	if (PLAYER.health <= 0)
		return false;
	if (PLAYER.actualZone == 8 && !shootable)
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

void Colleen::hurt(int damage, glm::vec3 pos) {
	Entity::hurt(damage, pos);
}

void Colleen::die() {
	currentAnimation = "dead";
	GameManager::zombiesOff = true;
}
#pragma once

#include <AL/al.h>
#include <queue>
#include <list>

#include "Entity.hpp"
#include "Beacon.hpp"

#include "glm/glm.hpp"


enum ColleenState {
	IDLE,
	SEES_PLAYER,
	HUNTING_PLAYER,
	PATROLING
};

class Colleen : public Entity {
public:
	Colleen(glm::vec3 pos);
	~Colleen();
	void update();

	ALuint loopSource;

	//ai
	ColleenState state = IDLE;

	void idleCheckTransitions();
	void attackingCheckTransitions();
	void pursuingCheckTransitions();
	void patrollingCheckTransitions();

	void idleUpdate();
	void attackingUpdate();
	void pursuingUpdate();
	void patrollingUpdate();

	//pathfinding
	bool seesPlayer;
	glm::vec3 currentGoal;
	std::queue<glm::vec3> goals;

	std::list<int> beaconHistory;
	std::map<uint32_t, uint32_t> path;

	Beacon* findNearestBeacon();
	void findPathToBeacon(Beacon* beacon);

	bool canSeePlayer();
};
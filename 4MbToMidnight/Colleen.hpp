#pragma once

#include <AL/al.h>
#include <queue>
#include <list>

#include "Entity.hpp"
#include "Beacon.hpp"

#include "glm/glm.hpp"


enum class ColleenState {
	IDLE,
	ATTACKING,
	PURSUING,
	PATROLLING
};

class Colleen : public Entity {
public:
	Colleen(glm::vec3 pos);
	~Colleen();
	void update();

	GLuint healthbarTex;

	ALuint loopSource;

	//ai
	ColleenState state = ColleenState::IDLE;

	void idleCheckTransitions();
	void attackingCheckTransitions();
	void pursuingCheckTransitions();
	void patrollingCheckTransitions();

	void idleUpdate();
	void attackingUpdate();
	void pursuingUpdate();
	void patrollingUpdate();

	int healthbarWidth = 0;

	int idleLastCheckedtransition = 0;

	int volleyCooldown = 100;
	int fireballCooldown = 10;
	int fireballs = 3;

	void move();
	void hurt(int damage, glm::vec3 pos);
	void die();

	//pathfinding
	bool seesPlayer;
	glm::vec3 currentGoal = glm::vec3(-1, -1, -1);
	std::queue<glm::vec3> goals;

	std::list<int> beaconHistory;
	std::map<uint32_t, uint32_t> path;

	//pursuit stuff
	int pathRecalculationTimer = 0;

	Beacon* findFarthestBeacon();

	bool canSeePlayer();
};
#pragma once

#include <queue>
#include <map>

#include <Entity.hpp>
#include "glm/glm.hpp"

#define ATTACK_RANGE 3.0f
#define GOAL_RANGE 0.5f

enum class ThrowerState {
	IDLE,
	ATTACKING,
	FOLLOW,
	HUNT
};

class Thrower : public Entity {
public:
	Thrower(glm::vec3 position);
	~Thrower();

	ThrowerState state;

	glm::vec3 lastSeenPlayer;
	std::map<uint32_t, uint32_t> path;
	std::queue<glm::vec3> goals;

	glm::vec3 currentGoal = glm::vec3(-1);

	int attackCooldown = 50;

	void update();
	void die();
	void hurt(int damage, glm::vec3 pos);
private:

	void idleCheckTransitions();
	void attackingCheckTransitions();
	void followCheckTransitions();
	void huntCheckTransitions();

	void idleUpdate();
	void attackingUpdate();
	void followUpdate();
	void huntUpdate();

	void move();

	void whoosh();
	void whack();

	bool canSeePlayer();
};
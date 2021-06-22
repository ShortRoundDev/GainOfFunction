#pragma once
#include "Entity.hpp"

#include "glm/glm.hpp"
class Fireball : public Entity
{
public:
	Fireball(glm::vec3 pos, glm::vec3 dir);
	~Fireball();

	void update();
	void hurt(int damage, glm::vec3 hitPos);
};

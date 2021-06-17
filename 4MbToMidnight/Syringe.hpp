#pragma once

#include "glm/glm.hpp"

#include "Entity.hpp"

class Syringe : public Entity {
public:
	Syringe(glm::vec3, int big);
	~Syringe();
	int heal = 1;
	void update();
};
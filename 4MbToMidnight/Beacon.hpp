#pragma once

#include "Entity.hpp"

class Beacon : public Entity {
public:
	Beacon(glm::vec3 pos);
	~Beacon();

	static int idIterator; // not thread safe
	int id;
	
	void update();
	void draw();
};
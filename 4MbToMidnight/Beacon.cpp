#include "Beacon.hpp"

#include <iostream>
#include "Managers.hpp"

int Beacon::idIterator = 0; // not thread safe

Beacon::Beacon(glm::vec3 pos): Entity(
	pos,
	"Resources/Beacon.png",
	glm::vec2(0, 0),
	glm::vec2(0.5, 0.5),
	1200
) {
	id = Beacon::idIterator++; // not thread safe
	std::cout << id << " - " << PRINT_VEC3(pos) << std::endl;
}

Beacon::~Beacon() {

}

void Beacon::update() {

}

void Beacon::draw() {
	return;
}
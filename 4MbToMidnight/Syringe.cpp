#include "Syringe.hpp"

#include <iostream>

#include "Player.hpp"
#include "Managers.hpp"

Syringe::Syringe(glm::vec3 pos, int big) : Entity(
	pos,
	big == 0
		? "Resources/Syringe.png"
		: "Resources/BigSyringe.png",
	glm::vec2(0.25, 0.25),
	glm::vec2(0.5, 0.5),
	1012
) {
	heal = big + 1;
	
}

Syringe::~Syringe() {

}

void Syringe::update() {
	Player* player = (&PLAYER);
	if (player->health + heal >= 6 + heal)
		return;
	if (glm::length(glm::vec2(player->pos.x, player->pos.z) - glm::vec2(position.x, position.z)) <= radiusX) {
		GameManager::deleteEntity(this);
		player->health += heal;
		PLAY_S("Resources/Audio/bubble.ogg", position);
		if (player->health > 6)
			player->health = 6;
	}
}

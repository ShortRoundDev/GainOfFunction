#include "Fireball.hpp"

#include "Managers.hpp"

Fireball::Fireball(glm::vec3 pos, glm::vec3 dir) : Entity(
	pos,
	"Resources/fireball.png",
	glm::vec2(0.1f, 0.2f),
	glm::vec2(0.1f, 0.1f),
	1051
) {
	totalFrames = 2;
	totalAngles = 1;
	animations["idle"] = { 0, 2, 0 };
	currentAnimation = "idle";
	front = dir;
	moveVec = front * 0.1f;
	PLAY_S("Resources/Audio/fire01.ogg", position);
	shootable = true;
	dead = false;
}

Fireball::~Fireball() {

}

void Fireball::update() {
	animations[currentAnimation].iterate(0.5f);
	auto currentTile = TRY_WALL((int)(position.x), (int)(position.z));
	if (currentTile != NULL && SOLID(*currentTile)) {
		GameManager::deleteEntity(this);
		PLAY_S("Resources/Audio/explosion.ogg", position);
		return;
	}
	if (DIST_2(position, PLAYER.pos) < 0.25f) {
		(&PLAYER)->hurt(2);
		(&PLAYER)->moveVec += glm::normalize(moveVec) * 0.3f;
		GameManager::deleteEntity(this);
		PLAY_S("Resources/Audio/explosion.ogg", position);
		return;
	}
	position += moveVec;
}

void Fireball::hurt(int damage, glm::vec3 hitPos) {
	GameManager::deleteEntity(this);
}

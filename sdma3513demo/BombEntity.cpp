#include "BombEntity.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "Constants.h"
#include "World.h"
#include "SmokeParticleEntity.h"
#include "PlayerTurretEntity.h"
#include "Helper.h"


BombEntity::BombEntity() :
	PhysicsEntity(),
	explosion_r_(100),
	explosion_damage_(100),
	player_id_for_scoring_(Entity::INVALID_ENTITY_ID)
{
	set_rectangle(sf::FloatRect(sf::Vector2f(), sf::Vector2f(20.0f, 20.0f)));
	set_respect_gravity(false);
	set_velocity(sf::Vector2f(0.0f, 0.75f));
}


BombEntity::~BombEntity()
{
}


void BombEntity::tick()
{
	smoke_time_ += Constants::FRAME_TIME;

	auto world = get_world();
	if (world) {
		if (world->blocks_test_rectangle_collision(get_rectangle()).first) {
			// we have a collision - explode!
			const auto pos = sf::Vector2f(get_position().x + (0.5f * get_rectangle().width), get_position().y + (0.5f * get_rectangle().height));
			world->explode_at(
				static_cast<uint32_t>(pos.x / Block::BLOCK_SIZE.x), static_cast<uint32_t>(pos.y / Block::BLOCK_SIZE.y),
				explosion_r_, explosion_damage_
			);

			auto player = static_cast<PlayerTurretEntity*>(world->get_entity(player_id_for_scoring_));
			if (player)
				player->increment_player_bombs_missed();

			mark_for_deletion();
		}
		else if (get_position().y > static_cast<float>(Constants::VIDEO_HEIGHT))
			mark_for_deletion();
		else if (smoke_time_.asSeconds() > 0.4f) {
			auto smoke = std::make_unique<SmokeParticleEntity>();
			smoke->set_position(get_position() + sf::Vector2f(get_rectangle().width * 0.5f, -1.0f * get_rectangle().height));
			world->add_entity(static_cast<std::unique_ptr<Entity>>(std::move(smoke)));
			smoke_time_ -= sf::seconds(0.4f);
		}
	}
	
	PhysicsEntity::tick();
}


void BombEntity::render(sf::RenderTarget& target)
{
	sf::CircleShape bomb(0.5f);
	bomb.setScale(sf::Vector2f(get_rectangle().width, get_rectangle().height));
	bomb.setFillColor(sf::Color(255, Helper::get_random_int(0, 185), 0));
	bomb.setPosition(get_position());
	target.draw(bomb);
}
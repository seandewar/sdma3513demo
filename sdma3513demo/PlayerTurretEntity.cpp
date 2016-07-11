#include "PlayerTurretEntity.h"

#include <SFML/Graphics/RectangleShape.hpp>
#include <SFML/Graphics/CircleShape.hpp>

#include "Constants.h"
#include "World.h"
#include "PlayerMissileEntity.h"
#include "Helper.h"


PlayerTurretEntity::PlayerTurretEntity() :
	PhysicsEntity(),
	aim_angle_(0.0f),
	player_score_(0),
	player_missed_bombs_(0),
	missile_shoot_delay_(sf::seconds(0.2f))
{
	set_rectangle(sf::FloatRect(sf::Vector2f(), sf::Vector2f(15.0f, 32.5f)));
	set_respect_gravity(false);
}


PlayerTurretEntity::~PlayerTurretEntity()
{
}


void PlayerTurretEntity::fire_missile()
{
	if (next_missile_available_time_.asSeconds() <= 0.0f) {
		auto world = get_world();
		if (world) {
			const auto aim_angle_rads = (3.141f / 180.0f) * (aim_angle_ - 90.0f);
			const auto missile_velo = 11.5f * sf::Vector2f(cosf(aim_angle_rads), sinf(aim_angle_rads));

			auto missile = std::make_unique<PlayerMissileEntity>();
			missile->set_position(get_position() + sf::Vector2f(2.0f - (0.5f * missile->get_rectangle().width), (0.5f * get_rectangle().height) - (0.5f * missile->get_rectangle().height)));
			missile->set_velocity(missile_velo);
			missile->assign_player_for_scoring(get_id());
			world->add_entity(static_cast<std::unique_ptr<Entity>>(std::move(missile)));

			// extra "collat" missiles
			for (int i = 0; i < 2; ++i) {
				auto missile_collat = std::make_unique<PlayerMissileEntity>();
				missile_collat->set_rectangle(sf::FloatRect(sf::Vector2f(), sf::Vector2f(8.0f, 8.0f)));
				missile_collat->set_position(get_position() + sf::Vector2f(2.0f - (0.5f * missile_collat->get_rectangle().width), (0.5f * get_rectangle().height) - (0.5f * missile_collat->get_rectangle().height)));
				missile_collat->set_velocity(Helper::get_random_float(1.05f, 1.15f) * missile_velo);
				missile_collat->assign_player_for_scoring(get_id());
				world->add_entity(static_cast<std::unique_ptr<Entity>>(std::move(missile_collat)));
			}
		}

		next_missile_available_time_ += missile_shoot_delay_;
	}
}


void PlayerTurretEntity::tick()
{
	PhysicsEntity::tick();
	if (next_missile_available_time_.asSeconds() > 0.0f)
		next_missile_available_time_ -= Constants::FRAME_TIME;

	auto world = get_world();
	if (world) {
		// snap to highest point of elevation under the player
		auto turret_bottom_y = get_position().y + get_rectangle().height - 5.0f;
		while (turret_bottom_y < Constants::VIDEO_HEIGHT) {
			const auto test_collision_rect = sf::FloatRect(sf::Vector2f(get_position().x + (get_rectangle().width * 0.5f), turret_bottom_y), Block::BLOCK_SIZE);
			if (world->blocks_test_rectangle_collision(test_collision_rect).first)
				break; // bottom already seated on a block

			turret_bottom_y += Block::BLOCK_SIZE.y; // try again 1 block lower
		}

		// valid snap found
		if (turret_bottom_y < Constants::VIDEO_HEIGHT)
			set_position(sf::Vector2f(get_position().x, turret_bottom_y - get_rectangle().height + 5.0f));
	}
}


void PlayerTurretEntity::render(sf::RenderTarget& target)
{
	sf::RectangleShape turret_cannon(sf::Vector2f(4.0f, 17.5f));
	turret_cannon.setPosition(get_position());
	turret_cannon.setFillColor(sf::Color(55, 55, 55));

	sf::Transform turret_cannon_transform;
	turret_cannon_transform.rotate(aim_angle_, get_position() + sf::Vector2f(2.0f, 18.5f));
	target.draw(turret_cannon, turret_cannon_transform);

	sf::CircleShape turret_base(7.5f);
	turret_base.setPosition(get_position() + sf::Vector2f(-5.5f, 17.0f));
	turret_base.setFillColor(sf::Color(100, 100, 100));
	target.draw(turret_base);
}

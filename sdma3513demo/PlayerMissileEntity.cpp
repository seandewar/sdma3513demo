#include "PlayerMissileEntity.h"

#include <SFML/Graphics/CircleShape.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "World.h"
#include "Helper.h"
#include "Constants.h"
#include "PlayerTurretEntity.h"
#include "SmokeParticleEntity.h"
#include "ExplosionEffectEntity.h"
#include "BombEntity.h"
#include "BlockGibEntity.h"


PlayerMissileEntity::PlayerMissileEntity() :
	PhysicsEntity(),
	player_id_for_scoring_(Entity::INVALID_ENTITY_ID),
	smoke_time_(sf::seconds(0.4f)) // smoke as soon as it comes out of the turret!
{
	set_rectangle(sf::FloatRect(sf::Vector2f(), sf::Vector2f(12.5f, 12.5f)));
	set_respect_gravity(true);
}


PlayerMissileEntity::~PlayerMissileEntity()
{
}


void PlayerMissileEntity::tick()
{
	smoke_time_ += Constants::FRAME_TIME;

	/*
	// @todo HACK HACK - eliminates a lot of cases of collision failing because we don't do sweeping
	set_velocity(sf::Vector2f(
		std::max(std::min(get_velocity().x, get_rectangle().width), -1.0f * get_rectangle().width),
		std::max(std::min(get_velocity().y, get_rectangle().height), -1.0f * get_rectangle().height)
	));
	*/

	auto world = get_world();
	if (world) {
		const auto collision_ent_id = world->entity_test_rectangle_collision(get_rectangle());
		if (collision_ent_id != Entity::INVALID_ENTITY_ID) {
			// collision with another entity - award score and explode if bomb entity (or derived of)
			auto collision_ent = dynamic_cast<BombEntity*>(world->get_entity(collision_ent_id));
			if (collision_ent) {
				// fire fx
				const int fire_fx_amount = Helper::get_random_int(50, 75);
				for (int i = 0; i < fire_fx_amount; ++i) {
					auto fire_fx_gib = std::make_unique<BlockGibEntity>();
					fire_fx_gib->set_position(sf::Vector2f(
						Helper::get_random_float(collision_ent->get_position().x, collision_ent->get_position().x + collision_ent->get_rectangle().width),
						Helper::get_random_float(collision_ent->get_position().y, collision_ent->get_position().y + collision_ent->get_rectangle().height)
					));
					fire_fx_gib->assign_block(std::move(std::make_unique<Block>(BlockType::FireFX)));
					fire_fx_gib->set_velocity(sf::Vector2f(Helper::get_random_float(0.1f, 0.4f) * get_velocity().x, Helper::get_random_float(0.4f, 1.25f) * get_velocity().y));
					world->add_entity(static_cast<std::unique_ptr<Entity>>(std::move(fire_fx_gib)));
				}
				
				auto explosion_effect = std::make_unique<ExplosionEffectEntity>();
				const auto explosion_size = 2.5f * sf::Vector2f(collision_ent->get_rectangle().width, collision_ent->get_rectangle().height);
				const auto collision_ent_size = sf::Vector2f(collision_ent->get_rectangle().width, collision_ent->get_rectangle().height);
				explosion_effect->set_rectangle(sf::FloatRect(collision_ent->get_position() + (0.5f * collision_ent_size) - (0.5f * explosion_size), explosion_size));
				world->add_entity(static_cast<std::unique_ptr<Entity>>(std::move(explosion_effect)));

				// award player score depending on air time? ... idk
				auto player = static_cast<PlayerTurretEntity*>(world->get_entity(player_id_for_scoring_));
				if (player)
					player->add_to_player_score(100);

				collision_ent->mark_for_deletion();
				mark_for_deletion();
				return;
			}
		}
		
		const auto collision_block = world->blocks_test_rectangle_collision(get_rectangle()).first;
		if (collision_block && collision_block->get_type() != BlockType::Glass && collision_block->get_type() != BlockType::Brick) {
			// collision with world - do no damage
			auto explosion_effect = std::make_unique<ExplosionEffectEntity>();
			const auto explosion_size = 2.5f * sf::Vector2f(get_rectangle().width, get_rectangle().height);
			explosion_effect->set_rectangle(sf::FloatRect(get_position() - (0.5f * explosion_size), explosion_size));
			world->add_entity(static_cast<std::unique_ptr<Entity>>(std::move(explosion_effect)));

			mark_for_deletion();
		}
		else if (get_position().y > static_cast<float>(Constants::VIDEO_HEIGHT))
			mark_for_deletion();
		else if (smoke_time_.asSeconds() > 0.4f) {
			auto smoke = std::make_unique<SmokeParticleEntity>();
			smoke->set_rectangle(get_rectangle()); // @todo - spawn smoke behind (depending on velo)
			world->add_entity(static_cast<std::unique_ptr<Entity>>(std::move(smoke)));
			smoke_time_ -= sf::seconds(0.4f);
		}
	}

	PhysicsEntity::tick();
}


void PlayerMissileEntity::render(sf::RenderTarget& target)
{
	sf::CircleShape bomb(0.5f);
	bomb.setScale(sf::Vector2f(get_rectangle().width, get_rectangle().height));
	bomb.setFillColor(sf::Color(Helper::get_random_int(100, 255), Helper::get_random_int(0, 185), Helper::get_random_int(0, 185)));
	bomb.setPosition(get_position());
	target.draw(bomb);
}

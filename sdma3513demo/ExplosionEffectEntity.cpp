#include "ExplosionEffectEntity.h"

#include <SFML/Graphics/Sprite.hpp>

#include "Constants.h"
#include "World.h"


ExplosionEffectEntity::ExplosionEffectEntity() :
	PhysicsEntity(true),
	explosion_density_(1.0f)
{
	set_respect_gravity(false);
}


ExplosionEffectEntity::~ExplosionEffectEntity()
{
}


void ExplosionEffectEntity::tick()
{
	PhysicsEntity::tick();

	explosion_density_ -= 3.0f * Constants::FRAME_TIME.asSeconds();
	explosion_density_ = std::min(explosion_density_, 1.0f);
	if (explosion_density_ <= 0.0f)
		mark_for_deletion();
}


void ExplosionEffectEntity::render(sf::RenderTarget& target)
{
	auto world = get_world();
	if (world) {
		const auto explosion_anim_textures = world->get_explosion_anim_textures();
		if (explosion_anim_textures && explosion_anim_textures->size() > 0) {
			const std::size_t anim_frame = std::min(static_cast<std::size_t>((1.0f - explosion_density_) * explosion_anim_textures->size()), explosion_anim_textures->size() - 1);
			const auto& frame_texture = (*explosion_anim_textures)[anim_frame];

			sf::Sprite explosion(frame_texture);
			explosion.setScale(sf::Vector2f(get_rectangle().width / frame_texture.getSize().x, get_rectangle().height / frame_texture.getSize().y));
			explosion.setPosition(get_position());
			target.draw(explosion);
		}
	}
}

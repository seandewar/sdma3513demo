#include "SmokeParticleEntity.h"

#include <SFML/Graphics/CircleShape.hpp>

#include "Helper.h"
#include "Constants.h"


SmokeParticleEntity::SmokeParticleEntity() :
	PhysicsEntity(true),
	smoke_density_(0.15f),
	smoke_angle_(Helper::get_random_float(0.0f, 360.0f))
{
	set_rectangle(sf::FloatRect(sf::Vector2f(), sf::Vector2f(10.0f, 10.0f)));
	set_respect_gravity(false);
	set_velocity(sf::Vector2f(Helper::get_random_float(-0.2f, 0.2f), Helper::get_random_float(-1.5f, -0.5f)));
}


SmokeParticleEntity::~SmokeParticleEntity()
{
}


void SmokeParticleEntity::tick()
{
	set_rectangle(sf::FloatRect(get_position(), 30.8f * Constants::FRAME_TIME.asSeconds() * sf::Vector2f(get_rectangle().width, get_rectangle().height)));
	smoke_density_ -= 0.08f * Constants::FRAME_TIME.asSeconds();
	smoke_angle_ += 45.0f * Constants::FRAME_TIME.asSeconds();

	if (smoke_density_ <= 0.0f)
		mark_for_deletion();
}


void SmokeParticleEntity::render(sf::RenderTarget& target)
{
	sf::CircleShape smoke(0.5f);
	smoke.setScale(sf::Vector2f(get_rectangle().width, get_rectangle().height));
	smoke.setPosition(get_position());
	smoke.setRotation(smoke_angle_);
	smoke.setFillColor(sf::Color(210, 210, 210, static_cast<sf::Uint8>(smoke_density_ * 200)));
	target.draw(smoke);
}
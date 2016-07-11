#include "PhysicsEntity.h"

#include "Constants.h"
#include "World.h"


PhysicsEntity::PhysicsEntity(bool fx_only) :
	Entity(fx_only),
	respect_gravity_(true)
{
}


PhysicsEntity::~PhysicsEntity()
{
}


void PhysicsEntity::tick()
{
	const auto world = get_world();
	if (world) {
		const auto new_pos = get_position() + velocity_;
		if (respect_gravity_)
			velocity_.y += world->get_gravity_accel() * Constants::FRAME_TIME.asSeconds();

		set_position(new_pos);
	}
}
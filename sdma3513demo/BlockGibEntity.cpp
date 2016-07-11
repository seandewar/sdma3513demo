#include "BlockGibEntity.h"

#include "World.h"
#include "Helper.h"
#include "Constants.h"


BlockGibEntity::BlockGibEntity() :
	PhysicsEntity(true)
{
	set_respect_gravity(true);
	set_rectangle(sf::FloatRect(sf::Vector2f(), 4.0f * Block::BLOCK_SIZE));
}


BlockGibEntity::~BlockGibEntity()
{
}


void BlockGibEntity::tick()
{
	PhysicsEntity::tick();

	auto world = get_world();
	if (world) {
		const auto collision_info = world->blocks_test_rectangle_collision(get_rectangle());
		if (collision_info.first || get_position().y > static_cast<float>(Constants::VIDEO_HEIGHT))
			mark_for_deletion();
	}
}



void BlockGibEntity::render(sf::RenderTarget& target)
{
	if (block_)
		block_->render(target, get_position(), sf::Vector2f(get_rectangle().width, get_rectangle().height));
}

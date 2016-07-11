#include "World.h"

#include <random>
#include <cassert>

#include <SFML/Graphics/Sprite.hpp>
#include <SFML/Graphics/RectangleShape.hpp>

#include "Helper.h"
#include "BlockGibEntity.h"
#include "ExplosionEffectEntity.h"


World::World(uint32_t blocks_width, uint32_t blocks_height) :
	blocks_width_(blocks_width),
	blocks_height_(blocks_height),
	update_blocks_render_texture_(true),
	entities_next_id_(0),
	explosion_anim_textures_(nullptr)
{
	blocks_.resize(blocks_width_ * blocks_height_);

	const unsigned int blocks_render_texture_width = static_cast<unsigned int>(blocks_width_ * Block::BLOCK_SIZE.x);
	const unsigned int blocks_render_texture_height = static_cast<unsigned int>(blocks_height_ * Block::BLOCK_SIZE.y);
	if (!blocks_render_texture_.create(blocks_render_texture_width, blocks_render_texture_height)) {
		fprintf(stderr, "Failed to create World blocks render texture! (%dx%d)\n",
			blocks_render_texture_width, blocks_render_texture_height);
		throw std::runtime_error("Failed to create blocks render texture");
	}

	printf("World created (%dx%d blocks)\n", blocks_width_, blocks_height_);
}


World::~World()
{
}


void World::refresh_blocks_render_texture()
{
	printf("Performing full refresh on blocks render texture..\n");
	blocks_marked_for_texture_update_.clear();

	blocks_render_texture_.clear(sf::Color(0, 0, 0, 0));

	for (uint32_t y = 0; y < blocks_height_; ++y) {
		printf("Texture refresh is %.2f%% complete.. (Hold on!)\n", ((y * 100.0f) / (blocks_height_ - 1)));

		for (uint32_t x = 0; x < blocks_width_; ++x) {
			const auto block = get_block_at(x, y);
			if (block) {
				block->render(
					blocks_render_texture_,
					sf::Vector2f(x * Block::BLOCK_SIZE.x, y * Block::BLOCK_SIZE.y)
				);
			}
		}
	}

	printf("Blocks texture refresh finished (and hopefully didn't take too long)!\n");
}


void World::remove_entity(decltype(entities_)::iterator it)
{
	if (it == entities_.end())
		return;

	const auto entity_id = it->first;
	const auto entity = it->second.get();

	const auto entity_name = entity ? entity->get_name() : "NULL";
	//printf("Removing entity %d (%s) from world\n", static_cast<int>(entity_id), entity_name.c_str());

	if (!entity->is_fx_only()) {
		auto non_fx_it = std::find(entities_non_fx_.begin(), entities_non_fx_.end(), entity_id);
		assert(non_fx_it != entities_non_fx_.end());
		
		// swap and pop to remove element
		printf("Removing non-fx entity %s (id %d)\n", entity_name.c_str(), static_cast<int>(entity_id));
		std::swap(*non_fx_it, entities_non_fx_.back());
		entities_non_fx_.pop_back();
	}

	entities_.erase(it);
}


void World::update_blocks_render_texture(uint32_t x, uint32_t y)
{
	if (!update_blocks_render_texture_)
		return;

	const auto block = get_block_at(x, y);
	if (block) {
		block->render(
			blocks_render_texture_,
			sf::Vector2f(x * Block::BLOCK_SIZE.x, y * Block::BLOCK_SIZE.y)
		);
	}
	else {
		sf::RectangleShape block_eraser(Block::BLOCK_SIZE);
		block_eraser.setFillColor(sf::Color(0, 0, 0, 0));
		block_eraser.setPosition(x * Block::BLOCK_SIZE.x, y * Block::BLOCK_SIZE.y);
		blocks_render_texture_.draw(block_eraser, sf::BlendNone);
	}
}


void World::clear()
{
	printf("Clearing world..\n");
	for (auto it = entities_.begin(); it != entities_.end();)
		remove_entity(it++);

	entities_next_id_ = 0;

	for (auto& b : blocks_)
		b.reset();

	blocks_render_texture_.clear(sf::Color(0, 0, 0, 0));
}


void World::generate_new_world(unsigned int seed)
{
	update_blocks_render_texture_ = false;

	WorldGen gen(*this, seed);
	clear();
	gen.generate_world();

	refresh_blocks_render_texture();
	update_blocks_render_texture_ = true;
}


void World::tick()
{
	// update blocks
	while (!blocks_marked_for_state_update_.empty()) {
		const auto block_pos = blocks_marked_for_state_update_.front();
		const auto block = get_block_at(block_pos.x, block_pos.y);
		if (block && block->is_destroyed())
			remove_block_at(block_pos.x, block_pos.y);

		blocks_marked_for_state_update_.pop();
	}

	// update ents
	for (auto it = entities_.begin(); it != entities_.end();) {
		auto& e = *it;
		auto entity = e.second.get();

		if (!entity || entity->is_marked_for_deletion())
			remove_entity(it++);
		else {
			entity->tick();
			++it;
		}
	}
}


void World::render(sf::RenderTarget& target)
{
	// render blocks
	uint32_t updated_blocks = 0;
	bool force_catchup = false;

	if (blocks_marked_for_texture_update_.size() > MAX_BLOCKS_TEXTURE_UPDATES_FOR_CATCHUP) {
		printf("!!! Too many block texture updates scheduled (%d scheduled) - Forcing catch-up for this frame!!!...\n", 
			static_cast<int>(blocks_marked_for_texture_update_.size()));

		force_catchup = true;
	}

	while (!blocks_marked_for_texture_update_.empty() && (updated_blocks <= MAX_BLOCKS_TEXTURE_UPDATES_PER_RENDER || force_catchup)) {
		const std::size_t i = Helper::get_random_int(0, blocks_marked_for_texture_update_.size() - 1);
		const auto block_pos = blocks_marked_for_texture_update_[i];
		update_blocks_render_texture(block_pos.x, block_pos.y);

		std::swap(blocks_marked_for_texture_update_[i], blocks_marked_for_texture_update_.back());
		blocks_marked_for_texture_update_.pop_back();
		++updated_blocks;
	}

	blocks_render_texture_.display();
	sf::Sprite blocks_sprite(blocks_render_texture_.getTexture());
	target.draw(blocks_sprite);

	// render ents
	for (auto& e : entities_) {
		auto entity = e.second.get();
		if (entity && !entity->is_marked_for_deletion())
			entity->render(target);
	}
}


EntityId World::add_entity(std::unique_ptr<Entity>& entity)
{
	assert(entities_.find(entities_next_id_) == entities_.end() && "next entity ID already assigned to active entity!");
	
	if (entity.get()) {
		//printf("Adding entity %d (%s) to world\n", static_cast<int>(entities_next_id_), entity.get()->get_name().c_str());
		entity->assign_world(this, entities_next_id_);

		if (!entity->is_fx_only())
			entities_non_fx_.emplace_back(entities_next_id_);

		entities_.emplace(entities_next_id_, std::move(entity));
		return entities_next_id_++;
	}
	else
		return Entity::INVALID_ENTITY_ID;
}


Entity* World::get_entity(EntityId id)
{
	auto it = entities_.find(id);
	if (it == entities_.end())
		return nullptr;

	return it->second.get();
}


void World::explode_at(uint32_t x_pos, uint32_t y_pos, uint16_t r, uint32_t center_damage, double gib_chance)
{
	if (r == 0 || static_cast<int64_t>(x_pos) - r >= static_cast<int64_t>(blocks_width_) || static_cast<int64_t>(y_pos) - r >= static_cast<int64_t>(blocks_height_))
		return;

	const uint32_t start_x = static_cast<uint32_t>(std::max(static_cast<int64_t>(x_pos) - r, static_cast<int64_t>(0)));
	const uint32_t start_y = static_cast<uint32_t>(std::max(static_cast<int64_t>(y_pos) - r, static_cast<int64_t>(0)));
	const uint32_t end_x = std::min(x_pos + r + 1, blocks_width_);
	const uint32_t end_y = std::min(y_pos + r + 1, blocks_height_);
	const uint64_t r_sq = r * r;

	assert(start_x <= end_x && start_y <= end_y);

	for (uint32_t y = start_y; y < end_y; ++y) {
		for (uint32_t x = start_x; x < end_x; ++x) {
			// back to a-level with circle equations!
			const uint64_t inside_r_sq = ((x - x_pos) * (x - x_pos)) + ((y - y_pos) * (y - y_pos));

			if (inside_r_sq <= r_sq) {
				auto block = get_block_at(x, y);
				if (block) {
					// min damage of explosion is 0.1 * center_damage on a block that is in-range
					const uint32_t block_damage = static_cast<uint32_t>(center_damage * (1.0f - std::max(0.1f, static_cast<float>(inside_r_sq) / r_sq)));
					block->damage(block_damage);
					mark_block_for_update(x, y);

					// roll to spawn a gib of this block if we destroyed it
					if ((block->is_destroyed() || block->get_type() == BlockType::Water) && Helper::get_random_bool(gib_chance)) {
						auto gib_entity = std::make_unique<BlockGibEntity>();
						gib_entity->set_position(sf::Vector2f(x * Block::BLOCK_SIZE.x, y * Block::BLOCK_SIZE.y));
						gib_entity->set_velocity(sf::Vector2f(Helper::get_random_float(-2.0f, 2.0f), Helper::get_random_float(-5.0f, -1.0f)));
						gib_entity->assign_block(std::make_unique<Block>(*block)); // copy of block

						add_entity(static_cast<std::unique_ptr<Entity>>(std::move(gib_entity)));
					}
				}
			}
		}
	}

	auto explosion_effect = std::make_unique<ExplosionEffectEntity>();
	explosion_effect->set_rectangle(sf::FloatRect(
		sf::Vector2f((x_pos - r) * Block::BLOCK_SIZE.x, (y_pos - r) * Block::BLOCK_SIZE.y),
		sf::Vector2f(2.0f * Block::BLOCK_SIZE.x * r, 2.0f * Block::BLOCK_SIZE.y * r)
	));
	
	add_entity(static_cast<std::unique_ptr<Entity>>(std::move(explosion_effect)));
}


std::pair<Block*, sf::Vector2<uint32_t>> World::blocks_test_rectangle_collision(sf::FloatRect rect)
{
	// this will probably allow for rectangles with -ve widths or heights to be supported
	if (rect.width < 0.0f) {
		rect.left += rect.width;
		rect.width *= -1.0f;
	}
	if (rect.height < 0.0f) {
		rect.top += rect.height;
		rect.height *= -1.0f;
	}

	int64_t start_x = static_cast<int64_t>(rect.left / Block::BLOCK_SIZE.x);
	int64_t start_y = static_cast<int64_t>(rect.top / Block::BLOCK_SIZE.y);
	int64_t end_x = static_cast<int64_t>(ceilf((rect.left + rect.width) / Block::BLOCK_SIZE.x) + 1);
	int64_t end_y = static_cast<int64_t>(ceilf((rect.top + rect.height) / Block::BLOCK_SIZE.y) + 1);

	// check if whole rect isn't OOB
	if ((start_x < 0 && end_x < 0) || (start_x >= blocks_width_ && end_x >= blocks_width_)
		|| (start_y < 0 && end_y < 0) || (start_y >= blocks_height_ && end_y >= blocks_height_))
		return std::make_pair(nullptr, sf::Vector2<uint32_t>(-1, -1)); // return null Block and bad pos

	start_x = std::max(start_x, static_cast<int64_t>(0));
	start_y = std::max(start_y, static_cast<int64_t>(0));
	end_x = std::min(end_x, static_cast<int64_t>(blocks_width_));
	end_y = std::min(end_y, static_cast<int64_t>(blocks_height_));

	for (uint32_t y = static_cast<uint32_t>(start_y); y < end_y; ++y) {
		for (uint32_t x = static_cast<uint32_t>(start_x); x < end_x; ++x) {
			auto block = get_block_at(x, y);
			if (block && !block->is_destroyed())
				return std::make_pair(block, sf::Vector2<uint32_t>(x, y));
		}
	}

	return std::make_pair(nullptr, sf::Vector2<uint32_t>(-1, -1)); // Just give a bad pos if we dont find anything
}


EntityId World::entity_test_rectangle_collision(sf::FloatRect rect)
{
	for (std::size_t i = 0; i < entities_non_fx_.size(); ++i) {
		const auto id = entities_non_fx_[i];
		const auto entity = dynamic_cast<IRectangle*>(get_entity(id));
		if (entity) {
			const auto ent_rect = entity->get_rectangle();
			if (rect.intersects(ent_rect))
				return id;
		}
	}
	
	return Entity::INVALID_ENTITY_ID;
}


Block* World::create_block_at(uint32_t x, uint32_t y, BlockType type)
{
	auto block = (blocks_[get_block_index(x, y)] = std::make_unique<Block>(type)).get();
	mark_block_for_update(x, y);
	return block;
}


void World::remove_block_at(uint32_t x, uint32_t y)
{
	blocks_[get_block_index(x, y)].reset();
	mark_block_for_update(x, y);
}


void WorldGen::gen_terrain(uint32_t y_top_min, uint32_t y_top_max, int8_t y_step_min, int8_t y_step_max, double y_step_change_chance)
{
	if (y_step_min > y_step_max || y_top_min > y_top_max)
		throw std::runtime_error("Invalid ranges for y_top_* or y_step_*");

	printf("Generating terrain (y_top_min: %d, y_top_max: %d, y_step_min: %d, y_step_max: %d, y_step_change_chance: %f) ..\n",
		y_top_min, y_top_max, y_step_min, y_step_max, y_step_change_chance);

	// start pos of this column of blocks
	uint32_t y_top = Helper::get_random_int(rng_, y_top_min, y_top_max);

	// rip cache
	for (uint32_t x = 0; x < world_.get_blocks_width(); ++x) {
		for (uint32_t y = y_top; y < world_.get_blocks_height(); ++y) {
			// 1.0 = bottom of column, 0.0 = top of column
			const float y_from_top = (y - y_top) / static_cast<float>(world_.get_blocks_height() - y_top);

			if (y_from_top <= 0.015f)
				world_.create_block_at(x, y, BlockType::Grass);
			else if (y_from_top <= 0.2f)
				world_.create_block_at(x, y, BlockType::Dirt);
			else if (y_from_top <= 0.9f)
				world_.create_block_at(x, y, BlockType::Stone);
			else
				world_.create_block_at(x, y, BlockType::Water);
		}

		if (Helper::get_random_bool(rng_, y_step_change_chance)) {
			y_top += Helper::get_random_int(rng_, y_step_min, y_step_max);
			y_top = std::max(std::min(y_top, y_top_max), y_top_min);
		}
	}
}


void WorldGen::gen_buildings(uint32_t building_x_min, uint32_t building_x_max, uint32_t building_y_min, uint32_t building_y_max,
	uint32_t building_foundation_size, uint32_t middle_clearance, double building_gen_chance)
{
	if (building_x_min > building_x_max || building_y_min > building_y_max)
		throw std::runtime_error("Invalid ranges for building_y_* or building_x_*");

	printf("Generating buildings (building_x_min: %d, building_x_max: %d, building_y_min: %d, building_y_max: %d, building_foundation_size: %d, middle_clearance: %d, building_gen_chance: %f) ..\n",
		building_x_min, building_x_max, building_y_min, building_y_max, building_foundation_size, middle_clearance, building_gen_chance);

	const uint32_t world_clearance_x_min = (world_.get_blocks_width() / 2) - middle_clearance;
	const uint32_t world_clearance_x_max = (world_.get_blocks_width() / 2) + middle_clearance;

	// rip cache again
	for (uint32_t j = 0; j < world_.get_blocks_width(); ++j) {
		// roll for building
		if ((j < world_clearance_x_min || j > world_clearance_x_max) && Helper::get_random_bool(building_gen_chance)) {
			const uint32_t building_w = Helper::get_random_int(building_x_min, building_x_max);
			const uint32_t building_h = Helper::get_random_int(building_y_min, building_y_max);
			uint32_t building_bottom = world_.get_blocks_height(); // take height of world as being invalid y

			// find top of terrain here
			for (uint32_t y = 0; y < world_.get_blocks_height() - building_foundation_size - 1; ++y) {
				if (world_.get_block_at(j, y)) {
					building_bottom = y + building_foundation_size;
					break;
				}
			}

			// we can start building at this level yay
			if (building_bottom != world_.get_blocks_height()) {
				for (uint32_t y = building_bottom; y >= 0 && y >= building_bottom - building_h; --y) {
					for (uint32_t x = j; x < world_.get_blocks_width() && x <= j + building_w; ++x) {
						if (!world_.get_block_at(x, y)) {
							if ((x - j) % 10 >= 6 && (y - building_bottom) % 16 >= 12 && x <= j + building_w - 2 && y > building_bottom - building_h + 4)
								world_.create_block_at(x, y, BlockType::Glass);
							else
								world_.create_block_at(x, y, BlockType::Brick);
						}
					}
				}

				j += building_w + 1;
			}
		}
	}
}


WorldGen::WorldGen(World & world, unsigned int seed) :
	world_(world),
	seed_(seed),
	rng_(seed)
{
}


WorldGen::~WorldGen()
{
}


void WorldGen::generate_world()
{
	printf("Generating world (seed: %u) ..\n", seed_);

	gen_terrain(
		static_cast<uint32_t>(0.65f * world_.get_blocks_height()),
		static_cast<uint32_t>(0.825f * world_.get_blocks_height()),
		-2, 2,
		0.25
	);

	gen_buildings(24, 24, 20, 110, 20, static_cast<uint32_t>(100 / Block::BLOCK_SIZE.x), 0.15);
}

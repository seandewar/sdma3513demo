#pragma once

#include <vector>
#include <queue>
#include <unordered_map>
#include <memory>
#include <random>

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/RenderTexture.hpp>

#include "Block.h"
#include "Entity.h"

class World
{
	const std::vector<sf::Texture>* explosion_anim_textures_;

	std::vector<std::unique_ptr<Block>> blocks_;
	uint32_t blocks_width_, blocks_height_;
	std::queue<sf::Vector2<uint32_t>> blocks_marked_for_state_update_;
	std::vector<sf::Vector2<uint32_t>> blocks_marked_for_texture_update_;

	sf::RenderTexture blocks_render_texture_;
	bool update_blocks_render_texture_;

	std::unordered_map<EntityId, std::unique_ptr<Entity>> entities_;
	std::vector<EntityId> entities_non_fx_;
	EntityId entities_next_id_;

	void remove_entity(decltype(entities_)::iterator it);

	inline std::size_t get_block_index(uint32_t x, uint32_t y)
	{
		if (x >= blocks_width_ || y >= blocks_height_)
			throw std::runtime_error("Block index out of range");

		return x + (blocks_width_ * y);
	}

	void update_blocks_render_texture(uint32_t x, uint32_t y);

public:
	static const uint32_t MAX_BLOCKS_TEXTURE_UPDATES_PER_RENDER = 3000;
	static const uint32_t MAX_BLOCKS_TEXTURE_UPDATES_FOR_CATCHUP = 8 * MAX_BLOCKS_TEXTURE_UPDATES_PER_RENDER;

	World(uint32_t blocks_width, uint32_t blocks_height);
	~World();

	void refresh_blocks_render_texture();

	void clear();
	void generate_new_world(unsigned int seed);

	inline void mark_block_for_update(uint32_t x, uint32_t y)
	{
		blocks_marked_for_state_update_.emplace(x, y);
		if (update_blocks_render_texture_)
			blocks_marked_for_texture_update_.emplace_back(x, y);
	}

	void tick();
	void render(sf::RenderTarget& target);

	inline float get_gravity_accel() const { return 4.5f; }

	EntityId add_entity(std::unique_ptr<Entity>& entity);
	Entity* get_entity(EntityId id);
	inline void remove_entity(EntityId id) { remove_entity(entities_.find(id)); }
	
	void explode_at(uint32_t x, uint32_t y, uint16_t r, uint32_t center_damage, double gib_chance = 0.2);

	/**
	 * Returns a pair with a pointer to the block and its x and y pos if the rectangle intersects with it.
	 * Returns nullptr Block if no collision.
	 */
	std::pair<Block*, sf::Vector2<uint32_t>> blocks_test_rectangle_collision(sf::FloatRect rect);

	/**
	 * Returns the ID of the entity the rectangle collided with, otherwise Entity::INVALID_ENTITY_ID
	 * Does not test fx-only entities.
	 */
	EntityId entity_test_rectangle_collision(sf::FloatRect rect);

	Block* create_block_at(uint32_t x, uint32_t y, BlockType type);
	void remove_block_at(uint32_t x, uint32_t y);

	inline void set_block_at(uint32_t x, uint32_t y, std::unique_ptr<Block>& block) { blocks_[get_block_index(x, y)] = std::move(block); }

	inline Block* get_block_at(uint32_t x, uint32_t y) { return blocks_[get_block_index(x, y)].get(); }
	inline const Block* get_block_at(uint32_t x, uint32_t y) const { return get_block_at(x, y); }

	inline uint32_t get_blocks_width() const { return blocks_width_; }
	inline uint32_t get_blocks_height() const { return blocks_height_; }

	inline void set_update_blocks_render_texture(bool val) { update_blocks_render_texture_ = val; }
	inline bool get_update_blocks_render_texture() const { return update_blocks_render_texture_; }

	inline void set_explosion_anim_textures(const std::vector<sf::Texture>* anim_frames) { explosion_anim_textures_ = anim_frames; }
	inline const std::vector<sf::Texture>* get_explosion_anim_textures() const { return explosion_anim_textures_; }
};

class WorldGen
{
	World& world_;
	unsigned int seed_;
	std::mt19937 rng_;

	void gen_terrain(uint32_t y_top_min, uint32_t y_top_max, int8_t y_step_min, int8_t y_step_max, double y_step_change_chance);
	void gen_buildings(uint32_t building_x_min, uint32_t building_x_max, uint32_t building_y_min, uint32_t building_y_max, 
		uint32_t building_foundation_size, uint32_t middle_clearance, double building_gen_chance);

public:
	WorldGen(World& world, unsigned int seed);
	~WorldGen();

	void generate_world();
};
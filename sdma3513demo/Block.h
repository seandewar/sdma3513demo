#pragma once

#include <algorithm>
#include <cstdint>

#include <SFML/Graphics/RenderTarget.hpp>

enum class BlockType
{
	Stone,
	Dirt,
	Grass,
	Brick,
	Glass,
	Bedrock,
	Water,
	FireFX
};

class Block
{
	BlockType type_;
	uint32_t health_;
	sf::Color block_color_mul_;

public:
	static const sf::Vector2f BLOCK_SIZE;

	static uint32_t get_block_max_health(BlockType type);

	// even setting the health through ctor will NOT allow it to be > max health
	Block(BlockType type, uint32_t health);
	Block(BlockType type);
	~Block();

	void render(sf::RenderTarget& target, const sf::Vector2f& draw_pos, const sf::Vector2f& draw_size = BLOCK_SIZE, float draw_rotation = 0.0f);
	
	inline BlockType get_type() const { return type_; }

	inline void set_health(uint32_t new_health) { health_ = std::min(new_health, get_max_health()); }
	inline void damage(uint32_t damage_amount)
	{
		if (type_ != BlockType::Bedrock && type_ != BlockType::Water)
			health_ -= std::min(damage_amount, health_);
	}

	inline uint32_t get_health() const { return health_; }
	inline uint32_t get_max_health() const { return get_block_max_health(type_); }

	inline bool is_destroyed() const { return health_ == 0; }
};


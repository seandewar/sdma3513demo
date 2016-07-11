#include "Block.h"

#include <SFML/Graphics/RectangleShape.hpp>

#include "Helper.h"


const sf::Vector2f Block::BLOCK_SIZE(0.5f, 0.5f);


uint32_t Block::get_block_max_health(BlockType type)
{
	switch (type) {
	case BlockType::Dirt:
		return 75;

	case BlockType::Brick:
		return 45;

	case BlockType::Glass:
		return 38;
		
	case BlockType::Grass:
		return 35;

	case BlockType::Stone:
		return 150;

	case BlockType::FireFX:
		return 1; // purely for FX

	case BlockType::Bedrock:
	case BlockType::Water:
		return 1000; // cannot be damaged anyway

	default:
		return 0;
	}
}


Block::Block(BlockType type, uint32_t health) :
	type_(type)
{
	set_health(health);

	const auto color_noise = Helper::get_random_int(205, 255);
	block_color_mul_ = sf::Color(color_noise, color_noise, color_noise, 255);
}


Block::Block(BlockType type) :
	Block(type, get_block_max_health(type))
{
}


Block::~Block()
{
}


void Block::render(sf::RenderTarget& target, const sf::Vector2f& draw_pos, const sf::Vector2f& draw_size, float draw_rotation)
{
	sf::Color block_color;

	switch (type_) {
	case BlockType::Dirt:
		block_color = sf::Color(153, 102, 51);
		break;

	case BlockType::Brick:
		block_color = sf::Color(128, 128, 128);
		break;

	case BlockType::Grass:
		block_color = sf::Color(51, 204, 51);
		break;

	case BlockType::Glass:
		block_color = is_destroyed() ? sf::Color(51, 153, 255) : sf::Color(255, 255, 0);
		break;

	case BlockType::Stone:
		block_color = sf::Color(168, 168, 168);
		break;

	case BlockType::Bedrock:
		block_color = sf::Color(25, 25, 25);
		break;

	case BlockType::Water:
		block_color = sf::Color(64, 164, 223);
		break;

	case BlockType::FireFX:
		block_color = sf::Color(255, Helper::get_random_int(0, 185), 0);
		break;

	default:
		return;
	}

	if (type_ != BlockType::FireFX && type_ != BlockType::Water) {
		// health of block modifies the color
		const auto color_multi = std::min(0.35f + (0.65f * (health_ / static_cast<float>(get_max_health()))), 1.0f);
		block_color.r = static_cast<sf::Uint8>(block_color.r * color_multi);
		block_color.g = static_cast<sf::Uint8>(block_color.g * color_multi);
		block_color.b = static_cast<sf::Uint8>(block_color.b * color_multi);

		// Apply noise
		block_color *= block_color_mul_;
	}

	sf::RectangleShape block(draw_size);
	block.setFillColor(block_color);
	block.setPosition(draw_pos);
	block.setRotation(draw_rotation);
	target.draw(block);
}
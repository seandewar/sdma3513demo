#pragma once

#include <SFML/Graphics/RenderTarget.hpp>
#include <SFML/Graphics/Font.hpp>
#include <SFML/System/Time.hpp>

#include "World.h"

enum class GameState
{
	PreGame,
	ActiveGame,
	GameOver
};

class Game
{
	const sf::Font& font_;
	const std::vector<sf::Texture>* explosion_anim_textures_;

	World world_;
	EntityId player_id_;
	sf::Time active_game_time_;
	sf::Time next_bomb_time_;
	GameState game_state_;
	bool schedule_new_game_;

	void spawn_player();
	void create_new_game();

public:
	static const uint32_t MAX_MISSED_BOMBS = 10;

	Game(const sf::Font& font, const std::vector<sf::Texture>* explosion_anim_textures);
	~Game();

	inline void new_game() { schedule_new_game_ = true; }

	void tick(const sf::Vector2f& window_mouse_pos);
	void render(sf::RenderTarget& target);
};


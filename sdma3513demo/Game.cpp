#include "Game.h"

#include <chrono>
#include <cassert>
#include <sstream>
#include <iomanip>

#include <SFML/Window/Keyboard.hpp>
#include <SFML/Window/Mouse.hpp>
#include <SFML/Graphics/Text.hpp>

#include "Helper.h"
#include "Constants.h"
#include "PlayerTurretEntity.h"
#include "BombEntity.h"


void Game::spawn_player()
{
	if (player_id_ != Entity::INVALID_ENTITY_ID) {
		printf("Removing existing player entity with id %d.\n", static_cast<int>(player_id_));
		world_.remove_entity(player_id_);
		player_id_ = Entity::INVALID_ENTITY_ID;
	}

	auto player = std::make_unique<PlayerTurretEntity>();
	player->set_position(sf::Vector2f(Constants::VIDEO_WIDTH * 0.5f, 0.0f));

	const auto player_id = world_.add_entity(static_cast<std::unique_ptr<Entity>>(std::move(player)));
	if (player_id == Entity::INVALID_ENTITY_ID)
		throw std::runtime_error("Failed to spawn player entity!");

	player_id_ = player_id;
	printf("Spawned player with id %d\n", static_cast<int>(player_id));
}


void Game::create_new_game()
{
	printf("Starting new game..\n");
	active_game_time_ = sf::Time::Zero;
	world_.generate_new_world(static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count()));

	spawn_player();

	next_bomb_time_ = sf::seconds(2.0f);
	
	game_state_ = GameState::ActiveGame;
	printf("New game active - Let's go!\n");
}


Game::Game(const sf::Font& font, const std::vector<sf::Texture>* explosion_anim_textures) :
	font_(font),
	explosion_anim_textures_(explosion_anim_textures),
	world_(static_cast<uint32_t>(Constants::VIDEO_WIDTH / Block::BLOCK_SIZE.x), static_cast<uint32_t>(Constants::VIDEO_HEIGHT / Block::BLOCK_SIZE.y)),
	player_id_(Entity::INVALID_ENTITY_ID),
	game_state_(GameState::PreGame),
	schedule_new_game_(false)
{
	world_.set_explosion_anim_textures(explosion_anim_textures_);
}


Game::~Game()
{
}


void Game::tick(const sf::Vector2f& window_mouse_pos)
{
	// load new game if scheduled
	if (schedule_new_game_) {
		schedule_new_game_ = false;
		create_new_game();
	} 
	else if ((game_state_ == GameState::PreGame || game_state_ == GameState::GameOver) && sf::Keyboard::isKeyPressed(sf::Keyboard::Space))
		schedule_new_game_ = true;

	// handle active game logic
	if (game_state_ == GameState::ActiveGame) {
		active_game_time_ += Constants::FRAME_TIME;

		// point player turret to mouse pos
		PlayerTurretEntity* player = nullptr;
		if (player_id_ != Entity::INVALID_ENTITY_ID) {
			player = static_cast<PlayerTurretEntity*>(world_.get_entity(player_id_));
		}

		if (player) {
			const auto player_center_pos = player->get_position() + sf::Vector2f(2.0f, 18.5f);
			const auto angle_player_mouse_rads = atan2f(player_center_pos.y - window_mouse_pos.y, player_center_pos.x - window_mouse_pos.x);
			const auto player_new_aim_angle = ((180.0f / 3.141f) * angle_player_mouse_rads) - 90.0f;
			player->set_aim_angle(player_new_aim_angle);

			// fire
			if (sf::Mouse::isButtonPressed(sf::Mouse::Left))
				player->fire_missile();
		}

		// check if game over
		if (player && player->get_player_bombs_missed() >= MAX_MISSED_BOMBS) {
			game_state_ = GameState::GameOver;
			printf("Game over!!!\n");
		}
		else {
			// spawn bomb (on left side or right side)
			next_bomb_time_ -= Constants::FRAME_TIME;
			if (next_bomb_time_.asSeconds() <= 0.0f) {
				const auto world_middle_pos_x = Constants::VIDEO_WIDTH * 0.5f;
				const auto world_middle_clearance = 100.0f;
				float bomb_x_pos;
				if (Helper::get_random_bool(0.5))
					bomb_x_pos = Helper::get_random_float(0.0f, world_middle_pos_x - world_middle_clearance);
				else {
					const auto bomb_x_min = world_middle_pos_x + world_middle_clearance;
					bomb_x_pos = Helper::get_random_float(bomb_x_min, static_cast<float>(Constants::VIDEO_WIDTH));
				}

				auto bomb = std::make_unique<BombEntity>();
				bomb->set_position(sf::Vector2f(bomb_x_pos, -20.0f));
				bomb->assign_player_for_scoring(player_id_);
				world_.add_entity(static_cast<std::unique_ptr<Entity>>(std::move(bomb)));
				next_bomb_time_ += sf::seconds(2.5f - (2.455f * std::min(active_game_time_.asSeconds() / (60.0f * 1.75f), 1.0f)));
				printf("Next bomb will be dropped in %.2f seconds\n", next_bomb_time_.asSeconds());
			}
		}
	}
	else if (game_state_ == GameState::PreGame) {
		// drop random bombs in pregame because it looks cool
		if (Helper::get_random_bool(0.10)) {
			auto bomb = std::make_unique<BombEntity>();
			bomb->set_position(sf::Vector2f(Helper::get_random_float(0.0f, static_cast<float>(Constants::VIDEO_WIDTH)), -20.0f));
			bomb->set_respect_gravity(true);
			world_.add_entity(static_cast<std::unique_ptr<Entity>>(std::move(bomb)));
		}
	}

	world_.tick();
}


void Game::render(sf::RenderTarget& target)
{
	target.clear(sf::Color(0, 0, 0));
	world_.render(target);

	// render ui
	if (schedule_new_game_) {
		sf::Text loading_new_game("Loading a new game...", font_, 30);
		const auto text_bounds = loading_new_game.getLocalBounds();
		loading_new_game.setOrigin(text_bounds.left + (text_bounds.width * 0.5f), text_bounds.top + (text_bounds.height * 0.5f));
		loading_new_game.setPosition(sf::Vector2f(Constants::VIDEO_WIDTH * 0.5f, Constants::VIDEO_HEIGHT * 0.5f));
		loading_new_game.setColor(sf::Color(255, 255, 0));
		target.draw(loading_new_game);
	}
	else if (game_state_ == GameState::PreGame) {
		sf::Text title("City Defender", font_, 40);
		const auto text_bounds_title = title.getLocalBounds();
		title.setOrigin(text_bounds_title.left + (text_bounds_title.width * 0.5f), text_bounds_title.top + (text_bounds_title.height * 0.5f));
		title.setPosition(sf::Vector2f(Constants::VIDEO_WIDTH * 0.5f, (Constants::VIDEO_HEIGHT * 0.5f) - 80.0f));
		title.setColor(sf::Color(255, 255, 0));
		target.draw(title);

		std::ostringstream oss;
		oss << "Your job is to defend a city from falling asteroids!" << std::endl;
		oss << "Be careful, though - if you allow " << MAX_MISSED_BOMBS << " asteroids to hit the city, it's game over!" << std::endl << std::endl;
		oss << "Move your mouse to aim and left click to shoot missiles.";

		sf::Text p1(oss.str(), font_, 20);
		const auto text_bounds_p1 = p1.getLocalBounds();
		p1.setOrigin(text_bounds_p1.left + (text_bounds_p1.width * 0.5f), text_bounds_p1.top + (text_bounds_p1.height * 0.5f));
		p1.setPosition(sf::Vector2f(Constants::VIDEO_WIDTH * 0.5f, Constants::VIDEO_HEIGHT * 0.5f));
		p1.setColor(sf::Color(255, 155, 0));
		target.draw(p1);

		sf::Text begin("Press the SPACE key to begin", font_, 20);
		const auto text_bounds_begin = begin.getLocalBounds();
		begin.setOrigin(text_bounds_begin.left + (text_bounds_begin.width * 0.5f), text_bounds_begin.top + (text_bounds_begin.height * 0.5f));
		begin.setPosition(sf::Vector2f(Constants::VIDEO_WIDTH * 0.5f, (Constants::VIDEO_HEIGHT * 0.5f) + 80.0f));
		begin.setColor(sf::Color(255, static_cast<sf::Uint8>(Helper::get_random_int(10, 100)), 0));
		target.draw(begin);
	}
	else if (game_state_ == GameState::ActiveGame || game_state_ == GameState::GameOver) {
		std::ostringstream oss;

		if (player_id_ != Entity::INVALID_ENTITY_ID) {
			auto player = static_cast<PlayerTurretEntity*>(world_.get_entity(player_id_));
			assert(player);

			oss << "Score: " << player->get_player_score();

			sf::Text player_score(oss.str(), font_);
			player_score.setPosition(sf::Vector2f(0.0f, 0.0f));
			player_score.setColor(sf::Color(255, 255, 0));
			target.draw(player_score);

			oss.str("");
			oss << "Missed: " << std::min(player->get_player_bombs_missed(), MAX_MISSED_BOMBS) << " / " << MAX_MISSED_BOMBS;

			sf::Text player_missed_bombs(oss.str(), font_, 26);
			player_missed_bombs.setPosition(sf::Vector2f(0.0f, 35.0f));
			player_missed_bombs.setColor(sf::Color(255, 100, 0));
			target.draw(player_missed_bombs);

			if (game_state_ == GameState::ActiveGame) {
				oss.str("");
				oss << "Aim angle: " << std::fixed << std::setw(4) << std::setprecision(2) << std::setfill('0') << (player->get_aim_angle() + 90.0f) << "deg";

				sf::Text player_aim(oss.str(), font_, 20);
				const auto text_bounds_aim = player_aim.getLocalBounds();
				player_aim.setOrigin(text_bounds_aim.left + (text_bounds_aim.width * 0.5f), text_bounds_aim.top + (text_bounds_aim.height * 0.5f));
				player_aim.setPosition(sf::Vector2f(Constants::VIDEO_WIDTH * 0.5f, 20.0f));
				player_aim.setColor(sf::Color(255, 100, 0));
				target.draw(player_aim);
			}
		}	

		oss.str("");
		oss << "Defended for: " << std::fixed << std::setw(4) << std::setprecision(2) << std::setfill('0') << active_game_time_.asSeconds() << "s";

		sf::Text game_elapsed_time(oss.str(), font_, 22);
		game_elapsed_time.setPosition(sf::Vector2f(0.0f, 70.0f));
		game_elapsed_time.setColor(sf::Color(155, 155, 0));
		target.draw(game_elapsed_time);

		if (game_state_ == GameState::GameOver) {
			sf::Text game_over("Game over!", font_, 40);
			const auto text_bounds = game_over.getLocalBounds();
			game_over.setOrigin(text_bounds.left + (text_bounds.width * 0.5f), text_bounds.top + (text_bounds.height * 0.5f));
			game_over.setPosition(sf::Vector2f(Constants::VIDEO_WIDTH * 0.5f, (Constants::VIDEO_HEIGHT * 0.5f) - 80.0f));
			game_over.setColor(sf::Color(255, 50, 0));
			target.draw(game_over);

			sf::Text again("Press the SPACE key to try again", font_, 20);
			const auto text_bounds_again = again.getLocalBounds();
			again.setOrigin(text_bounds_again.left + (text_bounds_again.width * 0.5f), text_bounds_again.top + (text_bounds_again.height * 0.5f));
			again.setPosition(sf::Vector2f(Constants::VIDEO_WIDTH * 0.5f, (Constants::VIDEO_HEIGHT * 0.5f) - 30.0f));
			again.setColor(sf::Color(255, static_cast<sf::Uint8>(Helper::get_random_int(10, 100)), 0));
			target.draw(again);
		}
	}
}

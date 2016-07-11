#pragma once

#include "PhysicsEntity.h"

#include <SFML/System/Time.hpp>

class PlayerTurretEntity : public PhysicsEntity
{
	float aim_angle_;
	int32_t player_score_;
	uint32_t player_missed_bombs_;
	sf::Time missile_shoot_delay_;
	sf::Time next_missile_available_time_;

public:
	PlayerTurretEntity();
	virtual ~PlayerTurretEntity();

	virtual void fire_missile();

	virtual void tick() override;
	virtual void render(sf::RenderTarget& target) override;

	inline virtual void set_missile_shoot_delay(const sf::Time& delay) { missile_shoot_delay_ = delay; }
	inline virtual sf::Time get_missile_shoot_delay() const { return missile_shoot_delay_; }

	inline virtual void set_aim_angle(float angle) { aim_angle_ = std::min(std::max(-90.0f, angle), 90.0f); }
	inline virtual void add_to_aim_angle(float val) { set_aim_angle(aim_angle_ + val); }
	inline virtual float get_aim_angle() const { return aim_angle_; }

	inline virtual void set_player_score(int32_t score) { player_score_ = score; }
	inline virtual void add_to_player_score(int32_t val) { player_score_ += val; }
	inline virtual int32_t get_player_score() const { return player_score_; }

	inline virtual void set_player_bombs_missed(uint32_t missed_bombs) { player_missed_bombs_ = missed_bombs; }
	inline virtual void increment_player_bombs_missed() { ++player_missed_bombs_; }
	inline virtual uint32_t get_player_bombs_missed() const { return player_missed_bombs_; }

	inline virtual std::string get_name() const override { return "PlayerTurretEntity"; }
};


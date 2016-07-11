#pragma once

#include "PhysicsEntity.h"

#include <SFML/System/Time.hpp>

class BombEntity : public PhysicsEntity
{
	uint32_t explosion_r_, explosion_damage_;
	sf::Time smoke_time_;
	EntityId player_id_for_scoring_;

public:
	BombEntity();
	virtual ~BombEntity();

	inline virtual void assign_player_for_scoring(EntityId player_id) { player_id_for_scoring_ = player_id; }

	virtual void tick() override;
	virtual void render(sf::RenderTarget& target) override;

	inline virtual void set_explosion_radius(uint32_t r) { explosion_r_ = r; }
	inline virtual uint32_t get_explosion_radius() const { return explosion_r_; }

	inline virtual void set_explosion_damage(uint32_t damage) { explosion_damage_ = damage; }
	inline virtual uint32_t get_explosion_damage() const { return explosion_damage_; }

	inline virtual std::string get_name() const override { return "BombEntity"; }
};


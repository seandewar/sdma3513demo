#pragma once

#include "PhysicsEntity.h"

#include <SFML/System/Time.hpp>

class PlayerMissileEntity : public PhysicsEntity
{
	sf::Time smoke_time_;
	EntityId player_id_for_scoring_;

public:
	PlayerMissileEntity();
	virtual ~PlayerMissileEntity();

	inline virtual void assign_player_for_scoring(EntityId player_id) { player_id_for_scoring_ = player_id; }

	virtual void tick() override;
	virtual void render(sf::RenderTarget& target) override;

	inline virtual std::string get_name() const override { return "PlayerMissileEntity"; }
};


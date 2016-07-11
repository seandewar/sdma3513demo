#pragma once

#include "PhysicsEntity.h"

class ExplosionEffectEntity : public PhysicsEntity
{
	float explosion_density_;

public:
	ExplosionEffectEntity();
	virtual ~ExplosionEffectEntity();

	virtual void tick() override;
	virtual void render(sf::RenderTarget& target) override;

	inline virtual std::string get_name() const override { return "ExplosionEffectEntity"; }
};


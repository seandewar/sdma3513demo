#pragma once

#include "PhysicsEntity.h"

class SmokeParticleEntity : public PhysicsEntity
{
	float smoke_density_;
	float smoke_angle_;

public:
	SmokeParticleEntity();
	virtual ~SmokeParticleEntity();

	virtual void tick() override;
	virtual void render(sf::RenderTarget& target) override;

	inline virtual void set_smoke_density(float density) { smoke_density_ = density; }
	inline virtual float get_smoke_density() const { return smoke_density_; }

	inline virtual void set_smoke_angle(float angle) { smoke_angle_ = angle; }
	inline virtual float get_smoke_angle() const { return smoke_angle_; }

	inline virtual std::string get_name() const override { return "SmokeParticleEntity"; }
};


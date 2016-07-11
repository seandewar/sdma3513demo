#pragma once

#include "Entity.h"

class PhysicsEntity : public Entity, public IRectangle, public IPoint
{
	sf::Vector2f velocity_;
	sf::FloatRect rect_;
	bool respect_gravity_;
	
public:
	PhysicsEntity(bool fx_only = false);
	virtual ~PhysicsEntity();
	
	virtual void tick() override;

	inline virtual sf::Vector2f get_velocity() const { return velocity_; }
	inline virtual void set_velocity(const sf::Vector2f velo) { velocity_ = velo; }

	inline virtual void set_rectangle(const sf::FloatRect& rect) override { rect_ = rect; }
	inline virtual sf::FloatRect get_rectangle() const override { return rect_; }

	inline virtual void set_position(const sf::Vector2f& pos) override { rect_ = sf::FloatRect(pos, sf::Vector2f(rect_.width, rect_.height)); }
	inline virtual sf::Vector2f get_position() const override { return sf::Vector2f(rect_.left, rect_.top); }

	inline virtual void set_respect_gravity(bool respect_gravity) { respect_gravity_ = respect_gravity; }
	inline virtual bool is_respecting_gravity() const { return respect_gravity_; }
};


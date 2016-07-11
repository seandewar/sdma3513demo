#pragma once

#include <cstdint>
#include <string>

#include <SFML/Graphics/RenderTarget.hpp>

typedef uint64_t EntityId;

class IRectangle
{
public:
	virtual void set_rectangle(const sf::FloatRect& rect) = 0;
	virtual sf::FloatRect get_rectangle() const = 0;
};

class IPoint
{
public:
	virtual void set_position(const sf::Vector2f& pos) = 0;
	virtual sf::Vector2f get_position() const = 0;
};

class World;

class Entity
{
	EntityId id_;
	World* world_;
	bool is_fx_only_;
	bool marked_for_deletion_;

public:
	// invalid ent id - should wrap around to max val of EntityId if unsigned
	static const EntityId INVALID_ENTITY_ID = -1;

	Entity(bool fx_only);
	virtual ~Entity();

	inline void assign_world(World* world, EntityId id) { world_ = world; id_ = id; }

	inline virtual void tick() { }
	inline virtual void render(sf::RenderTarget& target) { }

	inline void mark_for_deletion() { marked_for_deletion_ = true; }
	inline bool is_marked_for_deletion() const { return marked_for_deletion_; }

	inline EntityId get_id() const { return id_; }
	virtual std::string get_name() const = 0;

	inline bool is_fx_only() const { return is_fx_only_; }

	inline World* get_world() { return world_; }
};


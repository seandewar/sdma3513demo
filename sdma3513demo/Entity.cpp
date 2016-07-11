#include "Entity.h"



Entity::Entity(bool fx_only) :
	id_(INVALID_ENTITY_ID),
	world_(nullptr),
	marked_for_deletion_(false),
	is_fx_only_(fx_only)
{
}


Entity::~Entity()
{
}
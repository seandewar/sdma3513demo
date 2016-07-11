#pragma once

#include <memory>

#include "PhysicsEntity.h"
#include "Block.h"

class BlockGibEntity : public PhysicsEntity
{
	std::unique_ptr<Block> block_;

public:
	BlockGibEntity();
	virtual ~BlockGibEntity();

	inline virtual void assign_block(std::unique_ptr<Block>& block) { block_ = std::move(block); }
	inline virtual Block* get_block() { return block_.get(); }
	inline virtual std::unique_ptr<Block> disown_block() { return std::move(block_); }

	virtual void tick() override;
	virtual void render(sf::RenderTarget& target) override;

	inline virtual std::string get_name() const override { return "BlockGibEntity"; }
};

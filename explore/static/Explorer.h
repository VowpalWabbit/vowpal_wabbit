#pragma once

#include "Interaction.h"

// TODO: for exploration budget, exploration algo should implement smth like Start & Stop Explore, Adjust epsilon
class Explorer
{
public:
	virtual std::tuple<MWTAction, float, bool> Choose_Action(void* context, ActionSet& actions, u32 seed) = 0;
	virtual ~Explorer() { }
};
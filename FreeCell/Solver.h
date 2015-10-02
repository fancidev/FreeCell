///////////////////////////////////////////////////////////////////////////
// Solver.h

#pragma once

#include "Card.h"
#include "State.h"
#include <vector>

namespace FreeCell
{
	struct Solution
	{
		State start;
		bool isSolved;
		std::vector<CardMove> moves;

		size_t numStatesExpanded;
		size_t numStatesQueued;
		size_t numStatesProcessed;
	};

	void Solve(const State &start, Solution &solution);
}
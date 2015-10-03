///////////////////////////////////////////////////////////////////////////
// Solver.h

#pragma once

#include "Card.h"
#include "State.h"
#include <vector>

namespace FreeCell
{
	enum SolverResult
	{
		NotSolvable = 0,
		Solved = 1,
		Failed = 2,
	};

	struct Solution
	{
		State start;
		SolverResult result;
		std::vector<CardMove> moves;

		size_t numStatesExpanded;
		size_t numStatesQueued;
		size_t numStatesProcessed;
	};

	struct Strategy
	{
		size_t maximumNumberOfStatesToProcess;

		Strategy() :
			maximumNumberOfStatesToProcess(-1)
		{
		}
	};

	Solution Solve(const State &start, const Strategy &strategy);
}
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
		Solved = 0,
		NotSolvable = 1,
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
		int heuristic;

		Strategy() :
			maximumNumberOfStatesToProcess(-1), heuristic(0)
		{
		}
	};

	Solution Solve(const State &start, const Strategy &strategy);
}
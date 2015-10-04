///////////////////////////////////////////////////////////////////////////
// Heuristic.h

#pragma once

#include "State.h"

namespace FreeCell
{
	// ComputeHeuristic
	//
	// Computes a lower bound of the number of moves required to
	// collect all cards in the given state.
	//
	// A "move" is defined as moving one or more cards from one
	// place to another, or collecting a single card to its home
	// cell. Automatic collection also counts. Therefore at the
	// start of the game the lower bound is at least 52.
	//
	// For a given state, a higher return value is better. If the
	// return value is guaranteed to be less than or equal to the
	// true lower bound for any state, then the solver will produce
	// an optimal solution using the A* algorithm. Otherwise, the
	// solver may not produce an optimal solution, but may solve
	// the problem faster.

	size_t ComputeHeuristic(const State &state, int heuristic);
}
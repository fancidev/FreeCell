///////////////////////////////////////////////////////////////////////////
// IO.h -- input/output routines

#pragma once

#include "State.h"
#include "Solver.h"
#include <iostream>

namespace FreeCell
{
	std::istream& operator >> (std::istream &stream, State &state);

	std::ostream& operator << (std::ostream &os, const CARD &);
	std::ostream& operator << (std::ostream &os, const State &);
	std::ostream& operator << (std::ostream &os, const CardMove &);
	std::ostream& operator << (std::ostream &os, const Solution &);
	std::ostream& operator << (std::ostream &os, const SolverResult &);
}
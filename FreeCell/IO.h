///////////////////////////////////////////////////////////////////////////
// IO.h -- input/output routines

#pragma once

#include "State.h"
#include <iostream>

namespace FreeCell
{
	std::istream& operator >> (std::istream &stream, State &state);
	std::ostream& operator << (std::ostream &stream, const State &state);
}
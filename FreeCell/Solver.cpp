///////////////////////////////////////////////////////////////////////////
// Solver.cpp

#include "Card.h"
#include "State.h"
#include "Solver.h"
#include "Heuristic.h"
#include <queue>
#include <unordered_set>

namespace FreeCell
{
	typedef unsigned int score_t;

	struct SearchNode
	{
		State state;

		// The state that leads to this state.
		const SearchNode *parent;

		// The move that leads parent to this.
		const CardMove move;

		size_t numSteps; // number of steps from start to here

		score_t score; // smaller is better

		SearchNode(const State &start)
			: state(start), parent(nullptr), move(), numSteps(0), score(0)
		{
			numSteps = state.CollectSafely();
		}

		SearchNode(const SearchNode *parent, CardMove move)
			: state(parent->state), parent(parent), move(move),
			numSteps(parent->numSteps + 1)
		{
			state.MoveCard(move.card,
				move.FromArea(), move.FromIndex(),
				move.ToArea(), move.ToIndex());
			numSteps += state.CollectSafely();
		}
	};

	static bool CompareNodesByPriority(const SearchNode *p1, const SearchNode *p2)
	{
		return p1->score > p2->score;
	}

	static size_t ComputeStateHash(const SearchNode &node)
	{
		return node.state.GetHashCode();
	}

	static bool SearchNodeEquals(const SearchNode &a, const SearchNode &b)
	{
		return a.state.IsEquivalentTo(b.state);
	}

	// Try solve the position _start_.
	Solution Solve(const State &start, const Strategy &strategy)
	{
		Solution solution;
		solution.start = start;
		solution.result = NotSolvable;
		solution.moves.clear();
		solution.numStatesExpanded = 1;
		solution.numStatesQueued = 1;
		solution.numStatesProcessed = 0;

		// Store expanded states in a deque so that we can reference
		// its elements directly because they are never moved (while
		// std::vector does).
		// std::deque<SearchNode> expandedStates;

		// Store expanded states in a hash set to avoid duplicates.
		std::unordered_set<
			SearchNode,
			decltype(&ComputeStateHash),
			decltype(&SearchNodeEquals)>
			expandedStates(0, ComputeStateHash, SearchNodeEquals);

		// Maintain a priority queue of the expanded states so that
		// we can retrieve the most promising state at any time.
		std::priority_queue<
			const SearchNode *,
			std::vector<const SearchNode *>,
			decltype(&CompareNodesByPriority)>
			queue(CompareNodesByPriority);

		// Add starting state to the queue.
		queue.push(&(*expandedStates.emplace(start).first));

		// Expand each state in order of priority.
		while (!queue.empty())
		{
			const SearchNode *node = queue.top();
			queue.pop();
			++solution.numStatesProcessed;

			if (node->state.IsWinning())
			{
				solution.result = Solved;
				while (node->parent != nullptr)
				{
					solution.moves.push_back(node->move);
					node = node->parent;
				}
				std::reverse(solution.moves.begin(), solution.moves.end());
				break;
			}

			CardMove moves[80];
			int numMoves = node->state.GetPossibleMoves(moves);
			assert(numMoves <= 80);
			for (int i = 0; i < numMoves; i++)
			{
				CardMove move = moves[i];
				SearchNode newNode(node, move);
				
				newNode.score = newNode.numSteps + 
					ComputeHeuristic(newNode.state, strategy.heuristic);
				
				auto result = expandedStates.insert(newNode);
				if (result.second) // not duplicate
				{
					queue.push(&(*result.first));
					++solution.numStatesQueued;
				}
				++solution.numStatesExpanded;
			}

			if (solution.numStatesProcessed >= strategy.maximumNumberOfStatesToProcess)
			{
				solution.result = Failed;
				break;
			}
		}
		return solution;
	}
}
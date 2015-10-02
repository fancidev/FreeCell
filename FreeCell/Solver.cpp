///////////////////////////////////////////////////////////////////////////
// Solver.cpp

#include "Card.h"
#include "State.h"
#include "Solver.h"
#include <queue>
#include <unordered_set>

namespace FreeCell
{
	typedef unsigned int score_t;

	static score_t ComputeStateScore(const State &);

	struct SearchNode
	{
		State state;

		// The state that leads to this state.
		const SearchNode *parent;

		// The move that leads parent to this.
		const CardMove move;

		score_t score; // smaller is better

		SearchNode(const State &start)
			: state(start), parent(nullptr), move()
		{
			state.CollectSafely();
			score = ComputeStateScore(state);
		}

		SearchNode(const SearchNode *parent, CardMove move)
			: state(parent->state), parent(parent), move(move)
		{
			state.MoveCard(move.card, 
				move.FromArea(), move.FromIndex(), 
				move.ToArea(), move.ToIndex());
			state.CollectSafely();
			score = ComputeStateScore(state);
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
	void Solve(const State &start, Solution &solution)
	{
		solution.start = start;
		solution.isSolved = false;
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
				solution.isSolved = true;
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
				//auto result = expandedStates.emplace(node, move);
				auto result = expandedStates.insert(newNode);
				if (result.second) // not duplicate
				{
					queue.push(&(*result.first));
					++solution.numStatesQueued;
				}
				++solution.numStatesExpanded;
			}
		}
	}

	// Evaluates the given state and returns a non-negative score.
	// Lower score is better. A state where all cards are collected
	// must have score = 0.
	static score_t ComputeStateScore(const State &state)
	{
		score_t score = 0;

		//int cell_punish[5] = { 0, 1, 2, 4, 7 };

		//int nEmptyCell, nEmptyColumn;
		//int i;

		///* how many empty cells */
		//for (i = 0; (i < 4) && IS_FREECELL_EMPTY(st, i); i++);
		//nEmptyCell = i;
		//score += 10 * cell_punish[4 - nEmptyCell];

		///* how many empty columns */
		//for (i = 0; (i < 8) && IS_COLUMN_EMPTY(st, i); i++);
		//nEmptyColumn = i;
		//score += 5 * (8 - nEmptyColumn);

		// Go through each column.
		for (int columnIndex = 0; columnIndex < 8; columnIndex++)
		{
			//int depth = 1, cost = 1, pts = 1;
			CARD card = state.TopCardOfColumn(columnIndex);
			while (IsCard(card))
			{
				CARD c = card;
				while (IsCard(c = state.CardUnder(c)))
				{
					if (RankOf(c) <= RankOf(card))
					{
						++score;
					}
				}
				card = state.CardUnder(card);
			}
		}

		///* punish empty homecell */
		//for (i = 0; i < 4; i++) {
		//	if (IS_HOMECELL_EMPTY(st, INDEX2SUIT(i)))
		//		score += 20;
		//}

		return score;
	}
}
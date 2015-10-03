///////////////////////////////////////////////////////////////////////////
// Main.cpp -- entry point of the program

#include "Card.h"
#include "State.h"
#include "Solver.h"
#include "GenerateGame.h"
#include "IO.h"

#include <iostream>
#include <fstream>
#include <iomanip>

using namespace FreeCell;

static void TestIO(int gameNumber)
{
	State start = GenerateGame(gameNumber);
	std::cout << start;
}

static void TestGame(int gameNumber, int heuristic, size_t maxProcess = -1)
{
	State start = GenerateGame(gameNumber);
	std::cout << start;
	
	Strategy strategy;
	strategy.heuristic = heuristic;
	strategy.maximumNumberOfStatesToProcess = maxProcess;

	Solution solution = Solve(start, strategy);
	std::cout << solution;
}

static void TestEasy()
{
	std::fstream fs;
	fs.open("../Test/Easy.txt", std::ios::in);

	State state;
	fs >> state;
	std::cout << state;

	Solution solution = Solve(state, Strategy());
	std::cout << solution;
}

void TestMany(int numGames, int heuristic1, int heuristic2)
{
	Strategy strategy1;
	strategy1.maximumNumberOfStatesToProcess = 10000;
	strategy1.heuristic = heuristic1;

	Strategy strategy2;
	strategy2.maximumNumberOfStatesToProcess = 10000;
	strategy2.heuristic = heuristic2;

	struct Summary
	{
		size_t count[3];
		size_t numMoves[3];
		size_t numProcessed[3];
		size_t numExpanded[3];

#pragma warning(disable: 4351) // default initialize array
		Summary() :
			count(), numMoves(), numProcessed(), numExpanded()
		{
		}

		void Update(const Solution &solution)
		{
			assert(solution.result >= 0 && solution.result < 3);

			count[solution.result] += 1;
			numMoves[solution.result] += solution.moves.size();
			numProcessed[solution.result] += solution.numStatesProcessed;
			numExpanded[solution.result] += solution.numStatesExpanded;

			std::cout << solution.result;
			std::cout << " ";
			std::cout << std::setw(5) << solution.moves.size();
			std::cout << std::setw(10) << solution.numStatesProcessed;
			std::cout << " " << std::fixed << std::setprecision(1)
				<< (double)solution.numStatesExpanded / solution.numStatesProcessed;
		}

		size_t AverageMoves(int result) const
		{
			return count[result] == 0 ? 0 : numMoves[result] / count[result];
		}

		size_t AverageStates(int result) const
		{
			return count[result] == 0 ? 0 : numProcessed[result] / count[result];
		}

		double ExpansionRatio(int result) const
		{
			return numProcessed[result] == 0 ? 0 :
				(double)numExpanded[result] / numProcessed[result];
		}
	};
	Summary summary1, summary2;

	std::cout << "     # ?? Steps    States E/S       # ?? Steps    States E/S" << std::endl;
	std::cout << "-----------------------------  -----------------------------" << std::endl;

	for (int gameNumber = 1; gameNumber <= numGames; gameNumber++)
	{
		State start = GenerateGame(gameNumber);
		Solution solution1 = Solve(start, strategy1);
		Solution solution2 = Solve(start, strategy2);

		std::cout << std::setw(6) << gameNumber << " ";
		summary1.Update(solution1);

		std::cout << "  ";
		std::cout << std::setw(6) << gameNumber << " ";
		summary2.Update(solution2);

		std::cout << std::endl;
	}

	std::cout << "-----------------------------  -----------------------------" << std::endl;
	for (int result = 0; result < 3; result++)
	{
		std::cout
			<< std::setw(6) << summary1.count[result]
			<< " " << static_cast<SolverResult>(result)
			<< " " << std::setw(5) << summary1.AverageMoves(result)
			<< " " << std::setw(9) << summary1.AverageStates(result)
			<< " " << std::fixed << std::setprecision(1) << summary1.ExpansionRatio(result);
		std::cout << "  ";
		std::cout
			<< std::setw(6) << summary2.count[result]
			<< " " << static_cast<SolverResult>(result)
			<< " " << std::setw(5) << summary2.AverageMoves(result)
			<< " " << std::setw(9) << summary2.AverageStates(result)
			<< " " << std::fixed << std::setprecision(1) << summary2.ExpansionRatio(result);
		std::cout << std::endl;
	}
}

int main(int argc, char *argv[])
{
	// TestIO(5121496); // OK

	// TestEasy();
	// TestGame(739671); // cannot get
	// TestGame(2);

	//TestGame(5121496);
	//TestGame(6469, 1);
	//TestGame(6469, 4);
	//TestGame(28571, 4);

	TestMany(100, 3, 4);
}

#if 0
int main(int argc, char *argv[])
{
	STATE *st = (STATE *)malloc(sizeof(STATE));
	int i, n;
	int gamenumber;
	CARD_MOVE moves[80];
	SOLVER_INFO si, si_tot;

	int difficult_ones[] = {
		169, 178, 258, 454, 617, 718, 1689, 1941, 2021, 2350, 2577,
		2607, 2670, 2772, 3285, 3342, 3349, 3685, 3772, 3788, 3801,
		3973, 4257, 4368, 4540, 4591, 4714, 4946, 5179, 5374, 5453,
		5482, 5490, 5548, 5557, 6343, 6673, 6745, 6751, 6768, 7107,
		7160, 7600, 7700, 8005, 8323, 8534, 8591, 8652, 8678, 8749,
		8820, 9250, 9385, 9538, 9617, 9700, 10589, 11281, 11386, 11409,
		11430, 11677, 11854, 12211, 12313, 13015, 14051, 14188, 14676,
		14795, 14879, 14965, 14977, 15023, 15099, 15130, 15133, 15164,
		15227, 15238, 15710, 15746, 15905, 15939, 16191, 16575, 16576,
		17277, 17524, 17764, 17768, 18623, 18992, 19410, 19484, 19633,
		19763, 19861, 20055, 20251, 20589, 20715, 20912, 21051, 21185,
		21278, 21785, 21896, 21899, 22332, 24063, 24457, 24549, 24735,
		25123, 25155, 25450, 25599, 25602, 25790, 25856, 25995, 26093,
		26183, 26197, 26369, 26421, 26576, 26693, 26694, 26710, 27188,
		29001, 29154, 29198, 29345, 29704, 30000, 30108, 30394, 30615,
		30712, 30801, 31044, 31266, 31465, 31601, 31647, 31729, 31918,
		31945, 0
	};

	int games[] = {
		165, 495, 857, 740, 705, 932, 792, 253, 390, 433, 0,
		739671, /* easy     (expanded=   4,815/  2,922) */
		15893,  /* easy     (expanded=   2,216/ 14,616) */
		617,    /* easy     (expanded=   7,656/ 25,281) */
		10692,  /* hard     (expanded=  35,298/ 52,142) */
		35254,  /* extreme  (expanded= 134,104/ 20,963) */
		57148,  /* extreme  (expanded= 371,421/307,025) */
		11982,  /* unsolvable (expaned=61,643/0) */
		1941,   /* hard     (expanded=  33,307/ 31,266) */
		0 };


	/*
	if (read_state(st, "D:\\15893.txt") < 0) {
	return 1;
	}
	printf("Initial State:\n");
	print_state(st);
	*/

	//GenerateGame(st, gamenumber);
	//printf("Game #%d:\n", gamenumber);
	//PrintState(st);

	/*
	printf("Normalized State:\n");
	NormalizeState(st);
	PrintState(st);

	n = CollectSafely(st);
	printf("\nNumber of cards auto collected = %d\n", n);
	PrintState(st);
	*/

	/*
	strategy 1: (Aggressive)
	General: (fail=187,223,383)

	1-100:      83       883      6016      6900
	1-200:      77      1940      9625     11565
	1-500:      73      2474     13848     16322
	74      1720      8634     10355
	Hard ones:
	Avg.        54    176151    150873    327024

	strategy 2: (Benchmark)
	General: (fail=165)
	Avg.   499>       66      1525      7097      8622  ( 165 failed )
	Hard ones:
	Avg.        85     81307     56776    138084

	strategy 3: (Conservative)
	General (1-500):
	Avg.   499>       70      1359      6420      7780  ( 495 failed )
	Avg.   500>       71      1857      9535     11393  ( all solved; punish bottom card )
	* Avg.   500>       63      1238      5410      6648  ( all solved; chaos weight  .5 )
	Avg.   500>       63      1521      6461      7983  ( all solved; chaos weight 1.0 )
	Avg.   500>       63      1027      4921      5948  ( all solved; chaos weight  .67 )

	Avg.   499>       55      1476      5763      7239  ( 495 failed; chaos .5 )
	Avg.   499>       55      1400      5160      6560  ( 440 failed; chaos .67 )
	Avg.   500>       54      1744      7906      9650  ( all solved; chaos 1.0 )
	Avg.   499>       57      1354      3369      4723  ( 495 failed; one-sided chaos )

	1-1000:
	Avg.   997>       69      1603      8016      9619  ( failed 857, 740, 705 )
	Avg.   999>       63      1876      5231      7107  ( failed 932; chao .5 )
	Avg.   998>       63      1781      4684      6465  ( failed 792, 932; chao .67 )

	Hard ones:
	Avg.        80     83504     16759    100264

	Strategy 4: (1-500)
	Avg.   498>       57      2784      5564      8349  ( failed 282, 495 )


	[limit expand<=100,000]
	S1: Avg.   483>       55       814      4058      4872
	S2: Avg.   498>       57       913      4570      5483
	S3: Avg.   498>       57       935      3103      4039
	S4: Avg.   496>       55       752      2801      3553
	Avg.   494>       61      1940      1694      3635
	Avg.   497>       64      3005       885      3890
	Avg.   495>       63      2825      1247      4073

	So:
	- try fix chao method.
	- try compare pairs.

	*/
	strategy = 4;

	printf("      Game     Steps  Expanded     Fresh     Total\n");
	printf("----------------------------------------------------\n");

	si_tot.nodes_expanded = 0;
	si_tot.nodes_fresh = 0;
	si_tot.nsteps = 0;
	n = 0;

#if 0
	for (i = 0; gamenumber = games[i]; i++)
#elif 1
	for (gamenumber = 1; gamenumber <= 500; gamenumber++)
#else
	gamenumber = 932;
#endif
	{
		GenerateGame(st, gamenumber);
		solve(st, &si);

		printf("%10d  %8d  %8d  %8d  %8d\n", gamenumber,
			si.nsteps, si.nodes_expanded, si.nodes_fresh,
			si.nodes_expanded + si.nodes_fresh);

		if (si.nsteps > 0) {
			si_tot.nodes_expanded += si.nodes_expanded;
			si_tot.nodes_fresh += si.nodes_fresh;
			si_tot.nsteps += si.nsteps;
			n++;
		}
	}
	if (n == 0)
		n = 1;
	printf("----------------------------------------------------\n");
	printf("Avg.%6d> %8d  %8d  %8d  %8d\n", n,
		si_tot.nsteps / n, si_tot.nodes_expanded / n, si_tot.nodes_fresh / n,
		(si_tot.nodes_expanded + si_tot.nodes_fresh) / n);

	return 0;

	n = FindPossibleMoves(st, moves, 80);
	printf("\nPossible Moves = %d\n", n);
	for (i = 0; i < n; i++) {
		STATE st2;
		printf("**** Move %2d: %s\n", i + 1, move2str(moves[i]));
		DeriveState(&st2, st, moves[i]);
		PrintState(&st2);
	}

	//printf("\n\n\nBranching this state:\n-----------------------------\n");
	//n = branch(st, list, 100);
	//printf("\nNumber of children = %d\n", n);

	return 0;
}
#endif
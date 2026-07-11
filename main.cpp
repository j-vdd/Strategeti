#include "Protocol.h"

#include <chrono>

struct PerftEntry {
	Hash hash;
	uint64_t count;
};

constexpr int perftTableSize = 1 << 17;
PerftEntry perftTable[7][perftTableSize] = {};

uint64_t perft(Board& board, int depth, bool log) {
	if (depth == 0)
		return 1;

	// const Hash hash = hashBoard(board.state());
	// auto& tableEntry = perftTable[depth][hash & (perftTableSize - 1)];
	// if (tableEntry.count != 0 && tableEntry.hash == hash)
	// 	return tableEntry.count;

	Move moves[200];
	Move* moveListEnd = genMoves(board.state(), moves);
	int numMoves = moveListEnd - moves;
	if (depth == 1 && !log)
		return numMoves;

	uint64_t res = 0;
	for (int i = 0; i < numMoves; i++) {
		board.makeMove(moves[i]);
		uint64_t child = perft(board, depth - 1, false);
		res += child;
		board.undo(); 
		if (log)
			cerr << moveToString(moves[i]) << ": " << child << endl;
	}

	// tableEntry = { hash, res };

	return res;
}

void testPerft() {
	initTables();
	initHashes();
	initEvalTables();

	Board board;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	uint64_t total = perft(board, 6, true);
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	double ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	cerr << "Total: " << total << endl;
	cerr << double(total) / (ms / 1000.0) / 1000000.0 << "Mnps" << endl;
}

void testEval() {
	initTables();
	initHashes();
	initEvalTables();

	vector<pair<string, int>> moves = {
		{"Eb3", 8},
		{"Lc3", 9},
		{"Ec2", 9},
		{"Eb2", 10},
		{"Za3", 9},
		{"Gd2", 11},
		{"Gd3", 10},
		{"c3d3", 12},
		{"Zc1", 11},
		{"Lb4", 12}, // found mate
		{"Lc3", 11},
		{"Ec4", 10},
		{"c1d1", 9},
		{"c4b4", 8},
		{"Gc4", 7},
		{"Zc1", 6},
		{"c2d2", 5},
		{"a4a3", 4},
		{"Lc2", 3},
		{"b4c4", 2}
	};
	Board board;
	uint64_t totalNodeCount = 0;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
	for (auto [move, depth] : moves) {
		findBestMove(board, 0, depth);
		totalNodeCount += nodeCount;

		make(board, move);
	}
	std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();

	cout << "Total: " << double(totalNodeCount) / 1000000.0 << "Mn" << endl;
	double ms = std::chrono::duration_cast<std::chrono::milliseconds>(end - begin).count();
	cerr << double(totalNodeCount) / (ms / 1000.0) / 1000000.0 << "Mnps" << endl;
}

int main() {
	// testEval();
	// testPerft();
	startProtocol();

	return 0;
}
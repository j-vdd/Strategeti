#pragma once

#include "MoveGen.h"
#include "Zobrist.h"
#include "Evaluate.h"

#include <algorithm>
#include <chrono>

enum HashBound { NONE, EXACT, LOWER, UPPER };
struct HashEntry {
	HashEntry() = default;
	HashEntry(Hash hash, int score, int depth, HashBound bound, Move move)
		: hash(hash), score(score), depth(depth), bound(bound), move(move)
	{}
	Hash hash = 0;
	int score = 0;
	int depth = -1;
	HashBound bound = NONE;
	Move move = makeMove(0, 0, NoPiece);
};

constexpr int HashSize = 1 << 22;
inline HashEntry hashTable[HashSize] = {};

inline int readTable(Hash hash, int depth, int& alpha, int& beta, bool& succ, Move& hashMove) {
	HashEntry& entry = hashTable[hash & (HashSize - 1)];
	succ = false;
	if (entry.hash != hash)
		return 0;

	const bool isMate = (entry.score > MateScore - 100 && (entry.bound == EXACT || entry.bound == LOWER)) ||
		(entry.score < -MateScore + 100 && (entry.bound == EXACT || entry.bound == UPPER));
	if (isMate)
		entry.depth = depth;

	hashMove = entry.move;
	if (entry.depth < depth)
		return 0;

	succ = true;
	if (entry.bound == EXACT)
		return entry.score;
	if (entry.bound == LOWER) {
		alpha = max(alpha, entry.score);
		if (alpha >= beta)
			return alpha;
	}
	if (entry.bound == UPPER) {
		beta = min(beta, entry.score);
		if (beta <= alpha)
			return beta;
	}

	succ = false;
	return 0;
}
inline void writeTable(Hash hash, int depth, int score, HashBound bound, Move move) {
	if (hashTable[hash & (HashSize - 1)].depth > depth)
		return;

	hashTable[hash & (HashSize - 1)] = HashEntry(
		hash, score, depth, bound, move
	);
}

inline int nodeCount = 0;
inline std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> endTime;
inline std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> curTime;

int negamax(Board& board, const int depth, const int maxDepth, int alpha, int beta) {
	nodeCount++;
	if (hasLost(board.state()))
		return -MateScore + (maxDepth - depth);
	if (hasWon(board.state()))
		return MateScore - (maxDepth - depth);

	if (depth == 0)
		return evaluate(board.state());

	curTime = std::chrono::system_clock::now();
	if (curTime > endTime)
		return 0;
	
	const int alphaOrig = alpha;

	const Hash hash = hashBoard(board.state());
	Move hashMove = makeMove(0, 0, NoPiece); // ????
	bool succ;
	const int hashResult = readTable(hash, depth, alpha, beta, succ, hashMove);
	if (succ)
		return hashResult;

	Move moves[200];
	const Move* moveListEnd = genMoves(board.state(), moves);
	const size_t numMoves = moveListEnd - moves;
	if (numMoves == 0)
		return -MateScore + (maxDepth - depth);

	for (int i = 0; i < numMoves; i++) {
		if (moves[i] == hashMove) {
			swap(moves[0], moves[i]);
			break;
		}
	}

	Move bestMove = makeMove(0, 0, NoPiece);
	for (int i = 0; i < numMoves; i++) {
		board.makeMove(moves[i]);
		const int score = -negamax(board, depth - 1, maxDepth, -beta, -alpha);
		board.undo();
		
		if (score > alpha) {
			alpha = score;
			bestMove = moves[i];
		}
		if (alpha >= beta)
			break;
	}

	curTime = std::chrono::system_clock::now();
	if (curTime > endTime)
		return 0;

	if (alphaOrig < alpha && alpha < beta)
		writeTable(hash, depth, alpha, EXACT, bestMove);
	else if (alpha == alphaOrig)
		writeTable(hash, depth, alpha, UPPER, bestMove);
	else
		writeTable(hash, depth, alpha, LOWER, bestMove);

	return alpha;
}

Move findBestMove(Board& board, double timeLeft, int maxDepth = 100) {
	for (auto& i : hashTable)
		i = HashEntry();

	double maxMoveTime = timeLeft / 30.0; //0.5;
	if (maxDepth != 100)
		maxMoveTime = 1000;

	const auto startTime = std::chrono::system_clock::now();
	endTime = startTime + std::chrono::duration<double>(maxMoveTime);

	Move moves[200];
	const Move* moveListEnd = genMoves(board.state(), moves);
	const size_t numMoves = moveListEnd - moves;

	vector<pair<int, Move>> sortedMoves;
	for (int i = 0; i < numMoves; i++)
		sortedMoves.emplace_back( INT_MIN / 2, moves[i] );

	nodeCount = 0;
	for (int d = 2; d <= maxDepth; d++) {
		int alpha = INT_MIN / 2;
		for (auto& [prevScore, move] : sortedMoves) {
			board.makeMove(move);
			const int score = -negamax(board, d - 1, d, INT_MIN / 2, -alpha + 1);
			board.undo();

			alpha = max(alpha, score);
			prevScore = score;
		}

		curTime = std::chrono::system_clock::now();
		if (curTime > endTime)
			break;

		stable_sort(sortedMoves.rbegin(), sortedMoves.rend());
		const int bestScore = board.state().turn == White ? sortedMoves[0].first : -sortedMoves[0].first;
		cerr << "[SEARCH] Depth: " << d << ", score: " << bestScore << ", best move: " << moveToString(sortedMoves[0].second) << ", nodes: " << nodeCount << endl;
		if (abs(bestScore) > MateScore - 100)
			break;
	}

	cerr << "[SEARCH] Nps: " << double(nodeCount) / double(maxMoveTime) << endl;

	return sortedMoves[0].second;
}
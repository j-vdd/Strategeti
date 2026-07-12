#pragma once

#include "MoveGen.h"
#include "Zobrist.h"
#include "Evaluate.h"

#include <algorithm>
#include <chrono>
#include <cstring>
#include <climits>

enum HashBound : uint8_t { NONE, EXACT, LOWER, UPPER };
struct HashEntry {
	HashEntry() = default;
	HashEntry(Hash hash, int score, Depth depth, Ply leafPly, HashBound bound, Move move)
		: hash(hash), score(score), depth(depth), leafPly(leafPly), bound(bound), move(move)
	{}
	Move move = makeMove(0, 0, NoPiece);
	Hash hash = 0;
	int score = 0;
	Ply leafPly = -1000;
	Depth depth = -1;
	HashBound bound = NONE;
};

constexpr int HashSize = 1 << 25;
inline HashEntry hashTable[HashSize] = {};

inline int readTable(Hash hash, Depth depth, int& alpha, int& beta, bool& succ, Move& hashMove) {
	HashEntry& entry = hashTable[hash & (HashSize - 1)];
	succ = false;
	if (entry.hash != hash)
		return 0;

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
inline void writeTable(const Hash hash, const Depth maxDepth, const Depth depth, const Ply ply, const int score, const HashBound bound, const Move move) {
	const Ply newLeafPly = (Ply)maxDepth - ply;
	if (hashTable[hash & (HashSize - 1)].leafPly > newLeafPly)
		return;

	hashTable[hash & (HashSize - 1)] = HashEntry(
		hash, score, depth, newLeafPly, bound, move
	);
}

inline bool isQuiet(const Move& move) {
	return move.from != move.to || move.type == Gazelle;
}

inline int historyTable[16][16][4] = {}; // other indexing??
inline int moveOrder[2][4] = { //move, place
	{5, 8, 4, 6},
	{2, 7, 1, 3}
};
inline Move killerMoves[100][3] = {};
inline Hash repetitionTable[MAX_GAME_PLIES] = {};

inline int64_t scoreMove(const BoardState& state, const Move& hashMove, const Move& move, const int depth) {
	if (move == hashMove)
		return 100'000'000'000'000'000ll;

	if (killerMoves[depth][2] == move)
		return 10'000'000'000'000'001ll;
	if (killerMoves[depth][1] == move)
		return 10'000'000'000'000'001ll;
	if (killerMoves[depth][0] == move)
		return 10'000'000'000'000'000ll;

	if (!isQuiet(move)) {
		return 1'000'000'000'000'000ll * (9 - moveOrder[move.from == move.to][move.type]) + historyTable[move.from][move.to][move.type];
	}

	return historyTable[move.from][move.to][move.type];
}

inline void sortMovesPartial(Move moveList[200], int64_t moveScores[200], const int idx, const size_t numMoves) {
	int bestIdx = idx;
	for (int i = idx + 1; i < numMoves; i++) {
		if (moveScores[i] > moveScores[bestIdx])
			bestIdx = i;
	}

	swap(moveList[idx], moveList[bestIdx]);
	swap(moveScores[idx], moveScores[bestIdx]);
}

inline int pushScore(const int score) {
	if (score >= MateScore - MAX_GAME_PLIES)
		return score + 1;
	if (score <= -MateScore + MAX_GAME_PLIES)
		return score - 1;

	return score;
}
inline int pullScore(const int score) {
	if (score >= MateScore - MAX_GAME_PLIES)
		return score - 1;
	if (score <= -MateScore + MAX_GAME_PLIES)
		return score + 1;

	return score;
}

inline uint64_t nodeCount = 0;
inline std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> endTime;
inline std::chrono::time_point<std::chrono::system_clock, std::chrono::duration<double>> curTime;

int negamax(Board& board, const Depth depth, const Depth maxDepth, int alpha, int beta) {
	nodeCount++;
	if (hasLost(board.state()))
		return -MateScore;
	if (hasWon(board.state()))
		return MateScore;

	if (depth == 0)
		return evaluate(board.state());

	curTime = std::chrono::system_clock::now();
	if (curTime > endTime)
		return 0;

	if (board.isRepetition())
		return 0;

	const int alphaOrig = alpha;
	Move hashMove = makeMove(0, 0, NoPiece); // ????
	bool succ;
	const int hashResult = readTable(board.state().hash, depth, alpha, beta, succ, hashMove);
	if (succ)
		return hashResult;

	Move moves[200];
	const Move* moveListEnd = genMoves(board.state(), moves);
	const size_t numMoves = moveListEnd - moves;
	if (numMoves == 0)
		return -MateScore;

	int64_t moveScores[200];
	for (int i = 0; i < numMoves; i++)
		moveScores[i] = scoreMove(board.state(), hashMove, moves[i], maxDepth - depth);

	Move bestMove = makeMove(0, 0, NoPiece);
	for (int i = 0; i < numMoves; i++) {
		sortMovesPartial(moves, moveScores, i, numMoves);

		board.makeMove(moves[i]);
		const int score = -pullScore(negamax(board, depth - 1, maxDepth, pushScore(-beta), pushScore(-alpha)));
		board.undo();
		
		if (score > alpha) {
			alpha = score;
			bestMove = moves[i];
		}
		if (alpha >= beta) {
			// if (isQuiet(moves[i]))
				historyTable[moves[i].from][moves[i].to][moves[i].type] += depth * depth;

			killerMoves[maxDepth - depth][0] = killerMoves[maxDepth - depth][1];
			killerMoves[maxDepth - depth][1] = killerMoves[maxDepth - depth][2];
			killerMoves[maxDepth - depth][2] = moves[i];

			break;
		}
	}

	curTime = std::chrono::system_clock::now();
	if (curTime > endTime)
		return 0;

	if (alphaOrig < alpha && alpha < beta)
		writeTable(board.state().hash, maxDepth, depth, board.ply, alpha, EXACT, bestMove);
	else if (alpha == alphaOrig)
		writeTable(board.state().hash, maxDepth, depth, board.ply, alpha, UPPER, bestMove);
	else
		writeTable(board.state().hash, maxDepth, depth, board.ply, alpha, LOWER, bestMove);

	return alpha;
}

inline string findPv(Board& board) {
	string result;

	int i = 0;
	while (i < 10) {
		const auto& entry = hashTable[board.state().hash & (HashSize - 1)];
		if (entry.hash != board.state().hash || entry.bound != EXACT)
			break;

		i++;
		board.makeMove(entry.move);
		result += moveToString(entry.move) + " ";
	}
	while (i) {
		board.undo();
		i--;
	}

	return result;
}

inline Move findBestMove(Board& board, const double timeLeft, const Depth maxDepth = 100) {
	double maxMoveTime = timeLeft / 30.0; //0.5;
	if (maxDepth != 100)
		maxMoveTime = 1000;

	const auto startTime = std::chrono::system_clock::now();
	endTime = startTime + std::chrono::duration<double>(maxMoveTime);

	memset(historyTable, 0, sizeof(historyTable));
	// for (auto& i : hashTable)
	// 	i = HashEntry();
	for (auto& killerMove : killerMoves) {
		killerMove[0] = makeMove(0, 0, NoPiece);
		killerMove[1] = makeMove(0, 0, NoPiece);
		killerMove[2] = makeMove(0, 0, NoPiece);
	}

	Move moves[200];
	const Move* moveListEnd = genMoves(board.state(), moves);
	const size_t numMoves = moveListEnd - moves;

	vector<pair<int, Move>> sortedMoves;
	for (int i = 0; i < numMoves; i++)
		sortedMoves.emplace_back( INT_MIN / 2, moves[i] );

	nodeCount = 0;
	for (Depth d = 2; d <= maxDepth; d++) {
		int alpha = INT_MIN / 2;

		for (auto& [prevScore, move] : sortedMoves) {
			board.makeMove(move);
			const int score = -pullScore(negamax(board, d - 1, d, pushScore(INT_MIN / 2), pushScore(-alpha + 1)));
			board.undo();

			alpha = max(alpha, score);
			prevScore = score;

			// cerr << "[SEARCH] " << (++cnt) << "/" << sortedMoves.size() << " (" << nodeCount << "), (" <<
			// 	static_cast<double>(std::chrono::duration_cast<std::chrono::seconds>(curTime - startTime).count()) << ")" << endl;
		}

		curTime = std::chrono::system_clock::now();
		if (curTime > endTime)
			break;

		stable_sort(sortedMoves.rbegin(), sortedMoves.rend());
		writeTable(board.state().hash, d, d, board.ply, alpha, EXACT, sortedMoves[0].second);

		const int bestScore = sortedMoves[0].first;
		cout <<
			"info depth " << static_cast<int>(d) <<
			" score " << bestScore <<
			" nodes " << nodeCount <<
			" pv " << findPv(board) << endl;

		if (abs(bestScore) >= MateScore - MAX_GAME_PLIES)
			break;
	}

	curTime = chrono::system_clock::now();
	cout << "Nps: " <<
		static_cast<double>(nodeCount) /
			static_cast<double>(chrono::duration_cast<chrono::microseconds>(curTime - startTime).count()) * 1000000.0 << endl;

	return sortedMoves[0].second;
}
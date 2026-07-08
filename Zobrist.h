#pragma once

#include "Board.h"

#include <random>
#include <climits>

typedef uint64_t Hash;

inline Hash hashTablePieces[4][1 << 16];
inline Hash hashTableOcc[2][1 << 16];
inline Hash hashTableSides[2][1 << 8];
inline Hash hashTableTurns[2];

void initHashes();

inline Hash hashBoard(const BoardState& board) {
	Hash hash = 0;
	for (int i = 0; i < 4; i++)
		hash ^= hashTablePieces[i][board.pieceBoards[i]];
	hash ^= hashTableOcc[0][board.colorBoards[board.turn]];
	hash ^= hashTableOcc[1][board.colorBoards[!board.turn]];

	hash ^= hashTableSides[0][board.pieces[board.turn]];
	hash ^= hashTableSides[1][board.pieces[!board.turn]];

	hash ^= hashTableTurns[board.turn];

	return hash;
}
#pragma once

#include "Board.h"
#include "LookupTables.h"
#include "Move.h"

template<PieceType Pt>
Move* genPieceMoves(const BoardState& board, Move* moveList) {
	u16 mask = board.pieceBoards[Pt] & board.colorBoards[0];
	u16 occ = board.getOcc();
	BitLoop(mask) {
		Square from = SquareOf(mask);
		u16 moves;
		if constexpr (Pt == Elephant) {
			uint8_t lookupIdx1 = elephantPextTable[board.pieceBoards[Elephant]][from];
			uint8_t lookupIdx2 = elephantPextTable[board.colorBoards[0] | board.colorBoards[1]][from];
			moves = elephantTable[lookupIdx1][lookupIdx2][from];
		}
		else if constexpr (Pt == Gazelle)
			moves = gazelleTable[occ][from];
		else if constexpr (Pt == Lion)
			moves = board.getOcc() & lionMasks[from] & ~board.pieceBoards[Lion] & ~board.pieceBoards[Elephant];
		else
			moves = zebraTable[occ][from];

		BitLoop(moves) {
			const Square to = SquareOf(moves);
			*moveList++ = makeMove(from, to, Pt);
		}
	}
	
	return moveList;
}

template<PieceType Pt>
Move* genPiecePlacements(const BoardState& board, Move* moveList) {
	const u16 occ = board.getOcc();
	constexpr u16 corners = squareBB(0) | squareBB(3) | squareBB(12) | squareBB(15);
	u16 allowedPlacement = ~corners & ~occ;
	BitLoop(allowedPlacement) {
		const Square sq = SquareOf(allowedPlacement);
		if (board.pieces[0] & (3 << (2 * Pt)))
			*moveList++ = makeMove(sq, sq, Pt);
	}

	return moveList;
}

inline Move* genMoves(const BoardState& board, Move* moveList) {
	moveList = genPiecePlacements<Lion>(board, moveList);
	moveList = genPiecePlacements<Elephant>(board, moveList);
	moveList = genPiecePlacements<Zebra>(board, moveList);

	moveList = genPieceMoves<Lion>(board, moveList);
	moveList = genPieceMoves<Elephant>(board, moveList);
	moveList = genPieceMoves<Zebra>(board, moveList);

	moveList = genPiecePlacements<Gazelle>(board, moveList);
	moveList = genPieceMoves<Gazelle>(board, moveList);

	return moveList;
}
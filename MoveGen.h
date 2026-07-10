#pragma once

#include "Board.h"
#include "LookupTables.h"
#include "Move.h"

template<PieceType Pt>
Move* genPieceMoves(const BoardState& board, Move* moveList) {
	u16 mask = board.pieceBoards[Pt] & board.colorBoards[0];
	const u16 occ = board.getOcc();
	BitLoop(mask) {
		const auto from = SquareOf(mask);
		u16 moves;
		if constexpr (Pt == Elephant) {
			const uint8_t lookupIdx1 = elephantPextTable[board.pieceBoards[Elephant]][from];
			const uint8_t lookupIdx2 = elephantPextTable[board.colorBoards[0] | board.colorBoards[1]][from];
			moves = elephantTable[lookupIdx1][lookupIdx2][from];
		}
		else if constexpr (Pt == Gazelle)
			moves = gazelleTable[occ][from];
		else if constexpr (Pt == Lion)
			moves = board.getOcc() & lionMasks[from] & ~board.pieceBoards[Lion] & ~board.pieceBoards[Elephant];
		else
			moves = zebraTable[occ][from];

		BitLoop(moves) {
			const auto to = SquareOf(moves);
			*moveList++ = makeMove(from, to, Pt);
		}
	}
	
	return moveList;
}

inline Move* genPiecePlacements(const BoardState& board, Move* moveList) {
	const u16 occ = board.getOcc();
	constexpr u16 corners = squareBB(0) | squareBB(3) | squareBB(12) | squareBB(15);
	u16 allowedPlacement = ~corners & ~occ;
	BitLoop(allowedPlacement) {
		const auto sq = SquareOf(allowedPlacement);
		uint8_t mask = board.pieces[0] & 0b01010101;
		BitLoop(mask) {
			const auto pt = static_cast<PieceType>(SquareOf(mask) / 2);
			*moveList++ = makeMove(sq, sq, pt);
		}
	}

	return moveList;
}

inline Move* genMoves(const BoardState& board, Move* moveList) {
	moveList = genPiecePlacements(board, moveList);

	moveList = genPieceMoves<Lion>(board, moveList);
	moveList = genPieceMoves<Elephant>(board, moveList);
	moveList = genPieceMoves<Zebra>(board, moveList);
	moveList = genPieceMoves<Gazelle>(board, moveList);

	return moveList;
}
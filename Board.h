#pragma once

#include "LookupTables.h"
#include "Move.h"
#include "Zobrist.h"

struct BoardState {
	Hash hash = 0;
	u16 pieceBoards[4] = {};
	u16 colorBoards[2] = {};

	uint8_t pieces[2] = { 0xFF, 0xFF };
	Color turn = White;

	u16 getOcc() const {
		return colorBoards[0] | colorBoards[1];
	}
	PieceType getPiece(Square sq) const {
		u16 mask = squareBB(sq);
		if (pieceBoards[Elephant] & mask) return Elephant;
		if (pieceBoards[Gazelle] & mask) return Gazelle;
		if (pieceBoards[Lion] & mask) return Lion;
		if (pieceBoards[Zebra] & mask) return Zebra;
		return NoPiece;
	}
	void log() const {
		string pieceStrings = "EGLZ";

		cerr << "   +---------+" << endl;
		for (int row = 0; row < 4; row++) {
			if (pieces[turn] & (1 << (row * 2)))
				cerr << pieceStrings[row];
			else
				cerr << ".";
			if (pieces[turn] & (1 << (row * 2 + 1)))
				cerr << pieceStrings[row];
			else
				cerr << ".";

			cerr << " | ";
			for (int col = 0; col < 4; col++) {
				char piece = "eglz."[getPiece(row * 4 + col)];
				if (colorBoards[turn] & squareBB(row * 4 + col))
					piece = piece == '.' ? piece : piece + 'A' - 'a';

				cerr << piece << ' ';
			}
			cerr << "| ";

			if (pieces[!turn] & (1 << (row * 2)))
				cerr << char(pieceStrings[row] + 'a' - 'A');
			else
				cerr << ".";
			if (pieces[!turn] & (1 << (row * 2 + 1)))
				cerr << char(pieceStrings[row] + 'a' - 'A');
			else
				cerr << ".";

			cerr << endl;
		}
		cerr << "   +---------+" << endl;
	}

	void hashBoard() {
		hash = 0;
		for (int i = 0; i < 4; i++)
			hash ^= hashTablePieces[i][pieceBoards[i]];
		hash ^= hashTableOcc[0][colorBoards[turn]];
		hash ^= hashTableOcc[1][colorBoards[!turn]];

		hash ^= hashTableSides[0][pieces[turn]];
		hash ^= hashTableSides[1][pieces[!turn]];

		hash ^= hashTableTurns[turn];
	}
};

struct Board {
private:
	BoardState states[MAX_GAME_PLIES];

public:
	Ply ply = 0;

	Board() {
		states[0] = BoardState{};
		states[0].hashBoard();
	};
	Board(const vector<string>& strings, const Color turn) {
		states[0] = BoardState{};
		BoardState& state = states[0];

		state.turn = turn;

		for (int row = 0; row < 4; row++) {
			for (int col = 0; col < 4; col++) {
				char type = strings[row][col];
				if (type == '.')
					continue;

				bool isBlack = type >= 'a' && type <= 'z';
				PieceType pt;
				if (type == 'E' || type == 'e')
					pt = Elephant;
				else if (type == 'G' || type == 'g')
					pt = Gazelle;
				else if (type == 'L' || type == 'l')
					pt = Lion;
				else 
					pt = Zebra;

				const Square sq = row * 4 + col;

				state.colorBoards[isBlack] |= squareBB(sq);
				state.pieceBoards[pt] |= squareBB(sq);
				state.pieces[isBlack] ^= 1 << (pt * 2);
				state.pieces[isBlack] =
					(state.pieces[isBlack] & ~(3 << (pt * 2))) |
					((state.pieces[isBlack] & (2 << (pt * 2))) >> 1);
			}
		}

		states[0].hashBoard();
	}

	const BoardState& state() const {
		return states[ply];
	}

	bool isRepetition() const {
		for (int i = ply - 4; i >= max(0, ply - 20); i -= 2) {
			if (states[i].hash == states[ply].hash)
				return true;
		}

		return false;
	}

	void makeMove(const Move& move) {
		states[ply + 1] = states[ply];
		ply++;

		BoardState& state = states[ply];

		const Square from = move.from;
		const Square to = move.to;
		if (from == to) {
			state.colorBoards[0] |= squareBB(from);
			state.pieceBoards[move.type] |= squareBB(from);
			state.pieces[0] ^= 1 << (move.type * 2);
			state.pieces[0] =
				(state.pieces[0] & ~(3 << (move.type * 2))) |
				((state.pieces[0] & (2 << (move.type * 2))) >> 1);

			swap(state.colorBoards[0], state.colorBoards[1]);
			swap(state.pieces[0], state.pieces[1]);
			state.turn = !state.turn;

			state.hashBoard();
			return;
		}

		if (move.type == Elephant) {
			int8_t shift = elephantShifts[from][to];
			u16 shiftMask = elephantShiftMasks[from][to];
			if (from < to) {
				shiftMaskLeftElephant(state.pieceBoards[Elephant], shiftMask, shift);
				shiftMaskLeftElephant(state.pieceBoards[Gazelle], shiftMask, shift);
				shiftMaskLeftElephant(state.pieceBoards[Lion], shiftMask, shift);
				shiftMaskLeftElephant(state.pieceBoards[Zebra], shiftMask, shift);
				shiftMaskLeftElephant(state.colorBoards[0], shiftMask, shift);
				shiftMaskLeftElephant(state.colorBoards[1], shiftMask, shift);
			}
			else {
				shiftMaskRightElephant(state.pieceBoards[Elephant], shiftMask, shift);
				shiftMaskRightElephant(state.pieceBoards[Gazelle], shiftMask, shift);
				shiftMaskRightElephant(state.pieceBoards[Lion], shiftMask, shift);
				shiftMaskRightElephant(state.pieceBoards[Zebra], shiftMask, shift);
				shiftMaskRightElephant(state.colorBoards[0], shiftMask, shift);
				shiftMaskRightElephant(state.colorBoards[1], shiftMask, shift);
			}
		}
		else {
			if (move.type == Lion) {
				state.colorBoards[0] &= ~squareBB(to);
				state.colorBoards[1] &= ~squareBB(to);
				state.pieceBoards[state.getPiece(to)] ^= squareBB(to);
			}

			u16 fromTo = squareBB(from) | squareBB(to);
			state.colorBoards[0] ^= fromTo;
			state.pieceBoards[move.type] ^= fromTo;
		}

		swap(state.colorBoards[0], state.colorBoards[1]);
		swap(state.pieces[0], state.pieces[1]);
		state.turn = !state.turn;

		state.hashBoard();
	}
	void undo() {
		ply--;
	}

// private:
};
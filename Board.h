#pragma once

#include "LookupTables.h"
#include "Move.h"

struct BoardState {
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
};

struct Board {
private:
	BoardState states[MAX_GAME_PLIES];
	int stateIdx = 0;
	
public:
	Board() {
		states[0] = BoardState{};
	};
	Board(vector<string> strings, Color turn) {
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

				Square sq = row * 4 + col;

				state.colorBoards[isBlack] |= squareBB(sq);
				state.pieceBoards[pt] |= squareBB(sq);
				state.pieces[isBlack] ^= 1 << (pt * 2);
				state.pieces[isBlack] =
					(state.pieces[isBlack] & ~(3 << (pt * 2))) |
					((state.pieces[isBlack] & (2 << (pt * 2))) >> 1);
			}
		}
	}
	
	const BoardState& state() const {
		return states[stateIdx];
	}

	void makeMove(const Move& move) {
		states[stateIdx + 1] = states[stateIdx];
		stateIdx++;

		BoardState& state = states[stateIdx];

		Square from = move.from;
		Square to = move.to;
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
	}
	void undo() {
		stateIdx--;
	}
};
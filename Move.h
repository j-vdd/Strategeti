#pragma once

#include "Types.h"

struct Move {
	Square from, to;
	PieceType type;
};

inline bool operator<(const Move& a, const Move& b) {
	return false;
}
inline bool operator==(const Move& a, const Move& b) {
	return a.from == b.from && a.to == b.to && a.type == b.type;
}

inline Move makeMove(Square from, Square to, PieceType type) {
	return Move{ from, to, type };
}

inline string sqToString(Square square) {
	return string(1, 'a' + (square % 4)) + string(1, '1' + (3 - square / 4));
}
inline string moveToString(Move move) {
	string first = string(1, "EGLZ"[move.type]);
	if (move.from == move.to)
		return first + sqToString(move.from);

	if (move.type == Elephant) {
		if (move.from / 4 == move.to / 4 && move.from % 4 < move.to % 4)
			move.to = move.from + 1;
		else if (move.from / 4 == move.to / 4 && move.from % 4 > move.to % 4)
			move.to = move.from - 1;
		else if (move.from % 4 == move.to % 4 && move.from / 4 < move.to / 4)
			move.to = move.from + 4;
		else
			move.to = move.from - 4;
	}

	return sqToString(move.from) + sqToString(move.to);
}
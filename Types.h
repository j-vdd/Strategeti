#pragma once

#include <iostream>
#include <cstdint>
#include <immintrin.h>

// #pragma warning(disable : 26812)
// #pragma warning(disable : 4146)

typedef uint16_t u16;
typedef uint8_t Square;
typedef uint8_t Row;
typedef uint8_t Col;
typedef int8_t Depth;
typedef int16_t Ply;
typedef uint64_t Hash;

enum PieceType : uint8_t { Elephant, Gazelle, Lion, Zebra, NoPiece };
enum Color : uint8_t { White, Black };
inline Color operator !(Color c) {
	return Color(uint8_t(c) ^ 1);
}

constexpr int MAX_GAME_PLIES = 200;

#define PopCount(X) __builtin_popcount(X)
#define SquareOf(X) Square(__builtin_ctz(X)) // __tzcnt_u32
#define BitLoop(X) for(;X; X = (X & (X - 1))) // __blsr_u32
// #define LSB(X) (X & -X)
// #define MSB(X) (1 << _lzcnt_u64(X))

#define ENABLE_INCR_OPERATORS_ON(T)                                \
inline T& operator++(T& d) { return d = T(int(d) + 1); }           \
inline T& operator--(T& d) { return d = T(int(d) - 1); }

ENABLE_INCR_OPERATORS_ON(PieceType)

constexpr inline u16 squareBB(Row r, Col c) {
	return 1 << (r * 4 + c);
}
constexpr inline u16 squareBB(Square sq) {
	return 1 << sq;
}

constexpr u16 rows[4] = { 0x000F, 0x00F0, 0x0F00, 0xF000 };
constexpr u16 cols[4] = { 0x1111, 0x2222, 0x4444, 0x8888 };
constexpr u16 diag = squareBB(0, 0) | squareBB(1, 1) | squareBB(2, 2) | squareBB(3, 3);
constexpr u16 antiDiag = squareBB(3, 0) | squareBB(2, 1) | squareBB(1, 2) | squareBB(0, 3);

inline void logBB(u16 bb, u16 extra) {
	for (int row = 0; row < 4; row++) {
		for (int col = 0; col < 4; col++) {
			if (bb & squareBB(row, col))
				std::cerr << 'X';
			else if (extra & squareBB(row, col))
				std::cerr << 'O';
			else
				std::cerr << '.';
		}
		std::cerr << std::endl;
	}
}

inline void shiftMaskLeftElephant(u16& bb, u16 mask, int8_t shift) {
	u16 shiftedMask = mask | (mask << shift);
	bb = (bb & ~shiftedMask) | ((bb & mask) << shift);
}
inline void shiftMaskRightElephant(u16& bb, u16 mask, int8_t shift) {
	u16 shiftedMask = mask | (mask >> shift);
	bb = (bb & ~shiftedMask) | ((bb & mask) >> shift);
}
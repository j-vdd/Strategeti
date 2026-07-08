#pragma once

#include "Types.h"

#include <vector>

using namespace std;

inline uint8_t elephantPextTable[1 << 16][16] = {};
inline u16 elephantTable[1 << 6][1 << 6][16] = {};
inline u16 elephantShiftMasks[16][16] = {};
inline int8_t elephantShifts[16][16] = {};
inline u16 lionMasks[16] = {};
inline u16 gazelleTable[1 << 16][16] = {};
inline u16 zebraTable[1 << 16][16] = {};

inline void initTables() {
	vector<pair<u16, u16>> singleGazelleMoves;
	for (int idx = 0; idx < 4; idx++) {
		singleGazelleMoves.emplace_back( squareBB(idx, 0) | squareBB(idx, 2), squareBB(idx, 1) );
		singleGazelleMoves.emplace_back( squareBB(idx, 1) | squareBB(idx, 3), squareBB(idx, 2) );
		singleGazelleMoves.emplace_back( squareBB(idx, 0) | squareBB(idx, 3), squareBB(idx, 1) | squareBB(idx, 2) );

		singleGazelleMoves.emplace_back( squareBB(0, idx) | squareBB(2, idx), squareBB(1, idx) );
		singleGazelleMoves.emplace_back( squareBB(1, idx) | squareBB(3, idx), squareBB(2, idx) );
		singleGazelleMoves.emplace_back( squareBB(0, idx) | squareBB(3, idx), squareBB(1, idx) | squareBB(2, idx) );

		int i = idx % 2;
		int j = idx / 2;
		singleGazelleMoves.emplace_back( squareBB(i, j) | squareBB(i + 2, j + 2), squareBB(i + 1, j + 1) );
		singleGazelleMoves.emplace_back( squareBB(i + 2, j) | squareBB(i, j + 2), squareBB(i + 1, j + 1) );
	}
	singleGazelleMoves.emplace_back( squareBB(0, 0) | squareBB(3, 3), squareBB(1, 1) | squareBB(2, 2) );
	singleGazelleMoves.emplace_back( squareBB(3, 0) | squareBB(0, 3), squareBB(2, 1) | squareBB(1, 2) );

	for (Square sq = 0; sq < 16; sq++) {
		std::cerr << "Initializing: " << static_cast<int>(sq) + 1 << "/16" << std::endl;
		int row = sq / 4;
		int col = sq % 4;
		u16 elephantMask = (rows[row] | cols[col]) ^ squareBB(sq);
		if (row > 0)
			lionMasks[sq] |= squareBB(sq - 4);
		if (row < 3)
			lionMasks[sq] |= squareBB(sq + 4);
		if (col > 0)
			lionMasks[sq] |= squareBB(sq - 1);
		if (col < 3)
			lionMasks[sq] |= squareBB(sq + 1);

		for (int board = 0; board < (1 << 16); board++) {
			int sqIdx = 0;
			u16 elephantMaskCopy = elephantMask;
			BitLoop(elephantMaskCopy) {
				if (board & squareBB(SquareOf(elephantMaskCopy)))
					elephantPextTable[board][sq] |= 1 << sqIdx;
				sqIdx++;
			}

			u16 gazelle = squareBB(sq);
			u16 gazelleBoard = board & ~squareBB(sq);
			for (int iter = 0; iter < 16; iter++) {
				u16 mask = gazelle;
				u16 prevMask = gazelle;
				BitLoop(mask) {
					auto from = SquareOf(mask);
					u16 fromBB = squareBB(from);
					for (const auto& [fromTo, occ] : singleGazelleMoves) {
						if ((gazelleBoard & occ) == occ && (gazelleBoard & fromTo) == 0 && (fromBB & fromTo))
							gazelle |= fromTo ^ fromBB;
					}
				}
				if (prevMask == gazelle)
					break;
			}
			gazelle ^= squareBB(sq);
			gazelleTable[board][sq] = gazelle;

			for (int idx = 1; idx <= 3; idx++) {
				if (row + idx >= 4) break;
				if (board & squareBB(row + idx, col)) break;
				zebraTable[board][sq] |= squareBB(row + idx, col);
			}
			for (int idx = 1; idx <= 3; idx++) {
				if (row + idx >= 4 || col + idx >= 4) break;
				if (board & squareBB(row + idx, col + idx)) break;
				zebraTable[board][sq] |= squareBB(row + idx, col + idx);
			}
			for (int idx = 1; idx <= 3; idx++) {
				if (col + idx >= 4) break;
				if (board & squareBB(row, col + idx)) break;
				zebraTable[board][sq] |= squareBB(row, col + idx);
			}
			for (int idx = 1; idx <= 3; idx++) {
				if (row - idx < 0 || col + idx >= 4) break;
				if (board & squareBB(row - idx, col + idx)) break;
				zebraTable[board][sq] |= squareBB(row - idx, col + idx);
			}
			for (int idx = 1; idx <= 3; idx++) {
				if (row - idx < 0) break;
				if (board & squareBB(row - idx, col)) break;
				zebraTable[board][sq] |= squareBB(row - idx, col);
			}
			for (int idx = 1; idx <= 3; idx++) {
				if (row - idx < 0 || col - idx < 0) break;
				if (board & squareBB(row - idx, col - idx)) break;
				zebraTable[board][sq] |= squareBB(row - idx, col - idx);
			}
			for (int idx = 1; idx <= 3; idx++) {
				if (col - idx < 0) break;
				if (board & squareBB(row, col - idx)) break;
				zebraTable[board][sq] |= squareBB(row, col - idx);
			}
			for (int idx = 1; idx <= 3; idx++) {
				if (row + idx >= 4 || col - idx < 0) break;
				if (board & squareBB(row + idx, col - idx)) break;
				zebraTable[board][sq] |= squareBB(row + idx, col - idx);
			}
		}
	}

	// Elephants
	for (Square sq = 0; sq < 16; sq++) {
		Row row = sq / 4;
		Col col = sq % 4;
		u16 relMask = (rows[row] | cols[col]) ^ squareBB(sq);
		for (u16 elMask = relMask; true; elMask = (elMask - u16(1)) & relMask) {
			for (u16 occMask = relMask; true; occMask = (occMask - u16(1)) & relMask) {
				int elIdx = elephantPextTable[elMask][sq];
				int occIdx = elephantPextTable[occMask][sq];

				u16 elephantDirMove = 0;
				for (int r = row + 1; r < 4; r++) {
					if (elMask & squareBB(r, col)) {
						elephantDirMove = 0;
						break;
					}
					if (occMask & squareBB(r, col)) elephantDirMove = squareBB(r, col);
					else break;
				}
				elephantTable[elIdx][occIdx][sq] |= elephantDirMove;

				elephantDirMove = 0;
				for (int r = row - 1; r >= 0; r--) {
					if (elMask & squareBB(r, col)) {
						elephantDirMove = 0;
						break;
					}
					if (occMask & squareBB(r, col)) elephantDirMove = squareBB(r, col);
					else break;
				}
				elephantTable[elIdx][occIdx][sq] |= elephantDirMove;

				elephantDirMove = 0;
				for (int c = col + 1; c < 4; c++) {
					if (elMask & squareBB(row, c)) {
						elephantDirMove = 0;
						break;
					}
					if (occMask & squareBB(row, c)) elephantDirMove = squareBB(row, c);
					else break;
				}
				elephantTable[elIdx][occIdx][sq] |= elephantDirMove;

				elephantDirMove = 0;
				for (int c = col - 1; c >= 0; c--) {
					if (elMask & squareBB(row, c)) {
						elephantDirMove = 0;
						break;
					}
					if (occMask & squareBB(row, c)) elephantDirMove = squareBB(row, c);
					else break;
				}
				elephantTable[elIdx][occIdx][sq] |= elephantDirMove;

				elephantTable[elIdx][occIdx][sq] &= ~squareBB(sq);
				if (occMask == 0)
					break;
			}

			if (elMask == 0)
				break;
		}
	}

	for (Square from = 0; from < 16; from++) {
		Row row = from / 4;
		Col col = from % 4;
		for (Square to = 0; to < 16; to++) {
			Row row2 = to / 4;
			Col col2 = to % 4;
			if (row == row2)
				elephantShifts[from][to] = 1;
			if (col == col2)
				elephantShifts[from][to] = 4;

			u16 toBB = squareBB(to);
			if (row2 == row && col2 < col) {
				for (int idx = 0; idx <= 3; idx++) {
					if (col - idx < 1) break;
					elephantShiftMasks[from][to] |= squareBB(row, col - idx);
					if (toBB & squareBB(row, col - idx)) break;
				}
			}
			else if (row2 == row && col2 > col) {
				for (int idx = 0; idx <= 3; idx++) {
					if (col + idx >= 3) break;
					elephantShiftMasks[from][to] |= squareBB(row, col + idx);
					if (toBB & squareBB(row, col + idx)) break;
				}
			}
			else if (col2 == col && row2 < row) {
				for (int idx = 0; idx <= 3; idx++) {
					if (row - idx < 1) break;
					elephantShiftMasks[from][to] |= squareBB(row - idx, col);
					if (toBB & squareBB(row - idx, col)) break;
				}
			}
			else if (col2 == col && row2 > row) {
				for (int idx = 0; idx <= 3; idx++) {
					if (row + idx >= 3) break;
					elephantShiftMasks[from][to] |= squareBB(row + idx, col);
					if (toBB & squareBB(row + idx, col)) break;
				}
			}
		}
	}
}
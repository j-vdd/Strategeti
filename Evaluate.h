//
// Created by jelle on 7/8/26.
//

#pragma once

#include "Board.h"

inline bool winTable[1 << 16] = {};

inline int evalWeights[5] = { 0, 0, 214, 500, 0 };
inline int pieceWeights[4] = { 0, 30, 40, 30}; // Elephants can't get killed so their weight is irrelevant
inline int placedBonuses[4] = { 0, 0, 1, 0 };
inline int evalTable[1 << 16] = {};
constexpr int MateScore = 1000000;

inline int scoreSide(const BoardState& board, const Color color) {
    int score = evalTable[board.colorBoards[color]];
    score += (placedBonuses[Elephant] + pieceWeights[Elephant]) * PopCount(board.colorBoards[color] & board.pieceBoards[Elephant]);
    score += (placedBonuses[Gazelle] + pieceWeights[Gazelle]) * PopCount(board.colorBoards[color] & board.pieceBoards[Gazelle]);
    score += (placedBonuses[Lion] + pieceWeights[Lion]) * PopCount(board.colorBoards[color] & board.pieceBoards[Lion]);
    score += (placedBonuses[Zebra] + pieceWeights[Zebra]) * PopCount(board.colorBoards[color] & board.pieceBoards[Zebra]);

    score += pieceWeights[Elephant] * PopCount(board.pieces[color] & (3 << (2 * Elephant)));
    score += pieceWeights[Gazelle] * PopCount(board.pieces[color] & (3 << (2 * Gazelle)));
    score += pieceWeights[Lion] * PopCount(board.pieces[color] & (3 << (2 * Lion)));
    score += pieceWeights[Zebra] * PopCount(board.pieces[color] & (3 << (2 * Zebra)));
    return score;
}
inline int evaluate(const BoardState& board) {
    return scoreSide(board, White) - scoreSide(board, Black);
    return evalTable[board.colorBoards[0]] - evalTable[board.colorBoards[1]];
}

inline bool hasWon(const BoardState& board) {
    if (PopCount(board.pieces[1]) + PopCount(board.colorBoards[1]) <= 3)
        return true;
    return winTable[board.colorBoards[0]];
}
inline bool hasLost(const BoardState& board) {
    if (PopCount(board.pieces[0]) + PopCount(board.colorBoards[0]) <= 3)
        return true;
    return winTable[board.colorBoards[1]];
}


inline void initEvalTables() {
    for (int board = 0; board < (1 << 16); board++) {
        u16 mask = board;
        evalTable[board] += evalWeights[PopCount(diag & mask)];
        evalTable[board] += evalWeights[PopCount(antiDiag & mask)];
        for (int idx = 0; idx < 4; idx++) {
            evalTable[board] += evalWeights[PopCount(rows[idx] & mask)];
            evalTable[board] += evalWeights[PopCount(cols[idx] & mask)];
        }
    }

    // Check for victory
    for (int mask = 0; mask < (1 << 16); mask++) {
        for (int idx = 0; idx < 4; idx++) {
            if ((mask & rows[idx]) == rows[idx] || (mask & cols[idx]) == cols[idx])
                winTable[mask] = true;
            if ((mask & diag) == diag || (mask & antiDiag) == antiDiag)
                winTable[mask] = true;
        }
    }
}

//
// Created by jelle on 7/8/26.
//

#pragma once

#include "Board.h"

inline bool winTable[1 << 16] = {};

inline int evalWeightsOld[5] = { 0, 0, 214, 500, 0 };
inline int evalWeights[5][5] = {
    {0, 0, 0, 0, 0},
    {0, 0, 0, 0, 0},
    {214, 214, 0, 0, 0},
    {500, 500, 0, 0, 0},
    {0, 0, 0, 0, 0}
};
inline int pieceWeights[4] = { 0, 30, 50, 30}; // Elephants can't get killed so their weight is irrelevant
inline int placedBonuses[4] = { 0, 0, 1, 0 };
inline int evalPositionTables[4][16] = {
    {
        -1, 0, 0, -1,
        0, 0, 0, 0,
        0, 0, 0, 0,
        -1, 0, 0, -1,
    },
    {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
    },
    {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
    },
    {
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
        0, 0, 0, 0,
    },
};
inline int evalTable[1 << 16] = {};
constexpr int MateScore = 1000000;

inline int scoreSide(const BoardState& board, const Color color) {
    int score = evalTable[board.colorBoards[color]];
    // const u16 occ = board.colorBoards[color];
    // const u16 opp = board.colorBoards[!color];
    // score += evalWeights[PopCount(diag & occ)][PopCount(diag & opp)];
    // score += evalWeights[PopCount(antiDiag & occ)][PopCount(antiDiag & opp)];
    // for (int idx = 0; idx < 4; idx++) {
    //     score += evalWeights[PopCount(rows[idx] & occ)][PopCount(rows[idx] & opp)];
    //     score += evalWeights[PopCount(cols[idx] & occ)][PopCount(cols[idx] & opp)];
    // }

    score += (placedBonuses[Elephant] + pieceWeights[Elephant]) * PopCount(board.colorBoards[color] & board.pieceBoards[Elephant]);
    score += (placedBonuses[Gazelle] + pieceWeights[Gazelle]) * PopCount(board.colorBoards[color] & board.pieceBoards[Gazelle]);
    score += (placedBonuses[Lion] + pieceWeights[Lion]) * PopCount(board.colorBoards[color] & board.pieceBoards[Lion]);
    score += (placedBonuses[Zebra] + pieceWeights[Zebra]) * PopCount(board.colorBoards[color] & board.pieceBoards[Zebra]);

    score += pieceWeights[Elephant] * PopCount(board.pieces[color] & (3 << (2 * Elephant)));
    score += pieceWeights[Gazelle] * PopCount(board.pieces[color] & (3 << (2 * Gazelle)));
    score += pieceWeights[Lion] * PopCount(board.pieces[color] & (3 << (2 * Lion)));
    score += pieceWeights[Zebra] * PopCount(board.pieces[color] & (3 << (2 * Zebra)));

    // u16 mask = board.colorBoards[color] & board.pieceBoards[Elephant];
    // BitLoop(mask) {
    //     const Square sq = SquareOf(mask);
    //     score += evalPositionTables[Elephant][sq];
    // }

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
        evalTable[board] += evalWeightsOld[PopCount(diag & mask)];
        evalTable[board] += evalWeightsOld[PopCount(antiDiag & mask)];
        for (int idx = 0; idx < 4; idx++) {
            evalTable[board] += evalWeightsOld[PopCount(rows[idx] & mask)];
            evalTable[board] += evalWeightsOld[PopCount(cols[idx] & mask)];
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

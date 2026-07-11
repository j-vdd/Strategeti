//
// Created by jelle on 7/7/26.
//

#include "Zobrist.h"

#include <random>
#include <climits>

void initHashes() {
    std::mt19937_64 gen(3882119);
    std::uniform_int_distribution<uint64_t> dist(0, ULLONG_MAX);
    for (int mask = 0; mask < (1 << 16); ++mask) {
        hashTableOcc[0][mask] = dist(gen);
        hashTableOcc[1][mask] = dist(gen);
        for (auto& hashTablePiece : hashTablePieces)
            hashTablePiece[mask] = dist(gen);
    }

    for (int mask = 0; mask < (1 << 8); mask++) {
        hashTableSides[0][mask] = dist(gen);
        hashTableSides[1][mask] = dist(gen);
    }
    hashTableTurns[0] = dist(gen);
    hashTableTurns[1] = dist(gen);
}
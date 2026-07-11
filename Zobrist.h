#pragma once

#include "Types.h"

inline Hash hashTablePieces[4][1 << 16];
inline Hash hashTableOcc[2][1 << 16];
inline Hash hashTableSides[2][1 << 8];
inline Hash hashTableTurns[2];

void initHashes();
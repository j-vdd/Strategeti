#pragma once

#include "Search.h"

void make(Board& board, string move) {
    Move moves[200];
    Move* moveListEnd = genMoves(board.state(), moves);
    int numMoves = moveListEnd - moves;
    for (int i = 0; i < numMoves; i++) {
        if (moveToString(moves[i]) == move) {
            board.makeMove(moves[i]);
            break;
        }
    }
    board.state().log();
}

string moveToUSI(Move move) {
    return moveToString(move);
}

void startProtocol() {
    Board board{};
    while (true) {
        string line;
        getline(cin, line);
        if (line == "quit") {
            break;
        }
        else if (line == "usinewgame") {
            initTables();
            initHashes();
            initEvalTables();
            cout << "usiok" << endl;
        }
        else if (line == "isready") {
            cout << "readyok" << endl;
        }
        else if (line.substr(0, 2) == "go") {
            int wtime = 30000, btime = 30000; //time left for white in milliseconds and time left for black in milliseconds
            if (line.find("wtime") != string::npos) {
                stringstream s(line.substr(line.find("wtime") + 6));
                s >> wtime;
            }
            if (line.find("btime") != string::npos) {
                stringstream s(line.substr(line.find("btime") + 6));
                s >> btime;
            }
            if (board.state().turn == Black)
                swap(wtime, btime);

            Move bestmove = findBestMove(board, double(wtime) / 1000.0); //any debug prints with an endline at the end is fine in the bestMove function, as long as it does not contain the string bestmove
            board.makeMove(bestmove);
            board.state().log();
            cout << "bestmove " << moveToUSI(bestmove) << endl;
        }
        else {
            make(board, line);
            // makeMove(board, USItoMove(line, board));
        }
    }
}
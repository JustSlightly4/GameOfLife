/*
*   Ryan Montgomery
*   01/08/26
*   Conways Game Of Life
*/

#include <iostream>
#include <array>
#include <vector>
#include <algorithm>
#include <utility>
#include <sys/time.h>
using namespace std;

/* function to get wall clock time as double */
double gettime() {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

void setBoard(char *board, int sizeOfBoard) {
    int totalSize = (sizeOfBoard) * (sizeOfBoard);
    for (int i = 0; i < totalSize; ++i) {
        board[i] = 0;
    }

    int rng = gettime();
    for (int i = 1; i < sizeOfBoard-1; ++i) {
        for (int j = 1; j < sizeOfBoard-1; ++j) {
            int index = j + ((sizeOfBoard) * i);
            rng ^= rng >> 8;
            board[index] = rng & 1;
            rng ^= rng << 13;
            rng ^= rng >> 17;
            rng ^= rng << index;
        }
    }

}

void setBoardTestCase(char *board, int sizeOfBoard) {
    int totalSize = (sizeOfBoard) * (sizeOfBoard);
    for (int i = 0; i < totalSize; ++i) {
        board[i] = 0;
    }
    int center = (sizeOfBoard / 2) * sizeOfBoard + (sizeOfBoard / 2);

    board[center - 1 - sizeOfBoard] = 1;
    board[center - sizeOfBoard] = 1;
    board[center + 1 - sizeOfBoard] = 1;

    board[center - 1] = 1;
    board[center] = 1;
    board[center + 1] = 1;

    board[center - 1 + sizeOfBoard] = 1;
    board[center + sizeOfBoard] = 1;

}

void setBoardInfinite(char *board, int sizeOfBoard) {
    int totalSize = (sizeOfBoard) * (sizeOfBoard);
    for (int i = 0; i < totalSize; ++i) {
        board[i] = 0;
    }
    int center = (sizeOfBoard / 2) * sizeOfBoard + (sizeOfBoard / 2);

    board[center - sizeOfBoard - 1] = 1;
    board[center - sizeOfBoard] = 1;
    board[center - sizeOfBoard + 1] = 1;
    board[center] = 1;
    board[center + sizeOfBoard] = 1;

}

void printBoard(char *board, int sizeOfBoard) {
    for (int i = 0; i < sizeOfBoard; ++i) {
        for (int j = 0; j < sizeOfBoard; ++j) {
            cout << (board[j + (sizeOfBoard * i)] ? "1 " : "0 ");
        }
        cout << "\n";
    }
}

int getPopulationScore(char *board, int sizeOfBoard, int index) {
    int score = 0;
    score += board[index - 1 - sizeOfBoard];
    score += board[index - sizeOfBoard];
    score += board[index + 1 - sizeOfBoard];

    score += board[index - 1];
    score += board[index + 1];

    score += board[index - 1 + sizeOfBoard];
    score += board[index + sizeOfBoard];
    score += board[index + 1 + sizeOfBoard];
    return score;
}

int advanceGeneration(char *board, vector<int> &markForChange, int sizeOfBoard) {

    for (int i = 1; i < sizeOfBoard-1; ++i) {
        for (int j = 1; j < sizeOfBoard-1; ++j) {
            int index = j + ((sizeOfBoard) * i);
            int score = getPopulationScore(board, sizeOfBoard, index);
            if (board[index] && (score < 2 || score > 3)) markForChange.push_back(index);
            if (!board[index] && (score == 3)) markForChange.push_back(index);
        }
    }

    if (markForChange.size() < 1) return 0;

    for (int i = 0; i < markForChange.size(); ++i) {
        board[markForChange[i]] = 1 - board[markForChange[i]];
    }

    return 1;
}

int main(int argc, char **argv) {

    //Check to make sure that their are three command line arguments
    if (argc != 3) {
        printf("Usage: ./a.out <sizeOfBoard> <generations>\n");
        return 1;
    }

    //Initialize Values
    int const sizeOfBoard = atoi(argv[1]) + 2;
    int const generations = atoi(argv[2]);

    if (sizeOfBoard < 3) {
        printf("Error: <sizeOfBoard> must be 3 or more!\n");
        return 2;
    }

    if (generations < 1) {
        printf("Error: <generations> must be 1 or more!\n");
        return 3;
    }

    char *board = new char[(sizeOfBoard) * (sizeOfBoard)];
    setBoardInfinite(board, sizeOfBoard);


    //printBoard(board, sizeOfBoard);
    //cout << "\n";
    int i;
    vector<int> markForChange;
    markForChange.reserve(100);
    double startTime = gettime();
    for (i = 0; i < generations; ++i) {
        if (!advanceGeneration(board, markForChange, sizeOfBoard)) break;
        markForChange.clear();
    }
    double endTime = gettime();

    cout << "Stopped at " << i << " generations.\n";
    cout << "Time taken " << endTime - startTime << " seconds\n";
    //printBoard(board, sizeOfBoard);

    delete[] board;
    return 0;

}
/*
* Name: Eric Ryan Montgomery
* Email: ermontgomery1@crimson.ua.edu
* Course Section: CS 481
* Homework #: 0
* Instructions to compile the program: g++ -march=native -O3 gameOfLife.cpp
* Instructions to run the program: ./a 10000 5000
*/

#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#ifdef _OPENMP
#   include <omp.h>
#endif
using namespace std;

/* function to get wall clock time as double */
/*
double gettime() {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}
*/

/* chrono function to get wall clock time as double */
double gettime() {
    return chrono::duration<double>(
        chrono::high_resolution_clock::now().time_since_epoch()
    ).count();
}

//Intializes a given board to all zeros
void initBoard(unsigned char *board, int sizeOfBoard) {
    int totalSize = (sizeOfBoard) * (sizeOfBoard);
    for (int i = 0; i < totalSize; ++i) {
        board[i] = 0;
    }
}

//Sets a board to all random values
void setBoardRandom(unsigned char *board, int sizeOfBoard) {
    for (int i = 1; i < sizeOfBoard-1; ++i) {
        for (int j = 1; j < sizeOfBoard-1; ++j) {
            int index = j + ((sizeOfBoard) * i);
            board[index] = rand() % 2;
        }
    }

}

//Sets a board to the given test case in homework 0
void setBoardTestCase(unsigned char *board, int sizeOfBoard) {
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

//Sets the board to an infinite pattern
void setBoardInfinite(unsigned char *board, int sizeOfBoard) {
    int center = (sizeOfBoard / 2) * sizeOfBoard + (sizeOfBoard / 2);

    board[center - sizeOfBoard - 1] = 1;
    board[center - sizeOfBoard] = 1;
    board[center - sizeOfBoard + 1] = 1;
    board[center] = 1;
    board[center + sizeOfBoard] = 1;

}

//Prints the board to the screen
void printBoard(unsigned char *board, int sizeOfBoard) {
    for (int i = 0; i < sizeOfBoard; ++i) {
        for (int j = 0; j < sizeOfBoard; ++j) {
            cout << (board[j + (sizeOfBoard * i)] ? "1 " : "0 ");
        }
        cout << "\n";
    }
}

int main(int argc, char **argv) {

    //Check to make sure that their are three command line arguments
    if (argc != 4) {
        printf("Usage: ./a.out <sizeOfBoard> <generations> <threads>\n");
        return 1;
    }

    //Initialize Values
    int const sizeOfBoard = atoi(argv[1]) + 2;
    int const generations = atoi(argv[2]);
    int const threads = atoi(argv[3]);

    if (sizeOfBoard < 3) {
        printf("Error: <sizeOfBoard> must be 3 or more!\n");
        return 2;
    }

    if (generations < 0) {
        printf("Error: <generations> must be 0 or more!\n");
        return 3;
    }

    if (threads < 1) {
        printf("Error: <threads> must be 1 or more!\n");
        return 4;
    }

    //Sets random function
    srand(time(NULL));

    //Create, intialize, and set the pattern for the boards
    unsigned char *board = new unsigned char[(sizeOfBoard) * (sizeOfBoard)];
    unsigned char *nextBoard = new unsigned char[(sizeOfBoard) * (sizeOfBoard)];
    initBoard(board, sizeOfBoard);
    initBoard(nextBoard, sizeOfBoard);
    setBoardTestCase(board, sizeOfBoard);


    printBoard(board, sizeOfBoard);
    cout << "\n";
    int i = 0;
    double startTime = gettime();

    //This does not have the for directive so it doesn't parallelize the outside
    //for loop as far as I'm aware. It allows the inside for loop to reuse threads
    //between generations.
    unsigned char changed = 0; // shared across threads
    unsigned char finished = 0;
    #pragma omp parallel num_threads(threads) 
    { 
        for (int gen = 0; gen < generations; ++gen) {

            //Start of main loop where threads each take a piece of the work
            #pragma omp for reduction(|:changed)
            for (int r = 1; r < sizeOfBoard-1; ++r) {
                unsigned char* rowAbove   = board + (r - 1) * sizeOfBoard;
                unsigned char* rowCurrent = board + r * sizeOfBoard;
                unsigned char* rowBelow   = board + (r + 1) * sizeOfBoard;
                unsigned char* rowNext    = nextBoard + r * sizeOfBoard;
                unsigned char localChanged = 0;

                for (int c = 1; c < sizeOfBoard-1; ++c) {
                    unsigned char score =
                        rowAbove[c-1] + rowAbove[c] + rowAbove[c+1] +
                        rowCurrent[c-1] + rowCurrent[c+1] +
                        rowBelow[c-1] + rowBelow[c] + rowBelow[c+1];

                    unsigned char nextValue = (score == 3) || (rowCurrent[c] && score == 2);

                    rowNext[c] = nextValue;
                    localChanged |= (nextValue ^ rowCurrent[c]);
                }
                changed |= localChanged;
            }

            //Only one thread checked if all threads need to finish or not
            //Then that same single thread swaps the boards
            #pragma omp single
            {
                if (!changed) finished = 1;
                std::swap(board, nextBoard);
                i = gen + 1;
                changed = 0;
            }
            if (finished) break;
        }
    }

    double endTime = gettime();

    cout << "Stopped at " << (finished ? i-1 : i) << " generations.\n";
    cout << "Time taken " << endTime - startTime << " seconds\n";
    printBoard(board, sizeOfBoard);

    delete[] board;
    delete[] nextBoard;
    return 0;

}
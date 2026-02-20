/*
* Name: Eric Ryan Montgomery
* Email: ermontgomery1@crimson.ua.edu
* Course Section: CS 481
* Homework #: 0
* Instructions to compile the program: g++ -Wall -O3 -march=native -fopenmp -fopt-info-vec-optimized gameOfLifeParallel.cpp
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
using var = uint_fast8_t;
using var2 = uint_fast32_t;

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
void initBoard(var* __restrict__ board, var2 sizeOfBoard) {
    var2 totalSize = (sizeOfBoard) * (sizeOfBoard);
    for (var2 i = 0; i < totalSize; ++i) {
        board[i] = 0;
    }
}

//Sets a board to all random values
void setBoardRandom(var* board, var2 sizeOfBoard) {
    for (var2 i = 1; i < sizeOfBoard-1; ++i) {
        for (var2 j = 1; j < sizeOfBoard-1; ++j) {
            var2 index = j + ((sizeOfBoard) * i);
            board[index] = rand() % 2;
        }
    }

}

//Sets a board to the given test case in homework 0
void setBoardTestCase(var* board, var2 sizeOfBoard) {
    var2 center = (sizeOfBoard / 2) * sizeOfBoard + (sizeOfBoard / 2);

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
void setBoardInfinite(var*board, var2 sizeOfBoard) {
    var2 center = (sizeOfBoard / 2) * sizeOfBoard + (sizeOfBoard / 2);

    board[center - sizeOfBoard - 1] = 1;
    board[center - sizeOfBoard] = 1;
    board[center - sizeOfBoard + 1] = 1;
    board[center] = 1;
    board[center + sizeOfBoard] = 1;

}

//Prints the board to the screen
void printBoard(var*board, var2 sizeOfBoard) {
    for (var2 i = 0; i < sizeOfBoard; ++i) {
        for (var2 j = 0; j < sizeOfBoard; ++j) {
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
    var2 const sizeOfBoard = atoi(argv[1]) + 2;
    var2 const generations = atoi(argv[2]);
    var2 const threads = atoi(argv[3]);

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
    var* __restrict__ board = new var[(sizeOfBoard) * (sizeOfBoard)];
    var* __restrict__ nextBoard = new var[(sizeOfBoard) * (sizeOfBoard)];
    initBoard(board, sizeOfBoard);
    initBoard(nextBoard, sizeOfBoard);
    setBoardInfinite(board, sizeOfBoard);


    //printBoard(board, sizeOfBoard);
    //cout << "\n";
    var2 i = 0; //Is is to record the final generation
    double startTime = gettime();

    //This does not have the for directive so it doesn't parallelize the outside
    //for loop as far as I'm aware. It allows the inside for loop to reuse threads
    //between generations.
    var changed = 0; // shared across threads
    var finished = 1;
    #pragma omp parallel num_threads(threads) 
    { 
        for (var2 gen = 0; gen < generations && finished; ++gen) {

            //Start of main loop where threads each take a piece of the work
            #pragma omp for schedule(static) reduction(|:changed)
            for (var2 r = 1; r < sizeOfBoard-1; ++r) { //r is for the row

                //Pointers to the boards rows. The restrict keyword tells the compiler that
                //it can optimize and not have to worry about aliasing
                var* __restrict__ rowAbove = board + (r - 1) * sizeOfBoard;
                var* __restrict__ rowCurrent = board + r * sizeOfBoard;
                var* __restrict__ rowBelow = board + (r + 1) * sizeOfBoard;
                var* __restrict__ rowNext = nextBoard + r * sizeOfBoard;

                //Each thread having a local changed is faster for some reason
                //I figured out why. This avoid false sharing on the cache line
                var localChanged = 0;
                
                #pragma omp simd
                for (var2 c = 1; c < sizeOfBoard-1; ++c) { //c is for the column
                    var score =
                        rowAbove[c-1] + rowAbove[c] + rowAbove[c+1] +
                        rowCurrent[c-1] + rowCurrent[c+1] +
                        rowBelow[c-1] + rowBelow[c] + rowBelow[c+1];

                    var nextValue = (score == 3) || (rowCurrent[c] && score == 2);

                    rowNext[c] = nextValue;
                    localChanged |= (nextValue ^ rowCurrent[c]);

                }

                changed |= localChanged;

            }

            //Only one thread checked if all threads need to finish or not
            //Then that same single thread swaps the boards
            //#pragma omp barrier
            #pragma omp single
            {
                finished = finished && changed;
                std::swap(board, nextBoard);
                i = gen + 1;
                changed = 0;
            } //Implicit barrier at the end of omp single
        }
    }

    double endTime = gettime();

    cout << "Stopped at " << (finished ? i : i-1) << " generations.\n";
    cout << "Time taken " << endTime - startTime << " seconds\n";
    //printBoard(board, sizeOfBoard);

    delete[] board;
    delete[] nextBoard;
    return 0;

}
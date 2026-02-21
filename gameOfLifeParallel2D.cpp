/*
* Name: Eric Ryan Montgomery
* Email: ermontgomery1@crimson.ua.edu
* Course Section: CS 481
* Homework #: 2
* Instructions to compile the program: g++ -Wall -O3 -march=native -fopenmp gameOfLifeParallel2D.cpp
* Instructions to run the program: ./a.out <sizeOfBoard> <generations> <threads> <outputFile>
*/

#include <iostream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <stdio.h>
#include <stdlib.h>
#include <fstream>
#ifdef _OPENMP
#   include <omp.h>
#endif
using namespace std;
using var = uint_fast8_t;

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

/* function to allocate a 2-D array */
var **allocarray(int P, int Q) {
  int i;
  var *p, **a;
  
  p = (var *)malloc(P*Q*sizeof(var));
  a = (var **)malloc(P*sizeof(var*));

  if (p == NULL || a == NULL) 
    printf("Error allocating memory\n");

  /* for row major storage */
  for (i = 0; i < P; i++)
    a[i] = &p[i*Q];
  
  return a;
}

/* function to delete the 2-D array */
void freearray(var **a) {
  free(&a[0][0]);
  free(a);
}

/* function to initialize the array */
void initarray(var **a, int mrows, int ncols, var value) {
  int i,j;

  for (i=0; i<mrows; i++)
    for (j=0; j<ncols; j++)
      // a[i][j] = drand48()*value;
      a[i][j] = value;
}

//Sets a board to all random values
void setBoardRandom(var** board, int mrows, int ncols) {
    for (int i = 1; i < mrows-1; ++i) {
        for (int j = 1; j < ncols-1; ++j) {
            board[i][j] = rand() % 2;
        }
    }

}

//Sets a board to the given test case in homework 0
void setBoardTestCase(var **a, int mrows, int ncols) {
    int xMiddle = mrows/2;
    int yMiddle = ncols/2;
    a[xMiddle-1][yMiddle-1] = 1;
    a[xMiddle][yMiddle-1] = 1;
    a[xMiddle+1][yMiddle-1] = 1;
    a[xMiddle-1][yMiddle] = 1;
    a[xMiddle][yMiddle] = 1;
    a[xMiddle+1][yMiddle] = 1;
    a[xMiddle-1][yMiddle+1] = 1;
    a[xMiddle][yMiddle+1] = 1;

}

//Sets the board to an infinite pattern
void setBoardInfinite(var **a, int mrows, int ncols) {
    int xMiddle = mrows/2;
    int yMiddle = ncols/2;

    a[xMiddle][yMiddle] = 1;
    a[xMiddle-1][yMiddle] = 1;
    a[xMiddle+1][yMiddle] = 1;
    a[xMiddle][yMiddle-1] = 1;

}

//Prints the board to the screen
void printBoard(var **board, int mrows, int ncols) {
    for (int i = 0; i < mrows; ++i) {
        for (int j = 0; j < ncols; ++j) {
            cout << (board[i][j] ? "1" : "0") << " ";
        }
        cout << "\n";
    }
}

//Writes the board to a file
void writeBoard(var **board, int mrows, int ncols, string fileName, string title) {
    std::ofstream outFile(fileName);
    if (!outFile) {
        std::cerr << "Error: Unable to open file for writing." << std::endl;
        return;
    }
    outFile << title << ":\n";
    for (int i = 0; i < mrows; ++i) {
        for (int j = 0; j < ncols; ++j) {
            outFile << (board[i][j] ? "1" : "0") << " ";
        }
        outFile << "\n";
    }
    outFile.close();
}

void writeBoardInfo(double timeTaken, int generationsTaken, string fileName) {
    std::ofstream outFile(fileName);
    if (!outFile) {
        std::cerr << "Error: Unable to open file for writing." << std::endl;
        return;
    }
    outFile << "Stopped at " << generationsTaken << " generations.\n";
    outFile << "Time taken " << timeTaken << " seconds\n";
    outFile.close();
}

int playGameOfLife(var** board, var** nextBoard, int sizeOfBoard, int generations, int threads) {
    //This does not have the for directive so it doesn't parallelize the outside
    //for loop as far as I'm aware. It allows the inside for loop to reuse threads
    //between generations.
    int i = 0;
    var changed = 0; // shared across threads
    var finished = 1;
    #pragma omp parallel num_threads(threads)
    { 
        while(i < generations && finished) {

            //Start of main loop where threads each take a piece of the work
            #pragma omp for schedule(static) reduction(|:changed)
            for (int r = 1; r < sizeOfBoard-1; ++r) { //r is for the row

                var* __restrict__ rowAbove = board[r-1];
                var* __restrict__ rowCurrent = board[r];
                var* __restrict__ rowBelow = board[r+1];
                var* __restrict__ rowNext = nextBoard[r];

                //Each thread having a local changed is faster for some reason
                //I figured out why. This avoid false sharing on the cache line
                var localChanged = 0;
                
                #pragma omp simd
                for (int c = 1; c < sizeOfBoard-1; ++c) { //c is for the column
                    var score =
                        rowAbove[c-1] + rowAbove[c] + rowAbove[c+1] +
                        rowCurrent[c-1] + rowCurrent[c+1] +
                        rowBelow[c-1] + rowBelow[c] + rowBelow[c+1];

                    var nextValue = (score == 3) | (rowCurrent[c] & (score == 2));

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
                finished = finished & changed;
                std::swap(board, nextBoard);
                i++;
                changed = 0;
            } //Implicit barrier at the end of omp single
        }
    }
    return i;
}


int main(int argc, char **argv) {

    //Check to make sure that their are three command line arguments
    if (argc != 5) {
        printf("Usage: ./a.out <sizeOfBoard> <generations> <threads> <outputFile>\n");
        return 1;
    }

    //Initialize Values
    int const sizeOfBoard = atoi(argv[1]) + 2;
    int const generations = atoi(argv[2]);
    int const threads = atoi(argv[3]);
    string const outputFile = argv[4];

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
    //srand(time(NULL));
    srand(0);

    //Create, intialize, and set the pattern for the boards
    var** board = allocarray(sizeOfBoard, sizeOfBoard);
    var** nextBoard = allocarray(sizeOfBoard, sizeOfBoard);
    initarray(board, sizeOfBoard, sizeOfBoard, 0);
    initarray(nextBoard, sizeOfBoard, sizeOfBoard, 0);
    setBoardTestCase(board, sizeOfBoard, sizeOfBoard);

    double startTime = gettime();
    int i = playGameOfLife(board, nextBoard, sizeOfBoard, generations, threads);
    double endTime = gettime();

    cout << "Stopped at " << i << " generations.\n";
    cout << "Time taken " << endTime - startTime << " seconds\n";
    writeBoard(board, sizeOfBoard, sizeOfBoard, outputFile, "Final Board");

    freearray(board);
    freearray(nextBoard);
    return 0;

}
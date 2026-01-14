/*
* Name: Eric Ryan Montgomery
* Email: ermontgomery1@crimson.ua.edu
* Course Section: CS 481
* Homework #: 0
* Instructions to compile the program: 
* Instructions to run the program: ./a 10000 5000
*/

#include <iostream>
#include <algorithm>
//#include <sys/time.h>
#include <chrono>
#include <stdio.h>
#include <stdlib.h>
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

//This functions advances the game of life by one generation.
//If called only once then:
//Its best, worst, and averages cases are O(sizeOfBoard * sizeOfBoard)
//Where sizeOfBoard is the original size given.
unsigned char advanceGeneration(unsigned char *board, unsigned char *nextBoard, int sizeOfBoard) {

    //Flag to return if the board changed or not
    unsigned char changed = 0;

    //Double for loop to loop through the board but avoid the padded edges
    for (int i = 1; i < sizeOfBoard-1; ++i) {

        //Ptr arithmetic for optimization
        unsigned char* rowAbove = board + (i-1) * sizeOfBoard; //Determines the row above the current index
        unsigned char* rowCurrent = board + i * sizeOfBoard; //Determines the current row
        unsigned char* rowBelow = board + (i+1) * sizeOfBoard; //Determines the row below the current index
        unsigned char* rowNext = nextBoard + i*sizeOfBoard; //Determines the row for the nextBoard

        //Second for loop that loops through one row
        for (int j = 1; j < sizeOfBoard-1; ++j) {

            //Determine the cells 'population score'
            unsigned char score = 
                rowAbove[j-1] + rowAbove[j] + rowAbove[j+1] +
                rowCurrent[j-1] + rowCurrent[j+1] +
                rowBelow[j-1] + rowBelow[j] + rowBelow[j+1];

            //Bitwise operations to determine whether the cell lives or dies.
            // If population score == 3 then whether alive or dead the cell is set to 1
            // OR if the current cell is alive and its pop score is 2 then cell is set to 1
            // Short circuits so it does it faster than the bitwise operation
            unsigned char nextValue = (score == 3) || (rowCurrent[j] && score == 2);

            //Instead of branching just XOR the nextValue with the value of the current cell
            //Then OR that with changed. This will have to run each time but should be faster
            //than branching if statement.
            changed |= nextValue ^ rowCurrent[j];

            //Finally, set the nextBoard equivalent cell to be either 0 or 1
            rowNext[j] = nextValue;
        }
    }
    return changed;

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

    if (generations < 0) {
        printf("Error: <generations> must be 0 or more!\n");
        return 3;
    }

    //Sets random function
    srand(time(NULL));

    /*
    //Table to determine if the cell lives or dies
    int situationTable[2][9] = {
        {0, 0, 0, 1, 0, 0, 0, 0, 0}, //dead
        {0, 0, 1, 1, 0, 0, 0, 0, 0}, //alive
    };
    */

    //Create, intialize, and set the pattern for the boards
    unsigned char *board = new unsigned char[(sizeOfBoard) * (sizeOfBoard)];
    unsigned char *nextBoard = new unsigned char[(sizeOfBoard) * (sizeOfBoard)];
    initBoard(board, sizeOfBoard);
    initBoard(nextBoard, sizeOfBoard);
    setBoardInfinite(board, sizeOfBoard);


    //printBoard(board, sizeOfBoard);
    //cout << "\n";
    int i;
    double startTime = gettime();
    for (i = 0; i < generations; ++i) {
        if (!advanceGeneration(board, nextBoard, sizeOfBoard)) break;
        swap(board, nextBoard);
    }
    double endTime = gettime();

    cout << "Stopped at " << i << " generations.\n";
    cout << "Time taken " << endTime - startTime << " seconds\n";
    //printBoard(board, sizeOfBoard);

    delete[] board;
    delete[] nextBoard;
    return 0;

}
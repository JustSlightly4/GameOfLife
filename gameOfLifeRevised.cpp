/*
*   Ryan Montgomery
*   01/08/26
*   Conways Game Of Life
*/

#include <iostream>
#include <algorithm>
#include <sys/time.h>
#include <stdio.h>
#include <stdlib.h>
using namespace std;

/* function to get wall clock time as double */
double gettime() {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

void initBoard(char *board, int sizeOfBoard) {
    int totalSize = (sizeOfBoard) * (sizeOfBoard);
    for (int i = 0; i < totalSize; ++i) {
        board[i] = 0;
    }
}

void setBoardRandom(char *board, int sizeOfBoard) {
    for (int i = 1; i < sizeOfBoard-1; ++i) {
        for (int j = 1; j < sizeOfBoard-1; ++j) {
            int index = j + ((sizeOfBoard) * i);
            board[index] = rand() % 2;
        }
    }

}

void setBoardTestCase(char *board, int sizeOfBoard) {
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

//This functions advances the game of life by one generation.
//If called only once then:
//Its best, worst, and averages cases are O(sizeOfBoard * sizeOfBoard)
//Where sizeOfBoard is the original size given.
char advanceGeneration(char *board, char *nextBoard, int sizeOfBoard) {

    //Flag to return if the board changed or not
    char changed = 0;

    //Double for loop to loop through the board but avoid the padded edges
    for (int i = 1; i < sizeOfBoard-1; ++i) {

        //Ptr arithmetic for optimization
        char* rowAbove = board + (i-1) * sizeOfBoard; //Determines the row above the current index
        char* rowCurrent = board + i * sizeOfBoard; //Determines the current row
        char* rowBelow = board + (i+1) * sizeOfBoard; //Determines the row below the current index
        char* rowNext = nextBoard + i*sizeOfBoard; //Determines the row for the nextBoard

        //Second for loop that loops through one row
        for (int j = 1; j < sizeOfBoard-1; ++j) {

            //Determine the cells 'population score'
            int score = 
                rowAbove[j-1] + rowAbove[j] + rowAbove[j+1] +
                rowCurrent[j-1] + rowCurrent[j+1] +
                rowBelow[j-1] + rowBelow[j] + rowBelow[j+1];

            //Bitwise operations to determine whether the cell lives or dies.
            // If population score == 3 then whether alive or dead the cell is set to 1
            // OR if the current cell is alive and its pop score is 2 then cell is set to 1
            char nextValue = (score == 3) || (rowCurrent[j] && score == 2);

            //Uses a lookup table so there is no computation
            //Is slower for some reason though
            //char nextValue = situationTable[rowCurrent[j]][score];

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

    if (generations < 1) {
        printf("Error: <generations> must be 1 or more!\n");
        return 3;
    }

    //Set random function
    srand(time(NULL));

    /*
    //Table to determine if the cell lives or dies
    int situationTable[2][9] = {
        {0, 0, 0, 1, 0, 0, 0, 0, 0},
        {0, 0, 1, 1, 0, 0, 0, 0, 0},
    };
    */

    char *board = new char[(sizeOfBoard) * (sizeOfBoard)];
    char *nextBoard = new char[(sizeOfBoard) * (sizeOfBoard)];
    initBoard(board, sizeOfBoard);
    initBoard(nextBoard, sizeOfBoard);
    setBoardInfinite(nextBoard, sizeOfBoard);


    //printBoard(nextBoard, sizeOfBoard);
    //cout << "\n";
    int i;
    double startTime = gettime();
    for (i = 0; i < generations; ++i) {
        swap(board, nextBoard);
        if (!advanceGeneration(board, nextBoard, sizeOfBoard, situationTable)) break;
    }
    double endTime = gettime();

    cout << "Stopped at " << i << " generations.\n";
    cout << "Time taken " << endTime - startTime << " seconds\n";
    //printBoard(nextBoard, sizeOfBoard);

    delete[] board;
    delete[] nextBoard;
    return 0;

}
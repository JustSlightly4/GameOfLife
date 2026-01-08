#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/* function to allocate a 2-D array */
char **createBoard(int P, int Q) {
  int i;
  char *p, **a;

  P += 2;
  Q += 2;
  
  p = (char *)malloc(P*Q*sizeof(char));
  a = (char **)malloc(P*sizeof(char*));

  if (p == NULL || a == NULL) 
    printf("Error allocating memory\n");

  /* for row major storage */
  for (i = 0; i < P; i++)
    a[i] = &p[i*Q];
  
  return a;
}

char *createChangeLog(int P, int Q) {
    P += 2;
    Q += 2;
    char *changeLog = (char *)malloc(P*Q*sizeof(char))
}

/* function to delete the 2-D array */
void freeBoard(char **a) {
  free(&a[0][0]);
  free(a);
}

/* function to initialize the array */
void initBoardStart(char **a, int mrows, int ncols, char value) {
    int i,j;

    for (i=0; i<mrows+2; i++) {
        for (j=0; j<ncols+2; j++) {
        a[i][j] = value;
        }
    }

    if (mrows > 2 && ncols > 2) {
        for (int k = 0; k < 8; ++k) {
            int i = k / 3;
            int j = k % 3;
            a[mrows/2 + i][ncols/2 + j] = 1;
        }
    }
}

/* function to print the array */
void printBoard(char **a, int mrows, int ncols) {
  for (int i=0; i<mrows+2; i++) {
    for (int j=0; j<ncols+2; j++)
      if (a[i][j] == 0) printf("0 ");
      else printf("1 ");
    printf("\n");
  }
}

int GetPopulationScore(char **board, int xIndex, int yIndex) {
    int score = 0;
    for (int i = -1; i<2; i++) {
        for (int j = -1; j<2; j++) {
            if (i == 0 && j == 0) continue;
            score += board[xIndex + i][yIndex + j];
        }
    }
    return score;
}

void advanceGeneration(char **a, char **b, int mrows, int ncols) {
    int popScore = 0;
    for (int i=1; i<mrows+1; i++) {
        for (int j=1; j<ncols+1; j++) {
            popScore = GetPopulationScore(a, i, j);
            if (a[i][j] && (popScore < 2 || popScore > 3)) b[i][j] = 0;
            if (!a[i][j] && (popScore == 3)) b[i][j] = 1;
        }
    }
}

int main(int argc, char **argv){

    //Check to make sure that their are three command line arguments
    if (argc != 3) {
        printf("Usage: ./a.out <sizeOfBoard> <generations>\n");
        return 1;
    }

    //Initialize Values
    int sizeOfBoard = atoi(argv[1]);
    int generations = atoi(argv[2]);

    if (sizeOfBoard < 3) {
        printf("Error: <sizeOfBoard> must be 3 or more!\n");
        return 2;
    }

    if (generations < 1) {
        printf("Error: <generations> must be 1 or more!\n");
        return 3;
    }

    //Create the Board
    char **a = createBoard(sizeOfBoard, sizeOfBoard);
    char **b = createBoard(sizeOfBoard, sizeOfBoard);
    initBoardStart(a, sizeOfBoard, sizeOfBoard, 0);
    initBoardStart(b, sizeOfBoard, sizeOfBoard, 0);
    printBoard(a, sizeOfBoard, sizeOfBoard);
    advanceGeneration(a, b, sizeOfBoard, sizeOfBoard);
    advanceGeneration(a, b, sizeOfBoard, sizeOfBoard);
    printf("\n");
    printBoard(b, sizeOfBoard, sizeOfBoard);


    freeBoard(a);
    freeBoard(b);
    return 0;
}
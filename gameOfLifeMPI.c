#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <sys/time.h>
#define VAR int
#define MPI_VAR MPI_INT
//Compile: mpicc -g -Wall -o mpi_gameOfLife.out gameOfLifeMPI.c
//Run: mpiexec -n 4 ./mpi_gameOfLife.out

/* chrono function to get wall clock time as double */
double gettime(void) {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

void printArray(int rows, int cols, VAR **array) {
    for (int j = 0; j < cols; ++j) printf("0 ");
    printf("\n");
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            printf("%d ", array[i][j]);
        }
        printf("\n");
    }
    for (int j = 0; j < cols; ++j) printf("0 ");
    printf("\n");
}

VAR** allocarray(int rows, int cols) {
    VAR *p = (VAR *)malloc(rows * cols * sizeof(VAR));
    VAR **a = (VAR **)malloc(rows * sizeof(VAR*));
    for (int i = 0; i < rows; i++)
        a[i] = &p[i * cols];
    return a;
}

void InitArray(int rows, int cols, VAR **array) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            array[i][j] = 0;
        }
    }
}

//Sets a board to the given test case in homework 0
void setBoardTestCase(VAR **a, int mrows, int ncols) {
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
void setBoardInfinite(VAR **a, int mrows, int ncols) {
    int xMiddle = mrows/2;
    int yMiddle = ncols/2;

    a[xMiddle][yMiddle] = 1;
    a[xMiddle-1][yMiddle] = 1;
    a[xMiddle+1][yMiddle] = 1;
    a[xMiddle][yMiddle-1] = 1;

}

void freearray(VAR **a) {
    free(a[0]);
    free(a);
}

void GetSendCounts(int rows, int cols, int processAmt, int *sendcounts, int *displs) {
    int rem = rows % processAmt;
    int sum = 0;
    for (int i = 0; i < processAmt; i++) {
        int rows_for_this_proc = (rows / processAmt) + (i < rem ? 1 : 0);
        sendcounts[i] = rows_for_this_proc * cols; 
        displs[i] = sum;
        sum += sendcounts[i];
    }
}

void ExchangeGhosts(VAR **local_board, int my_rows, int cols, int my_rank, int comm_sz) {
    int top_neighbor = my_rank - 1;
    int bottom_neighbor = my_rank + 1;

    // Send bottom core row (index my_rows) to neighbor below, receive into top ghost (index 0)
    MPI_Sendrecv(local_board[my_rows], cols, MPI_VAR, (bottom_neighbor < comm_sz) ? bottom_neighbor : MPI_PROC_NULL, 0,
                 local_board[0],       cols, MPI_VAR, (top_neighbor >= 0) ? top_neighbor : MPI_PROC_NULL, 0,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);

    // Send top core row (index 1) to neighbor above, receive into bottom ghost (index my_rows + 1)
    MPI_Sendrecv(local_board[1],           cols, MPI_VAR, (top_neighbor >= 0) ? top_neighbor : MPI_PROC_NULL, 1,
                 local_board[my_rows + 1], cols, MPI_VAR, (bottom_neighbor < comm_sz) ? bottom_neighbor : MPI_PROC_NULL, 1,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
}

void ScatterBoard(int cols, VAR **matrix, int processAmt, int my_rank, int out_my_core_rows,
                    int *sendCounts, int *displs, int my_core_element_count, int **local_board) {
    
    MPI_Scatterv(my_rank == 0 ? matrix[0] : NULL, sendCounts, displs, MPI_VAR, 
                 local_board[1], my_core_element_count, MPI_VAR, 
                 0, MPI_COMM_WORLD);
}

void CopyArray(int rows, int cols, VAR **array1, VAR **array2) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            array2[i][j] = array1[i][j];
        }
    }
}

void GatherIntoBoard(int cols, VAR **local_board, VAR **board, int my_core_rows, int my_rank, int *recvCounts, int *displs) {
    // board[0] is the start of the contiguous block on Rank 0
    MPI_Gatherv(local_board[1], my_core_rows * cols, MPI_VAR, 
                (my_rank == 0) ? board[0] : NULL, 
                recvCounts, displs, MPI_VAR, 
                0, MPI_COMM_WORLD);
}

void swap(VAR ***a, VAR ***b) {
    VAR **temp = *a;
    *a = *b;
    *b = temp;
}

int main() {

    //Initialize MPI
    int comm_sz, my_rank;
    MPI_Init(NULL, NULL);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    //Initialize the board and its size
    //All processes will have the board pointer
    //but to save memory only process zero will
    //own the whole board.
    VAR **board = NULL;
    int size = 1000;
    int cols = size + 2;
    if (my_rank == 0) {
        board = allocarray(size, cols);
        InitArray(size, cols, board);
        setBoardInfinite(board, size, cols);
        //printf("Original Board on Rank 0:\n");
        //printArray(size, cols, board);
        //printf("\n");
    }

    //Calculate how many rows each process will be getting
    int *sendCounts = malloc(comm_sz * sizeof(int));
    int *displs = malloc(comm_sz * sizeof(int));
    GetSendCounts(size, cols, comm_sz, sendCounts, displs);

    //Get the amount of elements each process will handle
    int my_core_element_count = sendCounts[my_rank];
    int out_my_core_rows = my_core_element_count / cols;

    int total_local_rows = (out_my_core_rows) + 2;
    VAR **local_board = allocarray(total_local_rows, cols);
    VAR **local_next_board = allocarray(total_local_rows, cols);

    // Initialize
    InitArray(total_local_rows, cols, local_board);
    InitArray(total_local_rows, cols, local_next_board);

    //We scatter the board among the processes
    ScatterBoard(cols, board, comm_sz, my_rank, out_my_core_rows,
                    sendCounts, displs, my_core_element_count, local_board);

    int gen;
    int local_changed = 0;
    double startTime = 0;
    if (my_rank == 0) startTime = gettime();
    for (gen = 0; gen < 1000; ++gen) {

        //Exchange neighbors
        ExchangeGhosts(local_board, out_my_core_rows, cols, my_rank, comm_sz);

        //All processes are performing this work on their piece of the board
        //locally
        local_changed = 0;
        for (int i = 1; i <= out_my_core_rows; ++i) { 
            for (int j = 1; j < cols-1; ++j) {
                VAR score = local_board[i-1][j-1] + local_board[i-1][j] + local_board[i-1][j+1] +
                    local_board[i][j-1] + local_board[i][j+1] +
                    local_board[i+1][j-1] + local_board[i+1][j] + local_board[i+1][j+1];
                VAR nextValue = (score == 3) | (local_board[i][j] & (score == 2));
                local_next_board[i][j] = nextValue;
                local_changed |= (nextValue ^ local_board[i][j]);
            }
        }

        swap(&local_next_board, &local_board);
        // Optional: Check if any process changed
        int global_changed;
        MPI_Allreduce(&local_changed, &global_changed, 1, MPI_INT, MPI_MAX, MPI_COMM_WORLD);
        if (!global_changed) break;
    }
    // Finally, gather all the boards pieces back into process zero
    GatherIntoBoard(cols, local_board, board, out_my_core_rows, my_rank, sendCounts, displs);

    if (my_rank == 0) {
        double endTime = gettime();
        printf("Game stopped at %d generations:\n", gen);
        printf("Time taken: %f\n", endTime - startTime);
        //printArray(size, cols, board);
        freearray(board);
    }

    freearray(local_board);
    freearray(local_next_board);
    free(sendCounts);
    free(displs);

    MPI_Finalize();
    return 0;
}
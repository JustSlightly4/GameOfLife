#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#define VAR char
#define MPI_VAR MPI_CHAR
//Compile: mpicc -g -Wall -o mpi_gameOfLife.out gameOfLifeMPI.c
//Run: mpiexec -n 4 ./mpi_gameOfLife.out

void print_array(VAR **array, int rows, int cols) {
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

VAR** alloc_array(int rows, int cols) {
    VAR *p = (VAR *)malloc(rows * cols * sizeof(VAR));
    VAR **a = (VAR **)malloc(rows * sizeof(VAR*));
    for (int i = 0; i < rows; i++)
        a[i] = &p[i * cols];
    return a;
}

void init_array(VAR **array, int rows, int cols) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            array[i][j] = 0;
        }
    }
}

//Sets a board to the given test case in homework 0
void set_board_test_case(VAR **a, int mrows, int ncols) {
    int x_middle = mrows/2;
    int y_middle = ncols/2;
    a[x_middle-1][y_middle-1] = 1;
    a[x_middle][y_middle-1] = 1;
    a[x_middle+1][y_middle-1] = 1;
    a[x_middle-1][y_middle] = 1;
    a[x_middle][y_middle] = 1;
    a[x_middle+1][y_middle] = 1;
    a[x_middle-1][y_middle+1] = 1;
    a[x_middle][y_middle+1] = 1;

}

//Sets the board to an infinite pattern
void set_board_infinite_case(VAR **a, int mrows, int ncols) {
    int x_middle = mrows/2;
    int y_middle = ncols/2;

    a[x_middle][y_middle] = 1;
    a[x_middle-1][y_middle] = 1;
    a[x_middle+1][y_middle] = 1;
    a[x_middle][y_middle-1] = 1;

}

void free_array(VAR **a) {
    free(a[0]);
    free(a);
}

void get_send_counts(int rows, int cols, int processAmt, int *send_counts, int *displs) {
    int rem = rows % processAmt;
    int sum = 0;
    for (int i = 0; i < processAmt; i++) {
        int rows_for_this_proc = (rows / processAmt) + (i < rem ? 1 : 0);
        send_counts[i] = rows_for_this_proc * cols; 
        displs[i] = sum;
        sum += send_counts[i];
    }
}

void exchange_ghost(VAR **local_board, int my_rows, int cols, int my_rank, int comm_sz) {
    int top_neighbor = my_rank - 1;
    int bottom_neighbor = my_rank + 1;
    MPI_Request requests[4];
    MPI_Isend(local_board[my_rows], cols, MPI_VAR, (bottom_neighbor < comm_sz) ? bottom_neighbor : MPI_PROC_NULL, 0, MPI_COMM_WORLD, &requests[0]);
    MPI_Irecv(local_board[0], cols, MPI_VAR, (top_neighbor >= 0) ? top_neighbor : MPI_PROC_NULL, 0, MPI_COMM_WORLD, &requests[1]);
    MPI_Isend(local_board[1], cols, MPI_VAR, (top_neighbor >= 0) ? top_neighbor : MPI_PROC_NULL, 1, MPI_COMM_WORLD, &requests[2]);
    MPI_Irecv(local_board[my_rows + 1], cols, MPI_VAR, (bottom_neighbor < comm_sz) ? bottom_neighbor : MPI_PROC_NULL, 1, MPI_COMM_WORLD, &requests[3]);
    MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
}

void exchange_ghost_top(VAR **local_board, int my_rows, int cols, int my_rank, int comm_sz) {
    int bottom_neighbor = my_rank + 1;
    MPI_Request requests[2];
    MPI_Isend(local_board[my_rows], cols, MPI_VAR, bottom_neighbor, 0, MPI_COMM_WORLD, &requests[0]);
    MPI_Irecv(local_board[my_rows + 1], cols, MPI_VAR, bottom_neighbor, 1, MPI_COMM_WORLD, &requests[1]);
    MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
}

void exchange_ghost_bottom(VAR **local_board, int my_rows, int cols, int my_rank, int comm_sz) {
    int top_neighbor = my_rank - 1;
    MPI_Request requests[2];
    MPI_Irecv(local_board[0], cols, MPI_VAR, top_neighbor, 0, MPI_COMM_WORLD, &requests[0]);
    MPI_Isend(local_board[1], cols, MPI_VAR, top_neighbor, 1, MPI_COMM_WORLD, &requests[1]);
    MPI_Waitall(2, requests, MPI_STATUSES_IGNORE);
}

void exchange_ghost_inner(VAR **local_board, int my_rows, int cols, int my_rank, int comm_sz) {
    int top_neighbor = my_rank - 1;
    int bottom_neighbor = my_rank + 1;
    MPI_Request requests[4];
    MPI_Isend(local_board[my_rows], cols, MPI_VAR, bottom_neighbor, 0, MPI_COMM_WORLD, &requests[0]);
    MPI_Irecv(local_board[0], cols, MPI_VAR, top_neighbor, 0, MPI_COMM_WORLD, &requests[1]);
    MPI_Isend(local_board[1], cols, MPI_VAR, top_neighbor, 1, MPI_COMM_WORLD, &requests[2]);
    MPI_Irecv(local_board[my_rows + 1], cols, MPI_VAR, bottom_neighbor, 1, MPI_COMM_WORLD, &requests[3]);
    MPI_Waitall(4, requests, MPI_STATUSES_IGNORE);
}

void copy_array(int rows, int cols, VAR **array1, VAR **array2) {
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < cols; ++j) {
            array2[i][j] = array1[i][j];
        }
    }
}

static inline void swap_pointers(VAR ***a, VAR ***b) {
    VAR **temp = *a;
    *a = *b;
    *b = temp;
}

//Writes the board to a file
void write_board(const char *file_name, VAR **board, int rows, int cols) {
    FILE *file;
    file = fopen(file_name, "w");
    if (file == NULL) {
        printf("Error writing board to %s!", file_name);
        return;
    }
    for (int i = 0; i < rows; ++i) {
        for (int j = 1; j < cols - 1; ++j) {
            fprintf(file, "%d ", board[i][j]);
        }
        fprintf(file, "\n");
    }
    fclose(file);
}

int main(int argc, char *argv[]) {

    //Initialize MPI
    int comm_sz, my_rank;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    if (argc != 4) {
        if (my_rank == 0) printf("Usage: %s <sizeOfBoard> <total_generations> <outputFile>\n", argv[0]);
        MPI_Finalize();
        return 0;
    }

    //Assign the exchange ghost function
    void (*exchange_ghost_ptr)(VAR**, int, int, int, int);
    if (comm_sz == 1) { //If there is only one process
        exchange_ghost_ptr = exchange_ghost;
    } else if (my_rank == 0) { //If processes > 1 and my_rank == 0
        exchange_ghost_ptr = exchange_ghost_top;
    } else if (my_rank == comm_sz - 1) { //If processes > 1 and my_rank == last process
        exchange_ghost_ptr = exchange_ghost_bottom;
    } else { //If processes > 1 and my_rank != (0 or last process)
        exchange_ghost_ptr = exchange_ghost_inner;
    }

    //Initialize the board and its size
    //All processes will have the board pointer
    //but to save memory only process zero will
    //own the whole board.
    VAR **board = NULL;
    int size = atoi(argv[1]);
    int cols = size + 2;
    double start_time, end_time, local_elapsed_time, global_elapsed_time;
    if (my_rank == 0) {
        board = alloc_array(size, cols);
        init_array(board, size, cols);
        set_board_infinite_case(board, size, cols);
        if (size <= 10) {
            printf("Original Board on Rank 0:\n");
            print_array(board, size, cols);
            printf("\n");
        }
    }

    //Calculate how many rows each process will be getting
    int *send_counts = malloc(comm_sz * sizeof(int));
    int *displs = malloc(comm_sz * sizeof(int));
    get_send_counts(size, cols, comm_sz, send_counts, displs);

    //Get the amount of elements each process will handle
    int my_core_element_count = send_counts[my_rank];
    int out_my_core_rows = my_core_element_count / cols;

    int total_local_rows = (out_my_core_rows) + 2;
    VAR **local_board = alloc_array(total_local_rows, cols);
    VAR **local_next_board = alloc_array(total_local_rows, cols);

    // Initialize
    init_array(local_board, total_local_rows, cols);
    init_array(local_next_board, total_local_rows, cols);

    //We scatter the board among the processes
    MPI_Scatterv(my_rank == 0 ? board[0] : NULL, send_counts, displs, MPI_VAR, 
        local_board[1], my_core_element_count, MPI_VAR, 0, MPI_COMM_WORLD);

    int curr_generation;
    int total_generations = atoi(argv[2]);
    VAR local_changed = 0;
    VAR global_changed = 1;
    start_time = MPI_Wtime();
    for (curr_generation = 0; global_changed && (curr_generation < total_generations); ++curr_generation) {

        //Exchange neighbors
        exchange_ghost_ptr(local_board, out_my_core_rows, cols, my_rank, comm_sz);

        //All processes are performing this work on their piece of the board
        //locally
        local_changed = 0;
        for (int i = 1; i <= out_my_core_rows; ++i) {
            VAR* local_board_above_row = local_board[i-1];
            VAR* local_board_current_row = local_board[i];
            VAR* local_board_below_row = local_board[i+1];
            for (int j = 1; j < cols-1; ++j) {

                VAR score = local_board_above_row[j-1] + local_board_above_row[j] + local_board_above_row[j+1] +
                    local_board_current_row[j-1] + local_board_current_row[j+1] +
                    local_board_below_row[j-1] + local_board_below_row[j] + local_board_below_row[j+1];

                VAR next_value = (score == 3) | (local_board_current_row[j] & (score == 2));

                local_next_board[i][j] = next_value;

                local_changed |= (next_value ^ local_board_current_row[j]);
            }
        }

        swap_pointers(&local_next_board, &local_board);
        MPI_Allreduce(&local_changed, &global_changed, 1, MPI_VAR, MPI_MAX, MPI_COMM_WORLD);
    }
    // Finally, gather all the boards pieces back into process zero
    MPI_Gatherv(local_board[1], out_my_core_rows * cols, MPI_VAR, (my_rank == 0) ? board[0] : NULL, 
                send_counts, displs, MPI_VAR, 0, MPI_COMM_WORLD);

    end_time = MPI_Wtime();
    local_elapsed_time = end_time - start_time;
    MPI_Reduce(&local_elapsed_time, &global_elapsed_time, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);

    if (my_rank == 0) {
        write_board(argv[3], board, size, cols);
        printf("Game stopped at %d total_generations:\n", curr_generation);
        printf("Time taken: %f\n", global_elapsed_time);
        if (size <= 10) print_array(board, size, cols);
        free_array(board);
    }

    free_array(local_board);
    free_array(local_next_board);
    free(send_counts);
    free(displs);

    MPI_Finalize();
    return 0;
}
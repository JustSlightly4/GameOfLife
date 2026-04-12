#include <stdio.h>
#include <cuda.h>
#include <sys/time.h>

__global__ void game_of_life(char* board, char* next_board, int* element_counts, int size, int generations) {

    int my_id = threadIdx.x;
    int element_count = element_counts[my_id];

    // Compute prefix sum (offset)
    int offset = my_id * element_count;

    for (int i = 0; i < generations; ++i) {
        for (int i = 0; i < element_count; ++i) {

            int global_index = offset + i;

            // Convert to 2D (non-ghost coordinates)
            int row = global_index / (size-2);
            int col = global_index % (size-2);

            // Shift for ghost cells
            int ghost_row = row + 1;
            int ghost_col = col + 1;

            int idx = ghost_row * size + ghost_col;

            int row_above = idx - size;
            int row_below = idx + size;
            char score = 
                board[row_above-1] + board[row_above] + board[row_above+1] +
                board[idx-1] + board[idx+1] +
                board[row_below-1] + board[row_below] + board[row_below+1];

            char nextValue = (score == 3) || (board[idx] && score == 2);

            next_board[idx] = nextValue;
        }

        __syncthreads();

        if (my_id == 0) {
            char* temp = board;
            board = next_board;
            next_board = temp;
        }
    }
}

/* function to get wall clock time */
double get_time() {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

void init_board(char* board, int size) {
    int total_size_with_ghost = size * size;
    for (int i = 0; i < total_size_with_ghost; ++i) {
        board[i] = 0;
    }
}

void print_board(char* board, int size) {
    int row;
    for (int i = 0; i < size; ++i) {
        row = i * size;
        for (int j = 0; j < size; ++j) {
            printf("%d ", int(board[row + j]));
        }
        printf("\n");
    }
}

//Sets a board to the given test case in homework 0
void set_board_test_case(char *board, int size) {
    int center = (size / 2) * size + (size / 2);

    board[center - 1 - size] = 1;
    board[center - size] = 1;
    board[center + 1 - size] = 1;

    board[center - 1] = 1;
    board[center] = 1;
    board[center + 1] = 1;

    board[center - 1 + size] = 1;
    board[center + size] = 1;

}

int main(int argc, char* argv[]) {

    //Check inputs
    if (argc != 4) {
        printf("Usage: <executable> <size> <generations> <output_file>\n");
        return 1;
    }

    //Define variables
    int size = atoi(argv[1]) + 2;
    int total_inside_size = atoi(argv[1]) * atoi(argv[1]);
    int generations = atoi(argv[2]);
    char* file_name = argv[3];

    //Create the boards
    char* board;
    char* board_gpu;
    char* next_board_gpu;
    board = (char *)malloc(size * size * sizeof(char));
    cudaMalloc(&board_gpu, size * size * sizeof(char));
    cudaMalloc(&next_board_gpu, size * size * sizeof(char));

    //Initalize the boards
    init_board(board, size);
    set_board_test_case(board, size);
    cudaMemcpy(board_gpu, board, size * size * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(next_board_gpu, board, size * size * sizeof(char), cudaMemcpyHostToDevice);

    //Define the amount of threads and the thread counts array
    int threads = 1024;
    int* thread_counts = (int *)malloc(threads * sizeof(int));
    int* thread_counts_gpu;
    cudaMalloc(&thread_counts_gpu, threads * sizeof(int));

    //Initalize thread_counts
    for (int i = 0; i < threads; ++i) {
        thread_counts[i] = 0;
    }

    //Compute the amount of elements a single thread will work on
    int index;
    for (int i = 0; i < total_inside_size; ++i) {
        index = i % threads;
        thread_counts[index] += 1;
    }

    //Move the thread_counts to the GPU
    cudaMemcpy(thread_counts_gpu, thread_counts, threads * sizeof(int), cudaMemcpyHostToDevice);

    //Get current time
    double start_time = get_time();

    game_of_life<<<1, threads>>>(board_gpu, next_board_gpu, thread_counts_gpu, size, generations);
    cudaDeviceSynchronize();

    //Move the final board back to the host/CPU
    cudaMemcpy(board, next_board_gpu, size * size * sizeof(char), cudaMemcpyDeviceToHost);

    //Get the ending time and print out the total time taken
    double end_time = get_time();
    printf("Time taken: %f\n", (end_time - start_time));

    //Print out the board
    //print_board(board, size);

    //Free memory
    free(board);
    cudaFree(board_gpu);

    //End program
    return 0;
}
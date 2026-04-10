#include <stdio.h>
#include <cuda.h>

//Board needs to have ghost cells
//This function expects the size to be the original size and not the size + 2.
//In this version there is only going to be one block with 1024 threads so the logic is easier
__global__ void game_of_life(char* board, char* next_board, int* element_counts, int size) {

    int my_id = threadIdx.x;
    int element_count = element_counts[my_id];

    // Compute prefix sum (offset)
    int offset = my_id * element_count;

    // Loop over elements assigned to this thread
    for (int i = 0; i < element_count; ++i) {

        int global_index = offset + i;

        // Convert to 2D (non-ghost coordinates)
        int row = global_index / size;
        int col = global_index % size;

        // Shift for ghost cells
        int ghost_row = row + 1;
        int ghost_col = col + 1;

        int idx = ghost_row * (size + 2) + ghost_col;

        int row_above = idx - (size + 2);
        int row_below = idx + (size + 2);
        char score = 
            board[row_above-1] + board[row_above] + board[row_above+1] +
            board[idx-1] + board[idx+1] +
            board[row_below-1] + board[row_below] + board[row_below+1];

        char nextValue = (score == 3) || (board[idx] && score == 2);

        // Example update (replace with real Game of Life logic later)
        next_board[idx] = nextValue;
    }
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
    int size = atoi(argv[1]);
    int total_size = size * size;
    int total_size_with_ghost = (size + 2) * (size + 2);
    int generations = atoi(argv[2]);
    char* file_name = argv[3];

    //Create the boards
    char* board;
    char* board_gpu;
    char* next_board_gpu;
    board = (char *)malloc(total_size_with_ghost * sizeof(char));
    cudaMalloc(&board_gpu, total_size_with_ghost * sizeof(char));
    cudaMalloc(&next_board_gpu, total_size_with_ghost * sizeof(char));

    //Initalize the boards
    init_board(board, size + 2);
    set_board_test_case(board, size + 2);
    cudaMemcpy(board_gpu, board, total_size_with_ghost * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(next_board_gpu, board, total_size_with_ghost * sizeof(char), cudaMemcpyHostToDevice);

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
    for (int i = 0; i < total_size; ++i) {
        index = i % threads;
        thread_counts[index] += 1;
    }

    //Move the thread_counts to the GPU
    cudaMemcpy(thread_counts_gpu, thread_counts, threads * sizeof(int), cudaMemcpyHostToDevice);

    //Game of life function
    for (int i = 0; i < generations; ++i) {
        
        //One generation of the game
        game_of_life<<<1, threads>>>(board_gpu, next_board_gpu, thread_counts_gpu, size);
        cudaDeviceSynchronize();
        
        // swap
        char* temp = board_gpu;
        board_gpu = next_board_gpu;
        next_board_gpu = temp;

    }

    //Move the final board back to the host/CPU
    cudaMemcpy(board, next_board_gpu, total_size_with_ghost * sizeof(char), cudaMemcpyDeviceToHost);

    //Print out the board
    print_board(board, size + 2);

    //Free memory
    free(board);
    cudaFree(board_gpu);

    //End program
    return 0;
}
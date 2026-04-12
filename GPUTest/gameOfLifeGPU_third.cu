#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include <sys/time.h>

#define BLOCK_SIZE 16
#define TILE_SIZE 18 // BLOCK_SIZE + 2

__global__ void game_of_life(char* board, char* next_board, int size) {
    __shared__ char tile[TILE_SIZE * TILE_SIZE];

    // 1. Cooperative Load
    // We need to load 324 elements using 256 threads.
    int tid = threadIdx.y * blockDim.x + threadIdx.x;
    int tile_elements = TILE_SIZE * TILE_SIZE;
    
    // Starting point for this block in the global grid (top-left of the halo)
    int block_top_left_col = blockIdx.x * blockDim.x; 
    int block_top_left_row = blockIdx.y * blockDim.y;

    for (int i = tid; i < tile_elements; i += (BLOCK_SIZE * BLOCK_SIZE)) {
        int local_row = i / TILE_SIZE;
        int local_col = i % TILE_SIZE;
        int global_row = block_top_left_row + local_row;
        int global_col = block_top_left_col + local_col;

        if (global_row < size && global_col < size) {
            tile[i] = board[global_row * size + global_col];
        } else {
            tile[i] = 0;
        }
    }
    __syncthreads();

    // 2. Compute
    // Only threads mapped to the inner 16x16 of the tile perform work
    int col = blockIdx.x * blockDim.x + threadIdx.x + 1;
    int row = blockIdx.y * blockDim.y + threadIdx.y + 1;

    if (row < size - 1 && col < size - 1) {
        int tx = threadIdx.x + 1;
        int ty = threadIdx.y + 1;
        
        // Sum neighbors from shared memory
        int score = tile[(ty-1)*TILE_SIZE + (tx-1)] + tile[(ty-1)*TILE_SIZE + tx] + tile[(ty-1)*TILE_SIZE + (tx+1)] +
                    tile[ty*TILE_SIZE + (tx-1)]                                  + tile[ty*TILE_SIZE + (tx+1)]     +
                    tile[(ty+1)*TILE_SIZE + (tx-1)] + tile[(ty+1)*TILE_SIZE + tx] + tile[(ty+1)*TILE_SIZE + (tx+1)];

        next_board[row * size + col] = (score == 3) || (tile[ty*TILE_SIZE + tx] && score == 2);
    }
}

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
    int inner_size = atoi(argv[1]);
    int total_inner_size = inner_size * inner_size;
    int size = inner_size + 2;
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

    dim3 blockDim(16, 16);  // 256 threads per block (good default)

    dim3 gridDim(
        (inner_size + blockDim.x - 1) / blockDim.x,
        (inner_size + blockDim.y - 1) / blockDim.y
    );

    //Get current time
    double start_time = get_time();

    //Game of life function
    for (int i = 0; i < generations; ++i) {
        
        //One generation of the game
        game_of_life<<<gridDim, blockDim>>>(board_gpu, next_board_gpu, size);
        cudaDeviceSynchronize();
        
        // swap
        char* temp = board_gpu;
        board_gpu = next_board_gpu;
        next_board_gpu = temp;

    }

    //Move the final board back to the host/CPU
    cudaMemcpy(board, board_gpu, size * size * sizeof(char), cudaMemcpyDeviceToHost);

    //Get the ending time and print out the total time taken
    double end_time = get_time();
    printf("Time taken: %f\n", (end_time - start_time));

    //Print out the board
    //print_board(board, size);

    //Free memory
    free(board);
    cudaFree(board_gpu);
    cudaFree(next_board_gpu);

    //End program
    return 0;
}
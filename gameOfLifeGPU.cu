/* Fastest version so far */
#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include <sys/time.h>

__global__ void game_of_life(unsigned char* board, unsigned char* next_board, int size) {
    int inner_size = size - 2;

    int col = blockIdx.x * blockDim.x + threadIdx.x + 1;
    int row = blockIdx.y * blockDim.y + threadIdx.y + 1;

    if (row > inner_size || col > inner_size) return;

    int pos = row * size + col;

    int score =
        board[pos - size - 1] + board[pos - size] + board[pos - size + 1] +
        board[pos - 1] + board[pos + 1] +
        board[pos + size - 1] + board[pos + size] + board[pos + size + 1];

    next_board[pos] = (score == 3) || (board[pos] && score == 2);
}

double get_time() {
  struct timeval tval;

  gettimeofday(&tval, NULL);

  return( (double)tval.tv_sec + (double)tval.tv_usec/1000000.0 );
}

void init_board(unsigned char* board, int size) {
    int total_size_with_ghost = size * size;
    for (int i = 0; i < total_size_with_ghost; ++i) {
        board[i] = 0;
    }
}

void print_board(unsigned char* board, int size) {
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
void set_board_test_case(unsigned char *board, int size) {
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

void set_board_random(unsigned char* board, int size) {
    srand(1);  // seed once per program run

    for (int i = 1; i < size - 1; ++i) {
        for (int j = 1; j < size - 1; ++j) {
            int index = i * size + j;
            board[index] = rand() % 2;  // 0 or 1
        }
    }
}

void write_board_to_file(char* file_name, unsigned char* board, int size) {
    FILE* fptr = fopen(file_name, "w");
    if (fptr == NULL) return;
    for (int i = 1; i < size - 1; ++i) {
        for (int j = 1; j < size - 1; ++j) {
            int index = i * size + j;
            fprintf(fptr, "%d ", board[index]);
        }
        fprintf(fptr, "\n");
    }
    fclose(fptr);
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
    unsigned char* board;
    unsigned char* board_gpu;
    unsigned char* next_board_gpu;
    board = (unsigned char *)malloc(size * size * sizeof(unsigned char));
    cudaMalloc(&board_gpu, size * size * sizeof(unsigned char));
    cudaMalloc(&next_board_gpu, size * size * sizeof(unsigned char));

    //Initalize the boards
    init_board(board, size);
    set_board_random(board, size);
    cudaMemcpy(board_gpu, board, size * size * sizeof(unsigned char), cudaMemcpyHostToDevice);
    cudaMemcpy(next_board_gpu, board, size * size * sizeof(unsigned char), cudaMemcpyHostToDevice);

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
        unsigned char* temp = board_gpu;
        board_gpu = next_board_gpu;
        next_board_gpu = temp;

    }

    //Move the final board back to the host/CPU
    cudaMemcpy(board, board_gpu, size * size * sizeof(unsigned char), cudaMemcpyDeviceToHost);

    //Get the ending time and print out the total time taken
    double end_time = get_time();
    printf("Time taken: %f\n", (end_time - start_time));

    write_board_to_file(file_name, board, size);

    //Free memory
    free(board);
    cudaFree(board_gpu);
    cudaFree(next_board_gpu);

    //End program
    return 0;
}
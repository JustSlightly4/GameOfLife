#include <stdio.h>
#include <cuda.h>
//1024 threads to use in a single block
//This version just spawns one thread for each row

__global__ void board_add_one(int* board, int size) {
    int myId = threadIdx.x;
    int row = myId * size;
    for (int i = 0; i < size; ++i) {
        board[row + i] += 1;
    }
}

void print_board(int* board, int size) {
    int row;
    for (int i = 0; i < size; ++i) {
        row = i * size;
        for (int j = 0; j < size; ++j) {
            printf("%d ", board[row + j]);
        }
        printf("\n");
    }
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: <executable> <threads>\n");
        return 1;
    }
    
    //Define the size
    //The total size of the board will be size * size
    //The size is also the number of threads that will be used
    int size = atoi(argv[1]);
    if (size < 1 || size > 1024) {
        printf("Incorrect amount of threads produced! Valid range is 1 to 1024.\n");
        return 1;
    }

    //Initialize the board and fill it will zeros
    int* board = (int *)malloc(size * size * sizeof(int));
    for (int i = 0; i < size * size; ++i) {
        board[i] = 0;
    }

    print_board(board, size);

    //Initialize the gpu board and copy the CPU memory over
    int* board_gpu;
    cudaMalloc(&board_gpu, size * size * sizeof(int));
    cudaMemcpy(board_gpu, board, size * size * sizeof(int), cudaMemcpyHostToDevice);

    board_add_one<<<1, size>>>(board_gpu, size);

    //Copy memory back
    cudaMemcpy(board, board_gpu, size * size * sizeof(int), cudaMemcpyDeviceToHost);

    printf("\n");
    print_board(board, size);

    cudaFree(board_gpu);
    free(board);
    return 0;
}
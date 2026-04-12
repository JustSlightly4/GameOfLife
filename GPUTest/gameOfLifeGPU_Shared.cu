#include <stdio.h>
#include <cuda.h>
#include <sys/time.h>

#define BLOCK_SIZE 16

__global__ void game_of_life(char* board, char* next_board, int size) {
    int inner_size = size - 2;

    // 2D thread indices
    int tx = threadIdx.x;
    int ty = threadIdx.y;

    int col = blockIdx.x * blockDim.x + tx;
    int row = blockIdx.y * blockDim.y + ty;

    // Shared memory tile (+2 for halo)
    __shared__ char tile[BLOCK_SIZE + 2][BLOCK_SIZE + 2];

    // Load main cell
    if (row < inner_size && col < inner_size) {
        tile[ty + 1][tx + 1] = board[(row + 1) * size + (col + 1)];
    }

    // Halo loads
    if (tx == 0 && col > 0)
        tile[ty + 1][0] = board[(row + 1) * size + col];

    if (tx == BLOCK_SIZE - 1 && col < inner_size - 1)
        tile[ty + 1][tx + 2] = board[(row + 1) * size + (col + 2)];

    if (ty == 0 && row > 0)
        tile[0][tx + 1] = board[row * size + (col + 1)];

    if (ty == BLOCK_SIZE - 1 && row < inner_size - 1)
        tile[ty + 2][tx + 1] = board[(row + 2) * size + (col + 1)];

    // Corners
    if (tx == 0 && ty == 0 && row > 0 && col > 0)
        tile[0][0] = board[row * size + col];

    if (tx == BLOCK_SIZE - 1 && ty == 0 && row > 0 && col < inner_size - 1)
        tile[0][tx + 2] = board[row * size + (col + 2)];

    if (tx == 0 && ty == BLOCK_SIZE - 1 && row < inner_size - 1 && col > 0)
        tile[ty + 2][0] = board[(row + 2) * size + col];

    if (tx == BLOCK_SIZE - 1 && ty == BLOCK_SIZE - 1 && row < inner_size - 1 && col < inner_size - 1)
        tile[ty + 2][tx + 2] = board[(row + 2) * size + (col + 2)];

    __syncthreads();

    // Compute only valid cells
    if (row < inner_size && col < inner_size) {
        char score =
            tile[ty][tx] + tile[ty][tx+1] + tile[ty][tx+2] +
            tile[ty+1][tx] + tile[ty+1][tx+2] +
            tile[ty+2][tx] + tile[ty+2][tx+1] + tile[ty+2][tx+2];

        char current = tile[ty+1][tx+1];
        char nextValue = (score == 3) || (current && score == 2);

        next_board[(row + 1) * size + (col + 1)] = nextValue;
    }
}

double get_time() {
    struct timeval tval;
    gettimeofday(&tval, NULL);
    return (double)tval.tv_sec + (double)tval.tv_usec / 1000000.0;
}

void init_board(char* board, int size) {
    for (int i = 0; i < size * size; ++i) {
        board[i] = 0;
    }
}

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

    if (argc != 4) {
        printf("Usage: <executable> <size> <generations> <output_file>\n");
        return 1;
    }

    int inner_size = atoi(argv[1]);
    int size = inner_size + 2;
    int total_size = size * size;
    int generations = atoi(argv[2]);

    char* board = (char*)malloc(total_size);
    char *board_gpu, *next_board_gpu;

    cudaMalloc(&board_gpu, total_size);
    cudaMalloc(&next_board_gpu, total_size);

    init_board(board, size);
    set_board_test_case(board, size);

    cudaMemcpy(board_gpu, board, total_size, cudaMemcpyHostToDevice);
    cudaMemcpy(next_board_gpu, board, total_size, cudaMemcpyHostToDevice);

    dim3 threads(BLOCK_SIZE, BLOCK_SIZE);
    dim3 blocks(
        (inner_size + BLOCK_SIZE - 1) / BLOCK_SIZE,
        (inner_size + BLOCK_SIZE - 1) / BLOCK_SIZE
    );

    double start = get_time();

    for (int i = 0; i < generations; ++i) {
        game_of_life<<<blocks, threads>>>(board_gpu, next_board_gpu, size);
        cudaDeviceSynchronize();

        char* temp = board_gpu;
        board_gpu = next_board_gpu;
        next_board_gpu = temp;
    }

    cudaMemcpy(board, board_gpu, total_size, cudaMemcpyDeviceToHost);

    double end = get_time();
    printf("Time taken: %f\n", end - start);

    free(board);
    cudaFree(board_gpu);
    cudaFree(next_board_gpu);

    return 0;
}
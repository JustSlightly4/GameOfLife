#include <stdio.h>
#include <stdlib.h>
#include <cuda.h>
#include <sys/time.h>

__global__ void game_of_life(char* board, char* next_board, int* element_counts, int size, int generations) {

    int my_id = threadIdx.x;

    int element_count = element_counts[my_id];

    int offset = 0;
    for (int i = 0; i < my_id; ++i) {
        offset += element_counts[i];
    }

    int inner_size = size - 2;

    for (int g = 0; g < generations; ++g) {

        // FIX: choose active/inactive buffer instead of swapping pointers
        char* current = (g % 2 == 0) ? board : next_board;
        char* next    = (g % 2 == 0) ? next_board : board;

        for (int i = 0; i < element_count; ++i) {

            int current_index = offset + i;

            int row = current_index / inner_size;
            int col = current_index % inner_size;

            int ghost_row = row + 1;
            int ghost_col = col + 1;

            int idx = ghost_row * size + ghost_col;

            int row_above = idx - size;
            int row_below = idx + size;

            int score =
                current[row_above - 1] + current[row_above] + current[row_above + 1] +
                current[idx - 1] + current[idx + 1] +
                current[row_below - 1] + current[row_below] + current[row_below + 1];

            char alive = current[idx];

            char nextValue = (score == 3) || (alive && score == 2);

            next[idx] = nextValue;
        }

        // no __syncthreads needed (threads are independent here)
    }
}

/* timing */
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
    int generations = atoi(argv[2]);

    char* board = (char*)malloc(size * size * sizeof(char));

    char *board_gpu, *next_board_gpu;
    cudaMalloc(&board_gpu, size * size * sizeof(char));
    cudaMalloc(&next_board_gpu, size * size * sizeof(char));

    init_board(board, size);
    set_board_test_case(board, size);

    cudaMemcpy(board_gpu, board, size * size * sizeof(char), cudaMemcpyHostToDevice);
    cudaMemcpy(next_board_gpu, board, size * size * sizeof(char), cudaMemcpyHostToDevice);

    int threads = 1024;
    int* thread_counts = (int*)malloc(threads * sizeof(int));
    int* thread_counts_gpu;

    cudaMalloc(&thread_counts_gpu, threads * sizeof(int));

    for (int i = 0; i < threads; ++i) {
        thread_counts[i] = 0;
    }

    int total_inside_size = inner_size * inner_size;

    for (int i = 0; i < total_inside_size; ++i) {
        thread_counts[i % threads]++;
    }

    cudaMemcpy(thread_counts_gpu, thread_counts, threads * sizeof(int), cudaMemcpyHostToDevice);

    double start_time = get_time();

    // ✅ SINGLE KERNEL LAUNCH ONLY
    game_of_life<<<1, threads>>>(board_gpu, next_board_gpu, thread_counts_gpu, size, generations);
    cudaDeviceSynchronize();

    cudaMemcpy(board, board_gpu, size * size * sizeof(char), cudaMemcpyDeviceToHost);

    double end_time = get_time();
    printf("Time taken: %f\n", end_time - start_time);

    free(board);
    free(thread_counts);

    cudaFree(board_gpu);
    cudaFree(next_board_gpu);
    cudaFree(thread_counts_gpu);

    return 0;
}
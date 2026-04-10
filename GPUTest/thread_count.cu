#include <stdio.h>
#include <cuda.h>

__global__ void thread_count() {
    int total_threads = blockDim.x * blockDim.y * blockDim.z * gridDim.x * gridDim.y * gridDim.z;
    int threads_per_block = blockDim.x * blockDim.y * blockDim.z;

    //1D
    int global_id = blockIdx.x * blockDim.x + threadIdx.x;

    //2D
    int row = blockIdx.y * blockDim.y + threadIdx.y;
    int col = blockIdx.x * blockDim.x + threadIdx.x;

    //Printing Values
    printf("total_threads: %d, threads_per_block: %d, global_id: %d, row: %d, col: %d\n", total_threads, threads_per_block, global_id, row, col);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: <executable> <blocks> <threads>");
        return 1;
    }
    int blocks = atoi(argv[1]);
    int threads = atoi(argv[2]);
    thread_count<<<blocks, threads/blocks>>>();
    cudaDeviceSynchronize();
    return 0;
}
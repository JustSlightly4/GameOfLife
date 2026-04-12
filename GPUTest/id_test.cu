#include <stdio.h>
#include <cuda.h>

__global__ void my_id() {
    int my_id = threadIdx.x;
    int my_block = blockIdx.x;
    int global_id = blockIdx.x * blockDim.x + threadIdx.x;
    printf("Hello, my id is %d, my block is %d, and my global id is %d.\n", my_id, my_block, global_id);
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: <executable> <blocks> <threads>\n");
        return 1;
    }
    int blocks = atoi(argv[1]);
    int threads = atoi(argv[2]);
    my_id<<<blocks, threads>>>();
    cudaDeviceSynchronize();
    return 0;
}
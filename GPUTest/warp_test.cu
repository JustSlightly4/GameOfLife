#include <stdio.h>
#include <cuda.h>

__global__ void func() {
    int global_id = blockIdx.x * blockDim.x + threadIdx.x;
    int lane = threadIdx.x % warpSize;
    int active_threads = __popc(__activemask());
    printf("My global id is %d and my lane is %d. Active threads in my warp: %d\n", global_id, lane, active_threads);
}

int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: <executable> <threads>\n");
        return 1;
    }

    int threads = atoi(argv[1]);
    func<<<1, threads>>>();
    cudaDeviceSynchronize();

    return 0;
}
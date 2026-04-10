#include <stdio.h>
#include <cuda.h>

//Function that will add the two vectors on the device/GPU
__global__ void add_vectors(int* vector_1, int* vector_2, int* output_vector) {
    int myId = threadIdx.x;
    output_vector[myId] = vector_1[myId] + vector_2[myId];
}

//Function that will print a vector on the host/CPU
void print_vector(int* vector, int size) {
    printf("Vector:\n");
    for (int i = 0; i < size; ++i) {
        printf("%d ", vector[i]);
    }
    printf("\n");
}

//Main will run on the host/CPU
int main(int argc, char* argv[]) {
    if (argc != 2) {
        printf("Usage: <executable> <threads>\n");
        return 1;
    }
    const int thread_count = atoi(argv[1]);
    const int size = thread_count;

    //Create vectors 1, 2, and the output vector on the host/CPU
    int* vector_1 = (int*)malloc(size * sizeof(int));
    int* vector_2 = (int*)malloc(size * sizeof(int));
    int* output_vector = (int*)malloc(size * sizeof(int));

    //Fill vectors 1 and 2 on the host/CPU
    for (int i = 0; i < size; ++i) {
        vector_1[i] = 1;
        vector_2[i] = 1;
    }

    //Create vectors 1, 2, and the output vector on the device/GPU
    int* vector_1_gpu;
    int* vector_2_gpu;
    int* output_vector_gpu;
    cudaMalloc(&vector_1_gpu, size * sizeof(int));
    cudaMalloc(&vector_2_gpu, size * sizeof(int));
    cudaMalloc(&output_vector_gpu, size * sizeof(int));

    //Copy CPU vectors to the GPU
    //Only need to copy vectors 1 and 2
    //The output vector does not not need to be copied since it will be filled with
    //the result from adding the first two vectors.
    cudaMemcpy(vector_1_gpu, vector_1, size * sizeof(int), cudaMemcpyHostToDevice);
    cudaMemcpy(vector_2_gpu, vector_2, size * sizeof(int), cudaMemcpyHostToDevice);

    //Perform function
    add_vectors<<<1, thread_count>>>(vector_1_gpu, vector_2_gpu, output_vector_gpu);

    //Copy memory back to output_vector
    //This function will wait for the GPU to finish
    cudaMemcpy(output_vector, output_vector_gpu, size * sizeof(int), cudaMemcpyDeviceToHost);

    //Print Output vector to verify results
    print_vector(output_vector, size);

    //Free memory
    cudaFree(vector_1_gpu);
    cudaFree(vector_2_gpu);
    cudaFree(output_vector_gpu);
    free(vector_1);
    free(vector_2);
    free(output_vector);

    return 0;
}
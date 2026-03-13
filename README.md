# Introduction
This repostitory contains various implementations of the Game of Life program for The University of Alabama's CS 481 class.

# Table of Contents
- [MPI Function Headers](#mpi-function-headers)
    - [MPI_Init](#mpi_init)
    - [MPI_Comm_size](#mpi_comm_size)
    - [MPI_Comm_rank](#mpi_comm_rank)
    - [MPI_Finalize](#mpi_finalize)
    - [MPI_Send](#mpi_send)
    - [MPI_Isend](#mpi_isend)
    - [MPI_Recv](#mpi_recv)
    - [MPI_Irecv](#mpi_irecv)
    - [MPI_Bcast](#mpi_bcast)
    - [MPI_Reduce](#mpi_reduce)
    - [MPI_Ireduce](#mpi_ireduce)
    - [MPI_Allreduce](#mpi_allreduce)
    - [MPI_Iallreduce](mpi_iallreduce)
    - [MPI_Scatter](#mpi_scatter)
    - [MPI_Iscatter](#mpi_iscatter)
    - [MPI_Scatterv](#mpi_scatterv)
    - [MPI_Gather](#mpi_gather)
    - [MPI_Igather](#mpi_igather)
    - [MPI_Gatherv](mpi_gatherv)
    - [MPI_Barrier](mpi_barrier)
    - [MPI_Wait](mpi_wait)
    - [MPI_Waitall](mpi_waitall)
- [MPI Datatypes](#mpi-datatypes)
- [MPI Operators](#mpi-operators)

# MPI Function Headers
## MPI_Init
    int MPI_Init (
        int∗ argc_p / ∗ in / out ∗ / , 
        char∗∗∗ argv_p / ∗ in / out ∗ / ) ;
Initializes the MPI execution environment. This must be the first MPI function called in any MPI program to set up the communication infrastructure.

## MPI_Comm_size
    int MPI_Comm_size (
        MPI_Comm comm / ∗ in ∗ / ,
        int∗ comm_sz_p / ∗ out ∗ / ) ;
Determines the total number of processes currently active in a specific communicator (usually MPI_COMM_WORLD).

## MPI_Comm_rank
    int MPI_Comm_rank (
        MPI_Comm comm / ∗ in ∗ / ,
        int∗ my_rank_p / ∗ out ∗ / ) ;
Determines the unique ID (rank) of the calling process within a specific communicator, typically ranging from $0$ to $size - 1$.

## MPI_Finalize
    int MPI_Finalize ( void ) ;
Terminates the MPI execution environment. This must be the last MPI function called; it cleans up resources and shuts down the communication library.

## MPI_Send
    int MPI_Send (
        void ∗ msg_buf_p / ∗ in ∗ / ,
        int msg_size / ∗ in ∗ / ,
        MPI_Datatype msg_type / ∗ in ∗ / ,
        int dest / ∗ in ∗ / ,
        int tag / ∗ in ∗ / ,
        MPI_Comm communicator / ∗ in ∗ / ) ;
Performs a standard, blocking send operation to a specific destination process using a designated tag and communicator.

## MPI_Isend
    int MPI_Isend(
        const void* buf /* in */,
        int count /* in */,
        MPI_Datatype datatype /* in */,
        int dest /* in */,
        int tag /* in */,
        MPI_Comm comm /* in */,
        MPI_Request* request /* out */);
Starts a non-blocking send operation. Unlike MPI_Send, this function returns immediately, allowing the program to perform other computations while the message is being sent. It provides a MPI_Request handle that must later be checked with MPI_Wait or MPI_Test to ensure the buffer is safe to reuse.

## MPI_Recv
    int MPI_Recv (
        void∗ msg_buf_p / ∗ out ∗ / ,
        int buf_size / ∗ in ∗ / ,
        MPI_Datatype buf_type / ∗ in ∗ / ,
        int source / ∗ in ∗ / ,
        int tag / ∗ in ∗ / ,
        MPI_Comm communicator / ∗ in ∗ / ,
        MPI_Status∗ status_p / ∗ out ∗ / ) ;
Performs a blocking receive operation, waiting for a message that matches a specific source, tag, and communicator.

## MPI_Irecv
    int MPI_Irecv (
        void* buf_p /* out */,
        int buf_size /* in */,
        MPI_Datatype datatype /* in */,
        int source /* in */,
        int tag /* in */,
        MPI_Comm comm /* in */,
        MPI_Request* request_p /* out */ ) ;
Starts a non-blocking receive operation. Unlike MPI_Recv, this function returns immediately without waiting for a matching message to actually arrive. It provides a MPI_Request handle that tracks the progress of the operation. The programmer must call a completion function (like MPI_Wait) to ensure the data has been fully written to the buffer before attempting to read from it.

## MPI_Bcast
    int MPI_Bcast (
        void∗ data_p / ∗ in / out ∗ / ,
        int count / ∗ in ∗ / ,
        MPI_Datatype datatype / ∗ in ∗ / ,
        int source_proc / ∗ in ∗ / ,
        MPI_Comm comm / ∗ in ∗ / ) ;
Broadcasts a message from one process (the "source") to all other processes within the specified communicator.

## MPI_Reduce
    int MPI_Reduce (
        void∗ input_data_p / ∗ in ∗ / ,
        void∗ output_data_p / ∗ out ∗ / ,
        int count / ∗ in ∗ / ,
        MPI_Datatype datatype / ∗ in ∗ / ,
        MPI_Op operator / ∗ in ∗ / ,
        int dest_process / ∗ in ∗ / ,
        MPI_Comm comm / ∗ in ∗ / ) ;
Collects data from all processes in a communicator and performs a mathematical or logical operation (like sum or max), storing the result on a single "destination" process.

## MPI_Ireduce
    int MPI_Ireduce(
        const void* input_data_p /* in */,
        void* output_data_p /* out */,
        int count /* in */,
        MPI_Datatype datatype /* in */,
        MPI_Op operator /* in */,
        int dest_process /* in */,
        MPI_Comm comm /* in */,
        MPI_Request* request_p /* out */);
The non-blocking version of MPI_Reduce. it starts the process of collecting data from all processes in a communicator and performing a mathematical or logical operation. The result is eventually stored only on the dest_process. Since it is non-blocking, the output_data_p on the destination rank is not guaranteed to be valid until the MPI_Request is confirmed as complete by a function like MPI_Wait.

## MPI_Allreduce
    int MPI_Allreduce (
        void∗ input_data_p / ∗ in ∗ / ,
        void∗ output_data_p / ∗ out ∗ / ,
        int count / ∗ in ∗ / ,
        MPI_Datatype datatype / ∗ in ∗ / ,
        MPI_Op operator / ∗ in ∗ / ,
        MPI_Comm comm / ∗ in ∗ / ) ;
Similar to MPI_Reduce, but instead of storing the result on just one process, it distributes the final reduced value to all processes in the communicator.

## MPI_Iallreduce
    int MPI_Iallreduce(
        const void* input_data_p /* in */,
        void* output_data_p /* out */,
        int count /* in */,
        MPI_Datatype datatype /* in */,
        MPI_Op operator /* in */,
        MPI_Comm comm /* in */,
        MPI_Request* request_p /* out */);
The non-blocking version of MPI_Allreduce. It initiates a global reduction operation where the result is eventually distributed to all processes in the communicator. This allows processes to overlap the often-expensive global synchronization of a reduction with independent local computation. As with all non-blocking collectives, the result in output_data_p is not valid until the associated MPI_Request has been satisfied.

## MPI_Scatter
    int MPI_Scatter (
        void∗ send_buf_p / ∗ in ∗ / ,
        int send_count / ∗ in ∗ / ,
        MPI_Datatype send_type / ∗ in ∗ / ,
        void∗ recv_buf_p / ∗ out ∗ / ,
        int recv_count / ∗ in ∗ / ,
        MPI_Datatype recv_type / ∗ in ∗ / ,
        int src_proc / ∗ in ∗ / ,
        MPI_Comm comm / ∗ in ∗ / ) ;
Takes a buffer of data from a single process and splits it into equal chunks, sending one chunk to each process in the communicator.

## MPI_Iscatter
    int MPI_Iscatter(
        const void* send_buf_p /* in */,
        int send_count /* in */,
        MPI_Datatype send_type /* in */,
        void* recv_buf_p /* out */,
        int recv_count /* in */,
        MPI_Datatype recv_type /* in */,
        int src_proc /* in */,
        MPI_Comm comm /* in */,
        MPI_Request* request_p /* out */);
A non-blocking version of MPI_Scatter. It initiates the process of splitting a buffer into equal chunks and distributing them to all processes in the communicator. Because it returns immediately, the program can overlap the communication time with other localized calculations. Just like point-to-point non-blocking calls, you must use the MPI_Request handle with a function like MPI_Wait to ensure the recv_buf_p is fully populated before you try to use the data.

## MPI_Scatterv
    int MPI_Scatterv(
        const void* send_buf_p /* in */,
        const int* send_counts /* in */,
        const int* displacements /* in */,
        MPI_Datatype send_type /* in */,
        void* recv_buf_p /* out */,
        int recv_count /* in */,
        MPI_Datatype recv_type /* in */,
        int src_proc /* in */,
        MPI_Comm comm /* in */);
An extension of MPI_Scatter that allows a varying count of data to be sent to each process. It uses an array (send_counts) to specify how many elements go to each rank and an array (displacements) to specify the starting offset of each chunk in the send buffer.

## MPI_Gather
    int MPI_Gather (
        void ∗ send_buf_p / ∗ in ∗ / ,
        int send_count / ∗ in ∗ / ,
        MPI_Datatype send_type / ∗ in ∗ / ,
        void∗ recv_buf_p / ∗ out ∗ / ,
        int recv_count / ∗ in ∗ / ,
        MPI_Datatype recv_type / ∗ in ∗ / ,
        int dest_proc / ∗ in ∗ / ,
        MPI_Comm comm / ∗ in ∗ / ) ;
The inverse of scatter; it collects equal-sized chunks of data from all processes in a communicator and concatenates them into a single buffer on the destination process.

## MPI_Igather
    int MPI_Igather(
        const void* sendbuf /* in */,
        int sendcount /* in */,
        MPI_Datatype sendtype /* in */,
        void* recvbuf /* out */,
        int recvcount /* in */,
        MPI_Datatype recvtype /* in */,
        int root /* in */,
        MPI_Comm comm /* in */,
        MPI_Request* request /* out */);
A non-blocking version of MPI_Gather. It initiates the collection of data chunks from all processes to a single destination process. Because it is non-blocking, the calling process can continue executing other code while the collective communication proceeds in the background. Completion must be verified using the returned MPI_Request handle.

## MPI_Gatherv
    int MPI_Gatherv(
        const void* send_buf_p /* in */,
        int send_count /* in */,
        MPI_Datatype send_type /* in */,
        void* recv_buf_p /* out */,
        const int* recv_counts /* in */,
        const int* displacements /* in */,
        MPI_Datatype recv_type /* in */,
        int dest_proc /* in */,
        MPI_Comm comm /* in */);
The inverse of MPI_Scatterv. it collects chunks of varying sizes from all processes and assembles them into a single buffer on the destination process. The recv_counts and displacements arrays define how much data is coming from each process and where it should be placed in the destination buffer.

## MPI_Barrier
    int MPI_Barrier ( MPI_Comm comm / ∗ i n ∗ / ) ;
Acts as a synchronization point; no process in the communicator can proceed past this call until every process in that communicator has reached it.

## MPI_Wait
    int MPI_Wait(
        MPI_Request* request_p /* in/out */,
        MPI_Status* status_p   /* out */);
Blocks the execution of the calling process until the specific non-blocking operation associated with request_p has completed. Once this function returns, the memory buffer used in the non-blocking call is safe to read from or write to again.

## MPI_Waitall
    int MPI_Waitall(
        int count /* in */,
        MPI_Request array_of_requests[] /* in/out */,
        MPI_Status array_of_statuses[] /* out */);
Blocks the calling process until all non-blocking operations associated with the requests in the array have completed. This is more efficient than calling MPI_Wait in a loop when handling multiple simultaneous transfers, such as during a halo exchange or a complex collective.

# MPI Datatypes
| MPI Datatype | C Datatype |
| :--- | :--- |
| MPI_CHAR | signed char |
| MPI_SHORT | signed short int |
| MPI_INT | signed int |
| MPI_LONG | signed long int |
| MPI_LONG_LONG | signed long long int |
| MPI_UNSIGNED_CHAR | unsigned char |
| MPI_UNSIGNED_SHORT | unsigned short int |
| MPI_UNSIGNED | unsigned int |
| MPI_UNSIGNED_LONG | unsigned long int |
| MPI_FLOAT | float |
| MPI_DOUBLE | double |
| MPI_LONG_DOUBLE | long double |
| MPI_BYTE | (none) |
| MPI_PACKED | (none) |

# MPI Operators
| Operation Value | Meaning |
| :--- | :--- |
| MPI_MAX | Maximum |
| MPI_MIN | Minimum |
| MPI_SUM | Sum |
| MPI_PROD | Product |
| MPI_LAND | Logical and |
| MPI_BAND | Bitwise and |
| MPI_LOR | Logical or |
| MPI_BOR | Bitwise or |
| MPI_LXOR | Logical exclusive or |
| MPI_BXOR | Bitwise exclusive or |
| MPI_MAXLOC | Maximum and location of maximum |
| MPI_MINLOC | Minimum and location of minimum |
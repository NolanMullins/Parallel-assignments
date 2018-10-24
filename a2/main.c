#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <mpi.h>

#define output 0

void exitMPI()
{
    MPI_Finalize();
    exit(0);
}

void error(char* msg)
{
    printf("%s\n", msg);
    exitMPI();
}

int main(int argc, char *argv[])
{
    int comm_sz, rank, size, i, j; 

    /* Start up MPI */
    MPI_Init(NULL, NULL);

    /* Get the number of processes */
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    /* Get my rank among all the processes */
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 3)
        if (rank == 0)
            error("Not enough args");
        else
            exitMPI();

    size = strtol(argv[2], NULL, 10); 
    int* A = malloc(sizeof(int)*size);
    int* B = malloc(sizeof(int)*size*size);
    int* C = malloc(sizeof(int)*size);
    memset(C, 0, sizeof(int)*size);

    //Start timing here
    struct timespec startTime, finishTime;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &startTime);

    //Generate matrix
    if (rank == 0)
    {
        time_t t;
        srand((unsigned) time(&t));
        for (i = 0; i < size; i++)
        {
            A[i] = rand() % size;
            for (j = 0; j < size; j++)
                B[i*size+j] = rand() % 50;
        }
    }

    MPI_Bcast(A, size, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(B, size*size, MPI_INT, 0, MPI_COMM_WORLD);

    int numRows = size / comm_sz;
    if (numRows < 1)
        numRows = 1;
    int start = rank*numRows;
    int end = (rank+1)*numRows;
    if (rank == comm_sz - 1)
        end = size;

    //Calculate results
    if (start < size)
        for (i = start; i < end; i++)
        {
            int c = 0;
            for (j = 0; j < size; j++)
                c += A[i]*B[i*size+j];
            C[i] = c;
        }

    //join up data
    //int res[size];
    int* res = malloc(sizeof(int)*size);
    MPI_Reduce(C, res, size, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    //Finish timing
    clock_gettime(CLOCK_MONOTONIC, &finishTime);

    elapsed = (finishTime.tv_sec - startTime.tv_sec);
    elapsed += (finishTime.tv_nsec - startTime.tv_nsec) / 1000000000.0;

    if (rank == 0)
        printf("time taken: %lf\n", elapsed);

    if (rank == 0 && output)
    {
        for (i = 0; i < size; i++)
            printf("%d\t", res[i]);
        printf("\n");
    }

    free(A);
    free(B);
    free(C);
    free(res);

    /* Shut down MPI */
    MPI_Finalize();
    return 0;
}
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <ctype.h>
#include <mpi.h>

#define output 0 
#define displayAB 0 
#define random 0 

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

void getEndPoints(int rank, int comm_sz, int size, int* start, int* end)
{
    int numRows = size / comm_sz;
    if (numRows < 1)
        numRows = 1;
    *start = rank*numRows;
    *end = (rank+1)*numRows;
    if (rank == comm_sz - 1)
        *end = size;
}

void displayMatrix(int* A, int rows, int cols)
{
    int i,j;
    for (i = 0; i < rows; i++)
    {
        for (j = 0; j < cols; j++)
        {
            printf("%d ", A[i*cols+j]);
        }
        printf("\n");
    }
    printf("\n");
}

void generateMatricies(int* A, int* B, int size, int rank, int comm_sz)
{
    int i, j;
    if (random==-1)
        memset(B, 1, sizeof(int)*size*size);
    else
    {
        time_t t;
        if (random)
            srand((unsigned) time(&t));
        else
            srand(124);
        for (i = 0; i < size; i++)
        {
            A[i] = rand() % size;
            for (j = 0; j < size; j++)
                B[i*size+j] = rand() % size;
        }
    }
}

double run(int size, int comm_sz, int rank)
{
    int start,end, i, j;
    getEndPoints(rank, comm_sz, size, &start, &end);

    int* A = malloc(sizeof(int)*size);
    int* B;
    int* C = malloc(sizeof(int)*size);
    if (rank == 0)
        B = malloc(sizeof(int)*size*size);
    else
        B = malloc(sizeof(int)*(end-start)*size);

    if (rank == 0)
        generateMatricies(A, B, size, rank, comm_sz);

    if (A == NULL || B == NULL || C == NULL)
        error("Malloc returned NULL");
    memset(C, 0, sizeof(int)*size);

    //Start timing here
    struct timespec startTime, finishTime;
    double elapsed;

    clock_gettime(CLOCK_MONOTONIC, &startTime);

    //send information
    if (rank == 0)
    {
        int tmpS, tmpE;
        for (i = 1; i < comm_sz; i++)
        {
            getEndPoints(i, comm_sz, size, &tmpS, &tmpE);
            MPI_Request reqA, reqB;
            MPI_Isend(&(B[tmpS*size]), (tmpE-tmpS)*size, MPI_INT, i, 0, MPI_COMM_WORLD, &reqA);
            MPI_Isend(A, size, MPI_INT, i, 0, MPI_COMM_WORLD, &reqB);
        }
    }
    else if (rank < comm_sz)
    {
        MPI_Recv(B, (end-start)*size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(A, size, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    //MPI_Bcast(A, size, MPI_INT, 0, MPI_COMM_WORLD);

    //Calculate results
    if (start < size && rank < comm_sz)
        for (i = start; i < end; i++)
            for (j = 0; j < size; j++)
                C[j] += A[i]*B[(i-start)*(size)+j];

    //join up data
    int* res = malloc(sizeof(int)*size);
    MPI_Reduce(C, res, size, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

    //Finish timing
    clock_gettime(CLOCK_MONOTONIC, &finishTime);

    elapsed = (finishTime.tv_sec - startTime.tv_sec);
    elapsed += (finishTime.tv_nsec - startTime.tv_nsec) / 1000000000.0;

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

    if (rank == 0)
        return elapsed;
}

int main(int argc, char *argv[])
{
    int comm_sz, rank, size, i, j, g = 0; 
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
    if (argc > 3)
        if (strcmp(argv[3], "-g")==0)
            g = 1;

    size = strtol(argv[2], NULL, 10); 

    if (g)
    {
        if (rank == 0)
        {
            printf("Size\tProcesses\n");
            for (i = 0; i < comm_sz; i++)
                printf("\t\t%d", i+1);
            printf("\n");
        }
        int sizes[] = {100, 1000, 10000, 20000};
        int i, j;
        for (i = 0; i < 4; i++)
        {
            if (rank == 0)
                printf("%d\t\t", sizes[i]);
            for (j = 1; j <= comm_sz; j++)
            {
                MPI_Barrier(MPI_COMM_WORLD);
                double time = run(sizes[i], j, rank);
                if (rank == 0)
                    printf("%lf\t", time);
            }
            if (rank == 0)
                printf("\n");
        }
    }
    else
    {
        double time =  run(size, comm_sz, rank);
        if (rank == 0)
            printf("time taken: %lf\n", time);
    }
    

    /* Shut down MPI */
    MPI_Finalize();
    return 0;
}
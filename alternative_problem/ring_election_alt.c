/******************************************************************************
* FILE: ring_election_alt.c
* DESCRIPTION:
*   Elect a ring president and vice-president based on the smallest odd and
*   even number of a process
* AUTHOR: David Nguyen
* CONTACT: david@knytes.com
* LAST REVISED: 19/04/2020 19:55 GMT-7
******************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <mpi.h>

#define MINIMUM_PROC 6
#define MAXIMUM_PROC 20

// Function to randomly generate number
int randomInt()
{
    int randTmp = (rand() % 90) + 10;                 // generates random integer [10,99]
    randTmp = randTmp < 0 ? randTmp * (-1) : randTmp; // make positive if negative

    return randTmp;
}

// Function to concatenate 1 with random num, then with rank
// Modified from example: https://www.geeksforgeeks.org/how-to-concatenate-two-integer-values-into-one/
int concat(int gen, int rank)
{
    char s1[5] = "1";
    char s2[2];
    char s3[2];

    // Convert ints to string
    sprintf(s2, "%d", gen);
    if (rank < 10)
    {
        sprintf(s3, "0%d", rank);
    }
    else
    {
        sprintf(s3, "%d", rank);
    }

    // Concatenate strings
    strcat(s1, s2);
    strcat(s1, s3);

    return atoi(s1); // return string converted to int
}

int main(int argc, char *argv[])
{
    // Variable declarations
    int rank,          // Process rank number
        size,          // Number of processes
        temp,          // randomly generated value
        tempComp,      // computed value
        leftNeighbor,  // holds left neighbor's value
        rightNeighbor, // holds right neighbor's value
        results[4];    // results to hold smallOdd, oddProc, smallEven, and evenProc

    // Initiate MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Request ireqs;
    MPI_Status istatus;

    // Seed random number generator
    time_t t;
    srand((unsigned)time(&t) + rank);

    // Check if processes declared is within bounds, else terminate program
    if (size < MINIMUM_PROC || size > MAXIMUM_PROC)
    {
        if (rank == 0)
        {
            printf("Quitting. Minimum processes required is %d and maximum processes allowed is %d. Current: %d\n", MINIMUM_PROC, MAXIMUM_PROC, size);
            MPI_Finalize();
            return 0;
        }
    }
    else
    {
        // Call for random integer generator
        temp = randomInt();
        tempComp = concat(temp, rank);
        printf("Process %d generated random value %d, computed value of %d\n", rank, temp, tempComp);

        // Send value to neighbors
        if (rank == 0)
        {
            MPI_Isend(&tempComp, 1, MPI_INT, size - 1, rank, MPI_COMM_WORLD, &ireqs);
        }
        else
        {
            MPI_Isend(&tempComp, 1, MPI_INT, (rank - 1) % size, rank, MPI_COMM_WORLD, &ireqs);
        }
        MPI_Isend(&tempComp, 1, MPI_INT, (rank + 1) % size, rank, MPI_COMM_WORLD, &ireqs);

        // Receive neighbor's values
        if (rank == 0)
        {
            MPI_Irecv(&leftNeighbor, 1, MPI_INT, size - 1, size - 1, MPI_COMM_WORLD, &ireqs);
            MPI_Wait(&ireqs, &istatus);
        }
        else
        {
            MPI_Irecv(&leftNeighbor, 1, MPI_INT, (rank - 1) % size, (rank - 1) % size, MPI_COMM_WORLD, &ireqs);
            MPI_Wait(&ireqs, &istatus);
        }

        MPI_Irecv(&rightNeighbor, 1, MPI_INT, (rank + 1) % size, (rank + 1) % size, MPI_COMM_WORLD, &ireqs);
        MPI_Wait(&ireqs, &istatus);

        printf("Process %d, My Value: %d, Left Neighbor: %d, Right Neighbor: %d\n", rank, tempComp, leftNeighbor, rightNeighbor);

        // Compare and announce if leader
        if (tempComp > leftNeighbor && tempComp > rightNeighbor)
        {

            printf("I, process %d, am the leader of my immediate neighbors.\n", rank);
        }
    }

    MPI_Finalize();
    return 0;
}

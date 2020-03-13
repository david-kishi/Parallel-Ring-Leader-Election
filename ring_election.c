/******************************************************************************
* FILE: ring_election.c
* DESCRIPTION:
*   Elect a ring president and vice-president based on the smallest odd and
*   even number of a process
* AUTHOR: David Nguyen
* CONTACT: david@knytes.com
* LAST REVISED: 11/03/2020 12:32:26 GMT-7
******************************************************************************/
#include    <stdio.h>
#include    <stdlib.h>
#include    <string.h>
#include    <time.h>
#include    <mpi.h>

#define     MINIMUM_PROC    6
#define     MAXIMUM_PROC    20

// Function to randomly generate number
int randomInt(int rank)
{
    int randTmp = rand() % 100;
    randTmp = randTmp < 0 ? randTmp * (-1) : randTmp; // make positive if negative
    randTmp = randTmp < 10 ? randTmp + 11 : randTmp; // if less than 10, add 11
    randTmp = randTmp > 100 ? randTmp % 100 : randTmp; // if bigger than 100, modulos 100
    randTmp = randTmp < 10 ? randTmp + 11 : randTmp; // check again if less than 10, add 11

    //return concat(randTmp,rank);
    return concat(randTmp, rank);
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
    sprintf(s3, "%d", rank);

    // Concatenate strings 
    strcat(s1,s2);
    strcat(s1,s3);

    return atoi(s1); // return string converted to int
}

int main(int argc, char *argv[])
{
    // Variable declarations
    int     rank,   // Process rank number
            size,   // Number of processes
            temp,   // temp placeholder for randomly computed value by process
            results[4]; // results to hold smallOdd, oddProc, smallEven, and evenProc
    
    // Initiate MPI
    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    // Seed random number generator
    time_t t;
    srand((unsigned) time(&t)+rank);

    // Check if processes declared is within bounds
    if(size < MINIMUM_PROC || size > MAXIMUM_PROC)
    {
        if(rank==0)
        {
            printf("Quitting. Minimum processes required is %d and maximum processes allowed is %d. Current: %d\n", MINIMUM_PROC, MAXIMUM_PROC, size);
            MPI_Finalize();
            return 0;
        }
    }
    else if (rank != 0) // All other ranks processing
    {   
        // Get results from previous rank
        MPI_Recv(&results, 4, MPI_INT, rank - 1, rank, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process %d received in-progress results [%d,%d,%d,%d] from Process %d\n", rank, results[0], results[1], results[2], results[3], rank - 1);
        
        // Call for random integer generator
        temp = randomInt(rank);
        printf("Process %d generated computed value of %d\n", rank, temp);

        if (temp % 2 == 0) // Check if even
        {
            if (temp < results[2]){ // Check if smaller than stored smallEven, if so, replace.
                printf("Process %d's computed value %d is less than result's smallest Even %d from Process %d. Replacing.\n", rank, temp, results[2], results[3]);
                results[2] = temp;
                results[3] = rank;
            }
        }
        else if (temp < results[0]) // Check if smaller than stored smallOdd, if so, replace.
        {
            printf("Process %d's computed value %d is less than result's smallest Odd %d from Process %d. Replacing.\n", rank, temp, results[0], results[1]);
            results[0] = temp;
            results[1] = rank;
        }
    }
    else // Rank 0 processing
    {
        // Call for random integer generator
        temp = randomInt(rank);
        printf("Process %d generated computed value of %d\n", rank, temp);

        // Initiate results
        if (temp % 2 == 0)
        {   
            results[0] = 19999;
            results[1] = results[3] = 0;
            results[2] = temp;
            printf("Process 0 generated an even value. Initating results to [%d,%d,%d,%d]\n", results[0], results[1], results[2], results[3]);
        }
        else
        {
            results[0] = temp;
            results[1] = results[3] = 0;
            results[2] = 19990;
            printf("Process 0 generated an odd value. Initating results to [%d,%d,%d,%d]\n", results[0], results[1], results[2], results[3]);
        }
        
    }
    // Send results off to next rank
    MPI_Send(&results, 4, MPI_INT, (rank + 1) % size, (rank + 1) % size, MPI_COMM_WORLD);
    printf("Process %d sending in-progress results [%d,%d,%d,%d] to Process %d\n", rank, results[0], results[1], results[2], results[3], (rank + 1) % size);

    // Now process 0 can receive from the last process.
    if (rank == 0)
    {
        // Receive final results
        MPI_Recv(&results, 4, MPI_INT, size - 1, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Process 0 received final results [%d,%d,%d,%d] from Process %d\n", results[0], results[1], results[2], results[3], size - 1);

        // Output election results
        printf("\t-\n\tRESULTS\n\t-\n\tPresident: %d from Process %d\n\tVice-President: %d from Process %d\n", results[0], results[1], results[2], results[3]);
    }

    MPI_Finalize();
    return 0;
}

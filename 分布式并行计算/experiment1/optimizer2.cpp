/* 优化一：去掉偶数 */
/* 优化二：消除广播 */

#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define LL long long

int main(int argc, char* argv[])
{
    LL    count;        /* Local prime count */
    double elapsed_time; /* Parallel execution time */
    LL    first;        /* Index of first multiple */
    LL    global_count; /* Global prime count */
    LL    high_value;   /* Highest value on this proc */
    LL    i;
    LL    j;
    int    id;           /* Process ID number */
    LL    index;        /* Index of current prime */
    LL    low_value;    /* Lowest value on this proc */
    char* marked;       /* Portion of 2,...,'n' */
    LL    n;            /* Sieving from 2, ..., 'n' */
    int    p;            /* Number of processes */
    LL    proc0_size;   /* Size of proc 0's subarray */
    LL    prime;        /* Current prime */
    LL    size;         /* Elements in 'marked' */

    /* 优化二新增参数 */
    char* common_marked;       /* 每个进程所维护的前sqrt(n)个素数 */
    int common_marked_size;    /* 前sqrt(n)个素数的标记长度 */

    MPI_Init(&argc, &argv);

    /* Start the timer */

    MPI_Comm_rank(MPI_COMM_WORLD, &id);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    MPI_Barrier(MPI_COMM_WORLD);
    elapsed_time = -MPI_Wtime();

    if (argc != 2) {
        if (!id) printf("Command line: %s <m>\n", argv[0]);
        MPI_Finalize();
        exit(1);
    }

    n = atoll(argv[1]);

    /* Figure out this process's share of the array, as
       well as the integers represented by the first and
       last array elements */

    low_value = 3 + id * (n - 2) / p;
    high_value = 2 + (id + 1) * (n - 2) / p;
    size = (high_value - low_value) / 2 + 1;/* 只需要考虑偶数的情况 */

    /* we have an odd number of values to process */
    if (low_value % 2 == 0) {
        low_value++;
    }

    /* Bail out if all the primes used for sieving are
       not all held by process 0 */

    proc0_size = (n - 2) / p;

    if ((proc0_size + 2) < (int)sqrt((double)n)) {
        if (!id) printf("Too many processes\n");
        MPI_Finalize();
        exit(1);
    }

    /* 优化二：消除广播
	   核心思路：每个进程都计算3到sqrt(n)范围内的素数并打上标记，以消除广播 */
    common_marked_size = (int)sqrt((double)n) / 2 - 1;
    common_marked = (char*)malloc(common_marked_size);

    if (common_marked == NULL) {
        printf("Cannot allocate enough memory\n");
        MPI_Finalize();
        exit(1);
    }

    for (i = 0; i < common_marked_size; i++)common_marked[i] = 0;
    
	index = 0;
    prime = 3;
    do {
        for (i = (prime * prime - 3) / 2; i < common_marked_size; i += prime)common_marked[i] = 1;
        while (common_marked[++index]);
        prime = index * 2 + 3;
    } while (prime * prime <= (LL)sqrt((double)n));

    /* Allocate this process's share of the array. */

    marked = (char*)malloc(size);

    if (marked == NULL) {
        printf("Cannot allocate enough memory\n");
        MPI_Finalize();
        exit(1);
    }

    for (i = 0; i < size; i++)marked[i] = 0;
    for (j = 0; j < common_marked_size; j++) {
        if (common_marked[j])continue;
        prime = j * 2 + 3;
        /* 优化一：去掉偶数
           核心思路：找到每个进程所负责范围内第一个prime的倍数，然后进行标记，跳过偶数
        */
        if (prime * prime > low_value)
            /* distance = prime * prime - low_value */
            first = (prime * prime - low_value) / 2;
        else {
            if (!(low_value % prime))first = 0;
            else if (low_value % prime % 2 == 0)
                first = prime - ((low_value % prime) / 2);/* distance = (prime - (low_value % prime) + prime) / 2 */
            else
                first = (prime - (low_value % prime)) / 2;/* distance = (prime - (low_value % prime)) / 2 */
        }
        for (i = first; i < size; i += prime)marked[i] = 1;
    }
    
    count = 0;
    for (i = 0; i < size; i++)
        if (!marked[i])count++;
	/* 因为每个进程都维护了前sqrt(n)个素数，所以不用进行广播 */
    MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM,
        0, MPI_COMM_WORLD);


    /* Stop the timer */

    elapsed_time += MPI_Wtime();


    /* Print the results */

    if (!id) {
        /* 因为跳过了2，所以答案需要加1 */
        printf("There are %lld primes less than or equal to %lld\n", global_count + 1, n);
        printf("SIEVE (%d) %10.6f\n", p, elapsed_time);
    }
    MPI_Finalize();
    return 0;
}

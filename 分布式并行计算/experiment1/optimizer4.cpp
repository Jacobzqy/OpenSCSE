/* 优化一：去掉偶数 */
/* 优化二：消除广播 */
/* 优化三：cache优化 */
/* 优化四：自选优化
   本程序优化的几个思路：
   1. 把marked数组每一个数字用一个bit表示，进一步提高cache利用率
   2. 在返回一个字节中1的个数时，可通过预处理把查询时间复杂度降到O（1），预处理可采用动态规划思路
   3. marked数组初始化耗费的时间较长，故使用calloc代替malloc，省去了初始化的时间
   4. 针对代码中主体循环存在的数据冒险进行优化
   5. 针对代码中主体循环中的循环步长进行调整（即循环展开），提高CPU多发并行效率
   6. 考虑到素数分布很稀疏，故加入了分支优化（默认不是素数），减少CPU控制冒险，提高分支预测准确率
 */

#include "mpi.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#define LL long long
#define likely(x) __builtin_expect((x), 1)

int main(int argc, char* argv[])
{
    LL    count;        /* Local prime count */

	/* 增加count0，count1，count2，实现计数循环展开并消除数据冒险 */
	LL    count0;
	LL	  count1;
	LL    count2;

    double elapsed_time; /* Parallel execution time */
    LL    first;        /* Index of first multiple */
    LL    global_count; /* Global prime count */
    LL    high_value;   /* Highest value on this proc */
    LL    i;
    LL    j;
	LL    k;
    int    id;           /* Process ID number */
    LL    index;        /* Index of current prime */
    LL    low_value;    /* Lowest value on this proc */
    unsigned char* marked;       /* Portion of 2,...,'n' */
    LL    n;            /* Sieving from 2, ..., 'n' */
    int    p;            /* Number of processes */
    LL    proc0_size;   /* Size of proc 0's subarray */
    LL    prime;        /* Current prime */
    LL    size;         /* Elements in 'marked' */
	int sqrt_n;

    /* 优化2新增参数 */
    unsigned char* common_marked;
    int common_marked_size;

    /* 优化3新增参数 */
    LL    block_low;	         /*每一块的最小值*/
    int    chunk = 8e6;		 /*分块大小*/

	/* 预处理一个字节数据中1的个数，利用了动态规划的思想，使得后面查询的时间降到O（1） */
	int num[256] = {0};
	for(i = 0; i<256; i++)num[i] = (i&1) + num[i>>1];

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
    size = (high_value - low_value) / 2 + 1;/* 只需要考虑奇数，故size为原来的一半 */

    /* we have an odd number of values to process */
    if (low_value % 2 == 0) {
        low_value++;
    }

    /* Bail out if all the primes used for sieving are
       not all held by process 0 */

    proc0_size = (n - 2) / p;
	sqrt_n = (int)sqrt((double)n);

    if ((proc0_size + 2) < sqrt_n) {
        if (!id) printf("Too many processes\n");
        MPI_Finalize();
        exit(1);
    }

    /* 优化二：消除广播
	   核心思路：每个进程中预处理3到sqrt(n)中的素数，以消除广播 
	 */

	/* 将原本一个数据对应1byte优化为了一个数据对应一个二进制位（1bit），故空间得到优化 */
    common_marked_size = sqrt_n / 2 - 1;
    common_marked = (unsigned char*)calloc((common_marked_size + 7) / 8, sizeof(char));

    if (common_marked == NULL) {
        printf("Cannot allocate enough memory\n");
        MPI_Finalize();
        exit(1);
    }

    index = 0;
    prime = 3;
    do {
		/* 一个数对应一个bit之后，相应的，原本第i个数对应新数组第i/8个数的第i%8位，下同，不再注释 */
        for (i = (prime * prime - 3) / 2; i < common_marked_size; i += prime)common_marked[i / 8] |= 1 << (i % 8);
        while ((common_marked[((++index)/8)]>>(index%8))&1);
        prime = (index * 2) + 3;
    } while (prime * prime  < sqrt_n);

    /* Allocate this process's share of the array. */

    marked = (unsigned char*)calloc((size+7)/8, sizeof(char));

    if (marked == NULL) {
        printf("Cannot allocate enough memory\n");
        MPI_Finalize();
        exit(1);
    }
    
	/* 优化一：去掉偶数
	   核心思路：因为偶数除了2之外一定不是偶数，故可省去偶数的判断
	 */
	/* 优化三：cache优化
	   核心思路：根据服务器的cache数据，合理设置分块大小，每次把块内合数标记完再取下一块，减少内存读写
	 */
    for (k = 0; k < size; k += chunk) {
        block_low = low_value + (k * 2);//每一块的最小值
        for (j = 0; j < common_marked_size; j++) {
			/* 因为质数分布很稀疏，故设置默认分支判定不是素数，减少控制冒险 */
            if (likely((common_marked[j/8]>>(j%8))&1))continue;
            prime = (j * 2) + 3;
            if (prime * prime > block_low)
                /* distance = prime * prime - low_value */
                first = (prime * prime - block_low) / 2;
            else {
                if (!(block_low % prime))first = 0;
                else if (block_low % prime % 2 == 0)
                    first = prime - ((block_low % prime) / 2);/* distance = (prime - (low_value % prime) + prime) / 2 */
                else
                    first = (prime - (block_low % prime)) / 2;/* distance = (prime - (low_value % prime)) / 2 */
            }
			/* 循环展开，减少末尾表达式执行次数并提高CPU多发并行效率 */
            for (i = first + k; i + 15 * prime < size && i + 15 * prime < first + k + chunk; i += 16 * prime){
				marked[i/8] |= 1<<(i%8);
				marked[(i+prime)/8] |= 1<<((i+prime)%8);
				marked[(i + 2 * prime) / 8] |= 1 << ((i + 2 * prime) % 8);
    			marked[(i + 3 * prime) / 8] |= 1 << ((i + 3 * prime) % 8);
    			marked[(i + 4 * prime) / 8] |= 1 << ((i + 4 * prime) % 8);
    			marked[(i + 5 * prime) / 8] |= 1 << ((i + 5 * prime) % 8);
    			marked[(i + 6 * prime) / 8] |= 1 << ((i + 6 * prime) % 8);
    			marked[(i + 7 * prime) / 8] |= 1 << ((i + 7 * prime) % 8);
				marked[(i + 8 * prime) / 8] |= 1 << ((i + 8 * prime) % 8);
    			marked[(i + 9 * prime) / 8] |= 1 << ((i + 9 * prime) % 8);
    			marked[(i + 10 * prime) / 8] |= 1 << ((i + 10 * prime) % 8);
    			marked[(i + 11 * prime) / 8] |= 1 << ((i + 11 * prime) % 8);
    			marked[(i + 12 * prime) / 8] |= 1 << ((i + 12 * prime) % 8);
    			marked[(i + 13 * prime) / 8] |= 1 << ((i + 13 * prime) % 8);
    			marked[(i + 14 * prime) / 8] |= 1 << ((i + 14 * prime) % 8);
    			marked[(i + 15 * prime) / 8] |= 1 << ((i + 15 * prime) % 8);
			}
			for (; i < size && i < first + k + chunk; i += prime)marked[i / 8] |= 1 << (i % 8);
		}
    }

    
    
	global_count = 0;
    count = 0;
	count0 = 0;
	count1 = 0;
	count2 = 0;
	
	/* 此处也是循环展开，目的同上，因为都是对count进行操作，故增加count0、count1、count2以减少数据冒险 */
	for (i = 0; i + 15 < (size + 7) / 8; i+=16) {
        count += 32 - num[marked[i]] - num[marked[i+4]] - num[marked[i+8]] - num[marked[i+12]];
        count0 += 32 - num[marked[i + 1]] - num[marked[i+5]] - num[marked[i+9]] - num[marked[i+13]];
        count1 += 32 - num[marked[i + 2]] - num[marked[i+6]] - num[marked[i+10]] - num[marked[i+14]];
        count2 += 32 - num[marked[i + 3]] - num[marked[i+7]] - num[marked[i+11]] - num[marked[i+15]];
    }
    for (; i < (size + 7) / 8; i++)count += 8 - num[marked[i]];
    count = count + count0 + count1 + count2;

    MPI_Reduce(&count, &global_count, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);


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


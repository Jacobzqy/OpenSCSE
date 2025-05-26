#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include "timer.h"
#include "check.h"
#include <cuda_runtime.h>

#define SOFTENING 1e-9f
#define BLOCK_SIZE 128
#define BLOCK_TILE 32
#define BLOCK_NUM 32

/*
 * Each body contains x, y, and z coordinate positions,
 * as well as velocities in the x, y, and z directions.
 */

typedef struct { float x, y, z, vx, vy, vz; } Body;

/*
 * Do not modify this function. A constraint of this exercise is
 * that it remain a host function.
 */

void randomizeBodies(float *data, int n) {
    for (int i = 0; i < n; i++) {
        data[i] = 2.0f * (rand() / (float)RAND_MAX) - 1.0f;
    }
}

/*
 * This function calculates the gravitational impact of all bodies in the system
 * on all others, but does not update their positions.
 */

extern __shared__ float sp[];
__global__ void bodyForce(Body *p, int *state, float dt, int n) {
    int i = (threadIdx.x + blockDim.x * blockIdx.x) % n;

    float Fx = 0.0f; float Fy = 0.0f; float Fz = 0.0f;
    float sx0 = p[i].x; float sy0 = p[i].y; float sz0 = p[i].z;

    int j = blockIdx.x / BLOCK_NUM;
    sp[3 * threadIdx.x + 0] = p[j * BLOCK_SIZE + threadIdx.x].x;
    sp[3 * threadIdx.x + 1] = p[j * BLOCK_SIZE + threadIdx.x].y;
    sp[3 * threadIdx.x + 2] = p[j * BLOCK_SIZE + threadIdx.x].z;
    __syncthreads();
#pragma unroll 32
    for(int k = 0; k < BLOCK_SIZE; k++){
	float dx = sp[3 * k + 0] - sx0;
	float dy = sp[3 * k + 1] - sy0;
	float dz = sp[3 * k + 2] - sz0;
	float distSqr = dx*dx + dy*dy + dz*dz + SOFTENING;
	float invDist = rsqrtf(distSqr);
	float invDist3 = invDist * invDist * invDist;
	Fx += dx * invDist3; Fy += dy * invDist3; Fz += dz * invDist3;
    }
    atomicAdd(&p[i].vx, dt*Fx);
    atomicAdd(&p[i].vy, dt*Fy);
    atomicAdd(&p[i].vz, dt*Fz);
    atomicSub(&state[i], 1);
    if(!state[i]){
	p[i].x = sx0 + p[i].vx * dt;
	p[i].y = sy0 + p[i].vy * dt;
	p[i].z = sz0 + p[i].vz * dt;
	atomicExch(&state[i], BLOCK_TILE);
    }
}


int main(const int argc, const char** argv) {

    /*
     * Do not change the value for `nBodies` here. If you would like to modify it,
     * pass values into the command line.
     */

    int nBodies = 2<<11;
    int salt = 0;
    if (argc > 1) nBodies = 2<<atoi(argv[1]);

    /*
     * This salt is for assessment reasons. Tampering with it will result in automatic failure.
     */

    if (argc > 2) salt = atoi(argv[2]);

    const float dt = 0.01f; // time step
    const int nIters = 10;  // simulation iterations

    int bytes = nBodies * sizeof(Body);
    float *buf;
    int *h_state;
    int *d_state;

    buf = (float *)malloc(bytes);
    h_state = (int *)malloc(nBodies*sizeof(int));
    for(int i = 0; i < nBodies; i++)
        h_state[i] = BLOCK_TILE;

    Body *h_p = (Body*)buf;
    Body *d_p;

    size_t threadsPerBlock = BLOCK_SIZE;
    size_t numberOfBlocks = BLOCK_NUM;



    /*
     * As a constraint of this exercise, `randomizeBodies` must remain a host function.
     */

    randomizeBodies(buf, 6 * nBodies); // Init pos / vel data

    cudaMalloc((void**)&d_p, bytes);
    cudaMalloc((void**)&d_state, nBodies*sizeof(int));
    cudaMemcpy(d_p, h_p, bytes, cudaMemcpyHostToDevice);
    cudaMemcpy(d_state, h_state, nBodies*sizeof(int), cudaMemcpyHostToDevice);

    double totalTime = 0.0;

    /*
     * This simulation will run for 10 cycles of time, calculating gravitational
     * interaction amongst bodies, and adjusting their positions to reflect.
     */

    /*******************************************************************/
    // Do not modify these 2 lines of code.


    for (int iter = 0; iter < nIters; iter++) {
        StartTimer();
        /*******************************************************************/

        /*
         * You will likely wish to refactor the work being done in `bodyForce`,
         * as well as the work to integrate the positions.
         */

        bodyForce<<<numberOfBlocks*BLOCK_TILE, threadsPerBlock, 3*BLOCK_SIZE*sizeof(float)>>>(d_p, d_state, dt, nBodies); // compute interbody forces

        /*
         * This position integration cannot occur until this round of `bodyForce` has completed.
         * Also, the next round of `bodyForce` cannot begin until the integration is complete.
         */

        if(iter == nIters - 1)cudaMemcpy(h_p, d_p, bytes, cudaMemcpyDeviceToHost);

        /*******************************************************************/
        // Do not modify the code in this section.
        const double tElapsed = GetTimer() / 1000.0;
        totalTime += tElapsed;
    }

    double avgTime = totalTime / (double)(nIters);
    float billionsOfOpsPerSecond = 1e-9 * nBodies * nBodies / avgTime;

#ifdef ASSESS
    checkPerformance(buf, billionsOfOpsPerSecond, salt);
#else
    checkAccuracy(buf, nBodies);
    printf("%d Bodies: average %0.3f Billion Interactions / second\n", nBodies, billionsOfOpsPerSecond);
    salt += 1;
#endif
    /*******************************************************************/

    /*
     * Feel free to modify code below.
     */

    free(buf);
    cudaFree(d_p);
}


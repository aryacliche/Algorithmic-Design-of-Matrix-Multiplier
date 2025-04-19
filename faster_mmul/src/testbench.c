#include <signal.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <stdint.h>
#include <pthread.h>
#include <pthreadUtils.h>
#include <Pipes.h>
#include <pipeHandler.h>
#ifndef SW
#include "vhdlCStubs.h"
#endif

#define ORDER 16

int main(int argc, char* argv[])
{
	fprintf(stderr,"Waiting for HW to be interfaced...\n");
	int I, J;
	uint32_t A[ORDER][ORDER];
	uint32_t B[ORDER][ORDER];

	for(I = 0; I < ORDER; I++)
	{
		for(J = 0; J < ORDER; J++)
		{
			A[I][J] = I + 1;
			B[I][J] = J + 1;
			storeA(I, J,(uint32_t)  A[I][J]);
			storeB(I, J,(uint32_t)  B[I][J]);
		}
	}
	fprintf(stderr,"Stored A, B\n");
	
	fprintf(stderr,"Running latencyTest\n");
	clock_t begin = clock();

	latencyTest();

	clock_t end = clock();
	double time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	fprintf(stderr,"Time spent in latencyTest: %f seconds\n", time_spent);

	fprintf(stderr,"Multiplying now\n");
	begin = clock();

	mmul();

	end = clock();
	double new_time_spent = (double)(end - begin) / CLOCKS_PER_SEC;
	fprintf(stderr,"Time spent: %f seconds\n", new_time_spent);
	fprintf(stderr,"Time spent on mmul purely: %f seconds\n", new_time_spent - time_spent);

	fprintf(stderr,"Finished dot_product, verifying correctness...\n");
	for(I = 0; I < ORDER; I++)
	{
		for(J = 0; J < ORDER; J++)
		{
			// First we will multiply A and B ourselves to find C[i][j]
			uint32_t sum = 0;
			int k;
			for(k = 0; k < ORDER; k++)
			{
				sum += A[I][k] * B[k][J];
			}

			uint32_t result = loadC (I,J);
			
			if (sum != result){
				fprintf(stderr, "Noooooooo @ (%d, %d) actual answer = %d but we got back %d\n", I, J, sum, result);
				return(1);
			}

			fprintf(stderr,"C[%d][%d] = %d.\n",I, J, result);

		}
	}


	return(0);
}

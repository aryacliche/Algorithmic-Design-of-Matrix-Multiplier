#include <signal.h>
#include <stdio.h>
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

	uint32_t start_load_time = readCounter();
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
	uint32_t end_load_time = readCounter();
	fprintf(stderr,"Stored A, B in %d cycles\n", end_load_time - start_load_time);
	
	fprintf(stderr,"Running latencyTest\n");
	
	start_load_time = readCounter();
    
    latencyTest();
	
	end_load_time = readCounter();
    
    int cycles_spent = end_load_time - start_load_time;

	fprintf(stderr,"Time spent in latencyTest: %d cycle\n", cycles_spent);

	fprintf(stderr,"Multiplying now\n");
	
	start_load_time = readCounter();

	mmul();

	end_load_time = readCounter();
	int new_cycles_spent = end_load_time - start_load_time;
	fprintf(stderr,"Time spent: %d cycles\n", new_cycles_spent);
	fprintf(stderr,"Time spent on mmul purely: %d cycles\n", new_cycles_spent - cycles_spent);

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

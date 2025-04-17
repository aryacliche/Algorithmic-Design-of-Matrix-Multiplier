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
	int I, J;
	for(I = 0; I < ORDER; I++)
	{
		for(J = 0; J < ORDER; J++)
		{
			storeA(I, J,(uint32_t)  I+1);
			storeB(I, J,(uint32_t)  J+1);
		}
	}
	fprintf(stderr,"Stored A, B\n");
	
	mmul();

	fprintf(stderr,"finished dot_product, results:\n");
	for(I = 0; I < ORDER; I++)
	{
		for(J = 0; J < ORDER; J++)
		{
			uint32_t result = loadC (I,J);
			fprintf(stderr,"C[%d][%d] = %d.\n",I, J, result);

		}
	}

	return(0);
}

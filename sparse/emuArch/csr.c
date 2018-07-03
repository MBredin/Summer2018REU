#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
#include <memoryweb.h>
#include <sys/time.h>
#include <cilk.h>

#define SANITY 0

#define NODELETS 8
#define THREADS 4

replicated long *x;

void fill_x(void *ptr, long node){
	long *gto = (long *)ptr;

	for(int i = 0; i < 2000; i++)
		gto[i] = i+1;
}

long *alloc_x(int rowM){
	long *mr = mw_mallocrepl(rowM * sizeof(long));
	mw_replicated_init_generic(mr, fill_x);

	return mr;
}

struct timeval tval_before, tval_after, tval_result;

void emuMat(long *nodeId, int rowM, int colN);
void csr_solv(long **emuRows, long nodeRows, long colN, int i, long *loc_sol);

int main(){
	int rowM = 2000;
	int colN = 2000;
	long *tempX = alloc_x(rowM);
	mw_replicated_init((long *)&x, (long)tempX);
	
	long *nodes = mw_malloc1dlong(NODELETS * sizeof(long *));
	for(int i = 0; i < NODELETS; i++)
		nodes[i] = i;
	/*
	for(int i = 0; i < NODELETS; i++){
		MIGRATE(&nodes[i]);
		printf("Node %ld: [", nodes[i]);
		for(int j = 0; j < rowM; j++){
			printf("%3ld", x[j]);
		}
		printf("End Node: %ld}\n", NODE_ID());
	}
	*/

	//starttiming();
	for(int i = 0; i < NODELETS; i++){
		cilk_spawn emuMat(&nodes[i], rowM, colN);
	}
	cilk_sync;
	
	
	

	//Sanity Check
	#if SANITY
		//Print Statements
	#endif
	//printf("Compression Time: %ld\n", compTime);
	//printf("Execution Time: %ld\n", execTime);

}

void emuMat(long *nodeId, int rowM, int colN){
	MIGRATE(&nodeId);
	
	////////////////////////////////////
	/////Allocate and Define Matrix/////
	////////////////////////////////////
	int rowSplit = rowM / NODELETS;
	int nodeRows = rowSplit;
	if(*nodeId == NODELETS-1)
		nodeRows += rowM % NODELETS;
	long **emuRows = malloc(nodeRows * sizeof(long *));

	//Allocate memory for given node
	for(int j = 0; j < nodeRows; j++){
		emuRows[j] = malloc(colN * sizeof(long));
	}
		
	int temp = *nodeId * rowSplit;
	for(int i = 0; i < nodeRows; i++){
		for(int j = 0; j < colN; j++){
			if(i+temp == j)
				emuRows[i][j] = 2;
			else if(abs(i+temp - j) == 1)
				emuRows[i][j] = 1;
			else
				emuRows[i][j] = 0;
		}
	}
	
	//Spawn threads to run on the same node
	/*
	long *loc_sol = malloc(nodeRows * sizeof(long));
	for(int i = 0; i < THREADS; i++)
		cilk_spawn csr_solv(emuRows, nodeRows, colN, i, loc_sol);
	cilk_sync;
	*/
	
	/////////////////////////////////////
	/////////////COMPRESSION/////////////
	/////////////////////////////////////
	starttiming();
	long cnt = 0;
	for(int i = 0; i < nodeRows; i++){
		for(int j = 0; j < colN; j++){
			if(emuRows[i][j] != 0){
				cnt++;
			}
		}
	}
	long *loc_data = malloc(cnt * sizeof(long));
	long *loc_ind = malloc(cnt * sizeof(long));
	long *loc_ptr = malloc((nodeRows + 1) * sizeof(long));
	loc_ptr[0] = 0;

	cnt = 0;
	long pcnt = 0;
	for(int i = 0; i < nodeRows; i++){
		pcnt = 0;
		for(int j = 0; j < colN; j++){
			if(emuRows[i][j] != 0){				
				loc_data[cnt] = emuRows[i][j];
				loc_ind[cnt] = j;
				cnt++;
				pcnt++;
			}
		}
		loc_ptr[i+1] = loc_ptr[i] + pcnt;
	}

	
	//////////////////////////////////
	/////////////SOLUTION/////////////
	//////////////////////////////////
	
	long *loc_sol = malloc(nodeRows * sizeof(long));
	for(int i = 0; i < nodeRows; i++){
		loc_sol[i] = 0;
		for(int j = loc_ptr[i]; j < loc_ptr[i+1]; j++){
			loc_sol[i] += loc_data[j] * x[loc_ind[j]];
		}
	}	
}
/*
void csr_solv(long **emuRows, long nodeRows, long colN, int threadId, long *loc_sol){

	/////////////////////////////////////
	/////////////COMPRESSION/////////////
	/////////////////////////////////////
	int start = threadId * (nodeRows / THREADS);
	int stop = (threadId + 1) * (nodeRows / THREADS);
	if(threadId == THREADS - 1)
		stop += nodeRows % THREADS;
	long cnt = 0;
	
	for(int i = start; i < stop; i++){
		for(int j = 0; j < colN; j++){
			if(emuRows[i][j] != 0){
				cnt++;
			}
		}
	}
	
	long *loc_data = malloc(cnt * sizeof(long));
	long *loc_ind = malloc(cnt * sizeof(long));
	long *loc_ptr = malloc((stop - start + 1) * sizeof(long));
	loc_ptr[0] = 0;

	cnt = 0;
	long pcnt = 0;
	for(int i = start; i < stop; i++){
		pcnt = 0;
		for(int j = 0; j < colN; j++){
			if(emuRows[i][j] != 0){				
				loc_data[cnt] = emuRows[i][j];
				loc_ind[cnt] = j;
				cnt++;
				pcnt++;
			}
		}
		
		loc_ptr[i+1] = loc_ptr[i] + pcnt;
	}

	
	//////////////////////////////////
	/////////////SOLUTION/////////////
	//////////////////////////////////
	for(int i = start; i < stop; i++){
		loc_sol[i] = 0;
		for(int j = loc_ptr[i]; j < loc_ptr[i+1]; j++){
			loc_sol[i] += loc_data[j] * x[loc_ind[j]];
		}
	}
}
*/

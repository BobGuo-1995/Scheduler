#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
/*??? I am curious about how the following work? I looked up online and see that if we use the define directive it is like define our variable ahead of time*/
//Yes. It's like define a constant variable
#define MAXCHAR 100
#define MAXPROCESS 100
#define MAXTIME 1000

struct MList {
	int data[MAXPROCESS], n;
};
void ListInit(struct MList* mlist) {
	mlist->n = 0;
	for (int i = 0; i < MAXPROCESS; ++i) mlist->data[i] = 0;
}
void ListAdd(struct MList* mlist, int val) {
	mlist->data[mlist->n++] = val;
}
void ListDel(struct MList* mlist, int index)
{
	if (mlist->n == 0)return;
	for (int i = index; i < mlist->n - 1; ++i)
		mlist->data[i] = mlist->data[i + 1];
	--mlist->n;
}
int ListFind(struct MList* mlist, int val)
{
	for (int i = 0; i < mlist->n; ++i)
		if (mlist->data[i] == val)
			return i;
	return -1;
}
void ListDelVal(struct MList* mlist, int val)
{
	int index = ListFind(mlist, val);
	if (index >= 0) ListDel(mlist, index);
}

enum ALGO { ALGO_F, ALGO_P, ALGO_R, ALGO_SJ, ALGO_SR };

char* algoTypeString[] = { "First Come First Serve","Priority","Round Robin","Shortest Job Next","Shortest Remaining Time Next" };
//struct for allocate in shared memory


struct CPair {
	int cpuTime;
	int ioTime;
};
struct CProcess {
	int id;
	char name[MAXCHAR];
	int priority;
	int arrivalTime;
	int startTime;
	int totalCPU;
	int totalIO;
	int totalReady;
	int endTime;
	struct CPair workTime[MAXPROCESS];
	int workPos;
	int quantum;
};


//global variables
//Number of processes
int n;

int overhead = 0, slicetime = 0;
//Extern variables
int n1, t;
//data array. We fill it from file
struct CProcess* processes;
//maximum number of processes
int maxProcessNumber = 0;
char algName[MAXCHAR];
struct MList qready, qwait, qblock;
//functions definition
void WaitAlg();
void AlgRR();
void AlgFCFS();
void AlgPriority();
void AlgSJN();
void AlgSRTN();

int main() {
	//read data from input file;
	char* fname = "input.txt";
	FILE* f1;
	if ((f1 = fopen(fname, "r")) == NULL) {
		printf("Cannot open file %s.\n", fname);
		exit(1);
	}
	//read input file  
	// ??? I think in c this means I am reading in decimal, decimal and a string. This is in the input file. 
	//Yes. %d - integer variable, %s - string.
	fscanf(f1, "%d", &maxProcessNumber);
	fscanf(f1, "%d", &overhead);
	fscanf(f1, "%s", algName);

	int algType = 0;
	switch (algName[0])
	{
	case 'P':
		algType = ALGO_P;
		break;
	case 'F':
		algType = ALGO_F;
		break;
	case 'R':
		algType = ALGO_R;
		break;
	default:
	//??? What is the syntax doing here? 
	//it's the same if (algName[1] == 'J' )algType=ALGO_SJ; else algType=ALGO_SR;
		algType = algName[1] == 'J' ? ALGO_SJ : ALGO_SR;
		break;
	}
	//??? I see here we check if the algorithm is RR, then we read in the time slice. 
	//Yes. If we detect RR algorithm, it's mean we have slightly different input file (file has time slice)
	if (strcmp(algName, "RR") == 0) fscanf(f1, "%d", &slicetime);
	int cpuTime, ioTime;
	printf("\n");
	//??? I never seem this syntax before. can you tell me what we are doing here?
	//calloc allocates 	MAXPROCESS*sizeof(struct CProcess) memory and fill it with 0
	processes = calloc(MAXPROCESS, sizeof(struct CProcess));
	int i;
	for (i = 0; i < MAXPROCESS; ++i)
	{
		struct CProcess* process = &processes[i];
		process->id = i+1;
		//Process-name Priority arrival-time CPU I/O CPU I/O CPU I/O CPU I/O... CPU 0
		if (EOF == fscanf(f1, "%s%d%d", process->name, &process->priority, &process->arrivalTime))break;
		printf("%s ", process->name);
		//??? question 1: what is the mult doing here 
		// we transform expression 3(1 2) to 1 2  1 2  1 2. mult=3
		//??? question 2: what is int j=0 and the ";;" will do?
		//	for (int j = 0;;) is equal  int j=0; while(1){ ...  infinity loop
		for (int j = 0;;) {
			int mult = 0;
			int flagMult = fscanf(f1, "%d(%d", &mult, &cpuTime);
			if (flagMult == 2) {
				fscanf(f1, "%d)", &ioTime);
			}
			else {
				cpuTime = mult;
				mult = 1;
				fscanf(f1, "%d", &ioTime);
			}
			for (int k = 0; k < mult; ++k)
			{
				printf("%d %d ", cpuTime, ioTime);
				process->workTime[j].cpuTime = cpuTime;
				process->workTime[j].ioTime = ioTime;
				++j;
			}
			if (!ioTime)break;//exit from the loop
		}
		printf("\n");
	}
	fclose(f1);
	n = i;
	//********
	//simulator
	//********
	
	//??? ListInit(&qready); ListInit(&qwait); ListInit(&qblock); I never seem this syntax before, can you explain?
	//we need an object like vector<int> in the c++
	//for this we define the MList structure and several functions for adding, removing and finding for an element
	//The C language has no constructor, so we must manually initiate the structure. 
	ListInit(&qready); ListInit(&qwait); ListInit(&qblock);
	n1 = 0;
	switch (algType)
	{
	case ALGO_R:
		AlgRR();
		break;
	case ALGO_F:
		AlgFCFS();
		break;
	case ALGO_P:
		AlgPriority();
		break;
	case ALGO_SJ:
		AlgSJN();
		break;
	case ALGO_SR:
		AlgSRTN();
		break;
	default:
		printf("Bad algrorithm name");
		exit(0);
		break;
	}

	printf("\n\nresult\n************\n\n");
	printf("Wall clock: %d\n", t - 1);
	printf("Algorithm: %s\n", algoTypeString[algType]);
	//??? what is the number in front of it doing? 
	//%3s  %-15s%10s%10s%10s%10s%10s%10s - formatted output
	//%3s - ouput string, width 3 , right justification 
	//%-15s - ouput string, width 15 , left justification 
	printf("%3s  %-15s%10s%10s%10s%10s%10s%10s\n","Id", "Name", "Arrival", "Start", "CPU", "I/O","Ready", "End");
	for (int i = 0; i < 100; ++i)
	{
		struct CProcess* process = &processes[i];
		if (process->name[0] == 0)break;
		printf("%3d  %-15s%10d%10d%10d%10d%10d%10d\n",process->id, process->name, process->arrivalTime, process->startTime, process->totalCPU, process->totalIO, process->endTime- process->totalCPU- process->totalIO, process->endTime);

	}
}


void AlgRR()
{
	float quantum = (float)slicetime;//??? for round-robin we will read in the time-slice and the idea of time slice is only for the round robin right? not for the other algorithm.
	// yes	
	int  p, j;//??? what will our p and j represent? 
	//p - id of the current process, j - current running position of the current process 
	//??? why are we using preincrementation instead of post incrementation like"t++" here 
	//++t is slightly faster than t++
	for (t = 0; t < MAXTIME; ++t) {
	//??? I don't know what will the "first: do here " both regarding the syntax of how we use first and how the logic work here. 
	//first:  is label for goto. Command goto first;
	// for example loop for(int i=0;i<10;++i){cout<<i;}
	//int i=0;
	//while (1){
	//  if (++i>=10 )goto done;
	//  cout<<i;
	// }
	//done: 
	
	first:
		p = qready.data[0];
		if (0 < qready.n) {
			j = processes[p].workPos;
			if (processes[p].workTime[j].cpuTime&&processes[p].quantum < quantum)
			{
				--processes[p].workTime[j].cpuTime;
				++processes[p].totalCPU;
				++processes[p].quantum;
			}
			else
			{
				//Process done
				for (int k = 0; k < overhead; ++k)
				{
					WaitAlg();
					++t;
				}
				ListDel(&qready, 0);
				if (processes[p].workTime[j].cpuTime) {
					ListAdd(&qready, p);
				}
				else {
					ListAdd(&qwait, p);
				}
				processes[p].quantum = 0;
				//we need to choose other process at the same time (don't change t)
				goto first;
			}
		}
		WaitAlg();
		if (n1 == n) break;
	}
}
void AlgFCFS() //??? basically it is saying if the time slice is big enough RR will become first come first serve? 
//Yes
{
	slicetime = 10000;
	AlgRR();
}
//??? since we read in the piority from the file, how would we sort the piority when we read in the file? 
//This function is not related to the file reading.
//We use this function in the qsort function to order processes by priority
//qsort(qready.data, qready.n, sizeof(int), CompPriority);
int CompPriority(const void *pleft, const void *pright) {
	int left = *((int*)pleft);
	int right = *((int*)pright);
	return processes[right].priority - processes[left].priority;
}
void AlgPriority()
{
	int  p, j;
	for (t = 0; t < MAXTIME; ++t) {
	first:
		qsort(qready.data, qready.n, sizeof(int), CompPriority);
		p = qready.data[0];
		if (0 < qready.n) {
			j = processes[p].workPos;
			if (processes[p].workTime[j].cpuTime)
			{
				--processes[p].workTime[j].cpuTime;
				++processes[p].totalCPU;
			}
			else
			{
				//Process done
				for (int k = 0; k < overhead; ++k)
				{
					WaitAlg();
					++t;
				}
				ListDel(&qready, 0);
				ListAdd(&qwait, p);
				goto first;
			}
		}
		WaitAlg();
		if (n1 == n) break;
	}
}
int CompSJN(const void *pleft, const void *pright) {
	int left = *((int*)pleft);
	int right = *((int*)pright);
	return processes[left].workTime[processes[left].workPos].cpuTime - processes[right].workTime[processes[right].workPos].cpuTime;
}
void AlgSJN()
{
	int  p, j;
	for (t = 0; t < MAXTIME; ++t) {
	first:
	//??? I think 
	//we sort qready by shortest time
		qsort(qready.data, qready.n, sizeof(int), CompSJN);
		p = qready.data[0];
		if (0 < qready.n) {
			j = processes[p].workPos;
			if (processes[p].workTime[j].cpuTime)
			{
				--processes[p].workTime[j].cpuTime;
				++processes[p].totalCPU;
			}
			else
			{
				//Process done
				for (int k = 0; k < overhead; ++k)
				{
					WaitAlg();
					++t;
				}
				ListDel(&qready, 0);
				ListAdd(&qwait, p);
				goto first;
			}
		}
		WaitAlg();
		if (n1 == n) break;
	}
}
int CompSRTN(const void *pleft, const void *pright) {
	int left = *((int*)pleft);
	int right = *((int*)pright);
	int remainingTimeLeft = 0, remainingTimeRight=0;
	for (int k = processes[left].workPos;;++k) {
		if (processes[left].workTime[k].cpuTime == 0)break;
		remainingTimeLeft += processes[left].workTime[k].cpuTime;
		remainingTimeLeft += processes[left].workTime[k].ioTime;
	}
	for (int k = processes[right].workPos;; ++k) {
		if (processes[right].workTime[k].cpuTime == 0)break;
		remainingTimeRight += processes[right].workTime[k].cpuTime;
		remainingTimeRight += processes[right].workTime[k].ioTime;
	}
	return remainingTimeLeft - remainingTimeRight;
}
void AlgSRTN()
{
	int  p, j;
	for (t = 0; t < MAXTIME; ++t) {
	first:
		qsort(qready.data, qready.n, sizeof(int), CompSRTN);
		p = qready.data[0];
		if (0 < qready.n) {
			j = processes[p].workPos;
			if (processes[p].workTime[j].cpuTime)
			{
				--processes[p].workTime[j].cpuTime;
				++processes[p].totalCPU;
			}
			else
			{
				//Process done
				for (int k = 0; k < overhead; ++k)
				{
					WaitAlg();
					++t;
				}
				ListDel(&qready, 0);
				ListAdd(&qwait, p);
				goto first;
			}
		}
		WaitAlg();
		if (n1 == n) break;
	}
}
//??? what is this for ? 
//It's common function for all algorithms
//In this function, we process arrival and wait (I / O operation)
void WaitAlg()
{
	//*********
	//Wait IO
	int p, j;
	for (int i = 0;;)
	{
		if (i >= qwait.n)break;
		p = qwait.data[i];
		j = processes[p].workPos;
		if (processes[p].workTime[j].ioTime)
		{
			--processes[p].workTime[j].ioTime;
			++processes[p].totalIO;
			++i;
		}
		else
		{
			ListDel(&qwait, i);
			if (processes[p].workTime[j + 1].cpuTime) {
				ListAdd(&qready, p);
				++processes[p].workPos;
			}
			else
			{
				++n1;
				processes[p].endTime = t - 1;

			}
		}

	}
	//process arrives 
	for (int i = 0; i < n; ++i)
	{
		if (processes[i].arrivalTime == t)
		{
			ListAdd(&qblock, i);
		}
	}
	//Add to ready
	for (; 0 < qblock.n;)
		if (qready.n < maxProcessNumber)
		{
			p = qblock.data[0];
			processes[p].startTime = t;
			ListAdd(&qready, p);
			ListDel(&qblock, 0);
		}
		else break;
}

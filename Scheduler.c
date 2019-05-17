#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
//This define derective can help us define constant variable ahead of time
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
		algType = algName[1] == 'J' ? ALGO_SJ : ALGO_SR;
		break;
	}
	if (strcmp(algName, "RR") == 0) fscanf(f1, "%d", &slicetime);
	int cpuTime, ioTime;
	printf("\n");
	processes = calloc(MAXPROCESS, sizeof(struct CProcess));
	int i;
	for (i = 0; i < MAXPROCESS; ++i)
	{
		struct CProcess* process = &processes[i];
		process->id = i+1;
		if (EOF == fscanf(f1, "%s%d%d", process->name, &process->priority, &process->arrivalTime))break;
		printf("%s ", process->name);
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
			if (!ioTime)break;
		}
		printf("\n");
	}
	fclose(f1);
	n = i;
	//********
	//simulator
	//********
     /*note:we need an object like vector<int> in the c++ for this we define the MList structure and several functions for adding, removing and finding for an element The C language has no constructor, so we must manually initiate the structure. */
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
  //formatting for the output,nothing fancy here 
	printf("\n\nresult\n************\n\n");
	printf("Wall clock: %d\n", t - 1);
	printf("Algorithm: %s\n", algoTypeString[algType]);
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
	float quantum = (float)slicetime;// for round robin we need to read in the time slice
	int  p, j;// p - id of the current process, j - current running position of the current process
 // reason for preincrementation is ++t is slightly faster than t++
	for (t = 0; t < MAXTIME; ++t) {
	first://this is like the goto command we seem in assembly language
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
				goto first;
			}
		}
		WaitAlg();
		if (n1 == n) break;
	}
}
void AlgFCFS() // Round robin will become first come first serve if the time slice is big enough. 
{
	slicetime = 10000;
	AlgRR();
}
//qsort is a great little helper in my code
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

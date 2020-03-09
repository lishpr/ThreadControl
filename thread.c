#include <setjmp.h>
#include <string.h>
#include <stdio.h>
#include "thread.h"
#include <stdlib.h>

static int MyInitThreadsCalled = 0;
int curthr;
int prevthr;
int schedflag = 0;
int exitflag = 0;
int thrgenptr = 0;

static struct thread { // thread table
	int valid;	
	jmp_buf env;	
	jmp_buf env0;
	void (*func)();
	int param;
} thread[MAXTHREADS];

static struct Queue { // queue struct
	int prev;
	int next;
	int inq;
} Queue[MAXTHREADS];

#define STACKSIZE	65536		// maximum size of thread stack

int head = -1;
int tail = -1;
int qsize = 0;

// some functionalities of queue

void initqueue () {
	for (int i = 0; i < 10; i++) {
    	Queue[i].prev = -1;
    	Queue[i].next = -1;
    	Queue[i].inq = 0;
  	}
}

void pushback (int t) {
	if (qsize == 0) {
		head = t;
    	tail = t;
    	Queue[t].prev = -1;
    	Queue[t].next = -1;
  	}
  	else {
    	Queue[tail].next = t;
    	Queue[t].prev = tail;
    	Queue[t].next = -1;
    	tail = t;
  	}
  	Queue[t].inq = 1;
  	qsize++;
}

int pop () {
  	int r = head;
  	if (qsize == 0) {
    	return -1;
  	}
  	else {
    	int m = Queue[head].next;
    	Queue[m].prev = -1;
    	Queue[head].prev = -1;
    	Queue[head].next = -1;
    	head = m;
    	qsize--;
  	}
  	if (qsize == 0) {
    	head = -1;
    	tail = -1;
  	}
  	Queue[r].inq = 0;
 	return r;
}

void rmnode (int t) {
  	if (qsize == 0) return;
  	if (head == t) {
    	pop();
  	}
  	else if (tail == t) {
    	int m = Queue[tail].prev;
    	Queue[m].next = -1;
    	Queue[tail].prev = -1;
    	Queue[tail].next = -1;
    	tail = m;
    	qsize--;
  	}
  	else if (Queue[t].inq){
    	int n = Queue[t].next;
    	int p = Queue[t].prev;
    	Queue[n].prev = p;
    	Queue[p].next = n;
    	Queue[t].next = -1;
    	Queue[t].prev = -1;
    	qsize--;
  	}
  	Queue[t].inq = 0;
  	if (qsize == 0) {
    	head = -1;
    	tail = -1;
  	}
}

void display() {
	if (head == -1) {
    	printf("queue is empty!\n");
    	return;
  	}
  	int curr = head;
  	while (Queue[curr].next != -1) {
    	printf("[%d]", curr);
    	curr = Queue[curr].next;
  	}
  	if (head != -1) printf("[%d]\n", curr);
}

/* 	MyInitThreads() initializes the thread package. Must be the first
 * 	function called by any user program that uses the thread package. 
 */

void MyInitThreads() {
	int i;

	if (MyInitThreadsCalled) {
		printf("MyInitThreads: should be called only once\n");
		exit(0);
	}

	for (i = 0; i < MAXTHREADS; i++) {
		thread[i].valid = 0;
	}
  
    initqueue();
  
    for (i = 0; i < MAXTHREADS; i++) {
		char stack[i * STACKSIZE];
		if (((int) &stack[i * STACKSIZE - 1]) - ((int) &stack[0]) + 1 != STACKSIZE * i) {
			printf("Stack space reservation failed\n");
			exit(0);
		}

    if (setjmp(thread[i].env) != 0) {
        void (*f)() = thread[curthr].func;
        int p = thread[curthr].param;
        (*f)(p);
        MyExitThread();
        exit(0);
    }
    
    memcpy(&(thread[i].env0), &(thread[i].env), sizeof(jmp_buf));
  
    }

	thread[0].valid = 1;
    curthr = 0;
	MyInitThreadsCalled = 1;
}

/* 	MyCreateThread(f, p) creates a new thread to execute f(p),
 *   	where f is a function with no return value and p is an
 * 	integer parameter. The new thread does not begin executing
 *  	until another thread yields to it. 
 */

int MyCreateThread(void (*f)(), int p) {
	if (! MyInitThreadsCalled) {
		printf("MyCreateThread: Must call MyInitThreads first\n");
		exit(0);
	}
  	int i;
  	int flag = 0;
  	for (i = thrgenptr; i < MAXTHREADS; i++) {
    	if (thread[i].valid == 0) {
      		thread[i].valid = 1;
      		thread[i].func = f;
      		thread[i].param = p;
      		pushback(i);
      		memcpy(&(thread[i].env), &(thread[i].env0), sizeof(jmp_buf));
      		flag = 1; 
      		thrgenptr = i + 1;
      	break;
    	}
  	}	
  	if (flag == 0) {
    	for (i = 0; i < thrgenptr; i++) {
      		if (thread[i].valid == 0) {
        		thread[i].valid = 1;
				thread[i].func = f;
				thread[i].param = p;
				pushback(i);
				memcpy(&(thread[i].env), &(thread[i].env0), sizeof(jmp_buf));
				flag = 1;
				thrgenptr = i + 1;
				break;
      		}
    	}
  	}
  	thrgenptr %= MAXTHREADS;
  	if (flag) {
    	return i;
  	}
  	else {
    	return -1;
	}
}

/*  	MyYieldThread(t) causes the running thread, call it T, to yield to
 * 	thread t.  Returns the ID of the thread that yielded to the calling
 * 	thread T, or -1 if t is an invalid ID.  Example: given two threads
 * 	with IDs 1 and 2, if thread 1 calls MyYieldThread(2), then thread 2
 *   	will resume, and if thread 2 then calls MyYieldThread(1), thread 1
 * 	will resume by returning from its call to MyYieldThread(2), which
 *  	will return the value 2.
 */

int MyYieldThread(int t) {
	if (! MyInitThreadsCalled) {
		printf("MyYieldThread: Must call MyInitThreads first\n");
		exit(0);
	}

	if (t < 0 || t >= MAXTHREADS) {
		printf("MyYieldThread: %d is not a valid thread ID\n", t);
		return(-1);
	}
	if (! thread[t].valid) {
		printf("MyYieldThread: Thread %d does not exist\n", t);
		return(-1);
	}
  
  	if (setjmp(thread[curthr].env) == 0) {
    	if (!exitflag) pushback(curthr);
		rmnode(t);
		exitflag = 0;
		prevthr = curthr;
		curthr = t;
		longjmp(thread[t].env, 1);
  	}
  	int r = prevthr;
	if (schedflag || exitflag) {
		r = -1;
	}
	schedflag = 0;
	exitflag = 0;
	return r;
}


/*  	MyGetThread() returns ID of currently running thread. 
 */

int MyGetThread() {
	if (! MyInitThreadsCalled) {
		printf("MyGetThread: Must call MyInitThreads first\n");
		exit(0);
	}
  	int k = curthr;
  	return k;
}

/* 	MySchedThread() causes the running thread to simply give up the
 * 	CPU and allow another thread to be scheduled. Selecting which
 * 	thread to run is determined here. Note that the same thread may
 *   	be chosen (as will be the case if there are no other threads). 
 */

void MySchedThread() {
	if (! MyInitThreadsCalled) {
		printf("MySchedThread: Must call MyInitThreads first\n");
		exit(0);
	}
  
	if (qsize == 0) {
		return;
	}
	else {
		int t = pop();
		if (t == -1) return;
		schedflag = 1;
		MyYieldThread(t);
	}
}

/* 	MyExitThread() causes the currently running thread to exit.  
 */

void MyExitThread() {
	if (! MyInitThreadsCalled) {
		printf("MyExitThread: Must call MyInitThreads first\n");
		exit(0);
	}
	int k = curthr;
	rmnode(k);
	thread[k].valid = 0;
	if (qsize == 0) exit(0);
	exitflag = 1;
	MySchedThread();
}

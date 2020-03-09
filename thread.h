#define MAXTHREADS	10			// maximum number of threads

void MyInitThreads();				// initialize thread system
int MyCreateThread(void (*f)(), int p);		// create a thread to run f(p)
int MyGetThread();				// return ID of current thread
int MyYieldThread(int t);			// yield to thread t
void MySchedThread();				// yield to scheduler
void MyExitThread();				// exit this thread
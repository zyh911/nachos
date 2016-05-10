// threadtest.cc 
//	Simple test case for the threads assignment.
//
//	Create two threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield, 
//	to illustratethe inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "thread.h"
#include "synch.h"

// testnum is set in main.cc
int testnum = 1;
int xianchengnum = 0;
struct aa
{
	Thread *p;
	bool b = false;
};
aa *a = new aa[128];

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

void
SimpleThread(int which)
{
    int num;
    
    for (num = 0; num < 5; num++) {
	printf("*** thread %d looped %d times\n", which, num);
	printf("%d\n", currentThread->xianchengID);
        currentThread->Yield();
    }
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest1()
{
    DEBUG('t', "Entering ThreadTest1");

    Thread *t = new Thread("forked thread");

    t->Fork(SimpleThread, 1);
    SimpleThread(0);
}

void ThreadTest2() 
{
	DEBUG('t', "Entering myTest2"); 

	Thread *t1 = new Thread("thread1"); 
	t1->SetPriority(7); 
	t1->Fork(SimpleThread, 1);

	Thread *t2 = new Thread("thread2"); 
	t2->SetPriority(9);
	t2->Fork(SimpleThread, 2);

	SimpleThread(0);
}


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

extern void StartProcess(char *file);
#ifdef USER_PROGRAM

void usertest1(int which)
{
        char *testname = "../test/tt1";
        StartProcess(testname);
}

void usertest2(int which)
{
        char *testname = "../test/tt2";
        StartProcess(testname);
}


void DoubleThreadTest()
{
        printf("Running DoubleThreadTest()...\n");
    Thread *t1 = new Thread("forked thread 1");
    Thread *t2 = new Thread("forked thread 2");

    DEBUG('t', "Entering ThreadTest1");
    t1->Fork(usertest1, 1);
    DEBUG('t', "Entering ThreadTest2");
    t2->Fork(usertest2, 2);
}

#endif // USER_PROGRAM
void
ThreadTest()
{
    switch (testnum) {
    case 1:
        #ifdef USER_PROGRAM
        DoubleThreadTest();
        #endif // USER_PROGRAM

        #ifndef USER_PROGRAM  
	ThreadTest2();
        #endif
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}


int buffer[10];
int currentProduce = 0;
int currentConsume = 0;
int count = 0;
Lock *conditionLock = new Lock("condition lock");
Condition *empty = new Condition("empty condition");
Condition *full = new Condition("full condition");

void Producer(int which)
{
	for(int i = 0; i < 20; i++)
	{
		conditionLock->Acquire();
		if(count == 10)
			full->Wait(conditionLock);
		buffer[(currentProduce++)%10] = 1;
		count++;
		printf("Thread %d produces at place %d.\n", which, (currentProduce-1)%10);
		empty->Signal(conditionLock);
		conditionLock->Release();
		if(count == 4)
			currentThread->Yield();
	}
}

void Consumer(int which)
{
	for(int i = 0; i < 20; i++)
	{
		conditionLock->Acquire();
		if(count == 0)
			empty->Wait(conditionLock);
		buffer[(currentConsume++)%10] = 0;
		count--;
		printf("Thread %d consumes at place %d.\n", which, (currentConsume-1)%10);
		full->Signal(conditionLock);
		conditionLock->Release();
	}
}

void
myTest_ProducerConsumer()
{
	Thread *t0 = new Thread("Consumer");
	t0->Fork(Consumer, 0);
	
	Thread *t1 = new Thread("Producer");
	t1->Fork(Producer, 1);

	currentThread->Yield();
}


Semaphore *mutex = new Semaphore("mutex", 1);
Semaphore *w = new Semaphore("w", 1);
int rc = 0;

void reader(int which)
{
	for(int i = 0; i < 3; i++) {
		mutex->P();
		rc++;
		if(rc == 1)
			w->P();
		mutex->V();

		printf("Thread %d is reading\n", which);
		currentThread->Yield();
		printf("Thread %d finishes reading\n", which);
		currentThread->Yield();

		mutex->P();
		rc--;
		if(rc == 0)
			w->V();
		mutex->V();
	}
}

void writer(int which)
{
	for(int i = 0; i < 3; i++) {
		w->P();

		printf("Thread %d is writing\n", which);
		currentThread->Yield();
		printf("Thread %d finishes writing\n", which);

		w->V();
		currentThread->Yield();
	}
}

void
myTest_ReaderWriter()
{
	Thread *t0 = new Thread("Writer");
	t0->Fork(writer, 0);
	
	(new Thread("Reader1"))->Fork(reader, 1);
	(new Thread("Reader2"))->Fork(reader, 2);
	(new Thread("Reader3"))->Fork(reader, 3);

	currentThread->Yield();
}


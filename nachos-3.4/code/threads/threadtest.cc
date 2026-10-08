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

#ifdef HW1_SEMAPHORES
    #include "synch.h"
#endif

// testnum is set in main.cc
int testnum = 1;

//----------------------------------------------------------------------
// SimpleThread
// 	Loop 5 times, yielding the CPU to another ready thread 
//	each iteration.
//
//	"which" is simply a number identifying the thread, for debugging
//	purposes.
//----------------------------------------------------------------------

int SharedVariable;

#ifdef HW1_SEMAPHORES
    Semaphore *key;
    Semaphore *gate;

    int numThreadsActive; // used to implement barrier upon completion
    int threads;   
#endif

void SimpleThread(int which) {
    int num, val;
    for(num = 0; num < 5; num++) {

        #ifdef HW1_SEMAPHORES
            key->P(); // Here we are taking the key 
        #endif

        val = SharedVariable;
        printf("*** thread %d sees value %d\n", which, val);
        currentThread ->Yield();
        SharedVariable = val+1;

        #ifdef HW1_SEMAPHORES
            key->V();  // Here we are returning the key
        #endif
        
        currentThread->Yield();
        
    }
    #ifdef HW1_SEMAPHORES
        key->P();
        numThreadsActive--; 
        if(numThreadsActive==0){
            for(int i =0; i<threads+1; i++){
                gate->V(); // opening the gate for every thread to be able to see the SharedVariable.
            }

        } 
        key->V();
        gate->P(); // closing the gate
    #endif

    
    val = SharedVariable;
    printf("Thread %d sees final value %d\n", which, val);
}

//----------------------------------------------------------------------
// ThreadTest1
// 	Set up a ping-pong between two threads, by forking a thread 
//	to call SimpleThread, and then calling SimpleThread ourselves.
//----------------------------------------------------------------------

void ThreadTest1(int n) {
    DEBUG('t', "Entering ThreadTest1");
    for(int i=1; i<=n; i++){
        Thread *t = new Thread("forked a thread");
        t->Fork(SimpleThread, i);
    }
    SimpleThread(0);
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

#ifdef HW1_SEMAPHORES
void ThreadTest(int n) {
    DEBUG('t', "Entering SimpleTest");

    key = new Semaphore("Master Key",1);
    gate = new Semaphore("gate", 0);

    Thread *t;
    numThreadsActive = n+1;
    threads =n+1;
    printf("NumthreadsActive = %d\n", numThreadsActive);

    for(int i=1; i<=n; i++)
    {
        t = new Thread("forked thread");
        t->Fork(SimpleThread,i);
    }
    SimpleThread(0);
}

#else 

void ThreadTest(int n) {
    switch (testnum) {
    case 1:
	ThreadTest1(n);
	break;
    default:
	printf("No test specified.\n");
	break;
    }
}

#endif 

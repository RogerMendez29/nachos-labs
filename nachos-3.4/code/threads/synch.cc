// synch.cc
//	Routines for synchronizing threads.  Three kinds of
//	synchronization routines are defined here: semaphores, locks
//   	and condition variables (the implementation of the last two
//	are left to the reader).
//
// Any implementation of a synchronization routine needs some
// primitive atomic operation.  We assume Nachos is running on
// a uniprocessor, and thus atomicity can be provided by
// turning off interrupts.  While interrupts are disabled, no
// context switch can occur, and thus the current thread is guaranteed
// to hold the CPU throughout, until interrupts are reenabled.
//
// Because some of these routines might be called with interrupts
// already disabled (Semaphore::V for one), instead of turning
// on interrupts at the end of the atomic operation, we always simply
// re-set the interrupt state back to its original value (whether
// that be disabled or enabled).
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "synch.h"
#include "system.h"

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	Initialize a semaphore, so that it can be used for synchronization.
//
//	"debugName" is an arbitrary name, useful for debugging.
//	"initialValue" is the initial value of the semaphore.
//----------------------------------------------------------------------

Semaphore::Semaphore(const char* debugName, int initialValue)
{
    name = debugName;
    value = initialValue;
    queue = new List;
}

//----------------------------------------------------------------------
// Semaphore::Semaphore
// 	De-allocate semaphore, when no longer needed.  Assume no one
//	is still waiting on the semaphore!
//----------------------------------------------------------------------

Semaphore::~Semaphore()
{
    delete queue;
}

//----------------------------------------------------------------------
// Semaphore::P
// 	Wait until semaphore value > 0, then decrement.  Checking the
//	value and decrementing must be done atomically, so we
//	need to disable interrupts before checking the value.
//
//	Note that Thread::Sleep assumes that interrupts are disabled
//	when it is called.
//----------------------------------------------------------------------

void
Semaphore::P()
{
    IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

    while (value == 0) { 			// semaphore not available
	queue->Append((void *)currentThread);	// so go to sleep
	currentThread->Sleep();
    }
    value--; 					// semaphore available,
						// consume its value

    (void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
}

//----------------------------------------------------------------------
// Semaphore::V
// 	Increment semaphore value, waking up a waiter if necessary.
//	As with P(), this operation must be atomic, so we need to disable
//	interrupts.  Scheduler::ReadyToRun() assumes that threads
//	are disabled when it is called.
//----------------------------------------------------------------------

void
Semaphore::V()
{
    Thread *thread;
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    thread = (Thread *)queue->Remove();
    if (thread != NULL)	   // make thread ready, consuming the V immediately
	scheduler->ReadyToRun(thread);
    value++;
    (void) interrupt->SetLevel(oldLevel);
}

// Dummy functions -- so we can compile our later assignments
// Note -- without a correct implementation of Condition::Wait(),
// the test case in the network assignment won't work!
Lock::Lock(const char* debugName) {
    name = debugName;
    queue = new List;
    owner = NULL;
}
Lock::~Lock() {
    delete queue;
}

void Lock::Acquire() {

    // Disable interrupts -- similar to Semaphore P()
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // Check if lock is free
    // If yes, make the lock not free anymore
    
        while(owner != NULL){
            queue-> Append((void *)currentThread);
            currentThread->Sleep();
        }  
    owner = currentThread;

    (void) interrupt->SetLevel(oldLevel); // Enable interrupts
}
void Lock::Release() {
    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // check if thread has lock ... isHeldByCurrentThread ?
    
    DEBUG('t', "The following Thread %s is releasing lock %s\n they should be equal", currentThread->getName(), name);
    ASSERT(isHeldByCurrentThread());

    
    owner = NULL;
    Thread *nextThread = (Thread *)queue->Remove();
    if(nextThread != NULL){
    scheduler-> ReadyToRun(nextThread); // waking up the next thread in line
    }
    
     (void) interrupt->SetLevel(oldLevel); // Enable interrupts

}

bool Lock::isHeldByCurrentThread() {
    return owner == currentThread;
}

Condition::Condition(const char* debugName) {
    name = debugName; // init
    queue =  new List;
}
Condition::~Condition() {
    delete queue;
}

void Condition::Wait(Lock* conditionLock) {
    ASSERT(conditionLock != NULL);
    ASSERT(conditionLock->isHeldByCurrentThread());

    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // Release the lock and join the waiting queue.
    conditionLock->Release();
    queue->Append((void*) currentThread);

    // Sleep until another thread signals us.
    currentThread->Sleep();

    // Restore interrupt state and reacquire the lock.
    (void) interrupt->SetLevel(oldLevel);
    conditionLock->Acquire();
}

void Condition::Signal(Lock* conditionLock) {
    ASSERT(conditionLock != NULL);
    ASSERT(conditionLock->isHeldByCurrentThread());

    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // Wake one waiting thread, if there is one.
    Thread* thread = (Thread*) queue->Remove();

    if (thread != NULL) {
        scheduler->ReadyToRun(thread);
    }

    (void) interrupt->SetLevel(oldLevel);
}

void Condition::Broadcast(Lock* conditionLock) {
    ASSERT(conditionLock != NULL);
    ASSERT(conditionLock->isHeldByCurrentThread());

    IntStatus oldLevel = interrupt->SetLevel(IntOff);

    // Wake all waiting threads.
    while (!queue->IsEmpty()) {
        Thread* thread = (Thread*) queue->Remove();

        if (thread != NULL) {
            scheduler->ReadyToRun(thread);
        }
    }

    (void) interrupt->SetLevel(oldLevel);
}

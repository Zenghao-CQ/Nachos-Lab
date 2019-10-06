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
#include "elevatortest.h"

// testnum is set in main.cc
int testnum = 2;

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
    t->set_user_id(6324);
    t->Fork(SimpleThread, (void*)1);
    SimpleThread(0);
}
//modify lab1
void
ThreadTest2()
{
    DEBUG('t', "Entering Lab1 Test3:");

    int all_threads = 150;

    int i;
    for (i = 0; i < all_threads; ++i) {
        // Generate a Thread object
        Thread *t = new Thread("lab1 thread");
        printf("*** thread name = %s : user_id = %d, thread_id = %d\n", t->getName(),t->get_user_id(), t->get_thread_id());
    }
}

void
ThreadTest3()
{
    DEBUG('t', "Entering Lab1 Test3:");
    Thread *t1 = new Thread("thread 1");
    Thread *t2 = new Thread("thread 2");
    t2->setStatus(RUNNING);
    Thread *t3 = new Thread("thread 3");
    Thread *t4 = new Thread("thread 4");
    t4->setStatus(BLOCKED);


    TS();
}

//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    switch (testnum) {
    case 1:
	ThreadTest1();
	break;
    //modify lab1
    case 2:
	ThreadTest2();
	break;
    case 3:
	ThreadTest3();
	break;

    default:
	printf("No test specified.\n");
	break;
    }
}


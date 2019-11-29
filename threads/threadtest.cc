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
#include "synch.h"

// testnum is set in main.cc
int testnum = 100;

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
    DEBUG('t', "Entering Lab1 Test2:");

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

void
fork_func4(int mm)
{
    printf("*** %d,thread name = %s , TID = %d, priority = %d\n",mm, currentThread->getName(),currentThread->get_thread_id(),currentThread->get_pri());
    scheduler->Print();
    printf("\n\n");
    currentThread->Yield();
}

void
ThreadTest4()
{
    DEBUG('t', "Entering Lab1 Test4:");
    Thread * thigh = new Thread("high thread",80);
    Thread * tlow = new Thread("low thread",20);
    printf("***before Yield***");
    scheduler->Print();
    printf("\n\n***start YIELD***\n\n");
    thigh->Fork(fork_func4,(void*)0);
    tlow->Fork(fork_func4,(void*)0);
    fork_func4(0);
}

void
fork_func5(int mm)//also print message in thread.cc
{
    //printf("*** %d,thread name = %s , TID = %d, rm_time = %d\n",mm, currentThread->getName(),currentThread->get_thread_id(),currentThread->get_rmtime());
    for(int i = 0;i < 100;++i)
    {
        interrupt->OneTick();//move 10ticks
        printf("%d",currentThread->get_thread_id());
    }
    printf("\n");
    currentThread->Finish();
}
void
ThreadTest5()
{
    DEBUG('t', "Entering Lab1 Test5:");
    Thread * t1 = new Thread("no2");
    Thread * t2 = new Thread("no3");
    t1->Fork(fork_func5,(void*)0);
    t2->Fork(fork_func5,(void*)0);
    fork_func5(0);
}

Lock lock_6("test_6");
void func6(int dummy)
{
    lock_6.Acquire();
    for(int i=0;i<100;++i)
    {
        interrupt->OneTick();
        printf("%d",dummy);
    }
    lock_6.Release();
    currentThread->Finish();
}
void
ThreadTest6()
{
    DEBUG('t', "Entering Lab1 Test7:");
    Thread * t1 = new Thread("no2");
    t1->Fork(func6,(void*)2);
    lock_6.Acquire();
    for(int i=0;i<100;++i)
    {
        interrupt->OneTick();
        printf("%d",1);
    }
    lock_6.Release();
}
Lock* lock_7 = new Lock("lock7");
Condition* cod = new Condition("codition7");
void func7(int dummy)
{
    lock_7->Acquire();
    printf("***in thread pid=%d, it is waiting\n",currentThread->get_thread_id());
    cod->Wait(lock_7);
    currentThread->Yield();
    printf("***in thread pid=%d, waken by broadcast\n",currentThread->get_thread_id());
    lock_7->Release();
    currentThread->Finish();
}
void ThreadTest_lab3_2()
{
    DEBUG('t', "Entering Lab1 Test7:");
    currentThread->set_pri(20);
    Thread * t1 = new Thread("no1",0);
    Thread * t2 = new Thread("no2",0);
    t1->Fork(func7,(void*)2);
    t2->Fork(func7,(void*)2);
    
    currentThread->Yield();
    printf("***in main, broadcast\n");
    lock_7->Acquire();
    cod->Broadcast(lock_7);
    lock_7->Release();
}

const int buff_size = 10;
bool buffer[buff_size];
Semaphore *empty = new Semaphore("empty",0);
Semaphore *full = new Semaphore("full",buff_size);
Semaphore *buff_used = new Semaphore("buff",1);

void producer_sema(int dummy)
{
    //printf("enter %s\n",currentThread->getName());
    for(int i=0;i<50;++i)//should be while true
    {
        full->P();
        buff_used->P();
        for(int i=0;i<buff_size;++i)
        {
            if(!buffer[i])
            {
                buffer[i]=true;
                printf("***thread \"%s\" produces %d\n",currentThread->getName(),i);
                break;
            }
        }
        buff_used->V();
        empty->V();
        interrupt->OneTick();
    }
}
void consumer_sema(int dummy)
{
    //printf("enter %s\n",currentThread->getName());    
    for(int i=0;i<50;++i)//should be while true
    {
        empty->P();
        buff_used->P();
        for(int i=0;i<buff_size;++i)
        {
            if(buffer[i])
            {
                buffer[i]=false;
                printf("***thread \"%s\" consumes %d\n",currentThread->getName(),i);
                break;
            }
        }
        buff_used->V();
        full->V();
        interrupt->OneTick();
    }
}

void test_PS_1()
{
    DEBUG('t', "Entering Lab1 Test_lab3_PS1:");
    currentThread->set_pri(20);
    memset(buffer,0,sizeof(buffer));
    Thread * p1 = new Thread("producer1",0);
    Thread * p2 = new Thread("prodrcer2",0);
    Thread * p3 = new Thread("prodrcer3",0);
    Thread * c1 = new Thread("comsumer1",0);
    Thread * c2 = new Thread("comsumer2",0);
    p1->Fork(producer_sema,(void*)2);
    p2->Fork(producer_sema,(void*)2);
    p3->Fork(producer_sema,(void*)2);
    c1->Fork(consumer_sema,(void*)1);
    c2->Fork(consumer_sema,(void*)1);
    printf("leave main\n");
    currentThread->Yield();
}

Lock * lock_ = new Lock("inner");
Condition * cond = new Condition("cond_PS");
int item_size = 0;
int idx = 0;
void producer_cond(int dummy)
{
    for(int i=0;i<50;++i)
    {
        lock_->Acquire();
        while(item_size == buff_size)
            cond->Wait(lock_);
        ++item_size;
        buffer[idx] = true;
        printf("***thread \"%s\" produces %d\n",currentThread->getName(),idx);
        idx = (idx+1)%buff_size;
        cond->Signal(lock_);
        lock_->Release();
        interrupt->OneTick();
    }
}

void consumer_cond(int dummy)
{
    for(int i=0;i<50;++i)
    {
        lock_->Acquire();
        while(item_size == 0)
            cond->Wait(lock_);
        --item_size;
        buffer[idx] = false;
        printf("***thread \"%s\" consumes %d\n",currentThread->getName(),idx);
        idx = (idx+1)%buff_size;
        cond->Signal(lock_);
        lock_->Release();
        interrupt->OneTick();
    }
}

void test_PS_2()
{
    DEBUG('t', "Entering Lab1 Test_lab3_PS2:");
    currentThread->set_pri(20);
    memset(buffer,0,sizeof(buffer));
    Thread * p1 = new Thread("producer1",0);
    Thread * p2 = new Thread("prodrcer2",0);
    Thread * p3 = new Thread("prodrcer3",0);
    Thread * c1 = new Thread("comsumer1",0);
    Thread * c2 = new Thread("comsumer2",0);
    p1->Fork(producer_cond,(void*)2);
    p2->Fork(producer_cond,(void*)2);
    p3->Fork(producer_cond,(void*)2);
    c1->Fork(consumer_cond,(void*)1);
    c2->Fork(consumer_cond,(void*)1);
    printf("leave main\n");
    currentThread->Yield();
}

Barrier * barier = new Barrier("barrier",4);
void func_barrier(int dummy)
{
    printf("***Enter thread pid = %d,name = %s\n",
        currentThread->get_thread_id(),currentThread->getName());
    interrupt->OneTick();
    barier->MergePoint();
    printf("***Enter thread pid = %d,name = %s reachs barrier point\n",
        currentThread->get_thread_id(),currentThread->getName());
    currentThread->Finish();
}
void test_barrier()
{
    DEBUG('t', "Entering Lab1 Test_lab3_barrier:");
    Thread * t1 = new Thread("t1",0);
    Thread * t2 = new Thread("t2",0);
    Thread * t3 = new Thread("t3",0);
    Thread * t4 = new Thread("t4",0);
    t1->Fork(func_barrier,(void*)0);
    t2->Fork(func_barrier,(void*)0);
    t3->Fork(func_barrier,(void*)0);
    t4->Fork(func_barrier,(void*)0);
    currentThread->Yield();
}

void foo(int num)
{
    for(int i=0;i<100;++i)
    {
        printf("Im %s\n" , currentThread->getName());
        interrupt->OneTick();
    }
}
void quiz_1()
{
    Thread *t = new Thread("kitty");
    t->Fork(foo,(void*)1);
    Thread *t2 = new Thread("world");
    t2->Fork(foo,(void*)1);
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
    case 4:
	ThreadTest4();
	break;
    case 5:
	ThreadTest5();
	break;
    case 6:
	ThreadTest6();
	break;
    case 7:
	ThreadTest_lab3_2();
	break;
    case 8:
	test_PS_1();
	break;
    case 9:
	test_PS_2();
	break;
    case 11:
	test_barrier();
	break;

    default:
	printf("No test specified.\n");
	break;
    }
}


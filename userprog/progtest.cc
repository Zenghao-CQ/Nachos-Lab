// progtest.cc 
//	Test routines for demonstrating that Nachos can load
//	a user program and execute it.  
//
//	Also, routines for testing the Console hardware device.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "console.h"
#include "addrspace.h"
#include "synch.h"

//----------------------------------------------------------------------
// StartProcess
// 	Run a user program.  Open the executable, load it into
//	memory, and jump to it.
//----------------------------------------------------------------------

void
StartProcess(char *filename)
{
    OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    space = new AddrSpace(executable); 
    //modify lab4 exe6
    space->filename = filename;   
    currentThread->space = space;

    delete executable;			// close file

    space->InitRegisters();		// set the initial register values
    space->RestoreState();		// load page table register

    machine->Run();			// jump to the user progam
    ASSERT(FALSE);			// machine->Run never returns;
					// the address space exits
					// by doing the syscall "exit"
}
void innerFork(int i)
{
    printf("in thread: name = %s\n",currentThread->getName());
    machine->Run();
    ASSERT(FALSE);
}
void
StartTwoProcess(char *filename)
{
    OpenFile *executable1 = fileSystem->Open(filename);
    OpenFile *executable2 = fileSystem->Open(filename);
    AddrSpace *space1;
    AddrSpace *space2;
    
    if (executable1 == NULL) {
	printf("Unable to open file %s\n", filename);
	return;
    }
    printf("load addr t1\n");
    space1 = new AddrSpace(executable1);
    printf("load addr t2\n");   
    space2 = new AddrSpace(executable2); 
    
    Thread* t1 = new Thread("SubThread1",0,8);
    t1->space = space1;
    delete executable1;			// close file
    space1->InitRegisters();		// set the initial register values
    space1->RestoreState();		// load page table register
    space1->filename = filename;
    
    Thread* t2 = new Thread("SubThread2",0,8);
    t2->space=space2;
    delete executable2;			// close file
    space2->InitRegisters();		// set the initial register values
    space2->RestoreState();		// load page table register
    space2->filename = filename;
    
    t1->Fork(innerFork,(void*)0);
    t2->Fork(innerFork,(void*)0);
}

// Data structures needed for the console test.  Threads making
// I/O requests wait on a Semaphore to delay until the I/O completes.

static Console *console;
static Semaphore *readAvail;
static Semaphore *writeDone;

//----------------------------------------------------------------------
// ConsoleInterruptHandlers
// 	Wake up the thread that requested the I/O.
//----------------------------------------------------------------------

static void ReadAvail(int arg) { readAvail->V(); }
static void WriteDone(int arg) { writeDone->V(); }

//----------------------------------------------------------------------
// ConsoleTest
// 	Test the console by echoing characters typed at the input onto
//	the output.  Stop when the user types a 'q'.
//----------------------------------------------------------------------

void 
ConsoleTest_old (char *in, char *out)
{
    char ch;

    console = new Console(in, out, ReadAvail, WriteDone, 0);
    readAvail = new Semaphore("read avail", 0);
    writeDone = new Semaphore("write done", 0);
    
    for (;;) {
	readAvail->P();		// wait for character to arrive
	ch = console->GetChar();
	console->PutChar(ch);	// echo it!
	writeDone->P() ;        // wait for write to finish
	if (ch == 'q') return;  // if q, quit
    }
}

static SynchConsole *sy_Console;

void
ConsoleTest (char *in, char *out)
{
    char ch;

    sy_Console = new SynchConsole(in, out);

    while(true) 
    {
        ch = sy_Console->GetChar();
        sy_Console->PutChar(ch); // echo it!
        if (ch == 'q')
            return; // if q, quit
    }
}
// exception.cc 
//	Entry point into the Nachos kernel from user programs.
//	There are two kinds of things that can cause control to
//	transfer back to here from user code:
//
//	syscall -- The user code explicitly requests to call a procedure
//	in the Nachos kernel.  Right now, the only function we support is
//	"Halt".
//
//	exceptions -- The user code does something that the CPU can't handle.
//	For instance, accessing memory that doesn't exist, arithmetic errors,
//	etc.  
//
//	Interrupts (which can also cause control to transfer from user
//	code into the Nachos kernel) are handled elsewhere.
//
// For now, this only handles the Halt() system call.
// Everything else core dumps.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "noff.h"

//----------------------------------------------------------------------
// ExceptionHandler
// 	Entry point into the Nachos kernel.  Called when a user program
//	is executing, and either does a syscall, or generates an addressing
//	or arithmetic exception.
//
// 	For system calls, the following is the calling convention:
//
// 	system call code -- r2
//		arg1 -- r4
//		arg2 -- r5
//		arg3 -- r6
//		arg4 -- r7
//
//	The result of the system call, if any, must be put back into r2. 
//
// And don't forget to increment the pc before returning. (Or else you'll
// loop making the same system call forever!
//
//	"which" is the kind of exception.  The list of possible exceptions 
//	are in machine.h.
//----------------------------------------------------------------------

unsigned int next_tlb = -1;

#ifdef LRU
int LRU_next()
{
    int tempvpn;
    int minticks = __INT_MAX__;
    for(int i = 0; i < TLBSize; ++i)
    {
        if(machine->tlb[i].valid == false)
        {
            tempvpn = i;
            return tempvpn;
        }
        if(machine->tlb[i].last_ticks < minticks)
        {
            tempvpn = i;
            minticks = machine->tlb[i].last_ticks;
        }
    }
    return tempvpn;
}
#endif

void SwapHeader (NoffHeader *noffH)
{
	noffH->noffMagic = WordToHost(noffH->noffMagic);
	noffH->code.size = WordToHost(noffH->code.size);
	noffH->code.virtualAddr = WordToHost(noffH->code.virtualAddr);
	noffH->code.inFileAddr = WordToHost(noffH->code.inFileAddr);
	noffH->initData.size = WordToHost(noffH->initData.size);
	noffH->initData.virtualAddr = WordToHost(noffH->initData.virtualAddr);
	noffH->initData.inFileAddr = WordToHost(noffH->initData.inFileAddr);
	noffH->uninitData.size = WordToHost(noffH->uninitData.size);
	noffH->uninitData.virtualAddr = WordToHost(noffH->uninitData.virtualAddr);
	noffH->uninitData.inFileAddr = WordToHost(noffH->uninitData.inFileAddr);
}

void PCForward()
{
    machine->WriteRegister(PrevPCReg, machine->ReadRegister(PCReg));
    machine->WriteRegister(PCReg, machine->ReadRegister(NextPCReg));
    machine->WriteRegister(NextPCReg, machine->ReadRegister(NextPCReg)+sizeof(int));//it seems not need        
}

bool readStr(int addr, char* into)
{
    int pos = 0;
    int data;
    do 
    {
        bool success = machine->ReadMem(addr + pos, 1, &data);
        if(!success) return FALSE;
        into[pos++] = (char)data;
    } while(data != '\0');
    into[pos] = '\0';
    return true;
}

void
ExceptionHandler(ExceptionType which)
{
    int type = machine->ReadRegister(2);

    if ((which == SyscallException) && (type == SC_Halt)) 
    {
    	DEBUG('a', "Shutdown, initiated by user program.\n");
   	    interrupt->Halt();
    }
    else if((which == SyscallException) && (type == SC_Create))
    {
        printf("Syscall Create:");
        int addr = machine->ReadRegister(4);
        char filename[10];
        ASSERT(readStr(addr,filename));
        ASSERT(fileSystem->Create(filename,30));
        PCForward();
        printf(" filename:%s success\n",filename);
    }
    else if((which == SyscallException) && (type == SC_Open))
    {
        printf("Syscall Open");
        int addr = machine->ReadRegister(4);
        char filename[10];
        ASSERT(readStr(addr,filename));
        OpenFile* tmp = fileSystem->Open(filename);
        machine->WriteRegister(2,int(tmp));
        PCForward();
        printf(" filename:%s success\n",filename);
    }
    else if((which == SyscallException) && (type == SC_Close))
    {
        printf("Syscall Close");
        int fileId = machine->ReadRegister(4);
        OpenFile* tmp = (OpenFile*)fileId;
        delete tmp;
        PCForward();
        printf(" success\n");
    }
    else if((which == SyscallException) && (type == SC_Read))
    {
        printf("Syscall Read");
        int addr = machine->ReadRegister(4);
        int size = machine->ReadRegister(5);
        int fileId = machine->ReadRegister(6);
        OpenFile* fileIn = (OpenFile*) fileId;
        char buffer[size];
        int numBytes = fileIn->Read(buffer,size);
        for(int i = 0; i < numBytes; ++i)
            machine->WriteMem(addr + i, 1, (int)(buffer[i]));
        machine->WriteRegister(2,numBytes);
        PCForward();
        printf(" success\n");
    }
    else if((which == SyscallException) && (type == SC_Write))
    {
        printf("Syscall write");
        int addr = machine->ReadRegister(4);
        int size = machine->ReadRegister(5);
        int fileId = machine->ReadRegister(6);
        OpenFile* FileOut = (OpenFile*) fileId;
        char *buffer = new char[size];
        int temp;
        for(int i = 0; i < size; ++i)
        {
            machine->ReadMem(addr + i, 1, &temp);
            buffer[i] =temp;
        }
        FileOut->Write(buffer,size);
        PCForward();
        printf(" success\n");
    }
    else if ((which == SyscallException) && (type == SC_Exit))
    {
        printf("Thread id=%d Syscall exit\n",currentThread->get_thread_id());
        DEBUG('a',"syscall Exit\n");
        #ifdef USER_PROGRAM
        #ifdef USE_TLB
		printf("##TLB hit count:%d\n",hitcnt);
		#endif
        PCForward();
        machine->freePhyPage();
        currentThread->Finish();//multi threads
        #endif
    }
    else if(which == SyscallException)
    {
        printf("SyscallException type = %d is unhandled\n",type);
    }
    else if(which == PageFaultException) 
    {
        int BadVAddr = machine->ReadRegister(BadVAddrReg); // perhaps it should be unsighed_int
        unsigned int vpn = (unsigned int)((unsigned int)BadVAddr / PageSize) ;
            
        if (machine->tlb != NULL)//acctually a tlb fault
        { 
            DEBUG('a', "Pagefault handler: TLB miss, find a new one\n");
            TranslationEntry PhysPage = machine->pageTable[vpn];
            #ifdef FIFO
            next_tlb = (next_tlb+1)%TLBSize; //FIFO
            machine->tlb[next_tlb] = PhysPage;
            //printf("tlb %d\n",next_tlb);
            #endif 
            #ifdef LRU
            next_tlb = LRU_next();
            machine->tlb[next_tlb] = PhysPage;
            machine->tlb[next_tlb].last_ticks = stats -> userTicks;
            #endif
            #ifdef USE_TLB
            hitcnt++;
            #endif
            return;
        } 
        else //not in memoty
        { 
            DEBUG('m', "Pagefault handler: Page not in main memory.\n");
            char *filename = currentThread->space->filename;
            printf("thread: %d\n",currentThread->get_thread_id());
            
            char* switch_file_name = "woshale";
            OpenFile * switch_file = fileSystem->Open(switch_file_name);
            ASSERT(switch_file != NULL);            
            //int vpn = machine->registers[BadVAddrReg]/PageSize;
            int idx = machine->allocatePhyPage();                        
            if(idx == -1)//main memory is full
            {
                printf("in write back\n");
                for(int i = 0; i < machine->pageTableSize; ++i)
                {
                    //if(machine->pageTable[i].physicalPage == 0)//remove first page which ppn == 0,if(vpn==vpn){...}
                    if(i == vpn)    //mainMemory[ppn] = file[vpn]
                    {
                        TranslationEntry *entry = &(machine->pageTable[i]);
                        idx = entry->physicalPage;//phy page = 0 of course
                        if(entry->dirty)
                        {
                            printf("pagedault:write back from ppn:%d to vpn:%d\n",idx,vpn);
                            switch_file->WriteAt(&(machine->mainMemory[idx*PageSize]),
                                                    PageSize, entry->virtualPage*PageSize);//remove vpn==vpn
                            entry->valid = FALSE;
                        }
                        break;
                    }
                }
            }
            printf("pagefault:virtualaddr=%d load vpn=%d to ppn:%d\n",machine->registers[BadVAddrReg],vpn,idx);
            switch_file->ReadAt(&(machine->mainMemory[idx*PageSize]), PageSize, vpn*PageSize);
            #ifndef INVERSE_TABLE
            machine->pageTable[vpn].virtualPage = vpn;
            machine->pageTable[vpn].physicalPage = idx;
            machine->pageTable[vpn].valid = TRUE;
            machine->pageTable[vpn].dirty = FALSE;
            machine->pageTable[vpn].use = FALSE;
            machine->pageTable[vpn].readOnly = FALSE;
            #else 
            machine->pageTable[idx].virtualPage = vpn;
            machine->pageTable[idx].physicalPage = idx;
            machine->pageTable[idx].valid = TRUE;
            machine->pageTable[idx].dirty = FALSE;
            machine->pageTable[idx].use = FALSE;
            machine->pageTable[idx].readOnly = FALSE;
            machine->pageTable[idx].tid = currentThread->get_thread_id();
            #endif
            delete switch_file;
        }
        
        return;
    }
    else {
	printf("Unexpected user mode exception %d %d\n", which, type);
	ASSERT(FALSE);
    }
}

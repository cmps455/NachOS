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

#include <stdio.h>        // FA98
#include "copyright.h"
#include "system.h"
#include "syscall.h"
#include "addrspace.h"   // FA98
#include "sysdep.h"   // FA98

// begin FA98

static int SRead(int addr, int size, int id);
static void SWrite(char *buffer, int size, int id);

// end FA98

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

void SExit(int status) {

	if(currentThread->space) delete currentThread->space;
	currentThread->space = 0;

	printf("\nProcess %d exited %snormally(%d): ", currentThread->myID, status ? "ab" : "", status);

	switch(status) {
		case 0: 
			break;
		case 1: printf("NumExceptionTypes");
			break;
		case 2: printf("IllegalInstrException");
			break;
		case 3: printf("OverflowException");
			break;
		case 4: printf("AddressErrorException");
			break;
		case 5: printf("BusErrorException");
			break;
		case 6: printf("ReadOnlyException");
			break;
		case 7: printf("Write 0 byte");
			break;
		case 8: printf("No Contiguous Allocation");
			break;
		case 9: printf("Program file is not an executable file");
			break;
		default: printf("UnknownExceptionType");
			break;
	}
	
	currentThread->Finish();
}

void processCreator(int arg) {
	//Benjamin: ensuring nachos updates important variables
	currentThread->Yield();

	char* filename = (char*)arg;
	printf("\nCreating filename: %s, processID: %d", filename, currentThread->myID);
	OpenFile *executable = fileSystem->Open(filename);
    AddrSpace *space;

    if (executable == NULL) {
		printf("Unable to open file %s\n", filename);
		return;
    }
	
	space = new AddrSpace(executable);
    if(0 != space->code) {
		SExit(space->code);
		return;
	}

	currentThread->space = space;

    delete executable;			// close file
	delete [] filename;

	currentThread->space->InitRegisters();
	currentThread->space->RestoreState();//load page table register
	machine->Run();

	ASSERT(FALSE);
}

void
ExceptionHandler(ExceptionType which)
{
	int type = machine->ReadRegister(2);

	int arg1 = machine->ReadRegister(4);
	int arg2 = machine->ReadRegister(5);
	int arg3 = machine->ReadRegister(6);
	int Result;
	int i, j;
	char *ch = new char [500];

	switch ( which )
	{
	case NoException :
		break;
	case SyscallException :

//for debugging, in case we are jumping into lala-land
//Advance program counters.
		machine->registers[PrevPCReg] = machine->registers[PCReg];
		machine->registers[PCReg] = machine->registers[NextPCReg];
		machine->registers[NextPCReg] = machine->registers[NextPCReg] + 4;

		switch(type)
		{
				
			case SC_Yield:
				printf("\nYielding: [%d]", currentThread->myID);
				currentThread->Yield();
				break;

			case SC_Exit:
				{
					printf("\nExit: [%d]", currentThread->myID);
					IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts

//wake up our parent if he's there
					Thread *parent = currentThread->GetThread(currentThread->parentID);
					if(parent) {
						scheduler->ReadyToRun(parent);
					}
					(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
					SExit(0);
					
				}
				break;

			case SC_Join:
				{
					printf("\nJoining: [%d]", currentThread->myID);
//turn interrupts off
					IntStatus oldLevel = interrupt->SetLevel(IntOff);	// disable interrupts
					
					Thread *child = currentThread->GetThread(machine->ReadRegister(4));
					if(child) {
						printf("\nChild(%d) found, going to sleep", child->myID);
						child->parentID = currentThread->myID;
						currentThread->Sleep();
						printf("\nParent(%d) awakened", currentThread->myID);
					}
					else {
						printf("\nChild unfound, not going to sleep");
					}
					(void) interrupt->SetLevel(oldLevel);	// re-enable interrupts
//turn interrupts on
					
				}
				break;
		
			case SC_Exec:
				{
					printf("\nExec: [%d]", currentThread->myID);
//turn interrupts off
					IntStatus oldLevel = interrupt->SetLevel(IntOff);
					
					Thread *child = new Thread("Exec");

					machine->WriteRegister(2, child->myID);


//find a proper place to free this allocation
					char* filename = new char[100];
/* only one argument, so that’s in R4 */
					int buffadd = machine->ReadRegister(4);
					int readByte;
					if(!machine->ReadMem(buffadd,1,&readByte))
						return;
					i = 0;
					while( readByte!=0 ) {
						filename[i] = (char)readByte;
						buffadd += 1;
						i++;
						if(!machine->ReadMem(buffadd,1,&readByte))
							break;//returns but idk if i want program to return
					}
					filename[i] = 0;
					printf(filename);
/* now filename contains the file */ 

					child->Fork(processCreator, (int)filename);

					/*int pc = machine->ReadRegister(PCReg);
					machine->WriteRegister(PrevPCReg,pc);
					pc = machine->ReadRegister(NextPCReg);
					machine->WriteRegister(PCReg,pc);
					pc += 4;
					machine->WriteRegister(NextPCReg,pc);*/
					
					//turn interrupts on
					(void) interrupt->SetLevel(oldLevel);
				}
				break;
		case SC_Halt :
			DEBUG('t', "Shutdown, initiated by user program.\n");
			interrupt->Halt();
			break;

			
		case SC_Read :
			if (arg2 <= 0 || arg3 < 0){
				printf("\nRead 0 byte.\n");
			}
			Result = SRead(arg1, arg2, arg3);
			machine->WriteRegister(2, Result);
			DEBUG('t',"Read %d bytes from the open file(OpenFileId is %d)",
			arg2, arg3);
			break;

		case SC_Write :
			for (j = 0; ; j++) {
				if(!machine->ReadMem((arg1+j), 1, &i))
					j=j-1;
				else{
					ch[j] = (char) i;
					if (ch[j] == '\0') 
						break;
				}
			}
			if (j == 0){
				printf("\nWrite 0 byte.\n");
				SExit(7);
			} else {
				DEBUG('t', "\nWrite %d bytes from %s to the open file(OpenFileId is %d).", arg2, ch, arg3);
				SWrite(ch, j, arg3);
			}
			break;

			default :
			//Unprogrammed system calls end up here

			printf("\nUnprogrammed system call arg1: %d", arg1);

			break;
		}         
		break;

	case ReadOnlyException :
		puts ("ReadOnlyException");
		if (currentThread->getName() == "main")
		
		SExit(6);
		break;
	case BusErrorException :
		puts ("BusErrorException");
		if (currentThread->getName() == "main")
		
		SExit(5);
		break;
	case AddressErrorException :
		puts ("AddressErrorException");
		if (currentThread->getName() == "main")
		
		SExit(4);
		break;
	case OverflowException :
		puts ("OverflowException");
		if (currentThread->getName() == "main")
		
		SExit(3);
		break;
	case IllegalInstrException :
		puts ("IllegalInstrException");
		if (currentThread->getName() == "main")
		
		SExit(2);
		break;
	case NumExceptionTypes :
		puts ("NumExceptionTypes");
		if (currentThread->getName() == "main")
		
		SExit(1);
		break;

		default :
		      printf("\nUnexpected user mode exception %d %d\n", which, type);
		//      if (currentThread->getName() == "main")
		//      ASSERT(FALSE);
		      SExit(-1);
		break;
	}
	delete [] ch;
}


static int SRead(int addr, int size, int id)  //input 0  output 1
{
	char buffer[size+10];
	int num,Result;

	//read from keyboard, try writing your own code using console class.
	if (id == 0)
	{
		scanf("%s",buffer);

		num=strlen(buffer);
		if(num>(size+1)) {

			buffer[size+1] = '\0';
			Result = size+1;
		}
		else {
			buffer[num+1]='\0';
			Result = num + 1;
		}

		for (num=0; num<Result; num++)
		{  machine->WriteMem((addr+num), 1, (int) buffer[num]);
			if (buffer[num] == '\0')
			break; }
		return num;

	}
	//read from a unix file, later you need change to nachos file system.
	else
	{
		for(num=0;num<size;num++){
			Read(id,&buffer[num],1);
			machine->WriteMem((addr+num), 1, (int) buffer[num]);
			if(buffer[num]=='\0') break;
		}
		return num;
	}
}



static void SWrite(char *buffer, int size, int id)
{
	//write to terminal, try writting your own code using console class.
	if (id == 1)
	printf("%s", buffer);
	//write to a unix file, later you need change to nachos file system.
	if (id >= 2)
	WriteFile(id,buffer,size);
}

// end FA98


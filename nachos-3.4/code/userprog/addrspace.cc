// addrspace.cc 
//	Routines to manage address spaces (executing user programs).
//
//	In order to run a user program, you must:
//
//	1. link with the -N -T 0 option 
//	2. run coff2noff to convert the object file to Nachos format
//		(Nachos object code format is essentially just a simpler
//		version of the UNIX executable object code format)
//	3. load the NOFF file into the Nachos file system
//		(if you haven't implemented the file system yet, you
//		don't need to do this last step)
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation 
// of liability and disclaimer of warranty provisions.

#include "copyright.h"
#include "system.h"
#include "addrspace.h"
#include "synch.h"
#include "noff.h"
#ifdef HOST_SPARC
#include <strings.h>
#endif
#include "bitmap.h"
//----------------------------------------------------------------------
// SwapHeader
// 	Do little endian to big endian conversion on the bytes in the 
//	object file header, in case the file was generated on a little
//	endian machine, and we're now running on a big endian machine.
//----------------------------------------------------------------------



static void 
SwapHeader (NoffHeader *noffH)
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

//----------------------------------------------------------------------
// AddrSpace::AddrSpace
// 	Create an address space to run a user program.
//	Load the program from a file "executable", and set everything
//	up so that we can start executing user instructions.
//
//	Assumes that the object code file is in NOFF format.
//
//	First, set up the translation from program memory to physical 
//	memory.  For now, this is really simple (1:1), since we are
//	only uniprogramming, and we have a single unsegmented page table
//
//	"executable" is the file containing the object code to load into memory
//----------------------------------------------------------------------



//main memory bitmap
static BitMap mainMemoryBits(32);
static Semaphore mainMemorySemaphore("Main Memory Semaphore", 1);
extern int fitFunction;

/*Begin changes by Jaquincy Nelson*/

int BestFit(int frames) {
        int freeSpace = 0;
        int minFreeSpace = 32;
        int minIndex = 31;
		int i = 0;

        for(; i < 32; i++){
                if(!mainMemoryBits.Test(i)){
                        freeSpace++;
                }else{
                        if(minFreeSpace > freeSpace && freeSpace >= frames){
                                minFreeSpace = freeSpace;
                                minIndex = i;
                        }
                        freeSpace =0;
                }
        }
		if(minFreeSpace > freeSpace && freeSpace >= frames){
			minFreeSpace = freeSpace;
 			minIndex = i - 2;
        }
        if(minFreeSpace >= frames){
                return(minIndex - minFreeSpace + 1);
		}
        return -1;
}

int WorstFit(int frames) {
        int freeSpace = 0;
        int maxFreeSpace = 0;
        int maxIndex = 0;
		int i = 0;

        for(; i < 32; i++){
                if(!mainMemoryBits.Test(i)){
                        freeSpace++;
                }else{
                        if(maxFreeSpace < freeSpace){
                                maxFreeSpace = freeSpace;
                                maxIndex = i;
                        }
                        freeSpace =0;
                }
        }
        if(maxFreeSpace < freeSpace){
               maxFreeSpace = freeSpace;
               maxIndex = i - 2;
        }
        if(maxFreeSpace >= frames)
                return(maxIndex - maxFreeSpace + 1);
        return -1;
}

int FirstFit(int frames) {
        int freeSpace = 0;
        for( int i = 0; i < 32; i++){
                if(!mainMemoryBits.Test(i)){
                        freeSpace++;
                }else{
                        freeSpace =0;
                }
                if(freeSpace >= frames)
                        return(i - freeSpace + 1);
        }
        return -1;
}

/*Begin changes by Ian Callaway*/

AddrSpace::AddrSpace(OpenFile *executable)
{
	mainMemorySemaphore.P();
//idk what this does
    NoffHeader noffH;
	
//i is something,
//size is like all the memory in bytes we're allocating for this
    unsigned int size;

//ignore this wizardry
    executable->ReadAt((char *)&noffH, sizeof(noffH), 0);
    if ((noffH.noffMagic != NOFFMAGIC) && 
		(WordToHost(noffH.noffMagic) == NOFFMAGIC))
    	SwapHeader(&noffH);
    if(noffH.noffMagic != NOFFMAGIC) {
		printf("\nFilename is not an executable file\n");
		pageTable = 0;
		code = 9;
		return;
	}

//this is the address space in bytes
    size =
//size of the code that we have for some reason
		noffH.code.size
//size of the data that we have for some readon
		+ noffH.initData.size
//size of idk
		+ noffH.uninitData.size
//size of the stack
		+ UserStackSize;

//the address space will be divided into pages
    numPages = divRoundUp(size, PageSize);

	printf("\nNeed %d memory frames.", numPages);

//size is the address space in bytes but rounded up to a page
    size = numPages * PageSize;

//if there's not enough memory we kill this
    ASSERT(numPages <= NumPhysPages);

    DEBUG('a', "Initializing address space, num pages %d, size %d\n", numPages, size);
	
	printf("\nMain[]  (pre-op): ");
	for(int i = 0; i < 32; i++)
		printf("%c ", mainMemoryBits.Test(i) ? '1' : '0');

	int offset = fitFunction == 0 ? FirstFit(numPages) : fitFunction == 1 ? BestFit(numPages) : fitFunction == 2 ? WorstFit(numPages) : 0;
	code = offset >= 0 && offset < 32 ? 0 : 8;

// first, set up the translation
    pageTable = new TranslationEntry[numPages];
	for (int i = 0; i < (signed)numPages; i++) {
		if(code == 0) mainMemoryBits.Mark(offset + i);
		pageTable[i].virtualPage = i;	// for now, virtual page # = phys page #
		pageTable[i].physicalPage = i + offset;
		pageTable[i].valid = code == 0 ? TRUE : FALSE;
		pageTable[i].use = FALSE;
		pageTable[i].dirty = FALSE;
		pageTable[i].readOnly = FALSE;  // if the code segment was entirely on 
					// a separate page, we could set its 
					// pages to be read-only
    }
	
	if(code != 0) {
		printf("\nNot enough contiguous space.");
		printf("\nMain[]: ");
		for(int i = 0; i < 32; i++)
			printf("%c ", mainMemoryBits.Test(i) ? '1' : '0');
		return;
	}
    
// zero out the entire address space, to zero the unitialized data segment 
// and the stack segment
	printf("\n%d frames found at frame: %d", numPages, offset);
	printf("\nCleaning out address space");
    bzero(machine->mainMemory + (offset * PageSize), size);

// then, copy in the code and data segments into memory
    if (noffH.code.size > 0) {
        DEBUG('a', "Initializing code segment, at 0x%x, size %d\n", 
			noffH.code.virtualAddr, noffH.code.size);
        executable->ReadAt(&(machine->mainMemory[noffH.code.virtualAddr + (offset * PageSize)]),
			noffH.code.size, noffH.code.inFileAddr);
    }
    if (noffH.initData.size > 0) {
        DEBUG('a', "Initializing data segment, at 0x%x, size %d\n", 
			noffH.initData.virtualAddr, noffH.initData.size);
        executable->ReadAt(&(machine->mainMemory[noffH.initData.virtualAddr + (offset * PageSize)]),
			noffH.initData.size, noffH.initData.inFileAddr);
    }
	printf("\nMain[] (post-op): ");
	for(int i = 0; i < 32; i++)
		printf("%c ", mainMemoryBits.Test(i) ? '1' : '0');
	mainMemorySemaphore.V();
}

//----------------------------------------------------------------------
// AddrSpace::~AddrSpace
// 	Dealloate an address space.  Nothing for now!
//----------------------------------------------------------------------

AddrSpace::~AddrSpace()
{
	mainMemorySemaphore.P();
	for(unsigned i = 0; i < numPages; i++) {
		if(code == 0) mainMemoryBits.Clear(pageTable[i].physicalPage);
		pageTable[i].valid = 0;
		pageTable[i].dirty = 0;
	}
	printf("\nMain[]\t(done): ");
	for(int i = 0; i < 32; i++)
		printf("%c ", mainMemoryBits.Test(i) ? '1' : '0');
	delete pageTable;
	mainMemorySemaphore.V();
}

/*End changes by Ian Callaway*/







//----------------------------------------------------------------------
// AddrSpace::InitRegisters
// 	Set the initial values for the user-level register set.
//
// 	We write these directly into the "machine" registers, so
//	that we can immediately jump to user code.  Note that these
//	will be saved/restored into the currentThread->userRegisters
//	when this thread is context switched out.
//----------------------------------------------------------------------

void
AddrSpace::InitRegisters()
{
    int i;

    for (i = 0; i < NumTotalRegs; i++)
	machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of "Start"
    machine->WriteRegister(PCReg, 0);	

    // Need to also tell MIPS where next instruction is, because
    // of branch delay possibility
    machine->WriteRegister(NextPCReg, 4);

   // Set the stack register to the end of the address space, where we
   // allocated the stack; but subtract off a bit, to make sure we don't
   // accidentally reference off the end!
    machine->WriteRegister(StackReg, numPages * PageSize - 16);
    DEBUG('a', "Initializing stack register to %d\n", numPages * PageSize - 16);
}

//----------------------------------------------------------------------
// AddrSpace::SaveState
// 	On a context switch, save any machine state, specific
//	to this address space, that needs saving.
//
//	For now, nothing!
//----------------------------------------------------------------------

void AddrSpace::SaveState() 
{}

//----------------------------------------------------------------------
// AddrSpace::RestoreState
// 	On a context switch, restore the machine state so that
//	this address space can run.
//
//      For now, tell the machine where to find the page table.
//----------------------------------------------------------------------

void AddrSpace::RestoreState() 
{
    machine->pageTable = pageTable;
    machine->pageTableSize = numPages;
}

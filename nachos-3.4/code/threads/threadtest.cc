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
	printf("\n(%d", which);
	printf(";%d)", currentThread->Cycle());
}


//----------------------------------------------------------------------
// ThreadTest
// 	Invoke a test routine.
//----------------------------------------------------------------------

void
ThreadTest()
{
    DEBUG('t', "Entering ThreadTest");
	
    currentThread->Cycle();
   	(new Thread("forked thread"))->Fork(SimpleThread, 1);
   	(new Thread("forked thread"))->Fork(SimpleThread, 2);
   	(new Thread("forked thread"))->Fork(SimpleThread, 4);
   	(new Thread("forked thread"))->Fork(SimpleThread, 5);
   	(new Thread("forked thread"))->Fork(SimpleThread, 6);
	
	
}


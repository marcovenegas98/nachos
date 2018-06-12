// threadtest.cc
//	Simple test case for the threads assignment.
//
//	Create several threads, and have them context switch
//	back and forth between themselves by calling Thread::Yield,
//	to illustrate the inner workings of the thread system.
//
// Copyright (c) 1992-1993 The Regents of the University of California.
// All rights reserved.  See copyright.h for copyright notice and limitation
// of liability and disclaimer of warranty provisions.
//

#include <unistd.h>
#include "utility.h"
#include "synch.h"
#include "copyright.h"
#include "system.h"
#include "dinningph.h"
#include <iostream>

DinningPh * dp;

using namespace std;

void Philo( void * p ) {

    int eats, thinks;
    long who = (long) p;

    currentThread->Yield();

    for ( int i = 0; i < 10; i++ ) {

        printf(" Philosopher %ld will try to pickup sticks\n", who + 1);

        dp->pickup( who );
        dp->print();
        eats = Random() % 6;

        currentThread->Yield();
        sleep( eats );

        dp->putdown( who );

        thinks = Random() % 6;
        currentThread->Yield();
        sleep( thinks );
    }

}





//----------------------------------------------------------------------
// SimpleThread
// 	Loop 10 times, yielding the CPU to another ready thread
//	each iteration.
//
//	"name" points to a string with a thread name, just for
//      debugging purposes.
//----------------------------------------------------------------------

Semaphore * sem;
void
SimpleThread(void* name)
{
    // Reinterpret arg "name" as a string
    char* threadName = (char*)name;

    // If the lines dealing with interrupts are commented,
    // the code will behave incorrectly, because
    // printf execution may cause race conditions.
    for (int num = 0; num < 10; num++) {
        //IntStatus oldLevel = interrupt->SetLevel(IntOff);
	printf("*** El hilo %s repite el ciclo por %d vez\n", threadName, num);
	//interrupt->SetLevel(oldLevel);
        //currentThread->Yield();
    }
    //IntStatus oldLevel = interrupt->SetLevel(IntOff);
    printf(">>> El hilo %s se murió\n", threadName);
    //interrupt->SetLevel(oldLevel);
    sem->V();
}

Semaphore * sH;
Semaphore * sO;
Lock * mutex;
int cO = 0;
int cH = 0;

void H ( void *arg ) {

   int i;

   i = (long) arg;

   mutex->Acquire();		// Lock access to cH and cO variables

   if ( (cH > 0)  && (cO > 0) ) {	// I'm an H, have we the others?
      cH--;	// Using an H
      cO--;	// Using an O
      cout << "Soy un H, hice agua: " << i << endl;
      mutex->Release();		//Free the mutex
      sH->V();		// Free the waiting thread
      sO->V();		// Free the waiting thread
   } else {
      cH++;			// Increment the H count
      cout << "Soy un H, estoy esperando: " << i << endl;
      mutex->Release();		//Free the mutex
      sH->P();		// Wait for the others
   }
}


void O ( void *arg ) {

   int i;

   i = (long) arg;

   mutex->Acquire();	// Lock access to cH and cO variables

   if ( cH > 1 ) {	// I'm an O, have we at least two H's?
      cH-=2;		// We use two H's
      cout << "Soy un O, hice agua: " << i << endl;
      mutex->Release();	//Free the mutex
      sH->V();	// Free the waiting threads
      sH->V();
   } else {
      cO++;		// Increment O count, then wait
      cout << "Soy un O, estoy esperando: " << i << endl;
      mutex->Release();	//Free the mutex
      sH->P();	// Wait for other threads
   }
}

//----------------------------------------------------------------------
// ThreadTest
// 	Set up a ping-pong between several threads, by launching
//	ten threads which call SimpleThread, and finally calling
//	SimpleThread ourselves.
//----------------------------------------------------------------------

void
ThreadTest()
{
    //Thread * Ph;

    DEBUG('t', "Entering SimpleTest");

/*
    dp = new DinningPh();

    for ( long k = 0; k < 5; k++ ) {
        Ph = new Thread( "dp" );
        Ph->Fork( Philo, (void *) k );
    }

    return;
*/
    // sem = new Semaphore("Sem", 0);
    // for ( int k=1; k<=5; k++) {
    //   char* threadname = new char[100];
    //   sprintf(threadname, "Hilo %d", k);
    //   Thread* newThread = new Thread (threadname);
    //   newThread->Fork (SimpleThread, (void*)threadname);
    //   sem->P();
    // }

    sH = new Semaphore("SemH", 0);
    sO = new Semaphore("SemO", 0);
    mutex = new Lock("Mutex");

    for (int i = 0; i < 200; i++ ) {
        Thread * hilo = new Thread("Hilo");
        if ( 0 == ( rand() % 2 ) ){
            hilo->Fork(H, (void*)i);
        }
        else{
            hilo->Fork(O, (void*)i);
        }
    }

}

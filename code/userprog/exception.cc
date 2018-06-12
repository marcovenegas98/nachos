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
#include "synch.h"
#include "TablaNachos.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <iostream>
using namespace std;
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
void NachosHalt(){
    DEBUG('a', "Shutdown, initiated by user program.\n");
    interrupt->Halt();
}

void returnFromSystemCall() {

    int pc, npc;

    pc = machine->ReadRegister(PCReg);
    npc = machine->ReadRegister(NextPCReg);
    machine->WriteRegister(PrevPCReg, pc);        // PrevPC <- PC
    machine->WriteRegister(PCReg, npc);           // PC <- NextPC
    machine->WriteRegister(NextPCReg, npc + 4);   // NextPC <- NextPC + 4

}       // returnFromSystemCall

char * parToCharPtr(int position){
    char * name = new char[128];
    int currentChar = 0;
    int i = 0;
    do{
        machine->ReadMem(position, 1, &currentChar);
        name[i] = (char)currentChar;
        position++;
        i++;
    }while((char)currentChar != '\0');
    return name;
}

void NachosOpen(){
    cout << "Entrando Open" << '\n';
    tablaNachos->Print();
    char * path = parToCharPtr(machine->ReadRegister(4)); //Returns path as char *
    cout << path << '\n';
    int UnixHandle = open(path, O_RDWR);
    if(-1 == UnixHandle){
        cout << "error open\n";
        machine->WriteRegister(2, -1);
    }else{
        int NachosHandle = tablaNachos->Open(UnixHandle);
        cout << NachosHandle << "Nachos\n";
        machine->WriteRegister(2, NachosHandle);
    }
    cout << "Saliendo Open" << '\n';
    returnFromSystemCall();
}

void NachosClose(){
    cout << "Entrando Close" << '\n';
    //OpenFileID id -> Register 4
    int NachosHandle = machine->ReadRegister(4); //Reads the NachosHandle passed
    int UnixHandle = tablaNachos->getUnixHandle(NachosHandle); //Gets the unix handle
    int result = -1;
    if(-1 != UnixHandle){ //It is open.
        result = close(UnixHandle);
        if(-1 != result){
            result = tablaNachos->Close(NachosHandle);
        }
    }
    machine->WriteRegister(4, result);
    cout << "Saliendo Close" << '\n';
    returnFromSystemCall();
}

void NachosCreate(){
    cout << "Entrando Create" << '\n';
    char * path = parToCharPtr(machine->ReadRegister(4)); //Returns path as char *
    int result = creat(path, S_IRWXU); //Create file and open it with read, write and execute rights.
    machine->WriteRegister(4, result);
    cout << "Saliendo create" << '\n';
    returnFromSystemCall();
}

Semaphore * Console = new Semaphore("Sem", 0);
void NachosWrite() {                   // System call 7

/* System call definition described to user
        void Write(
		char *buffer,	// Register 4
		int size,	    // Register 5
		 OpenFileId id	// Register 6
	);
*/
    cout << "Entrando Write" << '\n';
    char * buffer = NULL;
    int size = machine->ReadRegister( 5 );	// Read size to write

    // buffer = Read data from address given by user;
    buffer = parToCharPtr(machine->ReadRegister(4));
    OpenFileId id = machine->ReadRegister( 6 );	// Read file descriptor
    // Need a semaphore to synchronize access to console
    //Console->P();

    switch (id) {
        case  ConsoleInput:	// User could not write to standard input
            machine->WriteRegister( 2, -1 );
            break;
        case  ConsoleOutput:
            buffer[ size ] = 0;
            printf( "%s", buffer );
            break;
        case ConsoleError:	// This trick permits to write integers to console
            printf( "%d\n", machine->ReadRegister( 4 ) );
            break;
        default:	// All other opened files
            // Verify if the file is opened, if not return -1 in r2
            if(tablaNachos->isOpened(id)){
                // Get the unix handle from our table for open files
                int UnixHandle = tablaNachos->getUnixHandle(id);
                // Do the write to the already opened Unix file
                int sizeWritten = static_cast<int>(write(UnixHandle, buffer, size));
                // Return the number of chars written to user, via r2
                machine->WriteRegister(2, sizeWritten);
            }else{
                machine->WriteRegister(2, -1);
            }
            break;

    }
    // Update simulation stats, see details in Statistics class in machine/stats.c
    // NO SE COMO SE ACTUALIZAN LOS STATS
    //Console->V();

    cout << "Saliendo write" << '\n';

    returnFromSystemCall();		// Update the PC registers

}       // NachosWrite

void NachosRead(){
    /* System call definition described to user
        void Read(
		char *buffer,	// Register 4
		int size,	    // Register 5
		OpenFileId id	// Register 6
	);
*/
    cout << "Entrando Read" << '\n';
    int position = machine->ReadRegister(4);
    int size = machine->ReadRegister( 5 );	// Read size to read
    char buffer[size]; //Buffer to store what is read
    OpenFileId id = machine->ReadRegister( 6 );	// Read file descriptor
    cout << id << "id\n";
    // Need a semaphore to synchronize access to console
    //Console->P();
    //cout << "1" << '\n';
    bool standard = (id == ConsoleInput || id == ConsoleOutput || id == ConsoleOutput);
    int sizeRead = -1;

    //Reads and stores into buffer.
    if(standard){
        sizeRead = static_cast<int>(read(id, buffer, size));
    }else{
        if(tablaNachos->isOpened(id)) {
            cout << "holi3\n";
            sizeRead = static_cast<int>(read(tablaNachos->getUnixHandle(id), buffer, size));
        }
    }

    //if there was no error
    if(-1 != sizeRead){
        for(int i = 0; i < size; ++i){
            machine->WriteMem(position++, 1, (int)buffer[i]);
        }
    }
    machine->WriteRegister(2, sizeRead); //Return the number of chars read, via r2
    // Update simulation stats, see details in Statistics class in machine/stats.c
    // NO SE COMO SE ACTUALIZAN LOS STATS
    //Console->V();
    cout << sizeRead << '\n';
    cout << "Saliendo read" << '\n';
    returnFromSystemCall();
}

//void NachosForkThread( int p ) { // for 32 bits version
void NachosForkThread( void * p ) { // for 64 bits version

    AddrSpace *space;

    space = currentThread->space;
    space->InitRegisters();             // set the initial register values
    space->RestoreState();              // load page table register

// Set the return address for this thread to the same as the main thread
// This will lead this thread to call the exit system call and finish
    machine->WriteRegister( RetAddrReg, 4 );

    machine->WriteRegister( PCReg, (long) p );
    machine->WriteRegister( NextPCReg, (long) p + 4 );

    machine->Run();                     // jump to the user progam
    ASSERT(false);

}

void NachosFork() {			// System call 9
	DEBUG( 'u', "Entering Fork System call\n" );
	// We need to create a new kernel thread to execute the user thread
	Thread * newT = new Thread( "child to execute Fork code" );

	// We need to share the Open File Table structure with this new child

	// Child and father will also share the same address space, except for the stack
	// Text, init data and uninit data are shared, a new stack area must be created
	// for the new child
	// We suggest the use of a new constructor in AddrSpace class,
	// This new constructor will copy the shared segments (space variable) from currentThread, passed
	// as a parameter, and create a new stack for the new child
	newT->space = new AddrSpace( currentThread->space );
printf("holi");
	// We (kernel)-Fork to a new method to execute the child code
	// Pass the user routine address, now in register 4, as a parameter
	// Note: in 64 bits register 4 need to be casted to (void *)
  int r4 = machine->ReadRegister( 4 ) ;
	newT->Fork( NachosForkThread, (void*) r4);
	returnFromSystemCall();	// This adjust the PrevPC, PC, and NextPC registers

	DEBUG( 'u', "Exiting Fork System call\n" );
}	// Kernel_Fork

void ExceptionHandler(ExceptionType which) {
    int type = machine->ReadRegister(2);

    if ((which == SyscallException)) {
        switch (type) {
            case SC_Halt: {
                NachosHalt();
                break;
            }
            case SC_Open: {
                NachosOpen();
                break;
            }
            case SC_Write: {
                NachosWrite();
                break;
            }
            case SC_Read: {
                NachosRead();
                break;
            }
            case SC_Create:{
                NachosCreate();
                break;
            }
            case SC_Close:{
                NachosClose();
                break;
            }
            case SC_Exit:{
                returnFromSystemCall();
                break;
            }
            case SC_Fork:{
              NachosFork();
              break;
            }
        }
    } else {
        printf("Unexpected user mode exception %d %d\n", which, type);
        ASSERT(false);
    }
}

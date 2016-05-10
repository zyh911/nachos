/*#include "syscall.h"

int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    char prompt[2], ch, buffer[60];
    int i;

    prompt[0] = '-';
    prompt[1] = '-';

    while( 1 )
    {
	Write(prompt, 2, output);

	i = 0;
	
	do {
	
	    Read(&buffer[i], 1, input); 

	} while( buffer[i++] != '\n' );

	buffer[--i] = '\0';

	if( i > 0 ) {
		newProc = Exec(buffer);
		Join(newProc);
	}
    }
}*/
#include "syscall.h"

int
main()
{
    SpaceId newProc;
    OpenFileId input = ConsoleInput;
    OpenFileId output = ConsoleOutput;
    OpenFileId fid;
    char prompt[2], ch, buffer[60];
    int i;

    prompt[0] = '-';
    prompt[1] = '$';

    while( 1 )
    {
	Write(prompt, 2, output);

	i = 0;
	
	do {
	
	    Read(&buffer[i], 1, input); 

	} while( buffer[i++] != '\n' );
	
	buffer[--i] = '\0';

	if( i > 0 ) {
		if(buffer[0] == '.' && buffer[1] == '/') {
			newProc = Exec(&buffer[2]);
			Join(newProc);
		}
		if(buffer[0] == 'm' && buffer[1] == 'a' && buffer[2] == 'n') {
			newProc = Exec("man");
			Join(newProc);
		}
		else if(buffer[0] == 'm' && buffer[1] == 'f') {
			Create(&buffer[3]);
		}
		else if(buffer[0] == 'r' && buffer[1] == 'f') {
			Remove(&buffer[3]);
		}
		else if(buffer[0] == 'm' && buffer[1] == 'd') {
			CreateDir(&buffer[3]);
		}
		else if(buffer[0] == 'r' && buffer[1] == 'd') {
			RemoveDir(&buffer[3]);
		}
		else if(buffer[0] == 'e' && buffer[1] == 'c' && buffer[2] == 'h' && buffer[3] == 'o') {
			Write(&buffer[5], i-4, output);
			Write("\n", 1, output);
		}
		else {
			//Write("Command not found!\n", sizeof("Command not found!\n"), output);
		}
		
		for(i = 0; buffer[i] != '\0'; i++) {
			if(buffer[i] == ' ' && buffer[i+1] == '>') {
				buffer[i] = '\0';	
				Create(buffer+i+3);
				fid = Open(buffer+i+3);
				Write(buffer, i, fid);
			}
		}
	}
    }
}


else if((which == SyscallException) && (type == SC_Shmget)) {
		DEBUG('a', "Exec, initiated by user program.\n");
		static int map[100];
		static int times = 0;
		int ppn;
		if(!times) {
			memset(map, -1, 100);
			times++;
		}

		int key = machine->ReadRegister(4);
		int size = machine->ReadRegister(5);
		ASSERT(size > 0 && size <= PageSize);
		
		if(map[key] == -1) {
			ppn = memBitMap->Find();
			ASSERT(ppn != -1);
			map[key] = ppn;
			printf("Allocating ppn %d to key %d\n", ppn, key);
		}
		else {
			ppn = map[key];
			printf("Mapping ppn %d to key %d\n", ppn, key);
		}

		machine->WriteRegister(2, ppn);			
		machine->registers[PCReg] = machine->registers[NextPCReg];
		machine->registers[NextPCReg] = machine->registers[PCReg] + 4; 
	}
	else if((which == SyscallException) && (type == SC_Shmat)) {
		DEBUG('a', "Exec, initiated by user program.\n");
		int shmid = machine->ReadRegister(4);
		
		int index = currentThread->space->numPages-2;
		for(; currentThread->space->pageTable[index].valid == TRUE; index++) {
			break;
		}
		
			currentThread->space->pageTable[index].valid = TRUE;
			currentThread->space->pageTable[index].physicalPage = shmid;
			currentThread->space->pageTable[index].dirty = FALSE;
			currentThread->space->pageTable[index].lastUsedTime = stats->totalTicks;
			currentThread->space->pageTable[index].numOfReference = 1;
			currentThread->space->pageTable[index].readOnly = FALSE;
			currentThread->space->pageTable[index].use = FALSE;
		printf("Mapping virtual addr %d to shmid %d\n", index*PageSize, shmid);
		machine->WriteRegister(2, index*PageSize);
		machine->registers[PCReg] = machine->registers[NextPCReg];
		machine->registers[NextPCReg] = machine->registers[PCReg] + 4; 
	}
	else if((which == SyscallException) && (type == SC_Shmdl)) {
		DEBUG('a', "Exec, initiated by user program.\n");
		
		int addr = machine->ReadRegister(4);
		currentThread->space->pageTable[addr/PageSize].valid = FALSE;
		for(int i = 0; i < TLBSize; i++)
			machine->tlb[i].valid = FALSE;
		
		machine->registers[PCReg] = machine->registers[NextPCReg];
		machine->registers[NextPCReg] = machine->registers[PCReg] + 4; 
	}



#include "syscall.h"

int spaceId;
char* executable;
int a;

void func()
{
	spaceId = Exec(executable);
	Join(spaceId);	
	Exit(a);
}


int 
main()
{
	a = 1;
	executable = "../test/mytest";
	Fork(func);
	Yield();
	Exit(2);
}



/*
int 
main()
{
	int spaceId;
	spaceId = Exec("../test/mytest");	
	Join(spaceId);
	Exit(1);
}
*/

#include "syscall.h"
int A[1024];
int 
main()
{
	/*int i;
	for (i = 0; i < 3; i++) {
                A[i] = i;
		Yield(); 
        }
	Exit(1); */
        int i;
	Read(0, 0, 0);
	A[0] = 0;
	for(i = 0; i < 1000; i++) {
		A[0] ++;
	}
	Exit(A[0]-999);//should be 1
}

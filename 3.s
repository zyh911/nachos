.section .data
	m1: .short 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
        m2: .short 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
        m3: .short 1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1
	s1: .int 4
	s2: .int 4
	s3: .int 4
	t1: .int 0
	t2: .int 0
	t3: .int 0
    xx: .short 0,0,0,0
.section .text
.globl _start
_start:
	movl $m3,%ebx #BX m3 ebx
	movl $0,t1
l1:
	movl $0,t2
l2:
	movl $m1,%ecx #SI m1 ecx
	movl t1,%eax
	mull s2
	addl %eax,%eax
	addl %eax,%ecx
	movl $m2,%edx #DI m2 edx
	movl t2,%eax
	addl %eax,%eax
	addl %eax,%edx
	movl $0,t3
        movl $0,%esi
l3:
    andl %eax,%eax
    movw (%ecx),%ax
    movl %eax,%edi
    movw (%edx),%ax
    pushl %edx
    mull %edi
    popl %edx
    addl %eax,%esi	
    addl $2,%ecx
    addl $8,%edx
    movl t3,%eax
    addl $1,%eax
    movl %eax,t3
    movl s2,%edi
    cmp %eax,%edi
    jnz l3
    movl %esi,%eax
    movw %ax,(%ebx)
    addl $2,%ebx
    movl t2,%eax
    addl $1,%eax
    movl %eax,t2
    movl s3,%edi
    cmp %edi,%eax
    jnz l2
    movl t1,%eax
    addl $1,%eax
    movl %eax,t1
    movl s1,%edi
    cmp %edi,%eax
    jnz l1
    movl $0,%ebx
    movl $1,%eax
    int $0x80










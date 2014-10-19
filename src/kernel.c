#include "sched.h"
#include "phyAlloc.h"
#include "hw.h"
#include <stdio.h>

int
divide(int dividend, int divisor)
{
	int result = 0;
	int remainder = dividend;
	while (remainder >= divisor) {
		result++;
		remainder -= divisor;
	}
	//ctx_switch();
	return result;
}
int
compute_volume(int rad)
{
	int rad3 = rad * rad * rad;
	return divide(4*355*rad3, 3*113);
}

void
funcA( void)
{
	int cptA = 0;
	while ( 1 ) {
		++cptA;
		if(cptA == 3000000){
			cptA=0;
		}
	}
}
void
funcB( void )
{
	int cptB = 1;
	while ( 1 ) {
		++cptB;
		if(cptB == 3000000){
			cptB=0;
		}
		
	}
}
//------------------------------------------------------------------------
int
kmain ( void )
{
	init_hw();
	create_process(funcB, NULL, STACK_SIZE);
	create_process(funcA, NULL, STACK_SIZE);
	start_sched();
	while(1){}
	/* Pas atteignable vues nos 2 fonctions */
	return 0;
}

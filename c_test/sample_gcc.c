#include <stdio.h>

/*
compile me with
LD_LIBRARY_PATH=../vowpalwabbit/.libs ; gcc -Werror -fpic sample_gcc.c -l:libvw_c_wrapper.so -I../vowpalwabbit/ -o sample_gcc
*/


typedef short char16_t;


#define bool int
#define true (1)
#define false (0)

#include "vwdll2.h"



int main()
{
	VW_HANDLE vw;
	VW_EXAMPLE example;
	float score;
	
	printf("this is a native c program calling vw\n");
	vw = VW_InitializeA("-q st --noconstant --quiet");
	example = VW_ReadExampleA(vw, "1 |s p^the_man w^the w^man |t p^un_homme w^un w^homme");
	score = VW_Learn(vw, example);
	VW_Finish(vw);
	printf("Score = %f\n", score);
	return 0;
	
}

#include "uart.h"


void printException(unsigned int cpsr,char *str)
{
	puts("Exception ~  cpsr = ");
	printHex(cpsr);//打印cpsr的值
	puts("     ");
	puts(str);//打印字符串
	puts("\r\n");
}


void printftest(void)
{
	puts("aaaaa\r\n");
}


void printswival(unsigned int *val)
{
	unsigned int temp = *val & ~0xff000000;
	puts("\r\n swi_val =");
	printHex(temp);//打印val的值
	puts("\r\n");
}


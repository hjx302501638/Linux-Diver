
#include "uart.h"


void printException(unsigned int cpsr,char *str)
{
	puts("Exception ~  cpsr = ");
	printHex(cpsr);//��ӡcpsr��ֵ
	puts("     ");
	puts(str);//��ӡ�ַ���
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
	printHex(temp);//��ӡval��ֵ
	puts("\r\n");
}


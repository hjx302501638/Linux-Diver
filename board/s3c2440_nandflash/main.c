
#include "s3c2440_soc.h"
#include "uart.h"
#include "init.h"
#include "nand_flash.h"
char g_Char = 'A';
char g_Char3 = 'a';
const char g_Char2 = 'B';
int g_A = 0;
int g_B;

int main(void)
{
	//uart0_init();
	//timer_init();
	puts("\n\rg_A = ");
	printHex(g_A);
	puts("\n\r");
	nand_init();
   nand_flash_test();
	
	
	return 0;
}




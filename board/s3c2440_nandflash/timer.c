
#include "s3c2440_soc.h"


void timer_irq()
{
	static unsigned int cont = 0;
	cont ++;
	puts("\r\ntimer_count =");
	printHex(cont);
}

void timer_init()
{
	//设置Timer0 时钟timer  clock   = PCLK / {prescaler value+1} / {divider value}
	TCFG0 = 99;//Prescaler 0 = 99 用于Timer0,1
	TCFG1 &=0xf;
	TCFG1 |=0x3;// mux0 0011 = 1/16
	//设置Timer0 初始值
	TCNTB0 = 31250;//一秒钟中断一次
	//加载初值 ，启动Timer0   
	TCON |= (1<<1);// 手动更新 Update TCNTB0 & TCMPB0
	//设置自动重装载 并启动
	TCON &=~(1<<1);
	TCON |=(1<<0)|(1<<3);
	//设置中断  
	irq_interrupt(10,timer_irq);
}



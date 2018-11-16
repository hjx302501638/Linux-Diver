#include "s3c2440_soc.h"


typedef void (*irq_fun)(int);//函数指针
irq_fun irq_arry[32]; //定义一个函数数组

//SRCPND用来显示那个中断产生了                需要清除对应位
//bit0--eint0
//bit2--eint2
//bit5--eint8-23

//INTMASK 屏蔽中断  1---masked
//bit0--eint0
//bit2--eint2
//bit5--eint8-23

//INTPND 用来显示当前优先级最高的 正在发生的中断  需要清除对应位
//bit0--eint0
//bit2--eint2
//bit5--eint8-23

//INTOFFSET  用来显示INTPND      那一位设置为了1 那个中断正在处理 

//初始化中断控制器 
void interrupt_init(void)  
{
	//INTMSK &= ~((1<<0)|(1<<2)|(1<<5));//使能中断
	//INTMSK &= ~(1<<10);  //enable Timer0 int
}



//读EINTPEND 分辨是那个外部中断产生 4~23 需要读
//清除中断时 写EINTPEND 相应位

void key_eint_irq(int irq)
{
	unsigned int val = EINTPEND;
	if(irq == 0) //对应外部中断0 s2
	{
		puts("this is eint 0\r\n");
	}
	else if(irq == 2)//  s3 
	{
		puts("this is eint 2\r\n");
	}
	else if(irq == 5)// s4 s5
	{
		if(val & (1<<11))
		{
			puts("this is eint 11\r\n");
		}else if(val & (1<<19))			
			puts("this is eint 19\r\n");
	}
	EINTPEND = val;
}
void handle_irq_c(void)
{
	//分辨中断源
	int bit = INTOFFSET;
	//调用对应的中断函数
	irq_arry[bit](bit);
	//清中断    从源头开始清除
	SRCPND = (1<<bit);
	INTPND = (1<<bit);
}



//注册函数
void irq_interrupt(int irq, irq_fun fd)
{
	irq_arry[irq] = fd;
	INTMSK &=~(1<<irq);//使能中断
}



/*
* mw.w aaa aa
* mw.w 554 55
* mw.w aaa 90
* md.w 0 1
* md.w 2 1
*/

//初始化按键中断 设置为中断源 
void key_eint_init(void)  
{
	/*
	* 1中断源初始化：设置按键中断引脚
	* 2中断控制器
	* 3cpu,cpsr,使能中断 
	*/


	//配置 gpio为中断引脚
	GPFCON &= ~((3<<0)|(3<<4));
	GPFCON |=  ((2<<0)|(2<<4));//s2 s3配置为中断引脚
	GPGCON &= ~((3<<6)|(3<<22));
	GPGCON |=  ((2<<6)|(2<<22));//
	//设置 中断触发方式 双边沿触发
	EXTINT0 |= (7<<0)|(7<<8);  //s2 s3
	EXTINT1 |= (7<<12);        //s4
	EXTINT2 |= (7<<12);        //s5
	//设置外部EINTMASK 使能外部中断11,19
	EINTMASK &= ~((1<<11)|(1<<19));
	//注册中断
	irq_interrupt(0,key_eint_irq);
	irq_interrupt(2,key_eint_irq);
	irq_interrupt(5,key_eint_irq);
	
}



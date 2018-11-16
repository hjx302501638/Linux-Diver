#include "s3c2440_soc.h"


typedef void (*irq_fun)(int);//����ָ��
irq_fun irq_arry[32]; //����һ����������

//SRCPND������ʾ�Ǹ��жϲ�����                ��Ҫ�����Ӧλ
//bit0--eint0
//bit2--eint2
//bit5--eint8-23

//INTMASK �����ж�  1---masked
//bit0--eint0
//bit2--eint2
//bit5--eint8-23

//INTPND ������ʾ��ǰ���ȼ���ߵ� ���ڷ������ж�  ��Ҫ�����Ӧλ
//bit0--eint0
//bit2--eint2
//bit5--eint8-23

//INTOFFSET  ������ʾINTPND      ��һλ����Ϊ��1 �Ǹ��ж����ڴ��� 

//��ʼ���жϿ����� 
void interrupt_init(void)  
{
	//INTMSK &= ~((1<<0)|(1<<2)|(1<<5));//ʹ���ж�
	//INTMSK &= ~(1<<10);  //enable Timer0 int
}



//��EINTPEND �ֱ����Ǹ��ⲿ�жϲ��� 4~23 ��Ҫ��
//����ж�ʱ дEINTPEND ��Ӧλ

void key_eint_irq(int irq)
{
	unsigned int val = EINTPEND;
	if(irq == 0) //��Ӧ�ⲿ�ж�0 s2
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
	//�ֱ��ж�Դ
	int bit = INTOFFSET;
	//���ö�Ӧ���жϺ���
	irq_arry[bit](bit);
	//���ж�    ��Դͷ��ʼ���
	SRCPND = (1<<bit);
	INTPND = (1<<bit);
}



//ע�ắ��
void irq_interrupt(int irq, irq_fun fd)
{
	irq_arry[irq] = fd;
	INTMSK &=~(1<<irq);//ʹ���ж�
}



/*
* mw.w aaa aa
* mw.w 554 55
* mw.w aaa 90
* md.w 0 1
* md.w 2 1
*/

//��ʼ�������ж� ����Ϊ�ж�Դ 
void key_eint_init(void)  
{
	/*
	* 1�ж�Դ��ʼ�������ð����ж�����
	* 2�жϿ�����
	* 3cpu,cpsr,ʹ���ж� 
	*/


	//���� gpioΪ�ж�����
	GPFCON &= ~((3<<0)|(3<<4));
	GPFCON |=  ((2<<0)|(2<<4));//s2 s3����Ϊ�ж�����
	GPGCON &= ~((3<<6)|(3<<22));
	GPGCON |=  ((2<<6)|(2<<22));//
	//���� �жϴ�����ʽ ˫���ش���
	EXTINT0 |= (7<<0)|(7<<8);  //s2 s3
	EXTINT1 |= (7<<12);        //s4
	EXTINT2 |= (7<<12);        //s5
	//�����ⲿEINTMASK ʹ���ⲿ�ж�11,19
	EINTMASK &= ~((1<<11)|(1<<19));
	//ע���ж�
	irq_interrupt(0,key_eint_irq);
	irq_interrupt(2,key_eint_irq);
	irq_interrupt(5,key_eint_irq);
	
}



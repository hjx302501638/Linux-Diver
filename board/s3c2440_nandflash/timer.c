
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
	//����Timer0 ʱ��timer  clock   = PCLK / {prescaler value+1} / {divider value}
	TCFG0 = 99;//Prescaler 0 = 99 ����Timer0,1
	TCFG1 &=0xf;
	TCFG1 |=0x3;// mux0 0011 = 1/16
	//����Timer0 ��ʼֵ
	TCNTB0 = 31250;//һ�����ж�һ��
	//���س�ֵ ������Timer0   
	TCON |= (1<<1);// �ֶ����� Update TCNTB0 & TCMPB0
	//�����Զ���װ�� ������
	TCON &=~(1<<1);
	TCON |=(1<<0)|(1<<3);
	//�����ж�  
	irq_interrupt(10,timer_irq);
}



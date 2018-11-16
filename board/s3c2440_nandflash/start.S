
.text
.global _start

_start:
	b reset  //vector 0 :reset
	ldr pc,und_addr //vector 4:und
	ldr pc,swi_addr //vector 0x8:swi
	b halt			//0x0c perfetcth abort
	b halt			//0x10 data abort
	b halt			//0x14 reserv	
	ldr pc,irq_addr//0x18 irq
	b halt			//0x1c fiq
und_addr:
	.word do_und
		
swi_addr:
	.word do_swi
		
irq_addr:
	.word do_irq		
		
do_und:

	/*
	*ִ�е�����֮ǰ
	*lr_und�����б��ж�ģʽ�е���һ��ִ�еĵ�ַ
	*SPSR_und�б��� ���ж�ģʽ��CPSR	
	*CPSR�е�M0-M4������Ϊ11011�����뵽undģʽ
	*����0x4�ĵط�ִ�г���
	*/
	/* sp_und û�����ã�����Ӧ��������  */
	ldr sp ,= 0x34000000
	//lr ���쳣�������ķ��ص�ַ ҲҪ����
	stmdb sp!,{r0-r12,lr}//��und�쳣�����������п��ܻ��޸�r0-r12�������ȱ�������  �ȼ����
	//�����ֳ�
	
	//�����쳣
	mrs r0,cpsr  //r0 = cpsr  
	ldr r1,=und_string
	bl printException
	//�ָ��ֳ�
	
	ldmia sp!,{r0-r12,pc}^ //^���spsr�Ĵ����е�ֵ�ָ��� cpsr��ȥ �ȶ����
//����һ���ַ��� �Զ����� \0
und_string:
		.string "undefined instruction exception"

.align 4
do_swi:

		/*
		*ִ�е�����֮ǰ
		*lr_sve�����б��ж�ģʽ�е���һ��ִ�еĵ�ַ
		*SPSR_sve�б��� ���ж�ģʽ��CPSR	
		*CPSR�е�M0-M4������Ϊ10011�����뵽undģʽ
		*����0x4�ĵط�ִ�г���
		*/
		/* sp_sve û�����ã�����Ӧ��������	*/
		ldr sp ,= 0x33e00000
		//lr ���쳣�������ķ��ص�ַ ҲҪ����
		stmdb sp!,{r0-r12,lr}//��swi�쳣�����������п��ܻ��޸�r0-r12�������ȱ�������	�ȼ����
		mov r4,lr   //r4�Ĵ������ᱻc�����ƻ�
		//�����ֳ�
		
		//�����쳣
		mrs r0,cpsr  //r0 = cpsr  
		ldr r1,=swi_string
		bl printException

		mov r0,r4
		sub r0,r0,#4
		bl printswival
		//�ָ��ֳ�
		
		ldmia sp!,{r0-r12,pc}^ //^���spsr�Ĵ����е�ֵ�ָ��� cpsr��ȥ �ȶ����
	//����һ���ַ��� �Զ����� \0
	swi_string:
			.string "swi instruction exception"

.align 4
do_irq:

	/*
	*ִ�е�����֮ǰ
	*lr_irq�����б��ж�ģʽ�е���һ��ִ�еĵ�ַ
	*SPSR_irq�б��� ���ж�ģʽ��CPSR	
	*CPSR�е�M0-M4������Ϊ10010�����뵽irqģʽ
	*����0x18�ĵط�ִ�г���
	*/
	/* sp_und û�����ã�����Ӧ��������	*/
	ldr sp ,= 0x33d00000
	//lr-4 ���쳣 �������ķ��ص�ַ ҲҪ����
	//�����ֳ�
	sub lr,lr,#4  //lr -4
	stmdb sp!,{r0-r12,lr}//��irq�쳣�����������п��ܻ��޸�r0-r12�������ȱ�������	�ȼ����
	
	
	//�����쳣
	bl handle_irq_c
	
	//�ָ��ֳ�	
	ldmia sp!,{r0-r12,pc}^ //^���spsr�Ĵ����е�ֵ�ָ��� cpsr��ȥ �ȶ����

.align 4
reset:
	/* �رտ��Ź� */
	ldr r0, =0x53000000
	ldr r1, =0
	str r1, [r0]

	/* ����MPLL, FCLK : HCLK : PCLK = 400m : 100m : 50m */
	/* LOCKTIME(0x4C000000) = 0xFFFFFFFF */
	ldr r0, =0x4C000000
	ldr r1, =0xFFFFFFFF
	str r1, [r0]

	/* CLKDIVN(0x4C000014) = 0X5, tFCLK:tHCLK:tPCLK = 1:4:8  */
	ldr r0, =0x4C000014
	ldr r1, =0x5
	str r1, [r0]

	/* ����CPU�������첽ģʽ */
	mrc p15,0,r0,c1,c0,0
	orr r0,r0,#0xc0000000   //R1_nF:OR:R1_iA
	mcr p15,0,r0,c1,c0,0

	/* ����MPLLCON(0x4C000004) = (92<<12)|(1<<4)|(1<<0) 
	 *  m = MDIV+8 = 92+8=100
	 *  p = PDIV+2 = 1+2 = 3
	 *  s = SDIV = 1
	 *  FCLK = 2*m*Fin/(p*2^s) = 2*100*12/(3*2^1)=400M
	 */
	ldr r0, =0x4C000004
	ldr r1, =(92<<12)|(1<<4)|(1<<0)
	str r1, [r0]

	/* һ������PLL, �ͻ�����lock timeֱ��PLL����ȶ�
	 * Ȼ��CPU�������µ�Ƶ��FCLK
	 */
	
	

	/* �����ڴ�: sp ջ */
	/* �ֱ���nor/nand����
	 * д0��0��ַ, �ٶ�����
	 * ����õ�0, ��ʾ0��ַ�ϵ����ݱ��޸���, ����Ӧram, �����nand����
	 * �������nor����
	 */
	mov r1, #0
	ldr r0, [r1] /* ����ԭ����ֵ���� */
	str r1, [r1] /* 0->[0] */ 
	ldr r2, [r1] /* r2=[0] */
	cmp r1, r2   /* r1==r2? �����ȱ�ʾ��NAND���� */
	ldr sp, =0x40000000+4096 /* �ȼ�����nor���� */
	moveq sp, #4096  /* nand���� */
	streq r0, [r1]   /* �ָ�ԭ����ֵ */

	//�ض�λ����
	bl sdram_init	 //��ʼ��sdram
	bl copy2asdram	 //���������
	bl clenbss       //���bss��

	//��λ֮�� cpu����svcģʽ
	//ִ�е����λ��ʱ�л��� usrģʽ
	mrs r0,cpsr   //����cpsr
	bic r0,r0,#0xf//�޸�m4-m0Ϊ 0b10000������usrģʽ  ���� ����λ
	bic r0,r0,#(1<<7) //���iλ  ʹ���ж�
	msr cpsr,r0  //��r0�е�ֵд��cpsr��

	//����sp_usr
	ldr sp,=0x33f00000
	
	ldr pc,=sdram  //ʹ�þ�����תָ�����sdram��ִ��
sdram:
	bl uart0_init
	.align 4
/* �������һ��δ����ָ�� */
	//bl printftest
und_code:
	.word 0xdeadc0de  /* δ����ָ�� */
	//bl printftest			
	swi 0x123 //����swi�쳣 ����0x8��ִַ��

	bl interrupt_init  //��ʼ���жϿ����� 
	bl key_eint_init  //��ʼ�������ж� ����Ϊ�ж�Դ 
	//bl main   //ʹ��bl�����ת ������Ȼ��Ƭ��sram��ִ��
	ldr pc, =main  //ʹ������ָ���������SDRAM��ִ��

halt:
	b halt
	
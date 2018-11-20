
.text
.global _start

_start:
b reset  //vector 0 :reset
ldr pc,und_addr //vector 4:und
ldr pc,swi_addr //vector 0x8:swi

und_addr:
	.word do_und
		
swi_addr:
	.word do_swi
do_und:

	/*
	*执行到这里之前
	*lr_und保存有被中断模式中的下一条执行的地址
	*SPSR_und中保存 了中断模式的CPSR	
	*CPSR中的M0-M4被设置为11011，进入到und模式
	*跳到0x4的地方执行程序
	*/
	/* sp_und 没有设置，我们应该先设置  */
	ldr sp ,= 0x34000000
	//lr 是异常处理完后的返回地址 也要保存
	stmdb sp!,{r0-r12,lr}//在und异常处理函数中有可能会修改r0-r12，所以先保存下来  先减后存
	//保存现场
	
	//处理异常
	mrs r0,cpsr  //r0 = cpsr  
	ldr r1,=und_string
	bl printException
	//恢复现场
	
	ldmia sp!,{r0-r12,pc}^ //^会把spsr寄存器中的值恢复到 cpsr中去 先读后加
//定义一个字符串 自动添加 \0
und_string:
		.string "undefined instruction exception"

.align 4
do_swi:

		/*
		*执行到这里之前
		*lr_sve保存有被中断模式中的下一条执行的地址
		*SPSR_sve中保存 了中断模式的CPSR	
		*CPSR中的M0-M4被设置为10011，进入到und模式
		*跳到0x4的地方执行程序
		*/
		/* sp_sve 没有设置，我们应该先设置	*/
		ldr sp ,= 0x33e00000
		//lr 是异常处理完后的返回地址 也要保存
		stmdb sp!,{r0-r12,lr}//在swi异常处理函数中有可能会修改r0-r12，所以先保存下来	先减后存
		mov r4,lr   //r4寄存器不会被c函数破坏
		//保存现场
		
		//处理异常
		mrs r0,cpsr  //r0 = cpsr  
		ldr r1,=swi_string
		bl printException

		mov r0,r4
		sub r0,r0,#4
		bl printswival
		//恢复现场
		
		ldmia sp!,{r0-r12,pc}^ //^会把spsr寄存器中的值恢复到 cpsr中去 先读后加
	//定义一个字符串 自动添加 \0
	swi_string:
			.string "swi instruction exception"

.align 4
reset:
	/* 关闭看门狗 */
	ldr r0, =0x53000000
	ldr r1, =0
	str r1, [r0]

	/* 设置MPLL, FCLK : HCLK : PCLK = 400m : 100m : 50m */
	/* LOCKTIME(0x4C000000) = 0xFFFFFFFF */
	ldr r0, =0x4C000000
	ldr r1, =0xFFFFFFFF
	str r1, [r0]

	/* CLKDIVN(0x4C000014) = 0X5, tFCLK:tHCLK:tPCLK = 1:4:8  */
	ldr r0, =0x4C000014
	ldr r1, =0x5
	str r1, [r0]

	/* 设置CPU工作于异步模式 */
	mrc p15,0,r0,c1,c0,0
	orr r0,r0,#0xc0000000   //R1_nF:OR:R1_iA
	mcr p15,0,r0,c1,c0,0

	/* 设置MPLLCON(0x4C000004) = (92<<12)|(1<<4)|(1<<0) 
	 *  m = MDIV+8 = 92+8=100
	 *  p = PDIV+2 = 1+2 = 3
	 *  s = SDIV = 1
	 *  FCLK = 2*m*Fin/(p*2^s) = 2*100*12/(3*2^1)=400M
	 */
	ldr r0, =0x4C000004
	ldr r1, =(92<<12)|(1<<4)|(1<<0)
	str r1, [r0]

	/* 一旦设置PLL, 就会锁定lock time直到PLL输出稳定
	 * 然后CPU工作于新的频率FCLK
	 */
	
	

	/* 设置内存: sp 栈 */
	/* 分辨是nor/nand启动
	 * 写0到0地址, 再读出来
	 * 如果得到0, 表示0地址上的内容被修改了, 它对应ram, 这就是nand启动
	 * 否则就是nor启动
	 */
	mov r1, #0
	ldr r0, [r1] /* 读出原来的值备份 */
	str r1, [r1] /* 0->[0] */ 
	ldr r2, [r1] /* r2=[0] */
	cmp r1, r2   /* r1==r2? 如果相等表示是NAND启动 */
	ldr sp, =0x40000000+4096 /* 先假设是nor启动 */
	moveq sp, #4096  /* nand启动 */
	streq r0, [r1]   /* 恢复原来的值 */

	//重定位部分
	bl sdram_init	 //初始化sdram
	bl copy2asdram	 //拷贝代码段
	bl clenbss       //清除bss段

	//复位之后 cpu处于svc模式
	//执行到这个位置时切换到 usr模式
	mrs r0,cpsr   //读出cpsr
	bic r0,r0,#0xf//修改m4-m0为 0b10000，进入usr模式  清零 低四位
	msr cpsr,r0  //将r0中的值写入cpsr中

	//设置sp_usr
	ldr sp,=0x33f00000
	
	ldr pc,=sdram  //使用绝对跳转指令调到sdram中执行
sdram:
	bl uart0_init
	.align 4
/* 故意加入一条未定义指令 */
	//bl printftest
und_code:
	.word 0xdeadc0de  /* 未定义指令 */
	//bl printftest			
	swi 0x123 //触发swi异常 进入0x8地址执行
	//bl main   //使用bl相对跳转 程序仍然在片内sram中执行
	ldr pc, =main  //使用这条指令才能跳到SDRAM中执行

halt:
	b halt
	

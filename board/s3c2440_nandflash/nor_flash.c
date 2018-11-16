#include "my_printf.h"
#include "string_utils.h"


#define NOR_FLASH_BASE  0  //jz 2440 nor --->cs0 addr =0

//  55h 98
//往  0+（55<<1） 写入0x98
void nor_write_word(unsigned int base,unsigned int offset,unsigned int val)
{
	volatile unsigned short *p =  (volatile unsigned short *)(base +(offset<<1));
	*p = val;
}

void nor_cmd(unsigned int offset,unsigned  int cmd)
{
	nor_write_word(NOR_FLASH_BASE,offset,cmd);
}


unsigned int  nor_read_word(unsigned int base,unsigned int offset)
{
	volatile unsigned short *p =  (volatile unsigned short *)(base +(offset<<1));	
	return *p;
}

unsigned int nor_data(unsigned int offset)
{
	return nor_read_word(NOR_FLASH_BASE,offset);
}



void wait_ready(unsigned int addr)
{
	unsigned int val ;
	unsigned int pre ;	
	pre = nor_data(addr >>1);
	val = nor_data(addr >>1);
	while((val &(1<<6)) != (pre &(1<<6)))
		{
			pre = val;
			val =  nor_data(addr >>1);
		}
}

void do_erase_nor_flash(void )
{
	unsigned int addr;
	
	//获得地址
	printf("Enter the address to erase:");
	addr = get_uint();

	printf("erase........\r\n");
	nor_cmd(0x555,0xaa);nor_cmd(0x2aa,0x55);//解锁
	nor_cmd(0x555,0x80);//发送擦除命令
	nor_cmd(0x555,0xaa);nor_cmd(0x2aa,0x55);//解锁
	
	nor_cmd(addr>>1,0x30);//发出扇区地址
	wait_ready(addr);
}



void do_write_nor_flash(void)
{
	unsigned int addr,i,j;
	unsigned char str[100];
	unsigned int val;
	//获得地址
	printf("Enter the address sector to write:");
	addr = get_uint();

	printf("Enter the string to write:");
	gets(str);

	printf("wrting........\r\n");
	//str[0],str[1] ---->16bit
	i=0;j=1;
	while(str[i] &&str[j])
		{
			val = str[i]+(str[j]<<8);
			nor_cmd(0x555,0xaa);nor_cmd(0x2aa,0x55);//解锁
			nor_cmd(0x555,0xa0);  //发送写命令
			nor_cmd(addr>>1,val);     //写入数据

			//等待烧写完成 读数据 q6 无变化时表示结束
			wait_ready(addr);
			i+=2;
			j+=2;
			addr+=2;
		}
	val = str[i];
	val = str[i]+(str[j]<<8);
	nor_cmd(0x555,0xaa);nor_cmd(0x2aa,0x55);//解锁
	nor_cmd(0x55,0xa0);//发送写命令
	nor_cmd(addr>>1,val);     //写入数据

	//等待烧写完成 读数据 q6 无变化时表示结束
	wait_ready(addr);
	
}

void do_read_nor_flash()
{
	unsigned int addr,i;
	volatile unsigned char *p;
	int j;
	unsigned char c;
	unsigned char str[16];
	//获得地址
	printf("Enter the address to read:");
	addr = get_uint();
	//长度
	printf("data:\r\n");
	p = (volatile unsigned char *)addr;
	for(i = 0;i<4;i++)
		{
			//每行打印十六个数据
			for(j=0;j<16;j++)
				{
					//先打印数值
					c = *p++;
					printf("%02x ",c);
					str[j] =c;
				}
			printf("      ;");
			for(j=0;j<16;j++)
				{
					//后打印字符		
					if(str[j]<0x20 || str[j]>0x7e)
						printf(".");
					else printf("%c",str[j]);				
				}	
			printf("\r\n");
		}
	//;
}

/***
*进入nor flash的cfi模式 读取设备信息
*
*/
void do_scan_nor_flash(void)
{
	char str[4];
	unsigned int size;
	int regions;
	int i;
	int region_info_base;
	int block_addr,blocks,blocks_size,j,cnt;

	int vendor,deveice;
	nor_cmd(0x55,0x98);//进入cfi模式
	//打印出厂家id
	str[0]  = nor_data(0x10);
	str[1]  = nor_data(0x11);
	str[2]  = nor_data(0x12);
	str[3]  = '\0';
	printf("%s\r\n",str);
	//设备id 
	
	nor_cmd(0x555,0xaa);nor_cmd(0x2aa,0x55);//解锁
	nor_cmd(0x555,0x90);//发送读id命令
	vendor  =  nor_data(0); //读厂家id
	deveice =  nor_data(1);//读设备id
	nor_cmd(0,0xf0);//退出读cfi模式
	nor_cmd(0x55,0x98);//进入cfi模式

	printf("vendor id = 0x%x\r\n",vendor);
	printf("deveice id = 0x%x\r\n",deveice);
	
	//打印容量
	size = 1<< nor_data(0x27);
	printf("size = 0x%xb , %dM\r\n",size,size/(1024*1024));
	//各个扇区的地址
	regions = nor_data(0x2c);
	region_info_base = 0x2d;
	block_addr=0;
	printf("Block/Sector start address:\r\n");
	cnt =0;
	for(i=0;i<regions;i++)
		{
			blocks =1+ nor_data(region_info_base)+(nor_data(region_info_base+1)<<8);
			blocks_size = 256*(nor_data(region_info_base+2)+(nor_data(region_info_base+3)<<8));
			region_info_base+=4;
			for(j =0;j<blocks;j++)
				{
					//打印每个block的起始地址
					printf("0x%08x ",block_addr);					
					cnt++;
					block_addr+=blocks_size;
					if(cnt %5 == 0)
						printf("\r\n");
				}
			
		}
	printf("\r\n");
	//退出cfi模式
	nor_cmd(0x0,0xf0);
}

void nor_flash_test(void)
{

	char c;
	while(1)
		{
			//打印菜单，提供测试内容
			printf(" [s] scan not flash \r\n ");
			printf("[e] erase not flash \r\n ");
			printf("[w] write not flash \r\n ");
			printf("[r] read not flash \r\n ");
			printf("[q] quit not flash \r\n ");
			printf("Enter selection ");
			c= getchar();
			printf("%c\r\n",c);

			switch(c)
				{
					case 'q':
					case 'Q':
						return; break;
					case 's':
					case 'S':
						do_scan_nor_flash();
						break;
					
					case 'e':
					case 'E':
						do_erase_nor_flash();
						break;
					case 'w':
					case 'W':
						do_write_nor_flash();
						break;
					case 'r':
					case 'R':
						do_read_nor_flash();
						break;
					default:break;					
						
				}
		}
	
	 //内容  
}













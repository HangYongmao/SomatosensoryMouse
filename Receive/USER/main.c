#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "string.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "24l01.h"

//ALIENTEK战舰STM32开发板实验49
//触控USB鼠标 实验  
//技术支持：www.openedv.com
//广州市星翼电子科技有限公司  		 	

extern u8 EP1BUSY;			//键盘数据发送忙标志 
extern u8 EP2BUSY;			//鼠标数据发送忙标志
extern u8 INIT_OK;
enum{
	KEYBOARD	= 0x01,
	MOUSE		= 0x02,
};
//设置USB 连接/断线
//enable:0,断开
//       1,允许连接	   
void usb_port_set(u8 enable)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	if (enable)_SetCNTR(_GetCNTR()&(~(1 << 1)));//退出断电模式
	else
	{
		_SetCNTR(_GetCNTR() | (1 << 1));  // 断电模式
		GPIOA->CRH &= 0XFFF00FFF;
		GPIOA->CRH |= 0X00033000;
		PAout(12) = 0;
	}
}

int main(void)
{
	u8 tmp_buf[5];	//发射寄存器
	short X, Y;		//发送到电脑端的坐标值
	delay_init();	//延时函数初始化
	uart_init(9600);//串口初始化为9600
	delay_ms(100);
	NRF24L01_Init();
	while (NRF24L01_Check())	//检查NRF24L01是否在位.	
	{
		printf("未检测到24l01\r\n");
		delay_ms(200);
	}
	printf("检测到24l01---OK\r\n");
	NRF24L01_RX_Mode();
	printf("设置24l01为接收模式成功！\r\n");

	usb_port_set(0); 	//USB先断开
	delay_ms(300);
	usb_port_set(1);	//USB再次连接
	
	//USB配置
	USB_Interrupts_Config();
	Set_USBClock();
	USB_Init();
	printf("USB都初始化完毕了！！！\r\n");
	while (1)
	{
		if (NRF24L01_RxPacket(tmp_buf) == 0)
		{
			if (tmp_buf[0] == KEYBOARD)	// 键盘
			{
				if(EP1BUSY == 0)
				{
					EP1BUSY = 1;
					SendKeyboardData(0x02, 0x00, 0x04, 0x05, 0x00,0x00,0x00, 0x00);
				}
			}
			else if(tmp_buf[0] == MOUSE)// 鼠标
			{
				if(EP2BUSY == 0)
				{
					EP2BUSY = 1;
					SendMouseData(0x01, 1, 1, 0);
				}
			}
			else
			{
				if(EP1BUSY == 0)
				{
					EP1BUSY = 1;
					SendKeyboardData(0x00, 0x00, 0x00, 0x00, 0x00,0x00,0x00, 0x00);
				}
				if(EP2BUSY == 0)
				{
					EP2BUSY = 1;
					SendMouseData(0, 0, 0, 0);
				}
			}
		}
	}
}


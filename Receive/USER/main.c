#include "delay.h"
#include "sys.h"
#include "usart.h"	 
#include "string.h"
#include "usb_lib.h"
#include "hw_config.h"
#include "usb_pwr.h"
#include "24l01.h"

//ALIENTEKս��STM32������ʵ��49
//����USB��� ʵ��  
//����֧�֣�www.openedv.com
//������������ӿƼ����޹�˾  		 	

extern u8 EP1BUSY;			//�������ݷ���æ��־ 
extern u8 EP2BUSY;			//������ݷ���æ��־
extern u8 INIT_OK;
enum{
	KEYBOARD	= 0x01,
	MOUSE		= 0x02,
};
//����USB ����/����
//enable:0,�Ͽ�
//       1,��������	   
void usb_port_set(u8 enable)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	if (enable)_SetCNTR(_GetCNTR()&(~(1 << 1)));//�˳��ϵ�ģʽ
	else
	{
		_SetCNTR(_GetCNTR() | (1 << 1));  // �ϵ�ģʽ
		GPIOA->CRH &= 0XFFF00FFF;
		GPIOA->CRH |= 0X00033000;
		PAout(12) = 0;
	}
}

int main(void)
{
	u8 tmp_buf[5];	//����Ĵ���
	short X, Y;		//���͵����Զ˵�����ֵ
	delay_init();	//��ʱ������ʼ��
	uart_init(9600);//���ڳ�ʼ��Ϊ9600
	delay_ms(100);
	NRF24L01_Init();
	while (NRF24L01_Check())	//���NRF24L01�Ƿ���λ.	
	{
		printf("δ��⵽24l01\r\n");
		delay_ms(200);
	}
	printf("��⵽24l01---OK\r\n");
	NRF24L01_RX_Mode();
	printf("����24l01Ϊ����ģʽ�ɹ���\r\n");

	usb_port_set(0); 	//USB�ȶϿ�
	delay_ms(300);
	usb_port_set(1);	//USB�ٴ�����
	
	//USB����
	USB_Interrupts_Config();
	Set_USBClock();
	USB_Init();
	printf("USB����ʼ������ˣ�����\r\n");
	while (1)
	{
		if (NRF24L01_RxPacket(tmp_buf) == 0)
		{
			if (tmp_buf[0] == KEYBOARD)	// ����
			{
				if(EP1BUSY == 0)
				{
					EP1BUSY = 1;
					SendKeyboardData(0x02, 0x00, 0x04, 0x05, 0x00,0x00,0x00, 0x00);
				}
			}
			else if(tmp_buf[0] == MOUSE)// ���
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


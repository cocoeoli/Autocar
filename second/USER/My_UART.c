#include "My_UART.h"
#include "sys.h"


u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		    	  //���ջ���,���USART2_MAX_RECV_LEN���ֽ�.
u16 USART2_RX_STA=0;   	                              //������

u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		    	  //���ջ���,���USART3_MAX_RECV_LEN���ֽ�.
u16 USART3_RX_STA=0;   	                              //������

extern unsigned char Rx3Flag;

void UART_send_char(const unsigned char Byte, USART_TypeDef* USARTx)
{
   while((USARTx->SR&0X40)==0);//�ȴ���һ�η������   
	 USARTx->DR=Byte;
}



char UART_recive_char(USART_TypeDef* USARTx)
{
	char Byte;
	Byte = USART_ReceiveData(USARTx);
	return Byte;
}



void UART_send_string(const char *sendbuf, USART_TypeDef* USARTx)
{
	while(*sendbuf != '\0')
	{
		UART_send_char(*sendbuf, USARTx);
		sendbuf ++;
	}
}


void USART2_Init(u32 bound)
{  

			GPIO_InitTypeDef GPIO_InitStructure;
			USART_InitTypeDef USART_InitStructure;
	        NVIC_InitTypeDef NVIC_InitStructure;

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOAʱ��
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

		     //	USART_DeInit(USART2);  //��λ����2
				 //USART2_TX   PA.2
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
			GPIO_Init(GPIOA, &GPIO_InitStructure); //��ʼ��PA2
			 
				//USART2_RX	  PA.3
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
			GPIO_Init(GPIOA, &GPIO_InitStructure);  //��ʼ��PA3
	
					//Usart1 NVIC ����
			NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//��ռ���ȼ�0
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
			NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
			
			USART_InitStructure.USART_BaudRate = bound;   //������
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
			USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
			USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
			USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
			USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
			
			USART_Init(USART2, &USART_InitStructure);  //��ʼ������	2
			USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
            USART_Cmd(USART2, ENABLE);                    //ʹ�ܴ���2

}

void USART3_Init(u32 bound)
{  

			GPIO_InitTypeDef GPIO_InitStructure;
			USART_InitTypeDef USART_InitStructure;
	        NVIC_InitTypeDef NVIC_InitStructure;

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOBʱ��
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);

		     //	USART_DeInit(USART3);  //��λ����3
				 //USART3_TX   PB10
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//�����������
			GPIO_Init(GPIOB, &GPIO_InitStructure); //��ʼ��PB10
			 
				//USART3_RX	  PB11
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
			GPIO_Init(GPIOB, &GPIO_InitStructure);  //��ʼ��PB11
	
					//Usart3 NVIC ����
			NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//��ռ���ȼ�0
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//�����ȼ�3
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQͨ��ʹ��
			NVIC_Init(&NVIC_InitStructure);	//����ָ���Ĳ�����ʼ��VIC�Ĵ���
			
			USART_InitStructure.USART_BaudRate = bound;   //������
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;//�ֳ�Ϊ8λ���ݸ�ʽ
			USART_InitStructure.USART_StopBits = USART_StopBits_1;//һ��ֹͣλ
			USART_InitStructure.USART_Parity = USART_Parity_No;//����żУ��λ
			USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//��Ӳ������������
			USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//�շ�ģʽ
			
			USART_Init(USART3, &USART_InitStructure);   //��ʼ������	3
			USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//�������ڽ����ж�
            USART_Cmd(USART3, ENABLE);                    //ʹ�ܴ���3

}



void USART2_IRQHandler(void)                	//����2�жϷ������
{
		u8 Res2;
	#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ��,��֧��OS.
		OSIntEnter();    
	#endif
		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //�����ж�(�������ݱ���0x0d,0x0a��β��
		{
			Res2=USART_ReceiveData(USART2);
			if((USART2_RX_STA&0x8000)==0)//����δ���
			{
				if(USART2_RX_STA&0x4000)//���յ���0x0d
					{
					if(Res2!=0x0a)USART2_RX_STA=0;//���մ���,���¿�ʼ
					else USART2_RX_STA|=0x8000;	//��������� 
					}
				else //��û�յ�0X0D
					{	
					if(Res2==0x0d)USART2_RX_STA|=0x4000;
					else
						{
						USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res2 ;
						USART2_RX_STA++;
						if(USART2_RX_STA>(USART2_MAX_RECV_LEN-1))USART2_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
						}		 
					}
			} 
		}						
/////////////////////////////////////////////////////////////////////////////////////////// ���Դ���
//						if(IS_unvantra==1){
//									if(Res2==0x23){
//										USART2_RX_STA|=0x8000;	//���������
////										USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res2 ;       //������������ֲ��ܴ��͵�Ī��BUG
////								  	USART2_RX_STA++;
//									}										
//					  		else{ //��û�յ�0X23			
//									USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res2 ;
//									USART2_RX_STA++;
//									if(USART2_RX_STA>(USART2_MAX_RECV_LEN-1))USART2_RX_STA=0;//�������ݴ���,���¿�ʼ����	  		 
//								}
//						}
//						
//								if(USART2_RX_STA&0x4000)//���յ���0x0d
//								{
//									if(Res2!=0x0a)USART2_RX_STA=0;//���մ���,���¿�ʼ
//									else USART2_RX_STA|=0x8000;	//��������� 
//								}
//					  		   else //��û�յ�0X0D
//								{	
//										if(Res2==0x0d)USART2_RX_STA|=0x4000;
//										else
//										{
//											USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res2 ;
//											USART2_RX_STA++;
//											if(USART2_RX_STA>(USART2_MAX_RECV_LEN-1))USART2_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
//										}		 
//								}
						
						 
}


void USART3_IRQHandler(void)                	//����2�жϷ������
{
		u8 Res3;
	#if SYSTEM_SUPPORT_OS 		//���SYSTEM_SUPPORT_OSΪ��,��֧��OS.
		OSIntEnter();    
	#endif
		if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //rfid�����ж�(�������ݱ���0x03,0x0f��β��
		{
			Res3=USART_ReceiveData(USART3);
			USART3_RX_BUF[USART3_RX_STA++]=Res3 ;
			if(USART3_RX_STA==12)
				Rx3Flag = 1;
//			if((USART3_RX_STA&0x8000)==0)//����δ���
//			{
//				if(USART3_RX_STA&0x4000)//���յ���0x03
//					{
//					if(Res3!=0x0f)USART3_RX_STA=0;//���մ���,���¿�ʼ
//					else {
//						USART3_RX_STA|=0x8000;	//���������
//						Rx3Flag = 1 ;
//					}						
//					}
//				else //��û�յ�0X03
//					{	
//					if(Res3==0x03)USART3_RX_STA|=0x4000;
//					else
//						{
//						USART3_RX_BUF[USART2_RX_STA&0X3FFF]=Res3 ;
//						USART3_RX_STA++;
//						if(USART3_RX_STA>(USART3_MAX_RECV_LEN-1))USART3_RX_STA=0;//�������ݴ���,���¿�ʼ����	  
//						}		 
//					}
//			} 
		}	
}



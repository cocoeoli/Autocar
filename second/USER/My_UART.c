#include "My_UART.h"
#include "sys.h"


u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 		    	  //接收缓冲,最大USART2_MAX_RECV_LEN个字节.
u16 USART2_RX_STA=0;   	                              //数据量

u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		    	  //接收缓冲,最大USART3_MAX_RECV_LEN个字节.
u16 USART3_RX_STA=0;   	                              //数据量

extern unsigned char Rx3Flag;

void UART_send_char(const unsigned char Byte, USART_TypeDef* USARTx)
{
   while((USARTx->SR&0X40)==0);//等待上一次发送完毕   
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

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);	// GPIOA时钟
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2,ENABLE);

		     //	USART_DeInit(USART2);  //复位串口2
				 //USART2_TX   PA.2
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2; //PA.2
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
			GPIO_Init(GPIOA, &GPIO_InitStructure); //初始化PA2
			 
				//USART2_RX	  PA.3
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
			GPIO_Init(GPIOA, &GPIO_InitStructure);  //初始化PA3
	
					//Usart1 NVIC 配置
			NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0;//抢占优先级0
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
			NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
			
			USART_InitStructure.USART_BaudRate = bound;   //波特率
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
			USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
			USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
			USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
			USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
			
			USART_Init(USART2, &USART_InitStructure);  //初始化串口	2
			USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);//开启串口接受中断
            USART_Cmd(USART2, ENABLE);                    //使能串口2

}

void USART3_Init(u32 bound)
{  

			GPIO_InitTypeDef GPIO_InitStructure;
			USART_InitTypeDef USART_InitStructure;
	        NVIC_InitTypeDef NVIC_InitStructure;

			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	// GPIOB时钟
			RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART3,ENABLE);

		     //	USART_DeInit(USART3);  //复位串口3
				 //USART3_TX   PB10
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10; 
			GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	//复用推挽输出
			GPIO_Init(GPIOB, &GPIO_InitStructure); //初始化PB10
			 
				//USART3_RX	  PB11
			GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
			GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
			GPIO_Init(GPIOB, &GPIO_InitStructure);  //初始化PB11
	
					//Usart3 NVIC 配置
			NVIC_InitStructure.NVIC_IRQChannel = USART3_IRQn;
			NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;//抢占优先级0
			NVIC_InitStructure.NVIC_IRQChannelSubPriority = 3;		//子优先级3
			NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;			//IRQ通道使能
			NVIC_Init(&NVIC_InitStructure);	//根据指定的参数初始化VIC寄存器
			
			USART_InitStructure.USART_BaudRate = bound;   //波特率
			USART_InitStructure.USART_WordLength = USART_WordLength_8b;//字长为8位数据格式
			USART_InitStructure.USART_StopBits = USART_StopBits_1;//一个停止位
			USART_InitStructure.USART_Parity = USART_Parity_No;//无奇偶校验位
			USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;//无硬件数据流控制
			USART_InitStructure.USART_Mode = USART_Mode_Rx | USART_Mode_Tx;	//收发模式
			
			USART_Init(USART3, &USART_InitStructure);   //初始化串口	3
			USART_ITConfig(USART3, USART_IT_RXNE, ENABLE);//开启串口接受中断
            USART_Cmd(USART3, ENABLE);                    //使能串口3

}



void USART2_IRQHandler(void)                	//串口2中断服务程序
{
		u8 Res2;
	#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真,则支持OS.
		OSIntEnter();    
	#endif
		if(USART_GetITStatus(USART2, USART_IT_RXNE) != RESET)  //接收中断(接收数据必须0x0d,0x0a结尾）
		{
			Res2=USART_ReceiveData(USART2);
			if((USART2_RX_STA&0x8000)==0)//接收未完成
			{
				if(USART2_RX_STA&0x4000)//接收到了0x0d
					{
					if(Res2!=0x0a)USART2_RX_STA=0;//接收错误,重新开始
					else USART2_RX_STA|=0x8000;	//接收完成了 
					}
				else //还没收到0X0D
					{	
					if(Res2==0x0d)USART2_RX_STA|=0x4000;
					else
						{
						USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res2 ;
						USART2_RX_STA++;
						if(USART2_RX_STA>(USART2_MAX_RECV_LEN-1))USART2_RX_STA=0;//接收数据错误,重新开始接收	  
						}		 
					}
			} 
		}						
/////////////////////////////////////////////////////////////////////////////////////////// 测试代码
//						if(IS_unvantra==1){
//									if(Res2==0x23){
//										USART2_RX_STA|=0x8000;	//接收完成了
////										USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res2 ;       //加上这两句出现不能传送的莫名BUG
////								  	USART2_RX_STA++;
//									}										
//					  		else{ //还没收到0X23			
//									USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res2 ;
//									USART2_RX_STA++;
//									if(USART2_RX_STA>(USART2_MAX_RECV_LEN-1))USART2_RX_STA=0;//接收数据错误,重新开始接收	  		 
//								}
//						}
//						
//								if(USART2_RX_STA&0x4000)//接收到了0x0d
//								{
//									if(Res2!=0x0a)USART2_RX_STA=0;//接收错误,重新开始
//									else USART2_RX_STA|=0x8000;	//接收完成了 
//								}
//					  		   else //还没收到0X0D
//								{	
//										if(Res2==0x0d)USART2_RX_STA|=0x4000;
//										else
//										{
//											USART2_RX_BUF[USART2_RX_STA&0X3FFF]=Res2 ;
//											USART2_RX_STA++;
//											if(USART2_RX_STA>(USART2_MAX_RECV_LEN-1))USART2_RX_STA=0;//接收数据错误,重新开始接收	  
//										}		 
//								}
						
						 
}


void USART3_IRQHandler(void)                	//串口2中断服务程序
{
		u8 Res3;
	#if SYSTEM_SUPPORT_OS 		//如果SYSTEM_SUPPORT_OS为真,则支持OS.
		OSIntEnter();    
	#endif
		if(USART_GetITStatus(USART3, USART_IT_RXNE) != RESET)  //rfid接收中断(接收数据必须0x03,0x0f结尾）
		{
			Res3=USART_ReceiveData(USART3);
			USART3_RX_BUF[USART3_RX_STA++]=Res3 ;
			if(USART3_RX_STA==12)
				Rx3Flag = 1;
//			if((USART3_RX_STA&0x8000)==0)//接收未完成
//			{
//				if(USART3_RX_STA&0x4000)//接收到了0x03
//					{
//					if(Res3!=0x0f)USART3_RX_STA=0;//接收错误,重新开始
//					else {
//						USART3_RX_STA|=0x8000;	//接收完成了
//						Rx3Flag = 1 ;
//					}						
//					}
//				else //还没收到0X03
//					{	
//					if(Res3==0x03)USART3_RX_STA|=0x4000;
//					else
//						{
//						USART3_RX_BUF[USART2_RX_STA&0X3FFF]=Res3 ;
//						USART3_RX_STA++;
//						if(USART3_RX_STA>(USART3_MAX_RECV_LEN-1))USART3_RX_STA=0;//接收数据错误,重新开始接收	  
//						}		 
//					}
//			} 
		}	
}



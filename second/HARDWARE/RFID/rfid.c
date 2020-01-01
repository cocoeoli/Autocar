#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "my_uart.h"
#include "rfid.h"
#include "inv_mpu.h"

u8 dir_flag=0;                                                  //方向标志 1--北  2--东  3--南  4--西
unsigned char Rx3Flag = 0;
extern u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		    	  //接收缓冲,最大USART3_MAX_RECV_LEN个字节.
extern u16 USART3_RX_STA;   	                              //数据量


unsigned char Cmd_Read_Id[8] = {0x01,0x08,0xa1,0x20,0x00,0x00,0x00,0x00};
unsigned char Cmd_Read_Block[8]	= {0x01,0x08,0xa3,0x20,0x00,0x00,0x00,0x00};
unsigned char Cmd_Write_Block[23] = {0x01,0x17,0xa5,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

unsigned char WBlockData[16] = {0x11,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
//CircularBuffer *Uart2_Circular_Buffer;

	 
//发送新封装的包
void RfidUart_Send_Data(unsigned char *buf,unsigned char num, USART_TypeDef* USARTx)
{
	unsigned char i;

	for(i=0;i<num;i++)
	{ 
	 	USART_SendData(USARTx, buf[i]);
	 	while (USART_GetFlagStatus(USARTx, USART_FLAG_TXE) == RESET);
	}	
}


unsigned char RxCheckSum(unsigned char *ptr,unsigned char len)
{
	unsigned char i;
	unsigned char checksum;
	checksum = 0;
	for(i=0;i<(len-1);i++)
	{
		   checksum ^= ptr[i];
	}
	checksum = ~checksum;                        //按位取反防止校验位影响
	if(ptr[len-1] == checksum)
		return 	STATUS_OK;
	else 
		return 	STATUS_ERR;
}

void TxCheckSum(unsigned char *ptr,unsigned char len)
{
	unsigned char i;
	unsigned char checksum;
	checksum = 0;
	for(i=0;i<(len-1);i++)
	{
		   checksum ^= ptr[i];
	}
	checksum = ~checksum;
	ptr[len-1] = checksum;
}

//ReadId（）:读IC卡ID号（卡号）
//参数：*idout，读取的卡号保存到它所指向的存储空间
//返回值：0：成功读取卡号，1：读卡号失败
unsigned char ReadId(unsigned char *idout)
{
	unsigned char status;
	unsigned char i;
	Cmd_Read_Id[5] = 0x01;//开启蜂鸣器提示
	//Cmd_Read_Id[5] = 0x00;//关闭蜂鸣器提示
	TxCheckSum(Cmd_Read_Id,Cmd_Read_Id[1]);		//计算校验和
	RfidUart_Send_Data(Cmd_Read_Id,Cmd_Read_Id[1],USART3);		 //发送读卡号ID命令
	delay_ms(200);//等待模块返回数据，大于150MS
 	if(Rx3Flag == 1)
 	{	
		Rx3Flag = 0;
//		status = RxCheckSum(USART3_RX_BUF,USART3_RX_BUF[1]);//对接收到的数据校验
//		if(status != STATUS_OK)  //判断校验和是否正确
//		{
//			return STATUS_ERR;
//		}
		status = USART3_RX_BUF[4];
		if(status != STATUS_OK)	//判断是否正确的读到卡
		{
		 	return STATUS_ERR;
		}
		else{
			for(i=0;i<6;i++)//获取卡号ID，6字节		 
			{
				idout[i] = USART3_RX_BUF[i+5];//从数组的第5个字节开始为卡号，长度为6字节
			}
			return STATUS_OK;		 //成功返回0 
		}
//		if((USART3_RX_BUF[0] == 0x01)&&(USART3_RX_BUF[2] == 0xa1))//判断是否为读卡号返回的数据包
//		{
//			for(i=0;i<6;i++)//获取卡号ID，6字节		 
//			{
//				idout[i] = USART3_RX_BUF[i+5];//从数组的第5个字节开始为卡号，长度为6字节
//			}
//			return STATUS_OK;		 //成功返回0 
//		}
 	} 
	return STATUS_ERR;			//失败返回1
}



////ReadId（）:读IC卡数据块
////参数：*idout，读取的数据保存到它所指向的存储空间
////参数：block，块号
////返回值：0：成功读取，1：读读取失败
//unsigned char ReadDataFromBlock(unsigned char *dataout,unsigned char block)
//{
//	unsigned char status;
//	unsigned char i;
//	Cmd_Read_Block[4] = block;
//	Cmd_Read_Block[5] = 0x01;//开启蜂鸣器提示
////	Cmd_Read_Block[5] = 0x00;//关闭蜂鸣器提示
//	TxCheckSum(Cmd_Read_Block,Cmd_Read_Block[1]);	//数据校验
//	RfidUart_Send_Data(Cmd_Read_Block,Cmd_Read_Block[1],USART3);		 //发送读数据块命令
//	delay_ms(200);//等待模块返回数据，大于150MS
// 	if(Rx3Flag == 1)
// 	{	
//		Rx3Flag = 0;
//		status = RxCheckSum(USART3_RX_BUF,USART3_RX_BUF[1]);//对接收到的数据校验
//		if(status != STATUS_OK)		 //判断校验和是否正确
//		{
//			return 	STATUS_ERR;
//		}
//		status = USART3_RX_BUF[4];		//获取返回包状态
//		if(status != STATUS_OK)	//判断是否正确的读到卡
//		{
//			return STATUS_ERR;
//		}
//		if((USART3_RX_BUF[0] == 0x01)&&(USART3_RX_BUF[2] == 0xa3))//判断是否为读块数据返回的数据包
//		{
//			for(i=0;i<16;i++)//获取块数据，16字节	，一个数据块的大小为16字节	 
//			{
//				dataout[i] = USART3_RX_BUF[i+5];//从数组的第5个字节开始为数据，长度为16字节
//			}
//			return STATUS_OK;		 //成功返回0
//		}
//	}
//	return STATUS_ERR;			//失败返回1
//}

////ReadId（）:写数据到指定的数据块
////参数：*idout，指向要写入数据的缓冲区
////参数：block，块号
////返回值：0：写入成功，1：写入失败
//unsigned char WriteDataToBlock(unsigned char *datain,unsigned char block)
//{
//	unsigned char status;
//	unsigned char i;
//	Cmd_Write_Block[4] = block;
//	for(i=0;i<16;i++)
//	{
//		Cmd_Write_Block[6+i] = datain[i];
//	}
//	TxCheckSum(Cmd_Write_Block,Cmd_Write_Block[1]);	//数据校验
//	RfidUart_Send_Data(Cmd_Read_Block,Cmd_Read_Block[1],USART3);		 //发送读数据块命令
//	delay_ms(200);//等待模块返回数据，大于150MS
// 	if(Rx3Flag == 1)
// 	{	
//		Rx3Flag = 0;
//		status = RxCheckSum(USART3_RX_BUF,USART3_RX_BUF[1]);//对返回的数据进行校验
//		if(status != STATUS_OK) //判断校验是否通过
//		{
//			return STATUS_ERR;
//		}
//		status = USART3_RX_BUF[4];
//		if(status != STATUS_OK) //判断校验是否通过
//		{
//			return STATUS_ERR;
//		}
//		if((USART3_RX_BUF[0] == 0x01)&&(USART3_RX_BUF[2] == 0xa4))//判断是否为写块数据返回的数据包
//		{
//				return STATUS_OK;		 //成功返回0
//		}
// 	} 
//	return STATUS_ERR;			//失败返回1
//}


void Read_mode(u8 Mode)
{	
        u8 i;
		unsigned char status;
		unsigned char id[10];
//		unsigned char blockdata[16];
		
        switch(Mode){
			case 1: {	
				    Rx3Flag = 0;
//					status = ReadId(id);  //读卡号
				    status = USART3_RX_BUF[4];
					if(status != STATUS_OK)	//判断是否正确的读到卡
					{
						UART_send_string("Rifd read error!\n", USART2);//错误处理
						USART3_RX_STA=0;
						return;
					}
					else{
						for(i=0;i<6;i++)//获取卡号ID，6字节		 
						{
							id[i] = USART3_RX_BUF[i+5];//从数组的第5个字节开始为卡号，长度为6字节
						}
					
					}
				    id[6] = matrix_num(); //卡矩阵值
				    id[7] = dir_pro();    //方向可行标志
			    	id[8] = dir_flag;     //现在面向方向
				    id[9] = 0x3f;         //rfid读卡标志位   0x3f结尾
					
					if(status == STATUS_OK)	//判断读卡号是否成功，等于0成功
					{
					//	UART_send_string("Rifd Card ID:", USART2);
						RfidUart_Send_Data(id, 10, USART2);	 //从串口2把读取到的卡号数据帧发送出去
						delay_ms(500);         //保护卡号
						USART3_RX_STA=0;
					}
					else
					{
						UART_send_string("Rifd read error!\n", USART2);//错误处理
						USART3_RX_STA=0;
					}
					break;
				}
//			case 2: {
//					status = WriteDataToBlock(WBlockData,2);  //把数组WBlockData[]中的数据写入指定块，这里写入到块2
//					if(status == STATUS_OK)		 //判读是否写入正确	，等于0成功
//					{
//						;	// 写入正确处理
//					}
//					else
//					{
//						;//错误处理
//					}
//					break;
//				}
//			case 3: {
//					status = ReadDataFromBlock(blockdata,2);	 //读块2的数据到数组blockdata[]中
//					if(status == STATUS_OK)	 //判断是否读取成功	，等于0成功
//					{
//						RfidUart_Send_Data(blockdata,6, USART2);	//从串口2把读取到的块数据发送出去
//					}
//					else
//					{
//						;//错误处理
//					}
//					break;
//				}
			default :   UART_send_string("Rifd read error!\n", USART2);//错误处理
				
		}
		RfidUart_Send_Data(Cmd_Read_Id,Cmd_Read_Id[1],USART3);		 //发送读卡号ID命令

}


u8 matrix_num(void)                //卡号处理函数（辅助标记方向）
{
	static u8 matrix_num = 0;
	float temp, dir;
	
	while(mpu_dmp_get_data(&temp,&temp,&dir)!=0);
	
	if( (dir>0 && dir<5) || (dir<0 && dir>-5)){       //北方向  
		matrix_num += 1;
		dir_flag = 1;
	}else{
		if( dir>85 && dir<95){                        //东方向
			matrix_num += 5;
			dir_flag = 2;
		}else{
			if( dir>175 || dir < -175 ){              //南方向
				matrix_num -= 1;
				dir_flag = 3;
				
			}else{                                    //西方向
				matrix_num -= 5;
				dir_flag = 4; 
			}
		}
	}
	
	return matrix_num;			//返回给定卡值
	
}


u8 dir_pro(void)                //方向标志位， 低四位代表哪个方向可行  bit0--北  bit1--东 bit2--南 bit3--西 
{
	u8 Flag=0;
	switch(dir_flag){                    
		case 1: Flag |= 0x04;
		break;
		case 2: Flag |= 0x08;
		break;
		case 3: Flag |= 0x01;
		break;
		case 4: Flag |= 0x02;
	}
	
	return Flag;
}



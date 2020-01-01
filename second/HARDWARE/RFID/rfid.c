#include "stm32f10x.h"
#include "usart.h"
#include "delay.h"
#include "my_uart.h"
#include "rfid.h"
#include "inv_mpu.h"

u8 dir_flag=0;                                                  //�����־ 1--��  2--��  3--��  4--��
unsigned char Rx3Flag = 0;
extern u8 USART3_RX_BUF[USART3_MAX_RECV_LEN]; 		    	  //���ջ���,���USART3_MAX_RECV_LEN���ֽ�.
extern u16 USART3_RX_STA;   	                              //������


unsigned char Cmd_Read_Id[8] = {0x01,0x08,0xa1,0x20,0x00,0x00,0x00,0x00};
unsigned char Cmd_Read_Block[8]	= {0x01,0x08,0xa3,0x20,0x00,0x00,0x00,0x00};
unsigned char Cmd_Write_Block[23] = {0x01,0x17,0xa5,0x20,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};

unsigned char WBlockData[16] = {0x11,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f};
//CircularBuffer *Uart2_Circular_Buffer;

	 
//�����·�װ�İ�
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
	checksum = ~checksum;                        //��λȡ����ֹУ��λӰ��
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

//ReadId����:��IC��ID�ţ����ţ�
//������*idout����ȡ�Ŀ��ű��浽����ָ��Ĵ洢�ռ�
//����ֵ��0���ɹ���ȡ���ţ�1��������ʧ��
unsigned char ReadId(unsigned char *idout)
{
	unsigned char status;
	unsigned char i;
	Cmd_Read_Id[5] = 0x01;//������������ʾ
	//Cmd_Read_Id[5] = 0x00;//�رշ�������ʾ
	TxCheckSum(Cmd_Read_Id,Cmd_Read_Id[1]);		//����У���
	RfidUart_Send_Data(Cmd_Read_Id,Cmd_Read_Id[1],USART3);		 //���Ͷ�����ID����
	delay_ms(200);//�ȴ�ģ�鷵�����ݣ�����150MS
 	if(Rx3Flag == 1)
 	{	
		Rx3Flag = 0;
//		status = RxCheckSum(USART3_RX_BUF,USART3_RX_BUF[1]);//�Խ��յ�������У��
//		if(status != STATUS_OK)  //�ж�У����Ƿ���ȷ
//		{
//			return STATUS_ERR;
//		}
		status = USART3_RX_BUF[4];
		if(status != STATUS_OK)	//�ж��Ƿ���ȷ�Ķ�����
		{
		 	return STATUS_ERR;
		}
		else{
			for(i=0;i<6;i++)//��ȡ����ID��6�ֽ�		 
			{
				idout[i] = USART3_RX_BUF[i+5];//������ĵ�5���ֽڿ�ʼΪ���ţ�����Ϊ6�ֽ�
			}
			return STATUS_OK;		 //�ɹ�����0 
		}
//		if((USART3_RX_BUF[0] == 0x01)&&(USART3_RX_BUF[2] == 0xa1))//�ж��Ƿ�Ϊ�����ŷ��ص����ݰ�
//		{
//			for(i=0;i<6;i++)//��ȡ����ID��6�ֽ�		 
//			{
//				idout[i] = USART3_RX_BUF[i+5];//������ĵ�5���ֽڿ�ʼΪ���ţ�����Ϊ6�ֽ�
//			}
//			return STATUS_OK;		 //�ɹ�����0 
//		}
 	} 
	return STATUS_ERR;			//ʧ�ܷ���1
}



////ReadId����:��IC�����ݿ�
////������*idout����ȡ�����ݱ��浽����ָ��Ĵ洢�ռ�
////������block�����
////����ֵ��0���ɹ���ȡ��1������ȡʧ��
//unsigned char ReadDataFromBlock(unsigned char *dataout,unsigned char block)
//{
//	unsigned char status;
//	unsigned char i;
//	Cmd_Read_Block[4] = block;
//	Cmd_Read_Block[5] = 0x01;//������������ʾ
////	Cmd_Read_Block[5] = 0x00;//�رշ�������ʾ
//	TxCheckSum(Cmd_Read_Block,Cmd_Read_Block[1]);	//����У��
//	RfidUart_Send_Data(Cmd_Read_Block,Cmd_Read_Block[1],USART3);		 //���Ͷ����ݿ�����
//	delay_ms(200);//�ȴ�ģ�鷵�����ݣ�����150MS
// 	if(Rx3Flag == 1)
// 	{	
//		Rx3Flag = 0;
//		status = RxCheckSum(USART3_RX_BUF,USART3_RX_BUF[1]);//�Խ��յ�������У��
//		if(status != STATUS_OK)		 //�ж�У����Ƿ���ȷ
//		{
//			return 	STATUS_ERR;
//		}
//		status = USART3_RX_BUF[4];		//��ȡ���ذ�״̬
//		if(status != STATUS_OK)	//�ж��Ƿ���ȷ�Ķ�����
//		{
//			return STATUS_ERR;
//		}
//		if((USART3_RX_BUF[0] == 0x01)&&(USART3_RX_BUF[2] == 0xa3))//�ж��Ƿ�Ϊ�������ݷ��ص����ݰ�
//		{
//			for(i=0;i<16;i++)//��ȡ�����ݣ�16�ֽ�	��һ�����ݿ�Ĵ�СΪ16�ֽ�	 
//			{
//				dataout[i] = USART3_RX_BUF[i+5];//������ĵ�5���ֽڿ�ʼΪ���ݣ�����Ϊ16�ֽ�
//			}
//			return STATUS_OK;		 //�ɹ�����0
//		}
//	}
//	return STATUS_ERR;			//ʧ�ܷ���1
//}

////ReadId����:д���ݵ�ָ�������ݿ�
////������*idout��ָ��Ҫд�����ݵĻ�����
////������block�����
////����ֵ��0��д��ɹ���1��д��ʧ��
//unsigned char WriteDataToBlock(unsigned char *datain,unsigned char block)
//{
//	unsigned char status;
//	unsigned char i;
//	Cmd_Write_Block[4] = block;
//	for(i=0;i<16;i++)
//	{
//		Cmd_Write_Block[6+i] = datain[i];
//	}
//	TxCheckSum(Cmd_Write_Block,Cmd_Write_Block[1]);	//����У��
//	RfidUart_Send_Data(Cmd_Read_Block,Cmd_Read_Block[1],USART3);		 //���Ͷ����ݿ�����
//	delay_ms(200);//�ȴ�ģ�鷵�����ݣ�����150MS
// 	if(Rx3Flag == 1)
// 	{	
//		Rx3Flag = 0;
//		status = RxCheckSum(USART3_RX_BUF,USART3_RX_BUF[1]);//�Է��ص����ݽ���У��
//		if(status != STATUS_OK) //�ж�У���Ƿ�ͨ��
//		{
//			return STATUS_ERR;
//		}
//		status = USART3_RX_BUF[4];
//		if(status != STATUS_OK) //�ж�У���Ƿ�ͨ��
//		{
//			return STATUS_ERR;
//		}
//		if((USART3_RX_BUF[0] == 0x01)&&(USART3_RX_BUF[2] == 0xa4))//�ж��Ƿ�Ϊд�����ݷ��ص����ݰ�
//		{
//				return STATUS_OK;		 //�ɹ�����0
//		}
// 	} 
//	return STATUS_ERR;			//ʧ�ܷ���1
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
//					status = ReadId(id);  //������
				    status = USART3_RX_BUF[4];
					if(status != STATUS_OK)	//�ж��Ƿ���ȷ�Ķ�����
					{
						UART_send_string("Rifd read error!\n", USART2);//������
						USART3_RX_STA=0;
						return;
					}
					else{
						for(i=0;i<6;i++)//��ȡ����ID��6�ֽ�		 
						{
							id[i] = USART3_RX_BUF[i+5];//������ĵ�5���ֽڿ�ʼΪ���ţ�����Ϊ6�ֽ�
						}
					
					}
				    id[6] = matrix_num(); //������ֵ
				    id[7] = dir_pro();    //������б�־
			    	id[8] = dir_flag;     //����������
				    id[9] = 0x3f;         //rfid������־λ   0x3f��β
					
					if(status == STATUS_OK)	//�ж϶������Ƿ�ɹ�������0�ɹ�
					{
					//	UART_send_string("Rifd Card ID:", USART2);
						RfidUart_Send_Data(id, 10, USART2);	 //�Ӵ���2�Ѷ�ȡ���Ŀ�������֡���ͳ�ȥ
						delay_ms(500);         //��������
						USART3_RX_STA=0;
					}
					else
					{
						UART_send_string("Rifd read error!\n", USART2);//������
						USART3_RX_STA=0;
					}
					break;
				}
//			case 2: {
//					status = WriteDataToBlock(WBlockData,2);  //������WBlockData[]�е�����д��ָ���飬����д�뵽��2
//					if(status == STATUS_OK)		 //�ж��Ƿ�д����ȷ	������0�ɹ�
//					{
//						;	// д����ȷ����
//					}
//					else
//					{
//						;//������
//					}
//					break;
//				}
//			case 3: {
//					status = ReadDataFromBlock(blockdata,2);	 //����2�����ݵ�����blockdata[]��
//					if(status == STATUS_OK)	 //�ж��Ƿ��ȡ�ɹ�	������0�ɹ�
//					{
//						RfidUart_Send_Data(blockdata,6, USART2);	//�Ӵ���2�Ѷ�ȡ���Ŀ����ݷ��ͳ�ȥ
//					}
//					else
//					{
//						;//������
//					}
//					break;
//				}
			default :   UART_send_string("Rifd read error!\n", USART2);//������
				
		}
		RfidUart_Send_Data(Cmd_Read_Id,Cmd_Read_Id[1],USART3);		 //���Ͷ�����ID����

}


u8 matrix_num(void)                //���Ŵ�������������Ƿ���
{
	static u8 matrix_num = 0;
	float temp, dir;
	
	while(mpu_dmp_get_data(&temp,&temp,&dir)!=0);
	
	if( (dir>0 && dir<5) || (dir<0 && dir>-5)){       //������  
		matrix_num += 1;
		dir_flag = 1;
	}else{
		if( dir>85 && dir<95){                        //������
			matrix_num += 5;
			dir_flag = 2;
		}else{
			if( dir>175 || dir < -175 ){              //�Ϸ���
				matrix_num -= 1;
				dir_flag = 3;
				
			}else{                                    //������
				matrix_num -= 5;
				dir_flag = 4; 
			}
		}
	}
	
	return matrix_num;			//���ظ�����ֵ
	
}


u8 dir_pro(void)                //�����־λ�� ����λ�����ĸ��������  bit0--��  bit1--�� bit2--�� bit3--�� 
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



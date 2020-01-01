#ifndef __RFID_H
#define __RFID_H	 
#include "sys.h"

#define STATUS_OK	  0x00
#define STATUS_ERR    0x01


void RfidUart_Send_Data(unsigned char *buf,unsigned char num, USART_TypeDef* USARTx);
unsigned char RxCheckSum(unsigned char *ptr,unsigned char len);
void TxCheckSum(unsigned char *ptr,unsigned char len);

unsigned char ReadId(unsigned char *idout);
//unsigned char ReadDataFromBlock(unsigned char *dataout,unsigned char block);
//unsigned char WriteDataToBlock(unsigned char *datain,unsigned char block);
void Read_mode(u8 Mode);
u8 matrix_num(void);
u8 dir_pro(void);

		 				    
#endif

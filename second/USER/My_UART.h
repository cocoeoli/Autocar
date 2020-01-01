
#ifndef MY_UART_H_
#define MY_UART_H_

#include "stm32f10x_usart.h"
#include "stm32f10x_rcc.h"

#define USART2_MAX_RECV_LEN		100				//�����ջ����ֽ���
#define USART2_MAX_SEND_LEN		50				//����ͻ����ֽ���

#define USART3_MAX_RECV_LEN		100				//�����ջ����ֽ���
#define USART3_MAX_SEND_LEN		50				//����ͻ����ֽ���


void UART_send_char(const unsigned char Byte, USART_TypeDef* USARTx);
char UART_recive_char(USART_TypeDef* USARTx);
void UART_send_string(const char *sendbuf, USART_TypeDef* USARTx);

void USART2_Init(u32 bound);
void USART3_Init(u32 bound);
#endif 

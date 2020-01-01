#include "ultrasonic.h"
#include "stm32f10x.h"
#include "delay.h"
#include "app_motor.h"
#include "my_uart.h"
#include "string.h"
#include "stdio.h"
#include "iwdg.h"


/*��¼��ʱ���������*/
unsigned int overcount = 0;

extern u8 Iscar_run;
extern float set_yaw;



void Barrier_detach(void)
{
	char p[10];
	static u8 errway=0;
	short dir_flag;
	float dir, lenth;	
	lenth = bsp_getUltrasonicDistance();
	dir = set_yaw;
	sprintf(p, "L: %.2f\n", lenth);
	UART_send_string(p, USART2);
	IWDG_Feed();
	if(lenth > 50){   //ǰ�����ϰ�
		Car_Run(Current_speed+300);
		delay_ms(500);
		Iscar_run = 1;
		USART_Cmd(USART3, ENABLE);
	}
	else{
		
	    delay_ms(500);
		if( (dir>=0 && dir<5) || (dir<=0 && dir>-5)){     //������  
		dir_flag = 1;
		}else{
			if( dir<-65 && dir>-125){                        //������
				dir_flag = 2;
			}else{
				if( dir > 155 || dir < -155 ){              //�Ϸ���
					dir_flag = 3;
					
				}else{                                    //������
					dir_flag = 4; 
				}
			}
		}
		if(errway < 2){
			switch(dir_flag){    //�ж��ϰ��﷽��
				case 1:{
					UART_send_string("B1", USART2);
					break;
				}
				case 2:{
					UART_send_string("B2", USART2);
					break;
				}
				case 3:{
					UART_send_string("B3", USART2);
					errway++;
					break;
				}
				case 4:{
					UART_send_string("B4", USART2);
					break;
				}
			
		    }
		}
		else{  //���������������������
			UART_send_string("B5", USART2);
			errway = 0;
		}
		delay_ms(500);   //�����ֶα���
	 }
	
}

float bsp_getUltrasonicDistance(void)
{
	float length = 0, sum = 0;
	u16 tim;
	unsigned int  i = 0;

	/*��5�����ݼ���һ��ƽ��ֵ*/
	while(i != 5)
	{
		GPIO_SetBits(TRIG_PORT, TRIG_PIN);  //�����źţ���Ϊ�����ź�
		delay_us(20);  						//�ߵ�ƽ�źų���10us
		GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

		/*�ȴ������ź�*/
		while(GPIO_ReadInputDataBit(ECHO_PORT, ECHO_PIN) == RESET);
		TIM_Cmd(TIM2,ENABLE);//�����źŵ�����������ʱ������
		
		i+=1; //ÿ�յ�һ�λ����ź�+1���յ�5�ξͼ����ֵ
		while(GPIO_ReadInputDataBit(ECHO_PORT, ECHO_PIN) == SET);//�����ź���ʧ
		TIM_Cmd(TIM2, DISABLE);//�رն�ʱ��
		
		tim = TIM_GetCounter(TIM2);//��ȡ��TIM2���Ĵ����еļ���ֵ��һ�߼�������ź�ʱ��
		
		length = (tim + overcount * 1000) / 58.0;//ͨ�������źż������
		
		sum = length + sum;
		TIM2->CNT = 0;  //��TIM2�����Ĵ����ļ���ֵ����
		overcount = 0;  //�ж������������
		delay_ms(50);
	}
	length = sum / 5;
	return length;		//������Ϊ��������ֵ
}


void Ultrasonic_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(TRIG_RCC, ENABLE);
     
        //IO��ʼ��
    GPIO_InitStructure.GPIO_Pin =TRIG_PIN;       //���͵�ƽ����
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//�������
    GPIO_Init(TRIG_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(TRIG_PORT,TRIG_PIN);
     
    GPIO_InitStructure.GPIO_Pin =   ECHO_PIN;     //���ص�ƽ����
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//��������
    GPIO_Init(ECHO_PORT, &GPIO_InitStructure);  
    GPIO_ResetBits(TRIG_PORT,ECHO_PIN);    
       

}

void Timer2_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);    //  ʹ��TIM3

	TIM_TimeBaseStructure.TIM_Period = 999;    //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler = 71; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  ����Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

    NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;        //�����ȼ�2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;               //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	TIM_Cmd(TIM2, DISABLE);//�رն�ʱ��ʹ��

}

void TIM2_IRQHandler(void) //�жϣ��������źźܳ��ǣ�����ֵ������ظ����������ж��������������
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);//����жϱ�־
		overcount++;	
	}
}

#include "ultrasonic.h"
#include "stm32f10x.h"
#include "delay.h"
#include "app_motor.h"
#include "my_uart.h"
#include "string.h"
#include "stdio.h"
#include "iwdg.h"


/*记录定时器溢出次数*/
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
	if(lenth > 50){   //前方无障碍
		Car_Run(Current_speed+300);
		delay_ms(500);
		Iscar_run = 1;
		USART_Cmd(USART3, ENABLE);
	}
	else{
		
	    delay_ms(500);
		if( (dir>=0 && dir<5) || (dir<=0 && dir>-5)){     //北方向  
		dir_flag = 1;
		}else{
			if( dir<-65 && dir>-125){                        //东方向
				dir_flag = 2;
			}else{
				if( dir > 155 || dir < -155 ){              //南方向
					dir_flag = 3;
					
				}else{                                    //西方向
					dir_flag = 4; 
				}
			}
		}
		if(errway < 2){
			switch(dir_flag){    //判断障碍物方向
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
		else{  //如果北东南三方都不能走
			UART_send_string("B5", USART2);
			errway = 0;
		}
		delay_ms(500);   //发送字段保护
	 }
	
}

float bsp_getUltrasonicDistance(void)
{
	float length = 0, sum = 0;
	u16 tim;
	unsigned int  i = 0;

	/*测5次数据计算一次平均值*/
	while(i != 5)
	{
		GPIO_SetBits(TRIG_PORT, TRIG_PIN);  //拉高信号，作为触发信号
		delay_us(20);  						//高电平信号超过10us
		GPIO_ResetBits(TRIG_PORT, TRIG_PIN);

		/*等待回响信号*/
		while(GPIO_ReadInputDataBit(ECHO_PORT, ECHO_PIN) == RESET);
		TIM_Cmd(TIM2,ENABLE);//回响信号到来，开启定时器计数
		
		i+=1; //每收到一次回响信号+1，收到5次就计算均值
		while(GPIO_ReadInputDataBit(ECHO_PORT, ECHO_PIN) == SET);//回响信号消失
		TIM_Cmd(TIM2, DISABLE);//关闭定时器
		
		tim = TIM_GetCounter(TIM2);//获取计TIM2数寄存器中的计数值，一边计算回响信号时间
		
		length = (tim + overcount * 1000) / 58.0;//通过回响信号计算距离
		
		sum = length + sum;
		TIM2->CNT = 0;  //将TIM2计数寄存器的计数值清零
		overcount = 0;  //中断溢出次数清零
		delay_ms(50);
	}
	length = sum / 5;
	return length;		//距离作为函数返回值
}


void Ultrasonic_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    RCC_APB2PeriphClockCmd(TRIG_RCC, ENABLE);
     
        //IO初始化
    GPIO_InitStructure.GPIO_Pin =TRIG_PIN;       //发送电平引脚
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;//推挽输出
    GPIO_Init(TRIG_PORT, &GPIO_InitStructure);
    GPIO_ResetBits(TRIG_PORT,TRIG_PIN);
     
    GPIO_InitStructure.GPIO_Pin =   ECHO_PIN;     //返回电平引脚
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;//浮空输入
    GPIO_Init(ECHO_PORT, &GPIO_InitStructure);  
    GPIO_ResetBits(TRIG_PORT,ECHO_PIN);    
       

}

void Timer2_Init(void)
{
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);    //  使能TIM3

	TIM_TimeBaseStructure.TIM_Period = 999;    //设置在下一个更新事件装入活动的自动重装载寄存器周期的值	
	TIM_TimeBaseStructure.TIM_Prescaler = 71; //设置用来作为TIMx时钟频率除数的预分频值  不分频
	TIM_TimeBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1; //设置时钟分割:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM向上计数模式
	TIM_TimeBaseInit(TIM2, &TIM_TimeBaseStructure); //根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位

    NVIC_InitStructure.NVIC_IRQChannel=TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=2;        //主优先级2
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;               //子优先级3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ITConfig(TIM2,TIM_IT_Update,ENABLE);
	
	TIM_Cmd(TIM2, DISABLE);//关闭定时器使能

}

void TIM2_IRQHandler(void) //中断，当回响信号很长是，计数值溢出后重复计数，用中断来保存溢出次数
{
	if(TIM_GetITStatus(TIM2,TIM_IT_Update) != RESET)
	{
		TIM_ClearITPendingBit(TIM2, TIM_IT_Update);//清除中断标志
		overcount++;	
	}
}

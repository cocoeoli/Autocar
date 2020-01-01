#include "ultrasonic.h"
#include "stm32f10x.h"
#include "delay.h"


/*记录定时器溢出次数*/
unsigned int overcount = 0;


/**
* Function       bsp_getUltrasonicDistance
* @author        liusen
* @date          2017.07.20    
* @brief         获取超声距离
* @param[in]     void
* @param[out]    void
* @return        距离浮点值
* @par History   无
*/

double bsp_getUltrasonicDistance(void)
{
	double length = 0, sum = 0;
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
		delay_ms(100);
	}
	length = sum / 5;
	return length;		//距离作为函数返回值
}

/**
* Function       bsp_Ultrasonic_Timer2_Init
* @author        liusen
* @date          2017.07.21    
* @brief         初始化定时器TIM2
* @param[in]     void
* @param[out]    void
* @return        距离浮点值
* @par History   无
*/
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

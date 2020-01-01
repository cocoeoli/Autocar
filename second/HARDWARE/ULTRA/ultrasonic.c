#include "ultrasonic.h"
#include "stm32f10x.h"
#include "delay.h"


/*��¼��ʱ���������*/
unsigned int overcount = 0;


/**
* Function       bsp_getUltrasonicDistance
* @author        liusen
* @date          2017.07.20    
* @brief         ��ȡ��������
* @param[in]     void
* @param[out]    void
* @return        ���븡��ֵ
* @par History   ��
*/

double bsp_getUltrasonicDistance(void)
{
	double length = 0, sum = 0;
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
		delay_ms(100);
	}
	length = sum / 5;
	return length;		//������Ϊ��������ֵ
}

/**
* Function       bsp_Ultrasonic_Timer2_Init
* @author        liusen
* @date          2017.07.21    
* @brief         ��ʼ����ʱ��TIM2
* @param[in]     void
* @param[out]    void
* @return        ���븡��ֵ
* @par History   ��
*/
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

#include "mpu6050.h"
#include "inv_mpu.h"
#include "sys.h"
#include "delay.h"
#include "usart.h"   
 
extern float set_yaw;

u8 mputime_nvic = 1;

//u8 MPU_Init(void)
//{ 
//	u8 res; 
//	MPU_IIC_Init();//��ʼ��IIC����
//	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X80);	//��λMPU6050
//    delay_ms(100);
//	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);	//����MPU6050 
//	MPU_Set_Gyro_Fsr(3);					//�����Ǵ�����,��2000dps
//	MPU_Set_Accel_Fsr(0);					//���ٶȴ�����,��2g
//	MPU_Set_Rate(500);						//���ò�����500Hz
//	MPU_Write_Byte(MPU_INT_EN_REG,0X00);	//�ر������ж�
//	MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);	//I2C��ģʽ�ر�
//	MPU_Write_Byte(MPU_FIFO_EN_REG,0X10);	//��fifo��zgbuff
//	MPU_Write_Byte(MPU_INTBP_CFG_REG,0X80);	//INT���ŵ͵�ƽ��Ч
//	res=MPU_Read_Byte(MPU_DEVICE_ID_REG); 
//	if(res==MPU_ADDR)//����ID��ȷ
//	{
//		MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X03);	//����CLKSEL,PLL Z��Ϊ�ο�
//		MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X38);	//���ٶȼ�standybyģʽ
//		MPU_Set_Rate(500);						//���ò�����Ϊ500Hz
// 	}else return 1;
//	return 0;
//}
////��ʼ��MPU6050
////����ֵ:0,�ɹ�
////    ����,�������
//u8 MPU_Init(void)
//{ 
//	u8 res; 
//	MPU_IIC_Init();//��ʼ��IIC����
//	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X80);	//��λMPU6050
//    delay_ms(100);
//	MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X00);	//����MPU6050 
//	MPU_Set_Gyro_Fsr(3);					//�����Ǵ�����,��2000dps
//	MPU_Set_Accel_Fsr(0);					//���ٶȴ�����,��2g
//	MPU_Set_Rate(50);						//���ò�����50Hz
//	MPU_Write_Byte(MPU_INT_EN_REG,0X00);	//�ر������ж�
//	MPU_Write_Byte(MPU_USER_CTRL_REG,0X00);	//I2C��ģʽ�ر�
//	MPU_Write_Byte(MPU_FIFO_EN_REG,0X00);	//�ر�FIFO
//	MPU_Write_Byte(MPU_INTBP_CFG_REG,0X80);	//INT���ŵ͵�ƽ��Ч
//	res=MPU_Read_Byte(MPU_DEVICE_ID_REG); 
//	if(res==MPU_ADDR)//����ID��ȷ
//	{
//		MPU_Write_Byte(MPU_PWR_MGMT1_REG,0X01);	//����CLKSEL,PLL X��Ϊ�ο�
//		MPU_Write_Byte(MPU_PWR_MGMT2_REG,0X00);	//���ٶ��������Ƕ�����
//		MPU_Set_Rate(50);						//���ò�����Ϊ50Hz
// 	}else return 1;
//	return 0;
//}
//����MPU6050�����Ǵ����������̷�Χ
//fsr:0,��250dps;1,��500dps;2,��1000dps;3,��2000dps
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_Gyro_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_GYRO_CFG_REG,fsr<<3);//���������������̷�Χ  
}
//����MPU6050���ٶȴ����������̷�Χ
//fsr:0,��2g;1,��4g;2,��8g;3,��16g
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_Accel_Fsr(u8 fsr)
{
	return MPU_Write_Byte(MPU_ACCEL_CFG_REG,fsr<<3);//���ü��ٶȴ����������̷�Χ  
}
//����MPU6050�����ֵ�ͨ�˲���
//lpf:���ֵ�ͨ�˲�Ƶ��(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_LPF(u16 lpf)
{
	u8 data=0;
	if(lpf>=188)data=1;
	else if(lpf>=98)data=2;
	else if(lpf>=42)data=3;
	else if(lpf>=20)data=4;
	else if(lpf>=10)data=5;
	else data=6; 
	return MPU_Write_Byte(MPU_CFG_REG,data);//�������ֵ�ͨ�˲���  
}
//����MPU6050�Ĳ�����(�ٶ�Fs=1KHz)
//rate:4~1000(Hz)
//����ֵ:0,���óɹ�
//    ����,����ʧ�� 
u8 MPU_Set_Rate(u16 rate)
{
	u8 data;
	if(rate>1000)rate=1000;
	if(rate<4)rate=4;
	data=1000/rate-1;
	data=MPU_Write_Byte(MPU_SAMPLE_RATE_REG,data);	//�������ֵ�ͨ�˲���
 	return MPU_Set_LPF(rate/2);	//�Զ�����LPFΪ�����ʵ�һ��
}

//�õ��¶�ֵ
//����ֵ:�¶�ֵ(������100��)
short MPU_Get_Temperature(void)
{
    u8 buf[2]; 
    short raw;
	float temp;
	MPU_Read_Len(MPU_ADDR,MPU_TEMP_OUTH_REG,2,buf); 
    raw=((u16)buf[0]<<8)|buf[1];  
    temp=36.53+((double)raw)/340;  
    return temp*100;;
}
//�õ�������ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
u8 MPU_Get_Gyroscope(short *gx,short *gy,short *gz)
{
    u8 buf[6],res;  
	res=MPU_Read_Len(MPU_ADDR,MPU_GYRO_XOUTH_REG,6,buf);
	if(res==0)
	{
		*gx=((u16)buf[0]<<8)|buf[1];  
		*gy=((u16)buf[2]<<8)|buf[3];  
		*gz=((u16)buf[4]<<8)|buf[5];
	} 	
    return res;;
}
//�õ����ٶ�ֵ(ԭʼֵ)
//gx,gy,gz:������x,y,z���ԭʼ����(������)
//����ֵ:0,�ɹ�
//    ����,�������
u8 MPU_Get_Accelerometer(short *ax,short *ay,short *az)
{
    u8 buf[6],res;  
	res=MPU_Read_Len(MPU_ADDR,MPU_ACCEL_XOUTH_REG,6,buf);
	if(res==0)
	{
		*ax=((u16)buf[0]<<8)|buf[1];  
		*ay=((u16)buf[2]<<8)|buf[3];  
		*az=((u16)buf[4]<<8)|buf[5];
	} 	
    return res;;
}
//IIC����д
//addr:������ַ 
//reg:�Ĵ�����ַ
//len:д�볤��
//buf:������
//����ֵ:0,����
//    ����,�������
u8 MPU_Write_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{
	u8 i; 
    MPU_IIC_Start(); 
	MPU_IIC_Send_Byte((addr<<1)|0);//����������ַ+д����	
	if(MPU_IIC_Wait_Ack())	//�ȴ�Ӧ��
	{
		MPU_IIC_Stop();		 
		return 1;		
	}
    MPU_IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ��
	for(i=0;i<len;i++)
	{
		MPU_IIC_Send_Byte(buf[i]);	//��������
		if(MPU_IIC_Wait_Ack())		//�ȴ�ACK
		{
			MPU_IIC_Stop();	 
			return 1;		 
		}		
	}    
    MPU_IIC_Stop();	 
	return 0;	
} 
//IIC������
//addr:������ַ
//reg:Ҫ��ȡ�ļĴ�����ַ
//len:Ҫ��ȡ�ĳ���
//buf:��ȡ�������ݴ洢��
//����ֵ:0,����
//    ����,�������
u8 MPU_Read_Len(u8 addr,u8 reg,u8 len,u8 *buf)
{ 
 	MPU_IIC_Start(); 
	MPU_IIC_Send_Byte((addr<<1)|0);//����������ַ+д����	
	if(MPU_IIC_Wait_Ack())	//�ȴ�Ӧ��
	{
		MPU_IIC_Stop();		 
		return 1;		
	}
    MPU_IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ��
    MPU_IIC_Start();
	MPU_IIC_Send_Byte((addr<<1)|1);//����������ַ+������	
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ�� 
	while(len)
	{
		if(len==1)*buf=MPU_IIC_Read_Byte(0);//������,����nACK 
		else *buf=MPU_IIC_Read_Byte(1);		//������,����ACK  
		len--;
		buf++; 
	}    
    MPU_IIC_Stop();	//����һ��ֹͣ���� 
	return 0;	
}
//IICдһ���ֽ� 
//reg:�Ĵ�����ַ
//data:����
//����ֵ:0,����
//    ����,�������
u8 MPU_Write_Byte(u8 reg,u8 data) 				 
{ 
    MPU_IIC_Start(); 
	MPU_IIC_Send_Byte((MPU_ADDR<<1)|0);//����������ַ+д����	
	if(MPU_IIC_Wait_Ack())	//�ȴ�Ӧ��
	{
		MPU_IIC_Stop();		 
		return 1;		
	}
    MPU_IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ�� 
	MPU_IIC_Send_Byte(data);//��������
	if(MPU_IIC_Wait_Ack())	//�ȴ�ACK
	{
		MPU_IIC_Stop();	 
		return 1;		 
	}		 
    MPU_IIC_Stop();	 
	return 0;
}
//IIC��һ���ֽ� 
//reg:�Ĵ�����ַ 
//����ֵ:����������
u8 MPU_Read_Byte(u8 reg)
{
	u8 res;
    MPU_IIC_Start(); 
	MPU_IIC_Send_Byte((MPU_ADDR<<1)|0);//����������ַ+д����	
	MPU_IIC_Wait_Ack();		//�ȴ�Ӧ�� 
    MPU_IIC_Send_Byte(reg);	//д�Ĵ�����ַ
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ��
    MPU_IIC_Start();
	MPU_IIC_Send_Byte((MPU_ADDR<<1)|1);//����������ַ+������	
    MPU_IIC_Wait_Ack();		//�ȴ�Ӧ�� 
	res=MPU_IIC_Read_Byte(0);//��ȡ����,����nACK 
    MPU_IIC_Stop();			//����һ��ֹͣ���� 
	return res;		
}

//void ExitC10_init(void)
//{
//	GPIO_InitTypeDef  GPIO_InitStructure;
//	EXTI_InitTypeDef EXTI_InitStructure; 
//    NVIC_InitTypeDef NVIC_InitStructure; 
// 
//    RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO|RCC_APB2Periph_GPIOC,ENABLE); //�ⲿ�жϣ���Ҫʹ�� AFIO ʱ�Ӻ�PCʱ��
//	
//	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;				 //LED0-->PC-10 �˿�����
//	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU; 		     //��������
//	GPIO_Init(GPIOC, &GPIO_InitStructure);					 //�����趨������ʼ��PC.10
//	GPIO_SetBits(GPIOC,GPIO_Pin_10);						 //PC.10
//  
//	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC,GPIO_PinSource10); 
//	EXTI_InitStructure.EXTI_Line=EXTI_Line10; 
//	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;  
//	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;//�½��ش��� 
//	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
//	EXTI_Init(&EXTI_InitStructure);      //���� EXTI_InitStruct ��ָ���Ĳ�����ʼ������ EXTI �Ĵ��� 
//   
//	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;     //ʹ���ⲿ�ж�ͨ�� 
//	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x01;  //��ռ���ȼ� 2��   
//	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;    //�����ȼ� 1 
//	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;    //ʹ���ⲿ�ж�ͨ�� 
//	NVIC_Init(&NVIC_InitStructure);   
//}

//void EXTI15_10_IRQHandler(void) 
//{ 
//	short *tmp;          
//    if(EXTI_GetITStatus(EXTI_Line10)!= RESET){ 
//		while(mpuget_yaw(&set_yaw)!=0);  //mpu6050�ж϶�ȡ
//		while(mpu_get_int_status(tmp));  //����жϱ�־λ
//    } 
//    EXTI_ClearITPendingBit(EXTI_Line10);    //��� LINE10 ��·����λ 
//	
//} 


void TIM3_Init(u16 arr,u16 psc)    //�ж϶�ȡmpu6050��ֵ
{  
	
	TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
	NVIC_InitTypeDef NVIC_InitStructure;
	
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);    //  ʹ��TIM3

	TIM_TimeBaseStructure.TIM_Period = arr;    //��������һ�������¼�װ�����Զ���װ�ؼĴ������ڵ�ֵ	
	TIM_TimeBaseStructure.TIM_Prescaler = psc; //����������ΪTIMxʱ��Ƶ�ʳ�����Ԥ��Ƶֵ  ����Ƶ
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; //����ʱ�ӷָ�:TDTS = Tck_tim
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  //TIM���ϼ���ģʽ
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); //����TIM_TimeBaseInitStruct��ָ���Ĳ�����ʼ��TIMx��ʱ�������λ

    NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=1;        //�����ȼ�1
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=3;               //�����ȼ�3
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	
	TIM_ITConfig(TIM3,TIM_IT_Update,ENABLE);
	TIM_Cmd(TIM3, ENABLE);  //ʹ��TIM3
    
}

void TIM3_IRQHandler(void)
{
	if(TIM_GetITStatus(TIM3,TIM_IT_Update)!=RESET){
		mputime_nvic = 1;
		TIM_ClearITPendingBit(TIM3,TIM_IT_Update);
		TIM_Cmd(TIM3, DISABLE);  //ʧ��TIM3	
	}
}


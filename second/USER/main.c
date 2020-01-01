#include "sys.h"
#include "string.h"
#include "stdio.h"
#include "led.h"
#include "delay.h"
#include "app_motor.h"
#include "bsp_gpio.h"
#include "bsp_motor.h"
#include "usart.h"
#include "esp8266.h"
#include "mpu6050.h"  
#include "inv_mpu.h"
#include "inv_mpu_dmp_motion_driver.h" 
#include "my_uart.h"
#include "pid.h"
#include "iwdg.h"
#include "rfid.h"

extern u8 Rx3Flag;
extern u8 mputime_nvic;                                     //��ʱ���жϱ�־λ
extern u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 			    //wifi���ջ���,���USART2_MAX_RECV_LEN���ֽ�.
extern u16 USART2_RX_STA;   	                           //��־λ���ж��Ƿ��յ���Ϣ

u8 pid_flag=0;
u8 Mode = 2;								     //С������ģʽλ
u8 Iscar_run = 0;								 //�ж�С���Ƿ����˶���
float set_yaw;

void Carsysterm_init(void);

 int main(void)
 {
	// char p[20], i;
	 float ave_yaw=0;
	 Carsysterm_init();
     while(1){
		 switch(Mode){
			 case 0: {
				 IWDG_Feed();  
				 if(USART2_RX_STA&0x8000){		           //wifi�Ƿ���������ж�
					select_mode();                
					USART2_RX_STA=0;				 
				 }
//				 for(i=0; i<3; i++){
//					while(mpuget_yaw(&yaw)!=0) ;
//					ave_yaw += yaw;
//				 }
//				 ave_yaw = ave_yaw/3.0; 
//				 sprintf(p, "yaw: %.3f \n", ave_yaw);
//				 UART_send_string(p, USART2);
				 if(Rx3Flag==1 && Iscar_run==1){           //rfid����� �����������˶���		
		            Car_Stop();
					delay_ms(1000);
					Read_mode(1);
					Iscar_run = 0;
				    USART_Cmd(USART3, DISABLE);                    //ʧ�ܴ���3
				 }
				 delay_ms(1000);
				 break;
			 }
			 case 2:{
			    if(USART2_RX_STA&0x8000){		           //wifi�Ƿ���������ж�
					select_mode();                
					USART2_RX_STA=0;   
				}
				while(mpuget_yaw(&ave_yaw)!=0);
//				 ave_yaw = ave_yaw/3.0; 
//				 sprintf(p, "yaw: %.3f \n", ave_yaw);
//				 UART_send_string(p, USART2);
				 IWDG_Feed(); 
				 if(mputime_nvic!=0){                      //�ж��Ƿ������ʱ���ж�
					if(Iscar_run){
						Car_Run(Current_speed);
						delay_ms(180);
					}
					check_yaw(ave_yaw);
					TIM_Cmd(TIM3, ENABLE);                //��������100MS
					mputime_nvic=0;
				 }
				 ave_yaw = 0;
				if(Rx3Flag==1 && Iscar_run==1){                 //rfid����ж�
					Car_Stop();
					Iscar_run=0;
					Read_mode(1);
				}
				 break;
			 }
			 case 3:{
				 Car_Right(Current_speed);
				 delay_ms(3000);
				 Car_Run(Current_speed);
				 delay_ms(3000);
				 Car_Stop();
				 IWDG_Feed(); 
				 break;
			 }
		 }
			
			
	}
 }

void Carsysterm_init(void)
{
      u8 times=0;
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);    //ʧ���ж����ȼ�2
	  delay_init();	                                     //��ʱ������ʼ��	  
	  LED_Init();		  	                             //��ʼ����LED���ӵ�Ӳ���ӿ�    $$PA8, PD2
	  uart_init(115200);	                             //���ڳ�ʼ��Ϊ115200          $$PA9, PA10  �ж����ȼ� 3 - 3
	  USART2_Init(115200);                               //bound 115200               $$PA2, PA3   �ж����ȼ� 0 - 3
	  USART3_Init(9600);								 //Rfid��Ϣ���տ�
	  ESP8266_Init();                                    //��ʼ��esp8266
	  soft_reset(1);                                     //�����λ�ɹ���־
	  MOTOR_GPIO_Init();  				                 //���GPIO��ʼ��              $$PB12,13,14,15
  	  Motor_PWM_Init(7200,0, 7200, 0);	                 //PWMƵ��10khz	TM4 ch3, ch4  $$PB8,PB9
	  TIM3_Init(8000, 719);                             //720��Ƶ1Mhz 0.1msΪ������λ              �ж����ȼ� 1 - 3
	  PID_Init(0);										 //PID��ֵ��ʼ��������������
	
	   while(mpu_dmp_init()){        //�ж�mpu��ʼ���Ƿ�ɹ�  $$PC10, PC11
			delay_ms(10);
			if(times++ > 100){      //����10sδ��Ӧ
				times = 0;
				UART_send_string("mpu6050_Init failed \n", USART2);
			}					
		} 
	    while(mpuget_yaw(&set_yaw)!=0);
	   
	   IWDG_Init(5,1250);                                   //���Ź����� ���Ƶ��Ϊ128,����ֵΪ625,���ʱ��Ϊ2s	
	   Car_Stop();
	   
}






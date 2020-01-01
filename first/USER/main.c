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
#include "ultrasonic.h"

extern u8 mputime_nvic;                                     //定时器中断标志位
extern u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 				//wifi接收缓冲,最大USART2_MAX_RECV_LEN个字节.
extern u8 USART3_RX_STA;   	                                //rfid标志位，判断是否都到卡
extern u8 Rx3Flag;
extern u16 USART2_RX_STA;   	                            //标志位，判断是否收到消息
extern u16 Ti;

u8 Mode = 0;                                     //Mode 模式标志 0--待机，  1--开始路径检测， 2--找到信息标志
u8 Iscar_run = 0;								 //判断小车是否在运动中
float set_yaw;

void Carsysterm_init(void);

int main(void)
 {
	 char p[20],j;
	 u8 count=0;
	 float ave_yaw=0;
	 Carsysterm_init();
     while(1){
		 switch(Mode){
			 case 0:{
				 IWDG_Feed();
				 if(USART2_RX_STA&0x8000){		           //wifi是否接收数据判断
					select_mode();                
					USART2_RX_STA=0;   
				 }
//				 delay_ms(200);
//				 for(i=0; i<3; i++){
//					while(mpuget_yaw(&yaw)!=0) ;
//					ave_yaw += yaw;
//				 }
//				 ave_yaw = ave_yaw/3.0; 
//				 sprintf(p, "yaw: %.3f \n", ave_yaw);
//				 UART_send_string(p, USART2);
				 if(Rx3Flag==1)                 //rfid检测中断
					Read_mode(0);
				 break;
				 
			 }
			 case 1:{			
				 IWDG_Feed();				 
				 if(USART2_RX_STA&0x8000){		           //wifi是否接收数据判断
					select_mode();                
					USART2_RX_STA=0;   
				 }
				 while(mpuget_yaw(&ave_yaw)!=0);
//				 if(j++ >20){
//				 sprintf(p, "yaw: %.3f \n", ave_yaw);
//				 UART_send_string(p, USART2);
//					 j=0;
//				 }
				 if(mputime_nvic!=0){                      //判断是否产生定时器中断
//					if(Iscar_run){
//						Car_Run(Current_speed);
//						delay_ms(100);                    //保持前进速度，消除pid影响
//					}
					if(Iscar_run==1 && (count++)%4==0){
						check_yaw(ave_yaw);
						if(count == 100)
							count = 0;
					}else if(Iscar_run==0) check_yaw(ave_yaw);
					TIM_Cmd(TIM3, ENABLE);                //运算周期100MS
					mputime_nvic=0;
				 }
				 
				 if(Rx3Flag==1 && Iscar_run==1){           //rfid检测中断 附加条件：运动中		
		            Car_Stop();
					delay_ms(1000);
					Read_mode(1);
					Iscar_run=0;
					USART_Cmd(USART3, DISABLE);                    //失能串口3
				 }			 
				 break;
			 }
			 case 2:{
				 IWDG_Feed();
				 Car_SpinLeft(Current_speed, Current_speed);
				 delay_ms(2000);
				 Car_Run(Current_speed);
				 delay_ms(3000);
				 Car_Stop();
				 Mode = 0;
				 break;
			 }
		 }
	}
 }

 
void Carsysterm_init(void)
{
      u8 times=0;
	  NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);    //失能中断优先级2
	  delay_init();	                                     //延时函数初始化	  
	  LED_Init();		  	                             //初始化与LED连接的硬件接口    $$PA8, PD2
	  uart_init(115200);	                             //串口初始化为115200          $$PA9, PA10  中断优先级 3 - 3
	  USART2_Init(115200);                               //bound 115200               $$PA2, PA3   中断优先级 0 - 3
	  USART3_Init(9600);								 //Rfid信息接收口                                     0 - 2
	  ESP8266_Init();                                    //初始化esp8266
	  soft_reset(1);                                     //软件复位成功标志
	  MOTOR_GPIO_Init();  				                 //电机GPIO初始化              $$PB12,13,14,15
  	  Motor_PWM_Init(7200,0, 7200, 0);	                 //PWM频率10khz	TM4 ch3, ch4  $$PB8,PB9
	  TIM3_Init(10000, 719);                             //720分频1Mhz 0.01ms为基本单位       200ms     中断优先级 1 - 3
	  PID_Init(0);										 //PID数值初始化
	  Ultrasonic_GPIO_Init();							 //超声波gpio初始化
	  Timer2_Init();								     //超声波定时器初始化
	  	
	   while(mpu_dmp_init()){        //mpu初始化， 判断是否成功
			delay_ms(10);
			if(times++ > 100){      //超过10s未反应
				times = 0;
				UART_send_string("mpu6050_Init failed \n", USART2);
			}					
		} 
	    while(mpuget_yaw(&set_yaw)!=0);
		
	    IWDG_Init(5,1250);                                   //看门狗程序  与分频数为128,重载值为625,溢出时间为4s
	    Car_Stop();
	   
}




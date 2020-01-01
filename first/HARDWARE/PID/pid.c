#include "pid.h"
#include "sys.h"
#include "stdio.h"
#include "string.h"
#include "delay.h"
#include "app_motor.h"
#include "my_uart.h"
#include "esp8266.h"
#include "inv_mpu.h"

#define allow_error 5      //死区，允许误差5度


PID_value  pid_yaw;

extern u8 Iscar_run;
extern u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 
extern float set_yaw;
u16 exter_speed=3700;

void PID_Init(float yaw)
{
	memset(&pid_yaw, 0, sizeof(PID_value));
	pid_yaw.KP = 3;
	pid_yaw.KD = 0.8;
	pid_yaw.KI = 1.2;
	pid_yaw.T = 0.1;            //单位s
}
//*A@3@0.8@1.2@#
 void check_yaw(float yaw)      //判断是否需要Pid调节
 {
		float dis_yaw=0;
		dis_yaw = yaw - set_yaw;
		if(Iscar_run==1){
			if((dis_yaw <= 0 && dis_yaw > -5) || (dis_yaw >= 0 && dis_yaw < 5)){  //判断角度偏差是否小于规定值
				Car_Run(Current_speed);
				delay_ms(100);
		    }else PID_out(set_yaw, yaw);
		}
		else{
			if((dis_yaw <= 0 && dis_yaw > -5) || (dis_yaw >= 0 && dis_yaw < 5)){  //判断角度偏差是否小于规定值
				Car_Stop();
		   }else PID_out(set_yaw, yaw);
		}
 }
 

void PID_operation(PID_value *p)
{   
    float a0,a1,a2;
    
    //计算中间变量a0,a1,a2;
    a0 = p->KP + p->KI*p->T + p->KD/p->T ;
    a1 = p->KP + 2*p->KD/p->T ;
    a2 = p->KD/p->T ;
    //计算输出
    p->output = p->output_1 + a0*p->error - a1*p->error_1 + a2*p->error_2 ;
    
    //为下次计算迭代
    //这里顺序千万不要搞错，不然输出占空比是错误的。
    p->output_1 = p->output;
    p->error_2 = p->error_1;
    p->error_1 = p->error;
    
}

void PID_out(float given_yaw, float this_yaw)
{
	static u8 j=0;
    u16  pid_speed;
	char send_buf[30];
	
	if(given_yaw<-175){
		if(this_yaw > 120)
			this_yaw = -180 - (180-this_yaw);
	}
	
	pid_yaw.error = this_yaw - given_yaw ;    //偏差   
	
	PID_operation(&pid_yaw);
		
//	if(j++>50){
//		sprintf(send_buf, "\n speed: %.2f \n", pid_yaw.output);
//		UART_send_string(send_buf, USART2);
//		j = 0;
//		}
	if(pid_yaw.output < 0)
		pid_speed = constrain_int(-pid_yaw.output, 0, 1000);       //线性范围适应4000--6000
	else pid_speed = constrain_int(pid_yaw.output, 0, 1000);
		
    if(Iscar_run==0){                           //判断是否控制端改变角度
		pid_speed += 3900;
		if(pid_yaw.error>allow_error){      //如果大于allow_error
			 Car_SpinRight(pid_speed, pid_speed);
		}
		else{				//误差小于-allow_error
			if(pid_yaw.error<-allow_error)
			 Car_SpinLeft(pid_speed, pid_speed);
		}
	}	
	else{
		pid_speed += exter_speed;
		if(pid_yaw.error>allow_error){      //如果大于allow_error
				 Car_Right(pid_speed);
			}
			else{				//误差小于-allow_error
				if(pid_yaw.error<-allow_error)
				 Car_Left(pid_speed);
			}
		}  
}

//		if(given_yaw<2 || given_yaw>358){         //判断是否在0附近
//			if(pid_yaw.error<180 && pid_yaw.error>allow_error){    //如果误差稍微大于0
//				 Car_SpinRight(pid_speed, pid_speed);
//			}
//			else{              //误差稍微小于360
//				 Car_SpinLeft(pid_speed, pid_speed);
//			}			
//		}else{
//			pid_yaw.error = this_yaw - given_yaw ;    //偏差
//			if(pid_yaw.error > 180){
//				pid_yaw.error = pid_yaw.error - 360;
//				flag = 1;
//			}
//			if(j++>5){
//					sprintf(send_buf, "\n ave_yaw: %.2f \n pid_yaw.error: %.2f \n", this_yaw, pid_yaw.error);
//					UART_send_string(send_buf, USART2);
//					j=0;
//				}
//			PID_operation(&pid_yaw);
//			
//			pid_speed = constrain_int(pid_yaw.output*100-4000, 0, 700);       
//			
//			pid_speed += (Current_speed-650);                             //线性范围适应3600--4400
//			
//			if(flag==1){
//				pid_yaw.error = pid_yaw.error + 360;
//				flag = 0;
//			}
//			if(pid_yaw.error > allow_error){        //如果误差角度大于5度，减小角度
//				 Car_SpinRight(pid_speed, pid_speed);
//			}
//			else{    //如果误差角度小于于-5度，增大角度
//				Car_SpinLeft(pid_speed, pid_speed);
//			}
//					
//		}

u16 constrain_int(u16 amt, u16 low, u16 high)
{
    return ((amt)<(low)?(low):((amt)>(high)?(high):(amt)));
}

//void str_float(void)                                   //字符串切割转浮点函数（串口调参） 字符串格式：" *A@kp@ki@kd@#" 前后@要不要都行
//{
//    u8 i=1, j=0;
//	char *temp, strp[30];
//    float num[5];                                      //最大切割数
//	
//	while(USART2_RX_BUF[++i] != 0x23)                  //没到字符串末尾
//		sprintf(strp+(i-2), "%c", USART2_RX_BUF[i]);   //将USART2_RX_BUF转换成字符串	 注：所有字符指针用字符数组代替！防止莫名bug
//	
//    for(i=1; i<strlen(strp)-1; i++){
//        if( strp[i] == '@' )
//        j++;                                          //判断有几个数待切割
//    }
//	
//    temp = strtok(strp, "@");
//    for(i=0; i<j+2; i++){                             //字符串转float函数
//        if(temp!=NULL){
//           sscanf(temp, "%f", &num[i]);
//           temp = strtok(NULL,"@");
//        }
//    }
//	
//	memset(strp, '\0', sizeof(strp));              
//	pid_yaw.KP = num[0];
//	pid_yaw.KI = num[1];
//	pid_yaw.KD = num[2];
//	sprintf(strp, "kp:%.4f ki:%.4f kd:%.4f \n", num[0], num[1], num[2]);
//	UART_send_string(strp, USART2);
// 	
//}

#include "pid.h"
#include "sys.h"
#include "stdio.h"
#include "string.h"
#include "app_motor.h"
#include "my_uart.h"
#include "esp8266.h"
#include "inv_mpu.h"

#define allow_error 2      //�������������2��


PID_value  pid_yaw;

extern u8 Iscar_run;
extern u8 USART2_RX_BUF[USART2_MAX_RECV_LEN]; 
extern u16 exter_speed;
extern float set_yaw;
void PID_Init(float yaw)
{
	memset(&pid_yaw, 0, sizeof(PID_value));
	pid_yaw.KP = 1;
	pid_yaw.KD = 0;
	pid_yaw.KI = 0;
	pid_yaw.T = 0.1;            //��λs
}
//*A@10@0@1.5@#  
 void check_yaw(float yaw)      //�ж��Ƿ���ҪPid����
 {
		 u8 i;
		 char send_buf[30];
		 float dis_yaw=0;
		dis_yaw = yaw - set_yaw;
		if((dis_yaw < 0 && dis_yaw > -2) || (dis_yaw > 0 && dis_yaw < 2)){  //�жϽǶ�ƫ���Ƿ�С�ڹ涨ֵ
			if(Iscar_run){	
			  Car_Run(Current_speed);
			}
			else Car_Stop();
		}else PID_out(set_yaw, yaw);
 }
 

void PID_operation(PID_value *p)
{   
    float a0,a1,a2;
    
    //�����м����a0,a1,a2;
    a0 = p->KP + p->KI*p->T + p->KD/p->T ;
    a1 = p->KP + 2*p->KD/p->T ;
    a2 = p->KD/p->T ;
    //�������
    p->output = p->output_1 + a0*p->error - a1*p->error_1 + a2*p->error_2 ;
    
    //Ϊ�´μ������
    //����˳��ǧ��Ҫ�����Ȼ���ռ�ձ��Ǵ���ġ�
    p->output_1 = p->output;
    p->error_2 = p->error_1;
    p->error_1 = p->error;
    
}

void PID_out(float given_yaw, float this_yaw)
{
    u16  pid_speed;
	
	if(given_yaw+162<5){
		if(this_yaw > 100)
			this_yaw = -180 - (180-this_yaw);
	}
	
	pid_yaw.error = this_yaw - given_yaw ;    //ƫ��   
	
	PID_operation(&pid_yaw);
		
//	if(j++>5){
//		sprintf(send_buf, "\n a_yaw: %.2f \n pid_err: %.2f \n", this_yaw, pid_yaw.error);
//		UART_send_string(send_buf, USART2);
//		memset(send_buf, '\0', sizeof(send_buf));
//		sprintf(send_buf, "\n speed: %.2f \n", pid_yaw.output);
//		UART_send_string(send_buf, USART2);
//		j = 0;
//		}
	if(pid_yaw.output < 0)
		pid_speed = constrain_int(-pid_yaw.output, 0, 800);       //���Է�Χ��Ӧ4000--6000
	else pid_speed = constrain_int(pid_yaw.output, 0, 800);
		
    if(Iscar_run==0){                           //�ж��Ƿ���ƶ˸ı�Ƕ�
		pid_speed += 1400;
		if(pid_yaw.error>allow_error){      //�������allow_error
			 Car_SpinRight(pid_speed, pid_speed);
		}
		else{				//���С��-allow_error
			if(pid_yaw.error<-allow_error)
			 Car_SpinLeft(pid_speed, pid_speed);
		}
	}	
	else{
		pid_speed += exter_speed;
		if(pid_yaw.error>allow_error-3){      //�������allow_error
				 Car_SpinRight(pid_speed, pid_speed);
			}
			else{				//���С��-allow_error
				if(pid_yaw.error<-allow_error+3)
				 Car_SpinLeft(pid_speed, pid_speed);
			}
		}  
}

u16 constrain_int(u16 amt, u16 low, u16 high)
{
    return ((amt)<(low)?(low):((amt)>(high)?(high):(amt)));
}

void clear_out(void)
{
	pid_yaw.output_1 = 0;
}


//void str_float(void)                                   //�ַ����и�ת���㺯�������ڵ��Σ� �ַ�����ʽ��" *A@kp@ki@kd@#" ǰ��@Ҫ��Ҫ����
//{
//    u8 i=1, j=0;
//	char *temp, strp[30];
//    float num[5];                                      //����и���
//	
//	while(USART2_RX_BUF[++i] != 0x23)                  //û���ַ���ĩβ
//		sprintf(strp+(i-2), "%c", USART2_RX_BUF[i]);   //��USART2_RX_BUFת�����ַ���	 ע�������ַ�ָ�����ַ�������棡��ֹĪ��bug
//	
//    for(i=1; i<strlen(strp)-1; i++){
//        if( strp[i] == '@' )
//        j++;                                          //�ж��м��������и�
//    }
//	
//    temp = strtok(strp, "@");
//    for(i=0; i<j+2; i++){                             //�ַ���תfloat����
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

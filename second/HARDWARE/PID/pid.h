#ifndef __PID_H
#define __PID_H	 
#include "sys.h"

typedef struct pid
{
 //  float ref;     //�趨ֵ
 //  float fdb;     //����ֵ
     
	 float T;          //��λms
     float KP;         //a0 = Kp+Ki*T+Kd/T
     float KD;         //a1 = Kp+2Kd/T
     float KI;         //a2 = Kd/T
     
     float error;      //�������
     float error_1;    //�ϴ����
     float error_2;    //���ϴ����
     
     float output;     //pid�������  
     float output_1;   //pid�ϴ����

}PID_value;                    

void clear_out(void);
void PID_Init(float yaw);
void check_yaw(float yaw);
void PID_operation(PID_value *p);
void PID_out(float given_yaw, float this_yaw);

u16 constrain_int(u16 amt, u16 low, u16 high);      //
//void str_float(void);


		 				    
#endif

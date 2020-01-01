#ifndef __PID_H
#define __PID_H	 
#include "sys.h"

typedef struct pid
{
 //  float ref;     //设定值
 //  float fdb;     //测量值
     
	 float T;          //单位ms
     float KP;         //a0 = Kp+Ki*T+Kd/T
     float KD;         //a1 = Kp+2Kd/T
     float KI;         //a2 = Kd/T
     
     float error;      //本次误差
     float error_1;    //上次误差
     float error_2;    //上上次误差
     
     float output;     //pid控制输出  
     float output_1;   //pid上次输出

}PID_value;                    

void clear_out(void);
void PID_Init(float yaw);
void check_yaw(float yaw);
void PID_operation(PID_value *p);
void PID_out(float given_yaw, float this_yaw);

u16 constrain_int(u16 amt, u16 low, u16 high);      //
//void str_float(void);


		 				    
#endif

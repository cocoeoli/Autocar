/**
* @par Copyright (C): 2010-2019, Shenzhen Yahboom Tech
* @file         app_motor.h
* @author       liusen
* @version      V1.0
* @date         2015.01.03
* @brief        С���˶����ƺ���
* @details      
* @par History  ������˵��
*                 
* version:	liusen_20170717
*/

#ifndef __APP_MOTOR_H__
#define __APP_MOTOR_H__


#include "bsp_motor.h"

#define Current_speed 4200            //С���ٶ�4200


void Car_Run(int Speed);
void Car_Back(int Speed);
void Car_Left(int Speed);
void Car_Right(int Speed);
void Car_Stop(void);
void Car_SpinStop(void);
void Car_SpinLeft(int LeftSpeed, int RightSpeed);
void Car_SpinRight(int LeftSpeed, int RightSpeed);
void Car_diva(int LeftSpeed, int RightSpeed);



#endif



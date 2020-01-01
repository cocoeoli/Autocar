
#ifndef __ULTRASONIC_H__
#define __ULTRASONIC_H__	


#define TRIG_RCC		RCC_APB2Periph_GPIOC
#define ECHO_RCC		RCC_APB2Periph_GPIOC

#define TRIG_PIN		GPIO_Pin_7
#define ECHO_PIN		GPIO_Pin_6

#define TRIG_PORT		GPIOC
#define ECHO_PORT		GPIOC


double bsp_getUltrasonicDistance(void);
void Timer2_Init(void);
void Ultrasonic_GPIO_Init(void);



#endif




#ifndef __ESP8266_H
#define __ESP8266_H

#include "sys.h"


void ESP8266_Init(void);

void select_mode(void);
//void esp_mesjug(void);
void esp_config(u16 len);
void esp_controlcar(u16 len);
void esp_routeplan(u16 len);

void soft_reset(u8 rflag);
void error_mes(void);
//void Turn_angle(float angle, u8 dir);

////////////////********ip, wifiµÿ÷∑ ≈‰÷√÷∏¡Ó******
//void esp_ipconfig(void);
//void esp_wificonfig(void);

#endif

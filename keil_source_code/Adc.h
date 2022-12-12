#include "MKL25Z4.h"

void ADC0_Init(void);
int ADC0_Calibrate(void);
void show_voltage(float);
void show_light_value(void);
void show_threshold_value(void);
void switch_channel(void);
void channel_handler(float);
uint16_t ADC0_Read(void);
void ADC0_IRQHandler(void);

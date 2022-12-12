#include "Adc.h"
#include "Uart.h"
#include "gpio.h"

int main() {
	UART0_Init(115200);
	RGBLed_Init(); // configurare RGBLED
	ADC0_Init();
	
	for(;;) {

	}
	
}

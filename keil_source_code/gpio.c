#include "gpio.h"

#define RED_LED_PIN (18) // PORT B
#define GREEN_LED_PIN (19) // PORT B
#define BLUE_LED_PIN (1) // PORT D

void RGBLed_Init(void){
	
	// Activarea semnalului de ceas pentru pinii folositi în cadrul led-ului RGB
	// R,G 	- PORT B
	// B 		- PORT D
	SIM_SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK; // activez semnalul de ceas pentru PORT urile B si D
	
	
	
	// --- RED LED ---
	// Utilizare GPIO ca varianta de multiplexare
	PORTB->PCR[RED_LED_PIN] &= ~PORT_PCR_MUX_MASK; 	// disable MUX
	PORTB->PCR[RED_LED_PIN] |= PORT_PCR_MUX(1); 		// enable MUX pe 1 (GPIO)
	// Configurare pin pe post de output
	GPIOB_PDDR |= (1<<RED_LED_PIN); // punem 1 pe pozitia x (x va reprezenta numarul PIN-ului folosit)
	// Stingerea LED-ului (punerea pe 0 logic)
	GPIOB_PSOR |= (1<<RED_LED_PIN); // vreau sa dau CLEAR la PIN ul RED 
	// => 0 va face 0 logic = 0V => dif de potential 3.3V = 1 logic => OFF
	
	
	
	// --- GREEN LED ---
	// Utilizare GPIO ca varianta de multiplexare
	PORTB->PCR[GREEN_LED_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTB->PCR[GREEN_LED_PIN] |= PORT_PCR_MUX(1);
	// Configurare pin pe post de output
	GPIOB_PDDR |= (1<<GREEN_LED_PIN);
	// Stingerea LED-ului (punerea pe 0 logic)
	GPIOB_PSOR |= (1<<GREEN_LED_PIN); // ii fac SET la PIN ul GREEN => 
	// 1 va face 1 logic = 3.3V => diferenta de potential va fi 0 => va fi OFF
	
	
	
	// --- BLUE LED ---
	// Utilizare GPIO ca varianta de multiplexare
	PORTD->PCR[BLUE_LED_PIN] &= ~PORT_PCR_MUX_MASK;
	PORTD->PCR[BLUE_LED_PIN] |= PORT_PCR_MUX(1);
	// Configurare pin pe post de output
	GPIOD_PDDR |= (1<<BLUE_LED_PIN);
	// Stingerea LED-ului (punerea pe 0 logic)
	GPIOD_PSOR |= (1<<BLUE_LED_PIN);
	// 1 va face 1 logic = 3.3V => diferenta de potential va fi 0 => va fi OFF
	
}
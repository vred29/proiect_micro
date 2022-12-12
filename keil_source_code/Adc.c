#include "Adc.h"
#include "Uart.h"

#define ADC_CHANNEL_LIGHT (11) // PORT C PIN 2 -> fiecare pin corespunde pe 1 canal => setam corespondenta PIN - CANAL
#define ADC_CHANNEL_ROTATION (8) // PORT B PIN 0 -> fiecare pin corespunde pe 1 canal => setam corespondenta PIN - CANAL


#define RED_LED_PIN (18) // PORT B
#define GREEN_LED_PIN (19) // PORT B
#define BLUE_LED_PIN (1) // PORT D

static uint8_t switch_channel_flag = 0;
static uint16_t valoare_prag; 
static uint16_t valoare_lumina_lux; 

static uint16_t threshold_1 = 2000; 
static uint16_t threshold_2 = 4000; 


void ADC0_Init() {
	
	// Activarea semnalului de ceas pentru modulul periferic ADC
	SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;
	
	// Functia de calibrare
	// ar trebui facut in while calibrarea pana intoarce 0 (s-a efectuat cu succes)
	ADC0_Calibrate();
	
	ADC0->CFG1 = 0x00;

	// Selectarea modului de conversie pe 16 biti single-ended --> MODE
	// Selectarea sursei de ceas pentru generarea ceasului intern --> ADICLK
	// Selectarea ratei de divizare folosit de periferic pentru generarea ceasului intern --> ADIV
	// Set ADC clock frequency fADCK less than or equal to 4 MHz (PG. 494)
	ADC0->CFG1 |= ADC_CFG1_MODE(3) |
							 ADC_CFG1_ADICLK(0) |
							 ADC_CFG1_ADIV(2);
	
	// DIFF = 0 --> Conversii single-ended (PG. 464)
	ADC0->SC1[0] = 0x00;
	ADC0->SC3 = 0x00;
	
	// Selectarea modului de conversii continue, 
	// pentru a-l putea folosi in tandem cu mecanismul de intreruperi
	ADC0->SC3 |= ADC_SC3_ADCO_MASK;
	
	// Activarea subsistemului de conversie prin aproximari succesive pe un anumit canal (PG.464)
	ADC0->SC1[0] |= ADC_SC1_ADCH(ADC_CHANNEL_ROTATION);
	
	// Enables conversion complete interrupts
	ADC0->SC1[0] |= ADC_SC1_AIEN_MASK;
	
	NVIC_ClearPendingIRQ(ADC0_IRQn);
	NVIC_EnableIRQ(ADC0_IRQn);	
}

int ADC0_Calibrate() {
	
	// ===== For best calibration results =====
	
	ADC0_CFG1 |= ADC_CFG1_MODE(3)  |  				 // 16 bits mode
                ADC_CFG1_ADICLK(1)|  // Input Bus Clock divided by 2
                ADC_CFG1_ADIV(3);   // Clock divide by 8
	
	// The calibration will automatically begin if the SC2[ADTRG] is 0. (PG. 495)
	ADC0->SC2 &= ~ADC_SC2_ADTRG_MASK;
	
	// Set hardware averaging to maximum, that is, SC3[AVGE]=1 and SC3[AVGS]=0x11 for an average of 32 (PG. 494)
	ADC0->SC3 |= (ADC_SC3_AVGE_MASK | ADC_SC3_AVGS(3));
	
	// To initiate calibration, the user sets SC3[CAL] (PG. 495)
	ADC0->SC3 |= ADC_SC3_CAL_MASK;
	
	// At the end of a calibration sequence, SC1n[COCO] will be set (PG. 495)
	while(!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));
	
	// At the end of the calibration routine, if SC3[CALF] is not
	// set, the automatic calibration routine is completed successfully. (PG. 495)
	if(ADC0->SC3 & ADC_SC3_CALF_MASK){
		return (1);
	}
	
	// ====== CALIBRATION FUNCTION (PG.495) =====
	
	// 1. Initialize or clear a 16-bit variable in RAM.
	uint16_t calibration_var = 0x0000;
	
	// 2. Add the plus-side calibration results CLP0, CLP1, CLP2, CLP3, CLP4, and CLPS to the variable.
	calibration_var += ADC0->CLP0;
	calibration_var += ADC0->CLP1;
	calibration_var += ADC0->CLP2;
	calibration_var += ADC0->CLP3;
	calibration_var += ADC0->CLP4;
	calibration_var += ADC0->CLPS;
	
	// 3. Divide the variable by two.
	calibration_var /= 2;
	
	// 4. Set the MSB of the variable. 
	calibration_var |= 0x8000;
	
	// 5. Store the value in the plus-side gain calibration register PG.
	ADC0->PG = ADC_PG_PG(calibration_var);
	
	// 6. Repeat the procedure for the minus-side gain calibration value.
	calibration_var = 0x0000;
	
	calibration_var += ADC0->CLM0;
	calibration_var += ADC0->CLM1;
	calibration_var += ADC0->CLM2;
	calibration_var += ADC0->CLM3;
	calibration_var += ADC0->CLM4;
	calibration_var += ADC0->CLMS;
	
	calibration_var /= 2;
	
	calibration_var |= 0x8000;
	
	ADC0->MG = ADC_MG_MG(calibration_var);
	
	// Incheierea calibrarii
	ADC0->SC3 &= ~ADC_SC3_CAL_MASK;
	
	return (0);
}

void switch_channel(){
	if(switch_channel_flag == 0){
		// s-a citit de pe ROTATION, NEXT -> LIGHT
		// DIFF = 0 --> Conversii single-ended (PG. 464)
		ADC0->SC1[0] |= ADC_SC1_ADCH_MASK;
		ADC0->SC1[0] = 0x00;
		ADC0->SC3 = 0x00;
		
		// Selectarea modului de conversii continue, 
		// pentru a-l putea folosi in tandem cu mecanismul de intreruperi
		ADC0->SC3 |= ADC_SC3_ADCO_MASK;
		
		// Activarea subsistemului de conversie prin aproximari succesive pe un anumit canal (PG.464)
		ADC0->SC1[0] |= ADC_SC1_ADCH(ADC_CHANNEL_LIGHT);
		
		// Enables conversion complete interrupts
		ADC0->SC1[0] |= ADC_SC1_AIEN_MASK;

		switch_channel_flag=1;
	}
	else{
		// s-a citit de pe LIGHT, NEXT -> ROTATION
		// DIFF = 0 --> Conversii single-ended (PG. 464)
		ADC0->SC1[0] = 0x00;
		ADC0->SC3 = 0x00;
		
		// Selectarea modului de conversii continue, 
		// pentru a-l putea folosi in tandem cu mecanismul de intreruperi
		ADC0->SC3 |= ADC_SC3_ADCO_MASK;
		
		// Activarea subsistemului de conversie prin aproximari succesive pe un anumit canal (PG.464)
		ADC0->SC1[0] |= ADC_SC1_ADCH(ADC_CHANNEL_ROTATION);
		
		
		// Enables conversion complete interrupts
		ADC0->SC1[0] |= ADC_SC1_AIEN_MASK;

		switch_channel_flag=0;
	}
}


void channel_handler(float measured_voltage){
	if(switch_channel_flag == 0){
		// am citit de la ROTATION => salvez PRAGUL setat
		uint16_t cat_imaprtire = measured_voltage/0.28;
		valoare_prag = cat_imaprtire * 500;
		show_threshold_value();
		
	} else{
		// am citit de la LIGHT => compar valoarea cu pragul si apind/modific LED-ul
		float valoare_lumina = ((float)6000.0/(float)3.29) * measured_voltage;
		valoare_lumina_lux = (uint16_t) valoare_lumina;
		if(valoare_lumina_lux > valoare_prag){
			// lumina inregistrata depaseste pragul => LED ON
			
			if(valoare_lumina_lux>0 && valoare_lumina_lux<threshold_1){
				// valori MICI -> GREEN (RED OFF, GREEN ON, BLUE OFF)
				GPIOB_PSOR |= (1<<RED_LED_PIN);
				GPIOB_PCOR |= (1<<GREEN_LED_PIN);
				GPIOD_PSOR |= (1<<BLUE_LED_PIN);
			} else if (valoare_lumina_lux>threshold_1 && valoare_lumina_lux<threshold_2) {
				// valori MEDII -> YELLOW (RED ON, GREEN ON, BLUE OFF)
				GPIOB_PCOR |= (1<<RED_LED_PIN);
				GPIOB_PCOR |= (1<<GREEN_LED_PIN);
				GPIOD_PSOR |= (1<<BLUE_LED_PIN);
			} else if (valoare_lumina_lux> threshold_2 && valoare_lumina_lux<6000){
				// valori MEDII -> RED (RED ON, GREEN OFF, BLUE OFF)
				GPIOB_PCOR |= (1<<RED_LED_PIN);
				GPIOB_PSOR |= (1<<GREEN_LED_PIN);
				GPIOD_PSOR |= (1<<BLUE_LED_PIN);
			}
		}else{
			// lumina este UNDER prag -> BLUE (RED OFF, BLUE ON, GREEN OFF)
			GPIOB_PSOR |= (1<<RED_LED_PIN);
			GPIOB_PSOR |= (1<<GREEN_LED_PIN);
			GPIOD_PCOR |= (1<<BLUE_LED_PIN);
		}
		show_light_value();
	}
}

void show_voltage(float measured_voltage){
	uint8_t parte_zecimala = (uint8_t) measured_voltage;
	uint8_t parte_fractionara1 = ((uint8_t)(measured_voltage * 10)) % 10;
	uint8_t parte_fractionara2 = ((uint8_t)(measured_voltage * 100)) % 10;
	UART0_Transmit('V');
	UART0_Transmit('o');
	UART0_Transmit('l');
	UART0_Transmit('t');
	UART0_Transmit('a');
	UART0_Transmit('g');
	UART0_Transmit('e');
	UART0_Transmit(' ');
	UART0_Transmit('=');
	UART0_Transmit(' ');
	UART0_Transmit(parte_zecimala + 0x30);
	UART0_Transmit('.');
	UART0_Transmit(parte_fractionara1 + 0x30);
	UART0_Transmit(parte_fractionara2 + 0x30);
	UART0_Transmit('V');
	UART0_Transmit(0x0A);
	UART0_Transmit(0x0D);
}



void show_threshold_value(){
	uint16_t aux = valoare_prag;
	
	
	UART0_Transmit('P');
	UART0_Transmit('r');
	UART0_Transmit('a');
	UART0_Transmit('g');
	UART0_Transmit('=');
	
	if(aux == 0){
		UART0_Transmit('0');
	}else{
		uint8_t letters[5];
		int index=0;

		while(aux)
		{
			letters[index] = aux%10+0x30;
			aux/=10;
			index++;
		}
		index--;
		while(index>=0)
		{
			UART0_Transmit(letters[index--]);	
		}

	}
	UART0_Transmit(' ');
	UART0_Transmit('L');
	UART0_Transmit('u');
	UART0_Transmit('x');
	UART0_Transmit(0x0A);
	UART0_Transmit(0x0D);

}

void show_light_value(){
	uint16_t aux = valoare_lumina_lux;
	
	UART0_Transmit('L');
	UART0_Transmit('i');
	UART0_Transmit('g');
	UART0_Transmit('h');
	UART0_Transmit('t');
	UART0_Transmit('=');
	
	if(aux == 0){
		UART0_Transmit('0');
	}else{
		uint8_t letters[5];
		int index=0;

		while(aux)
		{
			letters[index] = aux%10+0x30;
			aux/=10;
			index++;
		}
		index--;
		while(index>=0)
		{
			UART0_Transmit(letters[index--]);	
		}

	}
	UART0_Transmit(' ');
	UART0_Transmit('L');
	UART0_Transmit('u');
	UART0_Transmit('x');
	UART0_Transmit(0x0A);
	UART0_Transmit(0x0D);

}

void ADC0_IRQHandler(){
	
	uint16_t analog_input = (uint16_t) ADC0->R[0];
	float measured_voltage = (analog_input * 3.3f) / 65535;
	
	channel_handler(measured_voltage);
	//show_voltage(measured_voltage);
	switch_channel();

}


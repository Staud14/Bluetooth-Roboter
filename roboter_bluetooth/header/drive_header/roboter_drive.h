/*
 * roboter_drive.h
 *
 * Created: 21.10.2017 
 *  Author: Patrik Staudenmayer
 */ 


#ifndef ROBOTER_DRIVE_H_
#define ROBOTER_DRIVE_H_

//*************************************************************************************************
//*** MACROS ***
//*************************************************************************************************

#define JTAG_DISABLE()	MCUCR = MCUCR|(1<<JTD); MCUCR = MCUCR|(1<<JTD);




//*************************************************************************************************
//*** MOTOR ***
//*************************************************************************************************

#define MOTOR_R			6			// PB6  OC4B PWM
#define MOTOR_L			7			// PD7	OC4D PWM

// Motor Funktion
#define MOT_FAST_STOPP	2
#define MOT_FORWARD		1
#define MOT_BACKWARD	0

#define PWM_R_VOR_MAX	125			//2,02 ms (WALC: 2 ms)
#define PWM_R_RET_MAX	62			//1,01 ms (WALC: 0,992 ms)
#define PWM_R_STOPP		94			//1,52 ms (WALC: 1,504 ms)

#define PWM_L_VOR_MAX	62			//1,01 ms (WALC: 0,992 ms)
#define PWM_L_RET_MAX	125			//2,02 ms (WALC: 2 ms)
#define PWM_L_STOPP		94			//1,52 ms (WALC: 1,504 ms)

//*************************************************************************************************
//*** LED *****
//*************************************************************************************************

// LED Pins
#define LED_RV			0			// PB0
#define LED_RH			1			// PB1
#define LED_LV			2			// PB2
#define LED_LH			3			// PB3
// Duo LED
#define LED_GRUEN		4			// PB4
#define LED_ROT			5			// PB5


//*************************************************************************************************
//*** Akkuueberwachung *****
//*************************************************************************************************

#define MEASURE_UB		0			//ADC0	PF0


//************************************************************************************************
//***Prototypes*****
//************************************************************************************************

void drive(unsigned char dir_motr, unsigned char pwm_rechts, unsigned char dir_motl, unsigned char pwm_links);
unsigned int adc_measure(unsigned char channel);
void akkuzustand (void);
void roboter_init(void);

/*************************************************************************************************
***Function description*****
**************************************************************************************************
*
*drive:			dir_motr & dir_motl ....	1 -> forward
*											2 -> stop
*											0 -> backward
*
*				pwm_rechts ...	94 < pwm_rechts < 125 -> forward
*								62 < pwm_rechts < 94 -> backward
*
*				pwm_links ...	62 < pwm_links < 94 -> forward
*								94 < pwm_links < 125 -> backward
*
*adc_measure:	channel ...		0 -> Measure battery
*								1 -> Measure LF left (IR Receiver left)
*								4 -> Measure LF right (IR Receiver right)
*								
*akkuzustand:	7,2V >= Vcc > 6,8V	-> Battery ok & Led green					The values are only correct for one specific Roboter and should be meausred for every roboter individually!!!!!
				6,8V >= Vcc > 6,4V -> Battery half full & Led yellow
				6,4V >= Vcc -> battery empty & led red
*************************************************************************************************/


#endif /* ROBOTER_DRIVE_H_ */
/*
 * DSP_KB.h
 *
 *  Created on: Oct 26, 2015
 *      Author: Kyle
 */

#ifndef DSP_KB_H_
#define DSP_KB_H_

static float samplingRate = 44000;
#define SetCmdAddr() address = (unsigned int *)0x100000;
#define SetDataAddr() address = (unsigned int*)0x100001;
static unsigned int * address = (unsigned int *)0x100000;

interrupt void ADC_isr(void);
interrupt void DAC_isr(void);
interrupt void interp_isr(void);
interrupt void decimate_isr(void);
interrupt void monstermash_isr(void);
interrupt void audioEcho_isr(void);
interrupt void reverb_isr(void);
static unsigned int * outputPORT = (unsigned int *)0x4000;
static unsigned int * SRAMaddress = 0x260000;
 //pointer for SRAM address
static int a = 0; //variable for the timer interrupt vector change
static int decimate = 0;
static int b = 0;
static unsigned int delaytime = 0;
static long bufferLength = 44000;
static unsigned long reverbBuffer = 44000*0.3;
static int interpHold = 1;
//const float samplingRate = 44000;


void timerINIT(unsigned long ISRlocation, float samplingRate);
void CPUinit();
void outputEnable();
void kp_enable();
int keypadScan();

void interpolateAlgorithm();

void ADC_set(int a, int b);
void initADC();
unsigned int ADC_get();
void DAC_init();
void DAC_set(unsigned int data);
void delayLONG();
void setDelay(unsigned int value);


void changeFunctions(float samplingRate);

void LCDinit();
void LCDclear();
void string_push(char * sendString);
void char_push(char input);





#endif /* DSP_KB_H_ */

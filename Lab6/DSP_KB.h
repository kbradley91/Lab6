/*
 * DSP_KB.h
 *
 *  Created on: Oct 26, 2015
 *      Author: Kyle
 */

#ifndef DSP_KB_H_
#define DSP_KB_H_

#define SetCmdAddr() address = (unsigned int *)0x100000;
#define SetDataAddr() address = (unsigned int*)0x100001;
static unsigned int * address = (unsigned int *)0x100000;

void CPUinit();
void outputEnable();
void kp_enable();
int keypadScan();


void ADC_set(int a, int b);
void initADC();
unsigned int ADC_get();
void DAC_init();
void DAC_set(unsigned int data);
void delayLONG();


void changeFunctions();

void LCDinit();
void LCDclear();
void string_push(char * sendString);
void char_push(char input);





#endif /* DSP_KB_H_ */

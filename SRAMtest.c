/*
 * SRAMtest.c
 *
 *
 */

#include <DSP28x_Project.h>
#include "string.h"



#define SetCmdAddr() address = (unsigned int *)0x100000;
#define SetDataAddr() address = (unsigned int*)0x100001;
static unsigned int * SRAMaddress = (unsigned int *)0x200000; //pointer for SRAM address

static unsigned int * firstAddress = (unsigned int *)0;
static unsigned int * secondAddress = (unsigned int *)0;
static unsigned int * thirdAddress = (unsigned int *)0;
static unsigned int * address = (unsigned int *)0x100000;


extern unsigned int a2d();
unsigned int adc_val =0;


int main(void) {
	DisableDog();
	InitSysCtrl();
	EALLOW;
	EALLOW;

	outputEnable();
	LCDinit();

	SRAMwrite(0x00AA);

	SRAMread(firstAddress);

	SRAMwrite(0x55);

	SRAMread2(secondAddress);

	SRAMwrite2();

	SRAMread3(thirdAddress);



	
	return 0;
}



void outputEnable(){
//also enables the LCD
	SysCtrlRegs.PCLKCR3.bit.XINTFENCLK = 1;
	GpioCtrlRegs.GPAMUX2.all = 0x33000000;
	GpioCtrlRegs.GPBMUX1.all = 0xFFFFFFFF; //

	GpioCtrlRegs.GPCPUD.all = 0xFFFFFFFF;

	GpioCtrlRegs.GPCMUX1.all = 0xFFFFFFFF; //contains xd[15:0]
	GpioCtrlRegs.GPCMUX2.all = 0xFFFFFFFF; //enables A15:8
}


void SRAMwrite(unsigned int value){
	SRAMaddress = 0x200000; //reintializing value just in case

	while(SRAMaddress != 0x2FFFFF){

		*SRAMaddress = value;
		SRAMaddress++;

	}

	SRAMaddress = 0x200000; //reintializing value just in case

}

void SRAMwrite2(){
	SRAMaddress = 0x200000; //reintializing value just in case
	//unsigned int test = 0xFFFF;
	unsigned int  value =0;
	while(SRAMaddress != 0x2FFFFF){

		value = SRAMaddress;


		value &= 0x00FFFF;

		delayLONG();

		*SRAMaddress = value;

		SRAMaddress++;




	}

		SRAMaddress = 0x200000; //reintializing value just in case



}

void SRAMread(unsigned int * badAddress){
	SRAMaddress = 0x200000; //reintializing value just in case
	int a = 0;
	while(SRAMaddress != 0x2FFFFF){
		if(*SRAMaddress != 0x00AA && a == 0){
				badAddress = SRAMaddress;
				a = 1;
		}
		SRAMaddress++;


	}
	if(a==0){
		string_push("$AA");
	}
	else{
		string_push("$EE");
	}
	SRAMaddress = 0x200000; //reintializing value just in case




}

void SRAMread2(unsigned int * badAddress){
	SRAMaddress = 0x200000; //reintializing value just in case
	int a = 0;
	while(SRAMaddress != 0x2FFFFF){
		if(*SRAMaddress != 0x0055 && a == 0){
				badAddress = SRAMaddress;
				a = 1;
		}
		SRAMaddress++;


	}
	if(a==0){
		string_push("$55");
	}
	else{
		string_push("$EE");
	}
	SRAMaddress = 0x200000; //reintializing value just in case




}

void SRAMread3(unsigned int * badAddress){
	SRAMaddress = 0x200000; //reintializing value just in case
	int a = 0;

	while(SRAMaddress != 0x2FFFFF){
		if(*SRAMaddress != (SRAMaddress&&0xFFFF) && a == 0){
				badAddress = SRAMaddress;
				a = 1;
		}
		SRAMaddress++;



	}
	if(a==0){
		string_push("$AA");
	}
	else{
		string_push("$EE");
	}
	SRAMaddress = 0x200000; //reintializing value just in case




}

void delayLONG(){
	int i;
	for(i = 0x8FFF;i<0;i--){ //8FFF is the magic number for it not shitting the bed.
		asm volatile("nop");
	}
}



void LCDinit(){
	SetCmdAddr();
	int temp = 0x3f;
	*address = temp;
	delayLONG();
	temp = 0x0f;
	*address = temp;
	delayLONG();
	temp = 0x01;
	*address = temp;

	delayLONG();
	//delayREALLYLONG();

}
void LCDclear(){
	int temp;
	SetCmdAddr();
	delayLONG();
	temp = 0x01;
	*address = temp;

	delayLONG();
}

void string_push(char *sendString){
	int i;
	for(i=0;i<strlen(sendString);i++){
		delayLONG();


	//	delayREALLYLONG();
		SetDataAddr();
		*address = sendString[i];
	}
}
void char_push(char input){
	delayLONG();
	SetDataAddr();
	*address = input;
	delayLONG();
}

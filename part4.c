
#include <DSP28x_Project.h>
#include "DSP_KB.h"
#include "string.h"
#include "math.h"



extern float samplingRate;
extern long bufferLength; //I dont think this needs to be declared external but yolo

//const float samplingRate = 44000; //define the sampling rate here
unsigned long ISRvalue = &audioEcho_isr; //define the ISR to be used here

int main(void){

	enablethings(ISRvalue, samplingRate); //passes the ISR to the timer intialze function




	unsigned int value = 0;

	int oldvalue = 0;
	while(1){
		//constantly scanning the keypad to set the echo value
		//TODO: could also use this same method to set the reverbBuffer

		value = keypadScan();
		*outputPORT = value;

		if(value != oldvalue && value != 1 && value != 0xFF){

			oldvalue = value;
			setDelay(value);

		}


	}


	EALLOW;






	return 0;
}


void SRAMwrite(unsigned int value){
	//basically just using this function in order to intialze memory to 0 so it workslol

	SRAMaddress = 0x200000; //reintializing value just in case

	while(SRAMaddress != 0x2FFFFF){

		*SRAMaddress = value;
		SRAMaddress++;

	}

	SRAMaddress = 0x260000; //reintializing value just in case

}

//
//void changeFunctions(float samplingRate){
//	EALLOW;
//	ConfigCpuTimer(&CpuTimer1, 150,  1000000*1/samplingRate*.5);
//	CpuTimer1Regs.TCR.bit.TSS = 0;
//
//	EALLOW;
//	PieVectTable.XINT13 = &DAC_isr;
//}

void changeFrequency(long freq){
	//changing the frequency of the interrupt
	//I think this is only used when the DAC is set
	//TODO: nope not even used?
	EALLOW;
	ConfigCpuTimer(&CpuTimer1, 150,  1000000*1/freq*.5);
	CpuTimer1Regs.TCR.bit.TSS = 0;

}

void FreqSet(unsigned int value){
	//TODO: old code?

	if(value == 7){
		changeFrequency(50000);
	}
	else if(value == 2){
		 changeFrequency(20000);
	}
	else if(value ==3){
		changeFrequency(25000);
	}
	else if(value == 4){
		changeFrequency(30000);
	}
	else if(value == 5){
		changeFrequency(35000);
	}
	else if(value == 6){
		changeFrequency(40000);
	}


}


void enablethings(unsigned long ISRlocation, float samplingRate){
	//tiding things up

	DisableDog();
	EALLOW;
	CPUinit();
	EALLOW;
	DINT;
	outputEnable();
	LCDinit();
	LCDclear();
	initADC();
	EALLOW;
	outputEnable(); //having issues when not enabling twice
	SRAMwrite(0);
	SRAMaddress = 0x260000;
	DAC_init();
	timerINIT(ISRlocation, samplingRate);
	EALLOW;
}






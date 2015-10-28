
#include <DSP28x_Project.h>
#include "DSP_KB.h"
#include "string.h"
#include "math.h"



extern float samplingRate;
//const float samplingRate = 44000; //define the sampling rate here
unsigned long ISRvalue = &reverb_isr; //define the ISR to be used here

int main(void){

	enablethings(ISRvalue, samplingRate); //passes the ISR to the timer intialze function

	//TODO: can use timerINIT() in order to change ISRs if need be


	unsigned int value = 0;

	int oldvalue = 0;
	while(1){

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


void setDelay(unsigned int value){
	// This function assumes that the frequency of the interrupt is 44kHz
	if(value == 2){
		delaytime = 44000*0.25;
	}
	else if(value == 3){
		delaytime = 44000*0.5;
	}
	else if(value == 4){
		delaytime = 44000*0.75;
	}
	else if(value == 5){
		delaytime = 44000*1;
	}
	else{
		delaytime = 0;
	}
	bufferLength = delaytime;
}
void SRAMwrite(unsigned int value){
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
	EALLOW;
	ConfigCpuTimer(&CpuTimer1, 150,  1000000*1/freq*.5);
	CpuTimer1Regs.TCR.bit.TSS = 0;

}

void FreqSet(unsigned int value){

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






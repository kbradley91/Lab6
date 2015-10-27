#include <DSP28x_Project.h>
#include "DSP_KB.h"
#include "string.h"




void DAC_set(unsigned int data){
	while(SpiaRegs.SPISTS.bit.BUFFULL_FLAG == 1){}

	SpiaRegs.SPITXBUF = data;

}

void delayLONG(){
	int i;
	for(i = 0xFFFF;i<0;i--){ //8FFF is the magic number for it not shitting the bed.
		asm volatile("nop");
	}
}


void outputEnable(){
//also enables the LCD
	SysCtrlRegs.PCLKCR3.bit.XINTFENCLK = 1;
	GpioCtrlRegs.GPAMUX2.all = 0x33000000;
	GpioCtrlRegs.GPBMUX1.all = 0xFFFFFFFF; //

	GpioCtrlRegs.GPCPUD.all = 0xFFFFFFFF;

	GpioCtrlRegs.GPCMUX1.all = 0xFFFFFFFF; //contains xd[15:0]
	GpioCtrlRegs.GPCMUX2.all = 0xFFFFFFFF; //enables A15:8
	GpioCtrlRegs.GPAMUX2.all &= 0xFF00FFFF;
	GpioCtrlRegs.GPAMUX2.all |= 0x00FC0000;
	GpioCtrlRegs.GPAMUX2.all &= 0xFFFFFF00;
	GpioCtrlRegs.GPAMUX2.all |= 0x00000051;

}

void CPUinit(){
	InitSysCtrl();
	EALLOW;
	SysCtrlRegs.PLLSTS.bit.DIVSEL = 0;
	SysCtrlRegs.PLLCR.bit.DIV = 0xA;
	SysCtrlRegs.PLLSTS.bit.DIVSEL = 2;

}
void ADC_set(int a, int b){
	McbspbRegs.DXR2.all = b;
	McbspbRegs.DXR1.all = a;
}
void initADC(){
	GpioCtrlRegs.GPAMUX2.all &= 0xFF00FFFF;
	GpioCtrlRegs.GPAMUX2.all |= 0x00FC0000;

	McbspbRegs.SPCR2.all= 0x0000;
	McbspbRegs.SPCR1.all= 0x0000;
	McbspbRegs.SPCR1.bit.CLKSTP = 2;

	McbspbRegs.PCR.bit.CLKXP = 0;
	McbspbRegs.PCR.bit.CLKRP = 0;
	McbspbRegs.PCR.bit.CLKXM = 1;
	McbspbRegs.PCR.bit.SCLKME = 0;

	McbspbRegs.SRGR2.bit.CLKSM = 1;
	McbspbRegs.SRGR1.bit.CLKGDV = 2;

	McbspbRegs.PCR.bit.FSXM = 1;

	McbspbRegs.SRGR2.bit.FSGM = 0;
	McbspbRegs.PCR.bit.FSXP = 1;

	McbspbRegs.XCR2.bit.XDATDLY = 1;
	McbspbRegs.RCR2.bit.RDATDLY = 1;

	McbspbRegs.XCR1.bit.XFRLEN1 = 0;
	McbspbRegs.XCR1.bit.XWDLEN1 = 4;

	McbspbRegs.RCR1.bit.RFRLEN1 = 0;
	McbspbRegs.RCR1.bit.RWDLEN1 = 4;

	McbspbRegs.SPCR2.bit.GRST=1;
	//delay_loop();
	McbspbRegs.SPCR2.bit.XRST=1;
	McbspbRegs.SPCR1.bit.RRST=1;
	//delay_loop();
	McbspbRegs.SPCR2.bit.FRST=1;

}
void kp_enable(){
	GpioCtrlRegs.GPADIR.all  &= 0xFFFF00FF;
	GpioCtrlRegs.GPADIR.all  |= 0x000000FF;

}


int keypadScan(){
	kp_enable();
	int directionValue = 0x100;
	GpioCtrlRegs.GPADIR.all &= 0xFFFFF0FF;
	GpioCtrlRegs.GPADIR.all |= 0x0100; //setting pin 8 as output

	int i;

	for(i=0;i<4;i++){
		if(i == 4){
					break;
		}
		int data = GpioDataRegs.GPADAT.all;

		data = data>>12;
		data &= 0x0000000F;

		directionValue = directionValue<<1;
		GpioCtrlRegs.GPADIR.all &= 0xFFFFF0FF;
		GpioCtrlRegs.GPADIR.all |= directionValue;

		if(data == 0xF){
			//do nothing		//case 1
		}

		if(i == 0){

			if(data == 0xE){
				return 1;

			}
			else if(data == 0xD){
				return 4;
			}
			else if(data == 0xB){
				return 7;
			}
			else if(data == 0x7){
				return 0xE;
			}
		}
		else if(i == 1){

			if(data == 0xE){
				return 2;
			}
			else if(data == 0xD){
				return 5;
			}
			else if(data == 0xB){
				return 8;
			}
			else if(data == 0x7){
				return 0x0;
			}
		}
		else if(i == 2){

			if(data == 0xE){
				return 0x3;
			}
			else if(data == 0xD){
				return 0x6;
			}
			else if(data == 0xB){
				return 0x9;
			}
			else if(data == 0x7){
				return 0xF;
			}
		}
		else if(i == 3){
			if(data == 0xE){
				return 0xA;
			}
			else if(data == 0xD){
				return 0xb;
			}
			else if(data == 0xB){
				return 0xc;
			}
			else if(data == 0x7){
				return 0xD;
			}
		}

	}
	return 0xFF;


}
unsigned int ADC_get(){
	long fullIn;
	ADC_set(0,0);
	while(McbspbRegs.SPCR1.bit.RRDY == 0){
	}
	fullIn = McbspbRegs.DRR2.all;
	fullIn = fullIn<<16;
	fullIn |= McbspbRegs.DRR1.all;
	fullIn &= 0x0003FFFC;
	fullIn = fullIn>>2;
	return((unsigned int)fullIn);

}

void DAC_init(){
	GpioCtrlRegs.GPAMUX2.all &= 0xFFFFFF00;
	GpioCtrlRegs.GPAMUX2.all |= 0x00000051;

	SpiaRegs.SPICCR.bit.SPISWRESET= 0;
	SpiaRegs.SPICCR.bit.CLKPOLARITY = 1;
	SpiaRegs.SPICCR.bit.SPICHAR = 15;

	SpiaRegs.SPICTL.bit.CLK_PHASE = 0;
	SpiaRegs.SPICTL.bit.MASTER_SLAVE = 1;
	SpiaRegs.SPICTL.bit.TALK = 1;

	SpiaRegs.SPIBRR = 4;

	SpiaRegs.SPICCR.bit.SPISWRESET = 1;
	SpiaRegs.SPIPRI.bit.FREE = 1;

	SpiaRegs.SPIFFTX.all = 0xE040;
	SpiaRegs.SPIFFCT.all = 0x0;


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



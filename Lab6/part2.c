
#include <DSP28x_Project.h>
#include "string.h"

interrupt void test_isr(void);
interrupt void ADC_isr(void);
static unsigned int * outputPORT = (unsigned int *)0x4000;
static unsigned int * SRAMaddress = (unsigned int *)0x200000; //pointer for SRAM address
static int a = 0; //test variable for the timer flag


int main(void){
	DisableDog();
	EALLOW;
	int test = 0;
	int laggingValue = 0xFF;

	unsigned int value = 0;
	EALLOW;
	CPUinit();
	EALLOW;
	DINT;

	outputEnable();

	initADC();
	EALLOW;
	outputEnable(); //having issues when not enabling twice


	//DAC_init();
	DAC_init();
	timerINIT();
	EALLOW;



	while(1){

	}


	EALLOW;






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
				return 2;
			}
			else if(data == 0xB){
				return 3;
			}
			else if(data == 0x7){
				return 0xA;
			}
		}
		else if(i == 1){

			if(data == 0xE){
				return 4;
			}
			else if(data == 0xD){
				return 5;
			}
			else if(data == 0xB){
				return 6;
			}
			else if(data == 0x7){
				return 0xB;
			}
		}
		else if(i == 2){

			if(data == 0xE){
				return 0x7;
			}
			else if(data == 0xD){
				return 0x8;
			}
			else if(data == 0xB){
				return 0x9;
			}
			else if(data == 0x7){
				return 0xC;
			}
		}
		else if(i == 3){
			if(data == 0xE){
				return 0xE;
			}
			else if(data == 0xD){
				return 0;
			}
			else if(data == 0xB){
				return 0xF;
			}
			else if(data == 0x7){
				return 0xD;
			}
		}

	}
	return 0xFF;


}
int ADC_get(){
	long fullIn;
	ADC_set(0,0);
	while(McbspbRegs.SPCR1.bit.RRDY == 0){
	}
	fullIn = McbspbRegs.DRR2.all;
	fullIn = fullIn<<16;
	fullIn |= McbspbRegs.DRR1.all;
	fullIn &= 0x0003FFFC;
	fullIn = fullIn>>2;
	return((int)fullIn);

}

void DAC_init(){
	EALLOW;
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

void DAC_set(int data){
	while(SpiaRegs.SPISTS.bit.BUFFULL_FLAG == 1){}

	SpiaRegs.SPITXBUF = data;

}


void timerINIT(){


	// Step 1. Initialize System Control:
	// PLL, WatchDog, enable Peripheral Clocks
	// This example function is found in the DSP2833x_SysCtrl.c file.
	 //InitPeripheralClocks();

	// Step 2. Initalize GPIO:
	// This example function is found in the DSP2833x_Gpio.c file and
	// illustrates how to set the GPIO to it's default state.
	// InitGpio();  // Skipped for this example


	// Step 3. Clear all interrupts and initialize PIE vector table:
	// Disable CPU interrupts
	   DINT;

	// Initialize the PIE control registers to their default state.
	// The default state is all PIE interrupts disabled and flags
	// are cleared.
	// This function is found in the DSP2833x_PieCtrl.c file.
	   InitPieCtrl();

	// Disable CPU interrupts and clear all CPU interrupt flags:
	   IER = 0x0000;
	   IFR = 0x0000;

	// Initialize the PIE vector table with pointers to the shell Interrupt
	// Service Routines (ISR).
	// This will populate the entire table, even if the interrupt
	// is not used in this example.  This is useful for debug purposes.
	// The shell ISR routines are found in DSP2833x_DefaultIsr.c.
	// This function is found in DSP2833x_PieVect.c.
	   InitPieVectTable();

	// Interrupts that are used in this example are re-mapped to
	// ISR functions found within this file.
	   EALLOW;  // This is needed to write to EALLOW protected registers
	 // PieVectTable.TINT0 = &cpu_timer0_isr;
	   PieVectTable.XINT13 = &ADC_isr;
	  //PieVectTable.TINT2 = &cpu_timer2_isr;
	   EDIS;    // This is needed to disable write to EALLOW protected registers

	// Step 4. Initialize the Device Peripheral. This function can be
	//         found in DSP2833x_CpuTimers.c
	   InitCpuTimers();   // For this example, only initialize the Cpu Timers
	   EALLOW;


// Configure CPU-Timer 0, 1, and 2 to interrupt every second:
// 150MHz CPU Freq, 1 second Period (in uSeconds)

  // ConfigCpuTimer(&CpuTimer0, 150, 1000000);
   ConfigCpuTimer(&CpuTimer1, 150, 1000000*1/44000*.5); //1/10000 is the frequency we want to toggle timer1 to interrupt
   //ConfigCpuTimer(&CpuTimer2, 150, 1000000);




	#if (CPU_FRQ_100MHZ)
	// Configure CPU-Timer 0, 1, and 2 to interrupt every second:
	// 100MHz CPU Freq, 1 second Period (in uSeconds)

	  // ConfigCpuTimer(&CpuTimer0, 100, 1000000);
	   ConfigCpuTimer(&CpuTimer1, 150, 2000);
	   //ConfigCpuTimer(&CpuTimer2, 100, 1000000);
	#endif
	// To ensure precise timing, use write-only instructions to write to the entire register. Therefore, if any
	// of the configuration bits are changed in ConfigCpuTimer and InitCpuTimers (in DSP2833x_CpuTimers.h), the
	// below settings must also be updated.

	  // CpuTimer0Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
	   CpuTimer1Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0
	   //CpuTimer2Regs.TCR.all = 0x4001; // Use write-only instruction to set TSS bit = 0

	// Step 5. User specific code, enable interrupts:


	// Enable CPU int1 which is connected to CPU-Timer 0, CPU int13
	// which is connected to CPU-Timer 1, and CPU int 14, which is connected
	// to CPU-Timer 2:
	   //IER |= M_INT1;
	   IER |= M_INT13;
	   //IER |= M_INT14;

	// Enable TINT0 in the PIE: Group 1 interrupt 7
	   PieCtrlRegs.PIEIER1.bit.INTx7 = 1;

	// Enable global Interrupts and higher priority real-time debug events:
	   EINT;   // Enable Global interrupt INTM
	   ERTM;   // Enable Global realtime interrupt DBGM
	   EALLOW;

	// Step 6. IDLE loop. Just sit and loop forever (optional):


}
void SRAMwrite(unsigned int value){
	//this assumes that SRAM has been intialized to 0x200000

	*SRAMaddress = value;
	SRAMaddress++;
}

void changeFunctions(){
	EALLOW;
	ConfigCpuTimer(&CpuTimer1, 150, 1000000*1/20000*.5);
	CpuTimer1Regs.TCR.bit.TSS = 0;
	EALLOW;
	PieVectTable.XINT13 = &test_isr;
}

interrupt void ADC_isr(void){
	EALLOW;
	unsigned int value;
	initADC();
	value = ADC_get();
	DAC_set(value);


	CpuTimer1.InterruptCount++;
	EALLOW;
}

interrupt void test_isr(void){
	if (a == 1){
		EALLOW;
		DAC_init();
		a = 0;
	}

	if(SRAMaddress <= 0x2FFFFF){
		DAC_set(*SRAMaddress);
		SRAMaddress++;
	}

	CpuTimer1.InterruptCount++;
	EALLOW;

}








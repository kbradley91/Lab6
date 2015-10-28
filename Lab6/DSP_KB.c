#include <DSP28x_Project.h>
#include "DSP_KB.h"
#include "string.h"



/* Digital Mixing:
 *
 * This code takes a value into the ADC, and write the value to memory, when the first sample
 * is complete it then promts the user via the LCD for a new sample to be mixed with the first
 * This is accomplished by adding the 2 samples together and normalizing them
 *
 * TODO: this code is a little messy and could probably be cleaned up.
 */
interrupt void monstermash_isr(void){
	EALLOW;
	unsigned int value;
	unsigned long temp;
	value = ADC_get();

	if(SRAMaddress <= 0x29FFFF & a == 0){
		*SRAMaddress = value;
		SRAMaddress++;

	}
	else if(SRAMaddress <= 0x29FFFF && a ==1){
		temp = *SRAMaddress;
		temp = temp+value;
		temp = temp/2;
		value = temp;
		//TODO:this is where attenuation would go
		*SRAMaddress = value;
		SRAMaddress++;
		if(SRAMaddress > 0x29FFFF){

			b = 1;

		}
	}
	else if(b == 1){
		DINT;
		SRAMaddress = 0x260000;
		changeFunctions(samplingRate);
		EINT;
		ERTM;
	}
	else{


		SRAMaddress = 0x260000;
		a = 1;

		string_push("press 4 when second sample is ready");
		int test = keypadScan();
		while(test != 4){
			test = keypadScan();
		}
		LCDclear();
		//changeFunctions();
		//EINT;   // Enable Global interrupt INTM
		//ERTM;   // Enable Global realtime interrupt DBGM

	}
}


/*
 * This code samples the input and then saves it to the SRAM then
 * changes the function to output the values written to SRAM on the DAC
 */

interrupt void ADC_isr(void){
	EALLOW;
	unsigned int value;

	//initADC();
	value = ADC_get();
	//DAC_set(value);
	long justincase = SRAMaddress;

	if(justincase <= 0x29FFFF){
		*SRAMaddress = value;
		SRAMaddress++;
	}
	else{
		DINT;
		SRAMaddress = 0x260000;
		a = 1;
		changeFunctions(samplingRate);
		EINT;   // Enable Global interrupt INTM
		ERTM;   // Enable Global realtime interrupt DBGM
	}


	CpuTimer1.InterruptCount++;
	EALLOW;
}

/*
 * Outputs the value in memory out on the DAC FOREVER
 */

interrupt void DAC_isr(void){

	if(SRAMaddress <= 0x29FFFF){
		DAC_set(*SRAMaddress);
		SRAMaddress++;
	}
	else{ //resetting the SRAM address so it can play the waveform over again.
		SRAMaddress = 0x260000;

	}

	CpuTimer1.InterruptCount++;
	EALLOW;

}

/*
 * interp_isr interpolates the values in memory by writing twice for a single value, this allows us to use memory in order
 * to save processor space by needing only half the interrupt triggering time to hear the same output if we didn't
 * interpolate
 */



interrupt void interp_isr(void){

	EALLOW;
	unsigned int value;
	unsigned long test;
	//initADC();
	value = ADC_get();
	//DAC_set(value);

	if(SRAMaddress <= 0x29FFFF){
		*SRAMaddress = value;


		if(interpHold == 2){

			interpHold = 0;
			SRAMaddress--;
			unsigned int value2 = *SRAMaddress;
			SRAMaddress++;
			test = value2 + value;
			test = test/2;
			value2 = (unsigned int)test;

			*SRAMaddress = value2;
			SRAMaddress++;
			*SRAMaddress = value;


		}
		interpHold++;
		SRAMaddress++;

	}
	else{
		DINT;
		SRAMaddress = 0x260000;
		a = 1;
		changeFunctions(samplingRate*1.5);

		EINT;   // Enable Global interrupt INTM
		ERTM;   // Enable Global realtime interrupt DBGM
	}


	CpuTimer1.InterruptCount++;
	EALLOW;



}

/*
 * audioEcho emulates a real world echo by taking in a sample, then reading the previous sample
 * that occured in the same spot, N samples ago, where N is the length of the buffer. Once the sample is read
 * the value that was read in stores the input from the ADC in the current memory location
 *
 * this follows a DTFT formula as below: (I believe)
 * y[n] = x[n] + a*x[n-N] {where n is always the current sample, and N is the length of the buffer}
 *
 * If I understand signals correctly, which I don't this is a FIR filter
 */

interrupt void audioEcho_isr(void){
	unsigned long output;
	unsigned int value;

	value = ADC_get();


	unsigned int test2 = *SRAMaddress;
	*SRAMaddress = value;
	SRAMaddress++;

	output = (value+0.5*test2)/2;

	DAC_set((unsigned int)output);


	if(SRAMaddress > 0x260000+bufferLength){
		SRAMaddress = 0x260000;
	}



//
//	if(SRAMaddress < 0x260000+bufferLength){
//
//		SRAMaddress++;
//
//	}
//	else{
//
//		SRAMaddress = 0x260000;
//
//	}
//
//	*SRAMaddress = value;

}

/*
 * This is where things get real funky. Here we read in the ADC value and store the variable,
 * we then read in the value stored at the SRAM location. This value is the value that occured buffer length samples ago
 *
 * Then we calculate the output by making the output equal to the mix of the two sounds however, its extremely important
 * to note that the coefficent of the second value is < 1. If its not, the signal eventually reaches the max value and begins to clip
 *
 * I believe that the reverb is what is referred to as a IIR filter, as this is different from the echo in principal. In
 * the echo, we stored the input value, here we are putting the output value back in memory after they have been convovled??
 * (this im not sure of) either way the output value goes into memory instead of the input.
 *
 */

interrupt void reverb_isr(void){
	unsigned long output;
	unsigned int value;
	unsigned int value2;

	value = ADC_get();

	//TODO: The value that SRAMaddress should be equal to is the address space + buffersize
	value2 = *SRAMaddress;
	output = (value + 0.5*value2)/2;
	*SRAMaddress = output;

	SRAMaddress++;

	//TODO: The value that SRAMaddress should be equal to is the address space + buffersize

	DAC_set((unsigned int)output);


	if(SRAMaddress > 0x260000+reverbBuffer){
		SRAMaddress = 0x260000;
	}









}

interrupt void decimate_isr(void){
	EALLOW;
	unsigned int value;
	//initADC();
	value = ADC_get();
	//DAC_set(value);

	if(SRAMaddress <= 0x29FFFF & decimate == 4){
		*SRAMaddress = value;
		SRAMaddress++;
		decimate = 0;
	}
	else if(SRAMaddress > 0x29FFFF){

		DINT;
		SRAMaddress = 0x260000;
		a = 1;
		changeFunctions(samplingRate*2);
		EINT;   // Enable Global interrupt INTM
		ERTM;   // Enable Global realtime interrupt DBGM
	}
	else{

		decimate++;
	}


		CpuTimer1.InterruptCount++;
		EALLOW;





}

void timerINIT(unsigned long ISRlocation, float samplingRate){


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
	   PieVectTable.XINT13 =ISRlocation;
	  //PieVectTable.TINT2 = &cpu_timer2_isr;
	   EDIS;    // This is needed to disable write to EALLOW protected registers

	// Step 4. Initialize the Device Peripheral. This function can be
	//         found in DSP2833x_CpuTimers.c
	   InitCpuTimers();   // For this example, only initialize the Cpu Timers
	   EALLOW;


// Configure CPU-Timer 0, 1, and 2 to interrupt every second:
// 150MHz CPU Freq, 1 second Period (in uSeconds)

  // ConfigCpuTimer(&CpuTimer0, 150, 1000000);
   ConfigCpuTimer(&CpuTimer1, 150, 1000000*1/samplingRate*.5); //1/10000 is the frequency we want to toggle timer1 to interrupt
   //ConfigCpuTimer(&CpuTimer2, 150, 1000000);




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

void changeFunctions(float samplingRate){
	EALLOW;
	ConfigCpuTimer(&CpuTimer1, 150,  1000000*1/samplingRate*.5);
	CpuTimer1Regs.TCR.bit.TSS = 0;

	EALLOW;
	PieVectTable.XINT13 = &DAC_isr;
}


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



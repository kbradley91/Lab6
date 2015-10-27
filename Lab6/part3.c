
#include <DSP28x_Project.h>
#include "DSP_KB.h"
#include "string.h"


interrupt void ADC_isr(void);
interrupt void DAC_isr(void);
interrupt void interp_isr(void);
interrupt void decimate_isr(void);
static unsigned int * outputPORT = (unsigned int *)0x4000;
static unsigned int * SRAMaddress = (unsigned int *)0x200000; //pointer for SRAM address
static int a = 0; //variable for the timer interrupt vector change
static int decimate = 0;


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
	SRAMwrite(0);
	SRAMaddress = 0x260000;


	//DAC_init();
	DAC_init();
	timerINIT();

	EALLOW;


	int oldvalue = 0;
	while(1){
//		SRAMaddress = 0x2FFFFF;
//		*SRAMaddress = 0x77;
		if(a == 1){ //cross your fingers folks
			value = keypadScan();
			*outputPORT = value;
			if(value < 0xF && value != 1 && oldvalue != value){
				FreqSet(value); //setting the frequency based upon keypad input
				oldvalue = value;
			}
		}
	}


	EALLOW;






	return 0;
}


void SRAMwrite(unsigned int value){
	SRAMaddress = 0x200000; //reintializing value just in case

	while(SRAMaddress != 0x2FFFFF){

		*SRAMaddress = value;
		SRAMaddress++;

	}

	SRAMaddress = 0x200000; //reintializing value just in case

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
   ConfigCpuTimer(&CpuTimer1, 150, 1000000*1/20000*.5); //1/10000 is the frequency we want to toggle timer1 to interrupt
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

void changeFunctions(){
	EALLOW;
	ConfigCpuTimer(&CpuTimer1, 150,  1000000*1/20000*.5);
	CpuTimer1Regs.TCR.bit.TSS = 0;
	SRAMaddress = 0x260000;
	EALLOW;
	PieVectTable.XINT13 = &DAC_isr;
}

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

interrupt void ADC_isr(void){
	EALLOW;
	unsigned int value;
	//initADC();
	value = ADC_get();
	//DAC_set(value);
	long justincase = SRAMaddress;

	if(SRAMaddress <= 0x29FFFF){
		*SRAMaddress = value;
		SRAMaddress++;
	}
	else{
		DINT;
		SRAMaddress = 0x260000;
		a = 1;
		changeFunctions();
		EINT;   // Enable Global interrupt INTM
		ERTM;   // Enable Global realtime interrupt DBGM
	}


	CpuTimer1.InterruptCount++;
	EALLOW;
}

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

interrupt void interp_isr(void){

	EALLOW;
	unsigned int value;
	//initADC();
	value = ADC_get();
	//DAC_set(value);

	if(SRAMaddress <= 0x2FFFFF){
		*SRAMaddress = value;
		SRAMaddress++;
		*SRAMaddress = value;
		SRAMaddress++;
	}
	else{
		DINT;
		SRAMaddress = 0x280000;
		a = 1;
		changeFunctions();
		EINT;   // Enable Global interrupt INTM
		ERTM;   // Enable Global realtime interrupt DBGM
	}


	CpuTimer1.InterruptCount++;
	EALLOW;



}

interrupt void decimate_isr(void){
	EALLOW;
	unsigned int value;
	//initADC();
	value = ADC_get();
	//DAC_set(value);

	if(SRAMaddress <= 0x2FFFFF & decimate == 4){
		*SRAMaddress = value;
		SRAMaddress++;
		decimate = 0;
	}
	else if(SRAMaddress > 0x2FFFFF){

		DINT;
		SRAMaddress = 0x280000;
		a = 1;
		changeFunctions();
		EINT;   // Enable Global interrupt INTM
		ERTM;   // Enable Global realtime interrupt DBGM
	}
	else{
		SRAMaddress++;
		decimate++;
	}


		CpuTimer1.InterruptCount++;
		EALLOW;





}







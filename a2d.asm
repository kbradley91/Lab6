



GPAMUX1		 .set 0x6F68
GPAMUX2	     .set 0x6F88
GPBMUX1	     .set 0x6F96
GPBMUX2		 .set 0x6F98
GPCMUX1		 .set 0x6FA6
GPCMUX2		 .set 0x6FA8
PCLKCR3		 .set 0x7020
GPATOGGLE	 .set 0x6FC6
GPADIR		 .set 0x6F8A
GPADAT		 .set 0x6FC0
WDCR		 .set 0x7029
data_sect	 .set 0xa000
bss_sect	 .set 0xC000
output_PORT  .set 0x4000
LCD_command	 .set 0x100000
LCD_data	 .set 0x100001





	.global _a2d
	.global _adc_val


_a2d:



 			MOV ar0, #0x68 ; this should disable the watchdog timer
 			mov ar1, #0x7029
 			mov *ar1, ar0
 			mov AL, #0
 			mov ar4, #0


 			;LC outEN
 			LC ADC_EN
 			LC ADC_read

 			MOVZ DP, #_adc_val
			MOV @_adc_val, AL
			LRETR






end:
			LC ADC_EN
 			LC ADC_read
			B end, UNC




outEN:
			mov ar0, #PCLKCR3
			TSET *ar0, #12
			mov ar0, #GPAMUX2+1 ;enabling cs6
			mov *ar0, #0x3300
			mov ar0, #GPBMUX1 ;enabling alot of things
			mov *ar0, #0xFFFF
			inc ar0
			mov *ar0, #0xFFFF ;moving into GPBMUX+1
			mov ar0, #PCLKCR3
			TSET *ar0, #12
			mov ar0, #GPCMUX1
			mov *ar0, #0xFFFF
			mov ar0, #GPCMUX1+1
			mov *ar0, #0xFFFF
			LRET


ADC_EN:

			mov ar0, #GPADIR
			mov AH, *ar0
			OR AH, #0x00A0
			mov *ar0, AH
			inc ar0
			mov AH, *ar0
			AND AH, #0xFFEF
			mov *ar0, AH
			MOV ar0, #GPADAT
			mov AH, *ar0
			OR AH, #0xA0
			mov *ar0, AH
;// 	The above instructions enable GPIO7 and GPIO5 as outputs
;// 	and enable GPIO 21 as input

			LRET


ADC_read:
			mov ar0, #GPADAT
			mov AH, *ar0
			AND AH, #0xFF5F
			mov *ar0, AH  ;//setting CS and DCLOCK low

			LC toggle1 ;//toggles pin7 10 times, making 5 low-high clocks occur
			LC readADC
			LC toggle1

			mov ar0, #GPADAT
			mov AH, *ar0
			AND AH, #0xFFFF
			mov *ar0, AH  ;//setting CS and DCLOCK low




			LRET




readADC:
			mov ar1, #15
			LC toggleONCE
			LC toggleONCE
readLoop:

			LC toggleONCE
			mov ar0, #GPADAT+1
			mov AH, *ar0
			AND AH, #0x20 ;
			LC toggleONCE ;toggles pin7 once

			CMP AH, ar4
			B addVal, NEQ
			dec ar1 ;ar1 holds count to know if we are done.
			B doneRead, EQ
			LSL AL,1 ;shifts it left if 0
			B readLoop, UNC

addVal:
			TSET AL, #0

			dec ar1 ;ar1 holds count to know if we are done.
			B doneRead, EQ
			LSL AL,1
			B readLoop, UNC



doneRead:
			LSL AL,1
			LRET


toggleONCE:
			mov ar0, #GPATOGGLE
			mov *ar0, #0x80

			LRET


toggle1:
			mov AH, #10
toggleloop:
			mov ar0, #GPATOGGLE
			mov *ar0, #0x80 ;toggling pin7 which is DCLOCK
			dec AH
			B toggleloop, NEQ

			LRET

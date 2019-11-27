#include<p18f4520.h>
//-------------------------INITIAL CONFIGURATIONS--------------------------------------------------
#pragma config WDT=OFF
#pragma config OSC=HS
#pragma config LVP=OFF
#pragma config PBADEN=OFF


//--------------------------GLOBAL VARIABLES------------------------------------------------------
unsigned int val=0;
unsigned char timer0_count=0,flag=0;
unsigned int sine_val_high[21]={512.0000,615.1870,708.2734,782.1472,829.5770,845.9203,829.5770,782.1472,708.2734,615.1870,512.0000,408.8130,315.7266,241.8528,194.42300,178.0797,194.4230,241.8528,315.7266,408.8130,512.0000}
,sine_val_low[21]={512.0000,594.5499,669.0193,728.1186,766.0626,779.1372,766.0626,728.1186,669.0193,594.5499,512.0000,429.4501,354.9807,295.8814,257.9374,244.8628,257.9374,295.8814,354.9807,429.4501,512.0000},
my_array[11];

//-------------------------FUNCTION DECLARATIONS--------------------------------------------------
void delay(unsigned int d);
//void interrupt_func(void);
void adc_val(void);
void timer0_service(void);
void comparator(void);

//-------------------------INTERRUPT SERVICE ROUTINE DECLARATION & DEF----------------------------
#pragma interrupt chk

void chk()
{
	if(INTCONbits.TMR0IF=1)
	{
		timer0_service();
	}
	else if(PIR2bits.CMIF==0)
	{
		comparator();
	}
}
//------------------------INTERRUPT VECTOR TABLE--------------------------------------------------
#pragma code int_vect=0x0008

void int_vect(void)
{
	_asm
		GOTO timer0_service
	_endasm
}
#pragma code



//-------------------------------MAIN FUNCTION------------------------------------------------------
void main()
{
//............................CONFIGURING PORTS..........................................
	TRISAbits.TRISA0=1;		//Setting ANO as input
	TRISAbits.TRISA1=1;		//Setting AN1 as input
	TRISCbits.TRISC1=0;		//CCP2 pin as o/p
	TRISCbits.TRISC2=0;		//CCP1 pin as o/p
	TRISD=0;
	PORTCbits.RC1=0;
	PORTCbits.RC2=0;
//............................CONFIGURING ADC MODULE......................................
	ADRESL=0;
	ADRESH=0;
	ADCON1=0;				//All channels analog,VDD and VSS as ref
	ADCON0=0;				//AN0 as input analog channel
	ADCON2=0b10010101;		//Right justified, Fosc/16,4*TAD		 
	ADCON0bits.ADON=1;		//Turning on ADC module
//...........................CONFIGURING CCP MODULES......................................
	PR2=83.375;
	CCPR1L=37;
	T2CON=0x02;
    CCP1CON=0x0D;
	T2CONbits.TMR2ON=1;
	CCPR2L=37;
	TRISCbits.TRISC1=0;	
	T2CON=0x02;
	T2CONbits.TMR2ON=1;
	CCP2CON=0x00;
//............................CONFIGURING COMAPARATOR MODULE................................
	PIR2bits.CMIF=0;
	CMCON=0b00000110;			//RA0 as input, two comparators used
	CVRCON=0b10101100;			//vref=0;
//............................CONFIGURING TIMER 0 MODULE...................................
	T0CON=0b00001000;			//no prescalar for timer 0
	TMR0H=0XEC;
	TMR0L=0X78;
//............................INTERRUPT CONFIGURATION......................................
	INTCONbits.TMR0IE=1;
	INTCONbits.TMR0IF=0;
	INTCONbits.GIE=0;
//	INTCONbits.PEIE=1;
//	PIE2bits.CMIE=1;
//............................WHILE(1)......................................................
	while(1)
	{
		if(timer0_count==20)
		{
			timer0_count=0;
		}
		T0CONbits.TMR0ON=1;
		INTCONbits.GIE=1;
		PIR2bits.CMIF=0;
		while(PIR2bits.CMIF==0)
		{
			adc_val();				//Taking the adc value
		}
		PORTDbits.RD0=~PORTDbits.RD0;
	}
}


//-----------------------------------------FUNCTION DEFINATIONS------------------------

//......................DELAY FUNCTION................................................
void delay(unsigned int d)
{
	unsigned int i=0,j=0;
	for(i=0;i<d;i++)
		for(j=0;j<165;j++);
}
//.....................ADC_VAL FUNCTION...............................................
void adc_val(void)
{
	unsigned int temp_val=0;	
	ADCON0bits.GO=1;			//1:Starting the analog to digital conversion
	while(ADCON0bits.GO==1);
	temp_val=ADRESL;
	temp_val|=((unsigned int) ADRESH)<<8;
	val=temp_val;
}
//....................timer0service...................................................
void timer0_service()
{
	//interrupt_func();
	T0CONbits.TMR0ON=0;
	INTCONbits.TMR0IF=0;
	timer0_count++;					//incrementing timer_o count
	my_array[timer0_count]=val;
	if((val>=sine_val_low[timer0_count])&&(val<=(sine_val_high[timer0_count])))
	{
		if(flag==1)
		{
			delay(4000);
			CCP2CON=0;			//ccp2 off,mains returns
			PORTCbits.RC1=0;
			delay(20);
			CCP1CON=0x0D;		//ccp1 on
			flag=0;
		}
	
	}
	else if((val<=sine_val_low[timer0_count])&&(val>=(sine_val_high[timer0_count])))
	{
			CCP1CON=0;			//ccp1 off, mains gone
			PORTCbits.RC2=0;
			delay(20);
			CCP2CON=0x0D;		//ccp2 on
			flag=1;	
	}
	TMR0H=0xEC;
	TMR0L=0x78;
	T0CONbits.TMR0ON=1;
}
void comparator(void)
{
	


}
//------------------------------------------------------------------------------------------

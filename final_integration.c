//FINAL WORKING CODE WITH ALL INTEGRATION HAPPENING FINAL FINAL FINAL
//p1.24 for relay
//RAN ON THE DAY OF PIC TAKING

/*
*for ultrasonic sensor detecting proximity:
*echo pin:P2.10(interrupt) ; trigger pin: P2.11 ;
*for ultrasonic sensor detecting water level
*echo pin: P0.16 ; trigger pin P0.15
*/

#include <stdio.h> 
#include <LPC17xx.h> 
#include <string.h>
// Defining Constants
#define LED_Pinsel 0xff // P0.4-0.11 (LEDs)
#define TRIGGER_PIN_W (1 << 15) // P0.15 (Trigger Pin) WATER LEVEL
#define ECHO_PIN_W (1 << 16) // P0.16 (Echo Pin) WATER LEVEL
#define TRIGGER_PIN_P (1<<11) //P2.11 (Trigger pin) PROXIMITY
#define ECHO_PIN_P (1<<10) //P2.10 (Echo  pin) PROXIMITY

void EINT3_IRQHandler(void);

// Variable Declarations
int temp, temp1, temp2 = 0; int flag = 0, flag_command=0;
int flag1,flag2;
int empty=0;
int trig_flag=1; //used to see if a trigger pulse is to be sent to the ultrasonic sensor used to detect proximity
int i, j, k, l, r, echoTime = 5000,echoT=5000; float distance = 0,dist=0;
int lcd_init[]={0x30,0x30,0x30,0x20,0x28,0x0C,0x06,0x01,0x80}; //Command codes to initialize LCD
char s[20]="";
//Function Declarations 
void delay(int r1); 
void timer_start(void); 
float timer_stop(void); 
void timer_init(void);

void timer_prox_start(void); 
float timer_prox_stop(void); 
void timer_prox_init(void);

void delay_in_US(unsigned int microseconds);
void delay_in_MS(unsigned int milliseconds);

//Functions
void port_write()
{
	int a;
	LPC_GPIO0->FIOMASK=0xE07FFFFF;
	LPC_GPIO0->FIOPIN=temp2<<23;
	
	if(flag1==0)
	{
		LPC_GPIO0->FIOCLR=1<<27;
	}
	else
	{
		LPC_GPIO0->FIOSET=1<<27;
	}
	//enable LCD
	LPC_GPIO0->FIOSET=1<<28;
	for(a=0;a<20;a++);//delay
	LPC_GPIO0->FIOCLR=1<<28;
	LPC_GPIO0->FIOMASK=0;
	for(a=0;a<300000;a++);//delay for lcd to respond
	
}
void lcd_write()
{
	
	flag2 = (flag1 == 1) ? 0 :((temp1 == 0x30) || (temp1 == 0x20)) ? 1 : 0;
	
	
	temp2=temp1&0xF0;
	temp2=temp2>>4;
	port_write();
	if(flag2==0)
	{
		temp2=temp1&0xF;
		port_write();
	}
}
int main()
{
  SystemInit(); 
  SystemCoreClockUpdate(); 
	timer_init();
	timer_prox_init();
  LPC_PINCON->PINSEL0 &= 0xfffff00f; // Interface LEDs P0.4-P0.11 
  LPC_PINCON->PINSEL0 &= 0x3fffffff; // Interface TRIG P0.15 
	LPC_PINCON->PINSEL1 =0;
	LPC_PINCON->PINSEL3 =0;//for p1.24
	
	//for the GPIO interrupt using P2.10
	LPC_PINCON->PINSEL4|=0; //P2.10 and P2.11 in function 0
	LPC_GPIO2->FIODIR|=1<<11; //P2.11 in output mode (trigger)
	
	LPC_GPIOINT->IO2IntEnR=1<<10; //Rising edge P2.10
	
	NVIC_EnableIRQ(EINT3_IRQn); //enable the interrupt
	
	LPC_GPIO1->FIODIR|=1<<24;//p1.24
	
  LPC_GPIO0->FIODIR |= TRIGGER_PIN_W | 1 << 17; // Direction for TRIGGER pin 
	LPC_GPIO0->FIODIR|=0<<16|3<<27 | 0xF<<23;
  LPC_GPIO0->FIODIR |= LED_Pinsel << 4; // Direction for LED 
  LPC_PINCON->PINSEL1 |= 0;	

  i = 0;
  flag = 1;
  LPC_GPIO0->FIOCLR |= TRIGGER_PIN_W;
  while (1)
  {
		if(trig_flag==1)
		{
			LPC_GPIO2->FIOSET=1<<11; //Send high on P2.11 trigger pin
			delay_in_US(10);
			LPC_GPIO2->FIOCLR=1<<11; //Pull down the P2.11 pin to a low
			trig_flag=0;
		}
			
    LPC_GPIO0->FIOSET = 0x00000800;
    // Output 10us HIGH on the TRIGGER pin 
	  LPC_GPIO0->FIOMASK = 0xFFFF7FFF; 
	  LPC_GPIO0->FIOPIN |= TRIGGER_PIN_W;
    delay_in_US(10);
    LPC_GPIO0->FIOCLR |= TRIGGER_PIN_W; LPC_GPIO0->FIOMASK = 0x0;
    
		while (!(LPC_GPIO0->FIOPIN & ECHO_PIN_W));// Wait till ECHO PIN becomes high
    timer_start();
    while (LPC_GPIO0->FIOPIN & ECHO_PIN_W); // Wait till ECHO PIN becomes low 
    echoTime = timer_stop(); // Store the time taken on stopping the timer 
    distance = (0.00343 * echoTime) / 2; //Calculations of Distance in cm
		if(distance<=7.5)
		{
			empty=1;
						flag1=0;
		for(i=0;i<9;i++)
		{
			temp1=lcd_init[i];
			lcd_write();
		}
		flag1=1;
		sprintf(s,"Please refill");
		
		for(i=0;s[i]!='\0';i++)
		{
			if(i==16)
			{
				flag1=0;
				temp1=0xC0;
				lcd_write();
				flag1=1;
			}
			temp1=s[i];
			lcd_write();
		}
		continue;
		}
		else
			empty=0;
		flag1=0;
		for(i=0;i<9;i++)
		{
			temp1=lcd_init[i];
			lcd_write();
		}
		flag1=1;
		sprintf(s,"Depth: %.3f",distance);
		for(i=0;s[i]!='\0';i++)
		{
			if(i==16)
			{
				flag1=0;
				temp1=0xC0;
				lcd_write();
				flag1=1;
			}
			temp1=s[i];
			lcd_write();
		}
			
    delay(88000);
}
}

void delay_in_US(unsigned int microseconds)
{
	LPC_TIM0->TCR = 0x02;
  LPC_TIM0->PR = 0; // Set prescaler to the value of 0 
	LPC_TIM0->MR0 = microseconds - 1; // Set match register for 10us 
	LPC_TIM0->MCR = 0x01; // Interrupt on match
  LPC_TIM0->TCR = 0x01; // Enable timer
  while ((LPC_TIM0->IR & 0x01) == 0); // Wait for interrupt flag 
	LPC_TIM0->TCR = 0x00; // Stop the timer
  LPC_TIM0->IR = 0x01; // Clear the interrupt flag
}
void delay_in_MS(unsigned int milliseconds) // Using Timer0
{
  delay_in_US(milliseconds * 1000);
}
void timer_init(void)
{
// Timer for distance 
	LPC_TIM0->CTCR = 0x0;
  LPC_TIM0->PR = 11999999; //To maintain 12Mhz as per specified for LPC 1768 
}
void timer_start(void)
{
  LPC_TIM0->TCR = 0x02; // Reset Timer 
	LPC_TIM0->TCR = 0x01; // Enable timer
}
float timer_stop()
{
  LPC_TIM0->TCR = 0x0;
  return LPC_TIM0->TC;
}
void delay(int r1)
{
  for (r = 0; r < r1; r++);
}

void timer_prox_init()
{
	LPC_TIM1->CTCR = 0x0;
  LPC_TIM1->PR = 11999999; //To maintain 12Mhz as per specified for LPC 1768 
	
}
void timer_prox_start(void)
{
  LPC_TIM1->TCR = 0x02; // Reset Timer 
	LPC_TIM1->TCR = 0x01; // Enable timer
}
float timer_prox_stop()
{
  LPC_TIM1->TCR = 0x0;
  return LPC_TIM1->TC;
}

void EINT3_IRQHandler()
{
		LPC_GPIOINT->IO2IntClr=1<<10; //Clears the interrupt
    timer_start();
    while (LPC_GPIO2->FIOPIN & ECHO_PIN_P); // Wait till ECHO PIN becomes low 
    echoT = timer_stop(); // Store the time taken on stopping the timer 
    dist = (0.00343 * echoT) / 2; //Calculations of Distance in cm
		
	
    if (dist < 40)
      {
          LPC_GPIO0->FIOSET = LED_Pinsel << 4; 
          
					LPC_GPIO1->FIOCLR = 1 << 24;//turn on relay
      }
    else
      {
          LPC_GPIO0->FIOCLR = LED_Pinsel << 4; 
         
				  LPC_GPIO1->FIOSET = 1 << 24; //turn off relay
      }
		trig_flag=1;
}

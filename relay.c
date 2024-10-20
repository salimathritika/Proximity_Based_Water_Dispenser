#include <lpc17xx.h>

void timer_delay()
{
	LPC_TIM0->TCR=0X02;
	LPC_TIM0->CTCR=0;
	LPC_TIM0->MCR=0X4;
	LPC_TIM0->PR=299;
	LPC_TIM0->MR0=49999; //5S DELAY
	LPC_TIM0->EMR=0X20;
	LPC_TIM0->TCR=0X1;
	while(LPC_TIM0->EMR & 0x1==0);
}

int main()
{
	LPC_PINCON->PINSEL0=0;
	LPC_GPIO0->FIODIR=1<<3; //CONNECT RELAY TO P0.3
	while(1)
	{
		timer_delay();
		LPC_GPIO0->FIOSET=1<<3;
		timer_delay();
		LPC_GPIO0->FIOCLR=1<<3;
	}
	
	
}

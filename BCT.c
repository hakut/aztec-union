#include <msp430.h> 


/*  This program basically uses timer interrupt to
 *  measure time and using shift registers shows the
 *  clock in binary form.
 */

volatile unsigned int hour=0;//initializing clock parameters and time counter
volatile unsigned int sec=0;
volatile unsigned int min=0;
volatile unsigned int counter=0;

#define datapin BIT0//assigning msp pins as clock,latch and data
#define clockpin BIT1
#define latchpin BIT2
#define resetpin BIT3

void clock()//clock function produces clocks
{
    P1OUT |= clockpin;
    P1OUT &= ~clockpin;
}
void senddata(data)//sends data from data pin to shift registers
{
    if(data)//if logic operation gives 1 activate the data pin of msp
    {
        P1OUT |= datapin;
    }
    else//otherwise, deactivate data pin of msp
    {
        P1OUT &= ~datapin;
    }
}
int main(void)
{
	WDTCTL = WDTPW | WDTHOLD;	// stop watchdog timer
	
	P1DIR |= (datapin+clockpin+latchpin+resetpin);//set necessary ports as output direction

    P1REN = BIT3; // Enable P1.3 internal resistance
    P1OUT = BIT3; // Set P1.3 as pull up resistance
    P1IES = BIT3; // P1.3 High/Low Edge
    P1IFG &= ~BIT3; // P1.3 IFG Cleared
    P1IE = BIT3; // P1.3 Interrupt Enabled


    BCSCTL1=CALBC1_1MHZ;//adjust to 1MHz
    DCOCTL =CALDCO_1MHZ;
    CCTL0 = CCIE;//CCR0 interrupt enabled
    CCR0 = 50000;//adjust the timer to 50 ms
    TACTL = TASSEL_2 + MC_1;// SMCLK/8, upmode
    _BIS_SR(GIE);//power mode

}
#pragma vector = TIMER0_A0_VECTOR//this is where program runs
__interrupt void Timer_A (void)
    {
        counter++;//we count to obtain 1s in timer
        if((k%20)==0)
        {

            sec++;//every second we arrange sec, min and hour parameters
            if((sec%60)==0)
            {
                sec=0;
                min++;
            }
            if((min%60)==0)
            {
                min=0;
                hour++;
            }
            if((hour%24)==0)
            {
                hour=0;
            }

            counter=0;//assigning counter 0 to avoid overflow
            int i;

            P1OUT &= ~latchpin;//we deactivate latch pin so registers
            //can store information untill we completely send it.

            for(i=7;i>=0;i--)//we use AND operation from least bit to
            {//most bit and send by order.
                senddata((sec&(1<<(7-i))));
                clock();//we clock after every AND operation
            }
            for(i=7;i>=0;i--)//we use AND operation as above but this time for minute
            {
                senddata((min&(1<<(7-i))));
                clock();
            }
            for(i=7;i>=0;i--)//we apply above operations for hour.
            {
                senddata((hour&(1<<(7-i))));
                clock();
            }
            //in the end we shifted first seconds least to most then minute and hour
            //So, in shift registers last register carries second information, second one
            //does minute and first one hour information. Now, when we give latch, all info
            //goes to leds

            P1OUT |= latchpin;//activate latch
        }

    }

#pragma vector=PORT1_VECTOR
__interrupt void Port_1 (void) // When we press reset button
{
    _delay_cycles(500000);//delay to avoid unbalanced operation
    P1OUT |= resetpin;//we set reset pin so clear pin on register gets 0. And clears bct.
    P1IFG &= ~BIT3; // P1.3 IFG Cleared
}

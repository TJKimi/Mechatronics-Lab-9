#include "msp.h"
#include "driverlib.h"
#include "stdio.h"
void initializeI2C();
volatile int PWM = 0;
volatile int temp;
#define TEMP_TIMEOUT 1000;
#define TS_address 0x48;
volatile int counter = 0;
volatile int counter2 = 0;

//Setup Timer A
const Timer_A_UpModeConfig config =
 {
  TIMER_A_CLOCKSOURCE_SMCLK,
  TIMER_A_CLOCKSOURCE_DIVIDER_64,
  46875,
  TIMER_A_TAIE_INTERRUPT_ENABLE,
  TIMER_A_CCIE_CCR0_INTERRUPT_DISABLE,
  TIMER_A_DO_CLEAR,
 };

volatile eUSCI_I2C_MasterConfig i2cConfig =
{
 EUSCI_B_I2C_CLOCKSOURCE_SMCLK, // SMCLK Clock Source
 3000000, // Use clock of 3MHz
 EUSCI_B_I2C_SET_DATA_RATE_100KBPS, // Desired I2C Clock of 400khz
 0, // No byte counter threshold
 EUSCI_B_I2C_NO_AUTO_STOP};

void main(void)

{
    WDT_A_holdTimer();
    //Initialize Clock
    CS_setDCOFrequency(3E+6);
    CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);

    //Initialize Timer A
    Timer_A_configureUpMode(TIMER_A1_BASE,&config);
    Interrupt_enableInterrupt(INT_TA1_N);
    Timer_A_startCounter(TIMER_A1_BASE,TIMER_A_UP_MODE);
    Interrupt_enableMaster();
    initializeI2C();
    P2SEL0 |= 0x10 ;  // Set bit 4 of P2SEL0 to enable TA0.1 functionality on P2.4
    P2SEL1 &= ~0x10 ; // Clear bit 4 of P2SEL1 to enable TA0.1 functionality on P2.4
    P2DIR |= 0x10 ;   // Set pin 2.4 as an output pin
    TA0CCR0 = 3000 ;
    TA0CCTL1 = OUTMOD_7 ;
    TA0CTL = TASSEL__SMCLK | MC__UP | TACLR ;

    while(true){
        TA0CCR1 = PWM;
    }
}

void initializeI2C(void)
{
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6,GPIO_PIN5,GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6,GPIO_PIN4,GPIO_PRIMARY_MODULE_FUNCTION);
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6,GPIO_PIN3,GPIO_PRIMARY_MODULE_FUNCTION);
    I2C_initMaster(EUSCI_B1_BASE, (const eUSCI_I2C_MasterConfig *)&i2cConfig);
    I2C_setSlaveAddress(EUSCI_B1_BASE,0x48);
    I2C_setMode(EUSCI_B1_BASE,EUSCI_B_I2C_RECEIVE_MODE);
    I2C_enableModule(EUSCI_B1_BASE);
}

//Timer A 1 second interrupts
void TA1_N_IRQHandler(void){
    counter++;
    I2C_masterReceiveStart(EUSCI_B1_BASE);
    temp = I2C_masterReceiveSingleByte(EUSCI_B1_BASE);
  if(counter<=121){
      PWM = 1500;
   printf("%i \n",temp);
   } else if(counter==122){
       if(counter2 == 0){
       PWM = 1500;
       counter = 0;
       printf("Step Response\n");
       counter2 = 1;}
       else{
           printf("DONE");
       }
   }
    Timer_A_clearInterruptFlag(TIMER_A1_BASE);


}



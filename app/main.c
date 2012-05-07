#include "bsp.h"
#include "bsp_leds.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"
#include "nwk_pll.h"
#include "main.h"

typedef unsigned char Result;
typedef unsigned char Bool;

static char *flash_addr = (char*) 0x10F0;


/******************************************************************************
    delay_ms
*//**
    @brief Delays a certain number of milliseconds.
******************************************************************************/
void
delay_ms(uint32_t time_delay_ms)
{
    MRFI_DelayMs(time_delay_ms);
}


void
COM_Init(void)
{
  P3SEL |= 0x30;                            // P3.4,5 = USCI_A0 TXD/RXD
  UCA0CTL1 = UCSSEL_2;                      // SMCLK

#if (BSP_CONFIG_CLOCK_MHZ_SELECT == 1)
  UCA0BR0 = 104;                            // 9600 from 1Mhz
  UCA0BR1 = 0;
  UCA0MCTL = UCBRS_1;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 2)
  UCA0BR0 = 0xDA;                           // 9600 from 2Mhz
  UCA0BR1 = 0x0;
  UCA0MCTL = UCBRS_6;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 4)
  UCA0BR0 = 0xA0;                           // 9600 from 4Mhz
  UCA0BR1 = 0x1;
  UCA0MCTL = UCBRS_6;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 6)
  UCA0BR0 = 0x7B;                           // 9600 from 6Mhz
  UCA0BR1 = 0x2;
  UCA0MCTL = UCBRS_3;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 8)
  UCA0BR0 = 0x41;                           // 9600 from 8Mhz
  UCA0BR1 = 0x3;
  UCA0MCTL = UCBRS_2;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 10)
  UCA0BR0 = 0x79;                           // 9600 from 10Mhz
  UCA0BR1 = 0x4;
  UCA0MCTL = UCBRS_7;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 12)
  UCA0BR0 = 0xE2;                           // 9600 from 12Mhz
  UCA0BR1 = 0x4;
  UCA0MCTL = 0;
#elif (BSP_CONFIG_CLOCK_MHZ_SELECT == 16)
  UCA0BR0 = 0x82;                           // 9600 from 16Mhz
  UCA0BR1 = 0x6;
  UCA0MCTL = UCBRS_6;
#else
#error "ERROR: Unsupported clock speed.  Custom clock speeds are possible. See comments in code."
#endif

  UCA0CTL1 &= ~UCSWRST;                     // **Initialize USCI state machine**
  IE2 |= UCA0RXIE;                          // Enable USCI_A0 RX interrupt
  __enable_interrupt();
}

void
TXString(char *s)
{
    for (; *s; s++)
    {
        UCA0TXBUF = *s;

        /* Wait until USCI_A0 TX buffer ready. */
        while (!(IFG2&UCA0TXIFG))
        {
            ;
        }
    }
}

void
enter_low_power_mode_3(void)
{
    /* LPM3 with interrupts enabled. */
    __bis_SR_register(LPM3_bits + GIE);
}


int
main(int argc, char *argv[])
{
    /* Initialize board-specific hardware */
    BSP_Init();
    led1_off();
    led2_off();

#if 1
    /* Initialize TimerA and oscillator */
    BCSCTL3 |= LFXT1S_2;                      // LFXT1 = VLO
    TACCTL0 = CCIE;                           // TACCR0 interrupt enabled
    TACCR0 = 12000;                           // ~1 second
    TACTL = TASSEL_1 + MC_1;                  // ACLK, upmode
#endif

    while (1)
    {
        main_init();

        while (1)
        {
            main_task();
        }
    }

    return 0;
}


#pragma vector=TIMERA0_VECTOR
__interrupt void
Timer_A(void)
{
  __bic_SR_register_on_exit(LPM3_bits);        // Clear LPM3 bit from 0(SR)
}

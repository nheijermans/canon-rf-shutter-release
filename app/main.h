#ifndef MAIN_H
#define MAIN_H

#include "bsp_leds.h"

#define TRUE  1
#define FALSE 0

#define SUCCESS ((Result) 0)
#define FAILURE ((Result) 1)


#define led1_on()   \
    do { \
        if (!BSP_LED1_IS_ON()) \
        { \
            BSP_TOGGLE_LED1(); \
        } \
    } while (0)

#define led1_off()   \
    do { \
        if (BSP_LED1_IS_ON()) \
        { \
            BSP_TOGGLE_LED1(); \
        } \
    } while (0)

#define led2_on()   \
    do { \
        if (!BSP_LED2_IS_ON()) \
        { \
            BSP_TOGGLE_LED2(); \
        } \
    } while (0)

#define led2_off()   \
    do { \
        if (BSP_LED2_IS_ON()) \
        { \
            BSP_TOGGLE_LED2(); \
        } \
    } while (0)


#define CMD_RELEASE_SHUTTER 0x2a


/** @brief Function that initializes the main task. */
void
main_init(void);

/** @brief Function prototype for the main task function. */
void
main_task(void);

/** @brief Function that puts the micro into low power mode 3. */
void
enter_low_power_mode_3(void);

/** @brief Function to delay a specified number of milliseconds.
    @param[in] time_delay_ms    Number of milliseconds to delay.
 */
void
delay_ms(uint32_t time_delay_ms);

/** @brief Function prototype for Timer A ISR. */
__interrupt void
Timer_A (void);

void
COM_Init(void);

void
TXString(char *s);

#endif /* MAIN_H */

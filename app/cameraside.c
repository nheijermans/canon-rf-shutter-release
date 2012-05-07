#ifdef CAMERA_SIDE

#include "bsp.h"
#include "bsp_buttons.h"
#include "main.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"


/** @brief Amount of time to drive the output enable lines high for camera to
           react to the focus/shutter release signals.
*/
#define OUTPUT_HIGH_DELAY_TIME_MS   1000

/** @name Focus enable GPIO pin definitions.
    @{
*/
#define FOCUS_ENABLE_PORT   P2OUT
#define FOCUS_ENABLE_DDR    P2DIR
#define FOCUS_ENABLE_BIT    (1 << 0)
/** @} */

/** @name Shutter release GPIO pin definitions.
    @{
*/
#define SHUTTER_RELEASE_PORT P2OUT
#define SHUTTER_RELEASE_DDR  P2DIR
#define SHUTTER_RELEASE_BIT  (1 << 1)
/** @} */


/** @brief Link ID. */
static linkID_t g_link_id;

/** @brief Status flag indicating that a frame is ready to be read in from the
           radio.
*/
static uint8_t frames_ready = 0;

/** @brief Status flag indicating that a new RF peer has been seen. */
static uint8_t new_peer = FALSE;



static void
focus(void)
{
    FOCUS_ENABLE_PORT |= FOCUS_ENABLE_BIT;
    delay_ms(OUTPUT_HIGH_DELAY_TIME_MS);
    FOCUS_ENABLE_PORT &= ~FOCUS_ENABLE_BIT;
}



static void
release_shutter(void)
{
    SHUTTER_RELEASE_PORT |= SHUTTER_RELEASE_BIT;
    delay_ms(OUTPUT_HIGH_DELAY_TIME_MS);
    SHUTTER_RELEASE_PORT &= ~SHUTTER_RELEASE_BIT;
}



/******************************************************************************
    wait_for_command
*//**
    @brief Waits until a button press event has occurred.
******************************************************************************/
static void
wait_for_command(uint8_t *seconds_to_wait_p)
{
    bspIState_t intState;
    uint8_t msg[2];
    uint8_t len;

    while (1)
    {
        if (new_peer)
        {
            while (1)
            {
                if (SMPL_LinkListen(&g_link_id) == SMPL_SUCCESS)
                {
                    led2_off();
                    break;
                }
            }

            BSP_ENTER_CRITICAL_SECTION(intState);
            new_peer = FALSE;
            BSP_EXIT_CRITICAL_SECTION(intState);
        }

        if (frames_ready && SMPL_Receive(g_link_id, msg, &len) == SMPL_SUCCESS)
        {
            BSP_ENTER_CRITICAL_SECTION(intState);
            frames_ready--;
            BSP_EXIT_CRITICAL_SECTION(intState);
            
            if (len == sizeof(msg) && msg[0] == CMD_RELEASE_SHUTTER)
            {
                *seconds_to_wait_p = msg[1];
                break;
            }

            BSP_TOGGLE_LED2();
        }

        if (BSP_BUTTON1())
        {
            *seconds_to_wait_p = 5;
            break;
        }
    }
}


static uint8_t
callback(linkID_t link_id)
{
    if (link_id)
    {
        frames_ready++;
    }
    else
    {
        new_peer = TRUE;
    }
    /* Leave the frame to be read by the application. */
    return 0;
}



void
main_init(void)
{
    led2_on();

    /* Configure focus enable and shutter release pins as outputs. */
    FOCUS_ENABLE_DDR |= FOCUS_ENABLE_BIT;
    SHUTTER_RELEASE_DDR |= SHUTTER_RELEASE_BIT;

    new_peer = FALSE;
    frames_ready = 0;

    COM_Init();
    TXString("In init()\n");

    SMPL_Init(callback);
}

void
main_task(void)
{
    uint8_t seconds_to_wait = 0;

    wait_for_command(&seconds_to_wait);

    TXString("Got shutter release command.\n");

    /* Blink the LED until the shutter release has been triggered. */
    while (seconds_to_wait)
    {
        if (seconds_to_wait <= 2)
        {
            led1_on();
            delay_ms(1000);
        }
        else
        {
            led1_on();
            delay_ms(300);
            led1_off();
            delay_ms(700);
        }

        seconds_to_wait--;
    }

    focus();
    led1_off();

    release_shutter();
}


#endif /* CAMERA_SIDE */

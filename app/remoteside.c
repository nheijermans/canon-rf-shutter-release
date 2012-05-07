#ifdef REMOTE_SIDE

#include "bsp.h"
#include "bsp_buttons.h"
#include "main.h"
#include "mrfi.h"
#include "nwk_types.h"
#include "nwk_api.h"


/** @brief Link ID. */
static linkID_t link_id;


/** @brief Default delay for cameraside to delay before the shutter release. */
static uint8_t shutter_release_delay_time_s = 5;


/** @brief Amount of time to delay between reading button for debouncing. */
#define DEBOUNCE_DELAY_MS           50

/** @brief Amount of time the button needs to be pressed before entering the
            time configuration mode.
*/
#define TIME_CONFIG_ENABLE_MS       2000 

/** @brief Number of debounce loops we need to go through before entering the
           time configuration mode.
*/
#define TIME_CONFIG_MODE_LOOPS      (TIME_CONFIG_ENABLE_MS / DEBOUNCE_DELAY_MS)

/******************************************************************************
    wait_for_button_press
*//**
    @brief Waits until a button press event has occurred.
******************************************************************************/
static uint8_t
wait_for_button_press(void)
{
    uint8_t is_pressed = FALSE;
    uint8_t was_pressed = FALSE;
    uint16_t count = 0;
    uint8_t time_config_mode_enabled = FALSE;
    uint8_t new_delay_time = 0;

    while (TRUE)
    {
        is_pressed = BSP_BUTTON1()? TRUE: FALSE;

        if (was_pressed && !is_pressed)
        {
            if (time_config_mode_enabled)
            {
                count = 0;
                new_delay_time++;
            }
            else
            {
                /* Enter time configuration mode. */
                if (count >= TIME_CONFIG_MODE_LOOPS)
                {
                    BSP_TURN_ON_LED1();
                    time_config_mode_enabled = TRUE;
                    count = 0;
                }
                else
                {
                    break;
                }
            }
        }

        /* Keep track of how many loops the button state is the same. */
        if (was_pressed != is_pressed)
        {
            count = 0;
        }
        else
        {
            count++;
        }

        /* Leave time configuration mode. */
        if (time_config_mode_enabled && count >= TIME_CONFIG_MODE_LOOPS)
        {
            BSP_TURN_OFF_LED1();
            time_config_mode_enabled = FALSE;
            shutter_release_delay_time_s = new_delay_time;
        }

        was_pressed = is_pressed;

        delay_ms(DEBOUNCE_DELAY_MS);
    }

    return TRUE;
}


/******************************************************************************
    send_shutter_release_msg
*//**
    @brief Links to another node and sends a shutter release message.
******************************************************************************/
static void
send_shutter_release_msg(void)
{
    uint8_t msg[] = {CMD_RELEASE_SHUTTER, 0x05};

    msg[1] = shutter_release_delay_time_s;

    if (link_id == 0)
    {
        /* Keep trying to link... */
        while (SMPL_SUCCESS != SMPL_Link(&link_id))
        {
            BSP_TOGGLE_LED2();
            
            /* Go to sleep (LPM3 with interrupts enabled). Timer A0 interrupt
             * will wake the CPU up every second to retry linking.
             */
            enter_low_power_mode_3();
        }
    }

    /* No AP acknowledgement, just send a single message to the AP */
    SMPL_SendOpt(link_id, msg, sizeof(msg), SMPL_TXOPTION_NONE);
}



void
main_init(void)
{
    link_id = 0;

    while (SMPL_Init(NULL) != SMPL_SUCCESS)
    {
        BSP_TOGGLE_LED2();
        enter_low_power_mode_3();
    }
}


void
main_task(void)
{
    wait_for_button_press();
    send_shutter_release_msg();
}

#endif /* REMOTE_SIDE */

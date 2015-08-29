#include "ets_sys.h"        // Event signals and task priorities
#include "osapi.h"          // General ESP functions (timers, strings, memory)
#include "gpio.h"           // Interacting and configuring pins
#include "os_type.h"        // Mapping to ETS structures
#include "user_config.h"    // Any user-defined functions. Alway required.

// create os_timer that's not optimized by the compiler and is file-wide in scope
static volatile os_timer_t some_timer;

// Function to drive GPIO pin high or low
void some_timerfunc(void *arg)
{
    // If GPIO_2 is set HIGH, set it LOW
    if (GPIO_REG_READ(GPIO_OUT_ADDRESS) & BIT2)
    {
        //Set GPIO2 to LOW
        gpio_output_set(0, BIT2, BIT2, 0);
    }
    else
    {
        //Set GPIO2 to HIGH
        gpio_output_set(BIT2, 0, BIT2, 0);
    }
}

// Init function 
void ICACHE_FLASH_ATTR  // Store function in flash memory instead of RAM
user_init()
{
    // Initialize the GPIO subsystem.
    gpio_init();

    //Set GPIO2 to output mode
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

    //Set GPIO2 low
    gpio_output_set(0, BIT2, BIT2, 0);

    //Disarm timer
    os_timer_disarm(&some_timer);

    //Setup timer
    os_timer_setfn(&some_timer, (os_timer_func_t *)some_timerfunc, NULL);

    //Arm the timer
    //&some_timer is the pointer
    //1000 is the fire time in ms
    //0 for once and 1 for repeating
    os_timer_arm(&some_timer, 1000, 1);
}

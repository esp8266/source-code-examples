#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"
#include "user_interface.h"

#define user_procTaskPrio        0
#define user_procTaskQueueLen    1
os_event_t    user_procTaskQueue[user_procTaskQueueLen];
static void loop(os_event_t *events);


// variable modified indirectly by interrupt handler
volatile int whatyouwant;

// gpio interrupt handler. See below
LOCAL void  gpio_intr_handler(int * dummy);

//-------------------------------------------------------------------------------------------------
// loop function will be execute by "os" periodically
static void ICACHE_FLASH_ATTR  loop(os_event_t *events)
{
    static int level;

    os_printf("What you want %d \r\n",whatyouwant);
// change GPIO 2 level
    level = !level;
    GPIO_OUTPUT_SET(GPIO_ID_PIN(2), ((level) ? 1 : 0));
// Delay
    os_delay_us(100000);
// turn again
    system_os_post(user_procTaskPrio, 0, 0 );
}

//-------------------------------------------------------------------------------------------------
//Init function
void ICACHE_FLASH_ATTR  user_init()
{

// Initialize UART0 to use as debug
    uart_div_modify(0, UART_CLK_FREQ / 9600);

    os_printf(
                "GPIO Interrupt ESP8266 test\r\n"
                "---------------------------\r\n"
                "  This program out a square wave on gpio2 and count edges for gpio0\r\n"
                "  The program report count edges gpio0 periodically.\r\n"
                "  You Can\r\n"
                "   1.- Connect GPIO0 to GPIO2 so you can see increment count edge periodically\r\n"
                "   2.- Unconnect GPIO0 from GPIO2 so you can see stop count edge\r\n"
                "   3.- Connect GPIO0 to ground to increment count edge\r\n"
);

// Initialize the GPIO subsystem.
   gpio_init();

// =================================================
// Initialize GPIO2 and GPIO0 as GPIO
// =================================================
//
//
// PIN_FUNC_SELECT() defined in eagle_soc.h
// PERIPHS_IO_MUX_... for each pin defined in eagle_soc.h
// FUNC_... for each PERIPHS macros defined in eagle_soc.h
//
// From eagle_soc.h:
// GPIO0 only can be GPIO
// GPIO2 can be GPIO, TXD from uart0, or TXD from uart1
//
//   #define PERIPHS_IO_MUX_GPIO0_U          (PERIPHS_IO_MUX + 0x34)
//   #define FUNC_GPIO0                      0
//   #define PERIPHS_IO_MUX_GPIO2_U          (PERIPHS_IO_MUX + 0x38)
//   #define FUNC_GPIO2                      0
//   #define FUNC_U1TXD_BK                   2
//   #define FUNC_U0TXD_BK                   4
//

    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
    PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);

// You can select enable/disable internal pullup/pulldown for each pin
//
//  PIN_PULLUP_DIS(PIN_NAME)
//  PIN_PULLUP_EN(PIN_NAME)
//  PIN_PULLDWN_DIS(PIN_NAME)
//  PIN_PULLDWN_EN(PIN_NAME)

    PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO0_U);
    PIN_PULLDWN_DIS(PERIPHS_IO_MUX_GPIO0_U);
    PIN_PULLUP_EN(PERIPHS_IO_MUX_GPIO2_U);

//
//  void    gpio_output_set(uint32 set_mask, uint32 clear_mask,  uint32 enable_mask, uint32 disable_mask)
//
// set status and direction pin by mask gpio id pin. 

    gpio_output_set(0, GPIO_ID_PIN(2), GPIO_ID_PIN(2), GPIO_ID_PIN(0)); // set gpio 2 as output and low. set gpio 0 as input

// =================================================
// Activate gpio interrupt for gpio2
// =================================================

    // Disable interrupts by GPIO
    ETS_GPIO_INTR_DISABLE();

// Attach interrupt handle to gpio interrupts.
// You can set a void pointer that will be passed to interrupt handler each interrupt

    ETS_GPIO_INTR_ATTACH(gpio_intr_handler, &whatyouwant);

//    void gpio_register_set(uint32 reg_id, uint32 value);
//
// From include file
//   Set the specified GPIO register to the specified value.
//   This is a very general and powerful interface that is not
//   expected to be used during normal operation.  It is intended
//   mainly for debug, or for unusual requirements.
//
// All people repeat this mantra but I don't know what it means
//
     gpio_register_set(GPIO_PIN_ADDR(0),
                       GPIO_PIN_INT_TYPE_SET(GPIO_PIN_INTR_DISABLE)  |
                       GPIO_PIN_PAD_DRIVER_SET(GPIO_PAD_DRIVER_DISABLE) |
                       GPIO_PIN_SOURCE_SET(GPIO_AS_PIN_SOURCE));

// clear gpio status. Say ESP8266EX SDK Programming Guide in  5.1.6. GPIO interrupt handler

     GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, BIT(0));

// enable interrupt for his GPIO
//     GPIO_PIN_INTR_... defined in gpio.h

     gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_ANYEGDE);

     ETS_GPIO_INTR_ENABLE();

    //Start os task
    system_os_task(loop, user_procTaskPrio,user_procTaskQueue, user_procTaskQueueLen);

    system_os_post(user_procTaskPrio, 0, 0 );
}

//-------------------------------------------------------------------------------------------------
// interrupt handler
// this function will be executed on any edge of GPIO0
LOCAL void  gpio_intr_handler(int * dummy)
{
// clear gpio status. Say ESP8266EX SDK Programming Guide in  5.1.6. GPIO interrupt handler

    uint32 gpio_status = GPIO_REG_READ(GPIO_STATUS_ADDRESS);

// if the interrupt was by GPIO0
    if (gpio_status & BIT(0))
    {
// disable interrupt for GPIO0
        gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_DISABLE);

// Do something, for example, increment whatyouwant indirectly
        (*dummy)++;

//clear interrupt status for GPIO0
        GPIO_REG_WRITE(GPIO_STATUS_W1TC_ADDRESS, gpio_status & BIT(0));

// Reactivate interrupts for GPIO0
        gpio_pin_intr_state_set(GPIO_ID_PIN(0), GPIO_PIN_INTR_ANYEGDE);
    }
}


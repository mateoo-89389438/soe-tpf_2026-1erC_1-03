/********************** inclusions *******************************************/
/* Project includes */
#include "main.h"
#include "cmsis_os.h"

/* HAL includes */
#include "stm32f1xx_hal.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "app.h"
#include "task_gatekeeper.h"

/********************** macros and definitions *******************************/
#define SPI_TIMEOUT_MS  (pdMS_TO_TICKS(1000ul))

/********************** internal data declaration ****************************/
const char *p_task_gatekeeper_rx_ok   = "   ==> Task Gatekeeper - SPI Rx Complete";
const char *p_task_gatekeeper_rx_err  = "   ==> Task Gatekeeper - SPI Rx Error";

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
static uint32_t g_t_tx_us = 0ul;
static uint32_t g_wcet_tx_us = 0ul;

/********************** external data declaration ****************************/
extern SPI_HandleTypeDef hspi1;
extern QueueHandle_t h_queue_spi_rx;

/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
    s_spi_msg_t rx_msg;

    /* Print out: Task Initialized */
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    for (;;)
    {
        /* Block task indefinitely until a message is received from the ISR */
        if (pdTRUE == xQueueReceive(h_queue_spi_rx, &rx_msg, portMAX_DELAY))
        {
        	/* Calculate elapsed time in microseconds (us) AFTER CS is released */
        	g_t_tx_us = cycle_counter_get_time_us();

            /* De-assert CS (High) to release the Flash Memory bus */
            HAL_GPIO_WritePin(FLASH_CS_GPIO_Port, FLASH_CS_Pin, GPIO_PIN_SET);

            /* Update the WCET if the current reception time is greater */
			if (g_t_tx_us > g_wcet_tx_us)
			{
				g_wcet_tx_us = g_t_tx_us;
			}

            /* Print out: Reception successful and the actual JEDEC ID data */
            LOGGER_INFO(p_task_gatekeeper_rx_ok);
        }
    }
}

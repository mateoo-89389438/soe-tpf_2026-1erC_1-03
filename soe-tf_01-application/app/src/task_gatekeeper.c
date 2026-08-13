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
#define G_TASK_GATEKEEPER_CNT_INI	0ul

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_gatekeeper_tx_ok 		= "   ==> Gatekeeper: SPI Tx Complete";
const char *p_task_gatekeeper_tx_error 	= "   ==> Gatekeeper: SPI Tx Error";

static uint32_t g_t_tx_us = 0ul;
static uint32_t g_wcet_tx_us = 0ul;

/********************** external data declaration ****************************/
uint32_t g_task_gatekeeper_cnt;
extern SPI_HandleTypeDef hspi1;
extern QueueHandle_t h_queue_spi;

/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
	g_task_gatekeeper_cnt = G_TASK_GATEKEEPER_CNT_INI;

    s_spi_msg_t rx_msg;

    /* Print out: Task Initialized */
	LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    for (;;)
    {
    	/* Update Task Counter */
    	g_task_gatekeeper_cnt++;

        if (xQueueReceive(h_queue_spi, &rx_msg, portMAX_DELAY) == pdPASS)
        {
            /* Chip Select (CS) to LOW in PA4 */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

            /* Reset the clock cycle counter just before starting TX */
            cycle_counter_reset();

            /* Start non-blocking transmission */
            HAL_SPI_Transmit(&hspi1, rx_msg.p_data, rx_msg.size, HAL_MAX_DELAY);

            /* Calculate elapsed time in microseconds (us) */
            g_t_tx_us = cycle_counter_get_time_us();

            /* Chip Select (CS) to HIGH in PA4 */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

            if (g_t_tx_us > g_wcet_tx_us)
            {
                g_wcet_tx_us = g_t_tx_us;
            }

            LOGGER_INFO(p_task_gatekeeper_tx_ok);
        }
        else
        {
        	LOGGER_INFO(p_task_gatekeeper_tx_error);
        }

    }
}

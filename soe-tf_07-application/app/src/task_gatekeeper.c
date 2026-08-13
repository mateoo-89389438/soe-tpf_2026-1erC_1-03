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
#define G_TASK_GATEKEEPER_CNT_INI    0ul
#define SPI_TIMEOUT_MS  (pdMS_TO_TICKS(1000ul))

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_gatekeeper_rx_ok 	= "   ==> Gatekeeper: SPI Rx Complete";
const char *p_task_gatekeeper_rx_err 	= "   ==> Gatekeeper: SPI Rx Error";

static uint32_t g_t_tx_us = 0ul;
static uint32_t g_wcet_tx_us = 0ul;

/********************** external data declaration ****************************/
uint32_t g_task_gatekeeper_cnt;
extern SPI_HandleTypeDef hspi1;
extern QueueHandle_t h_queue_spi;

/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
	/* Declare & Initialize Task Function variables */
	g_task_gatekeeper_cnt = G_TASK_GATEKEEPER_CNT_INI;

    s_spi_msg_t rx_msg;

    /* Print out: Task Initialized */
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    for (;;)
    {
        /* Wait for a message in the queue (Block indefinitely) */
        if (xQueueReceive(h_queue_spi, &rx_msg, portMAX_DELAY) == pdPASS)
        {
            /* Reset cycle counter BEFORE starting the SPI driver transaction */
            cycle_counter_reset();

            /* Assert CS (Active Low on PA4) */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

            /* Receive Data via Polling (Known Length) */
            HAL_SPI_Receive(&hspi1, rx_msg.p_data, rx_msg.size, HAL_MAX_DELAY);

            /* De-assert CS */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

            /* Calculate elapsed time in microseconds (us) AFTER CS is released */
            g_t_tx_us = cycle_counter_get_time_us();

            /* Update the WCET if the current reception time is greater */
            if (g_t_tx_us > g_wcet_tx_us)
            {
                g_wcet_tx_us = g_t_tx_us;
            }

            LOGGER_INFO(p_task_gatekeeper_rx_ok);
        }
        else
        {
        	LOGGER_INFO(p_task_gatekeeper_rx_err);
        }
    }
}

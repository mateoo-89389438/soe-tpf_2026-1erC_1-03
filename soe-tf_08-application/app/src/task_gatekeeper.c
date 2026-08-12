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
#define SPI_POLLING_TIMEOUT_MS       100ul

#define W25Q32_CMD_READ_JEDEC_ID     0x9F

/********************** internal data declaration ****************************/
const char *p_task_gatekeeper_rx_ok   = "   ==> Task Gatekeeper - SPI Rx Complete";
const char *p_task_gatekeeper_rx_err  = "   ==> Task Gatekeeper - SPI Rx Error";

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
static uint32_t g_t_tx_us = 0ul;
static uint32_t g_wcet_tx_us = 0ul;

/********************** external data declaration ****************************/
extern SPI_HandleTypeDef hspi1;
uint32_t g_task_gatekeeper_cnt;

/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
    s_spi_msg_t spi_msg;
    HAL_StatusTypeDef hal_status;
    uint8_t cmd = W25Q32_CMD_READ_JEDEC_ID;

    /* Declare & Initialize Task Function variables */
    g_task_gatekeeper_cnt = G_TASK_GATEKEEPER_CNT_INI;

    /* Print out: Task Initialized */
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    /* As per most tasks, this task is implemented in an infinite loop. */
    for (;;)
    {
        /* Wait indefinitely for SPI request messages from the queue */
        if (pdPASS == xQueueReceive(h_queue_spi, &spi_msg, portMAX_DELAY))
        {
            g_task_gatekeeper_cnt++;

            /* Assert Chip Select (PA4 -> Low) */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

            /* Reset the clock cycle counter just before starting TX */
			cycle_counter_reset();

            /* Send Read JEDEC ID Command */
            hal_status = HAL_SPI_Transmit(&hspi1, &cmd, 1, SPI_POLLING_TIMEOUT_MS);

            if (HAL_OK == hal_status)
            {
                /* Receive payload of known length via Polling */
                hal_status = HAL_SPI_Receive(&hspi1, spi_msg.p_data, spi_msg.size, SPI_POLLING_TIMEOUT_MS);
            }

            /* Calculate elapsed time in microseconds (us) */
			g_t_tx_us = cycle_counter_get_time_us();

            /* Deassert Chip Select (PA4 -> High) */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

            /* Update the WCET if the current time is greater */
			if (g_t_tx_us > g_wcet_tx_us)
			{
				g_wcet_tx_us = g_t_tx_us;
			}

            if (HAL_OK == hal_status)
            {
                LOGGER_INFO(p_task_gatekeeper_rx_ok);
            }
            else
            {
                LOGGER_INFO(p_task_gatekeeper_rx_err);
            }
        }
    }
}

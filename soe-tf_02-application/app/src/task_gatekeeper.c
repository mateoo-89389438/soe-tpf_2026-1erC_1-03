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
#define G_TASK_GATEKEEPER_CNT_INI   0ul

/********************** internal data declaration ****************************/
const char *p_task_gatekeeper_rx_ok   = "   ==> Task Gatekeeper - SPI Tx Complete";
const char *p_task_gatekeeper_rx_err  = "   ==> Task Gatekeeper - SPI Tx Error";

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
uint32_t g_task_gatekeeper_cnt;
static uint32_t g_t_tx_us = 0ul;
static uint32_t g_wcet_tx_us = 0ul;

/********************** external data declaration ****************************/
extern SPI_HandleTypeDef hspi1;
extern QueueHandle_t h_queue_spi;

/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
	/* Declare & Initialize Task Function variables */
	s_spi_msg_t rx_msg;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running", pcTaskGetName(NULL));

	for (;;)
	{
		/* Wait indefinitely for a message in the queue */
		if (xQueueReceive(h_queue_spi, &rx_msg, portMAX_DELAY) == pdTRUE)
		{
			/* Chip Select (CS) to LOW in PA4 */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

			/* Reset Cycle Counter */
			cycle_counter_reset();

			/* Transmit via SPI using Polling */
			HAL_SPI_Transmit(&hspi1, rx_msg.p_data, rx_msg.size, HAL_MAX_DELAY);

			/* Get Transmission Time in us */
			g_t_tx_us = cycle_counter_get_time_us();

			/* Chip Select (CS) to HIGH in PA4 */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

			/* Update Worst-Case Execution Time (WCET) */
			if (g_t_tx_us > g_wcet_tx_us)
			{
				g_wcet_tx_us = g_t_tx_us;
			}

			/* Print out: SPI Tx Complete */
			LOGGER_INFO(p_task_gatekeeper_rx_ok);

			/* Free Memory Pool allocation */
			vPortFree(rx_msg.p_data);
		}
	}
}

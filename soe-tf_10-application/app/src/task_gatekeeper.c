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
#define SPI_TIMEOUT_MS  (pdMS_TO_TICKS(1000ul))

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
const char *p_task_gatekeeper_rx_ok   = "   ==> Task Gatekeeper - SPI Rx Complete";
const char *p_task_gatekeeper_rx_err  = "   ==> Task Gatekeeper - SPI Rx Error";

static uint32_t g_t_tx_us = 0ul;
static uint32_t g_wcet_tx_us = 0ul;

/********************** external data declaration ****************************/
uint32_t g_task_gatekeeper_cnt;
extern SPI_HandleTypeDef hspi1;
extern QueueHandle_t h_queue_spi;
extern QueueHandle_t h_queue_spi_pool;
extern SemaphoreHandle_t h_sem_spi;

/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
	g_task_gatekeeper_cnt = G_TASK_GATEKEEPER_CNT_INI;

	s_spi_msg_t *p_msg = NULL;
	HAL_StatusTypeDef hal_status;
	uint8_t cmd = 0x9F;

	/* Print out: Task Initialized */
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    for (;;)
    {
    	/* Update Task Counter */
		g_task_gatekeeper_cnt++;

        /* Block until a new SPI message arrives */
    	if (pdPASS == xQueueReceive(h_queue_spi, &p_msg, portMAX_DELAY))
		{
    		/* Assert Chip Select (PA4 -> Low) */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

			/* Reset the clock cycle counter just before starting TX */
			cycle_counter_reset();

			hal_status = HAL_SPI_Transmit(&hspi1, &cmd, 1, SPI_TIMEOUT_MS);

			if (HAL_OK == hal_status)
			{
				/* Receive payload directly into the Memory Pool Buffer */
				HAL_SPI_Receive_IT(&hspi1, p_msg->p_data, p_msg->size);
				xSemaphoreTake(h_sem_spi, portMAX_DELAY);
			}

			/* Calculate elapsed time in microseconds (us) */
			g_t_tx_us = cycle_counter_get_time_us();

			/* Deassert Chip Select (PA4 -> High) */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

			if (g_t_tx_us > g_wcet_tx_us)
			{
				g_wcet_tx_us = g_t_tx_us;
			}

			xQueueSend(h_queue_spi_pool, &p_msg, 0ul);

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

/********************** end of file ******************************************/

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
const char *p_task_gatekeeper_rx_ok   = "   ==> Task Gatekeeper - SPI Rx Complete";
const char *p_task_gatekeeper_rx_err  = "   ==> Task Gatekeeper - SPI Rx Error";
/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
uint32_t g_task_gatekeeper_cnt;
static uint32_t g_t_tx_us = 0ul;
static uint32_t g_wcet_tx_us = 0ul;

/********************** external data declaration ****************************/
extern SPI_HandleTypeDef hspi1;
extern QueueHandle_t h_queue_spi;
extern SemaphoreHandle_t h_sem_spi_dma_cplt;

/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
    s_spi_msg_t spi_msg;

    /* Initialize Task Function variables */
    g_task_gatekeeper_cnt = G_TASK_GATEKEEPER_CNT_INI;

    /* Print out: Task Initialized */
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    for (;;)
    {
    	/* Wait for a message in the queue indefinitely */
		if (xQueueReceive(h_queue_spi, &spi_msg, portMAX_DELAY) == pdPASS)
		{
			/* Set Chip Select (CS) LOW manually - PA4 */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

			/* Reset cycle counter before starting SPI transfer */
			cycle_counter_reset();

			/* Start the DMA transfer. */
			HAL_SPI_Receive_DMA(&hspi1, spi_msg.p_data, spi_msg.size);

			/* Block the task until DMA finishes (ISR releases the semaphore) */
			xSemaphoreTake(h_sem_spi_dma_cplt, portMAX_DELAY);

			/* Get elapsed time in microseconds */
			g_t_tx_us = cycle_counter_get_time_us();

			/* Set Chip Select (CS) HIGH */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

			/* Update Worst-Case Execution Time (WCET) */
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

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
extern QueueHandle_t h_queue_spi_rx;
extern SemaphoreHandle_t h_sem_spi_rx_cplt;


/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
    s_spi_msg_t current_msg;

    /* Print out: Task Initialized */
	LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running", pcTaskGetName(NULL));

    for (;;)
    {
    	/* Wait for a read message in the queue (blocking) */
		if (xQueueReceive(h_queue_spi_rx, &current_msg, portMAX_DELAY) == pdPASS)
		{

			/* Set Chip Select LOW (Manual GPIO) */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

			/* Reset cycle counter */
			cycle_counter_reset();

			/* Start DMA reception */
			HAL_SPI_Receive_DMA(&hspi1, current_msg.p_data, current_msg.size);

			/* Wait for DMA to finish (ISR -> Task synchronization) */
			xSemaphoreTake(h_sem_spi_rx_cplt, portMAX_DELAY);

			/* Compute elapsed time in microseconds */
			g_t_tx_us = cycle_counter_get_time_us();

			/* Set Chip Select HIGH */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

			/* Update Worst-Case Execution Time (WCET) */
			if (g_t_tx_us > g_wcet_tx_us)
			{
				g_wcet_tx_us = g_t_tx_us;
			}

			LOGGER_INFO(p_task_gatekeeper_rx_ok);
		}
		else{
			LOGGER_INFO(p_task_gatekeeper_rx_err);
		}
    }
}

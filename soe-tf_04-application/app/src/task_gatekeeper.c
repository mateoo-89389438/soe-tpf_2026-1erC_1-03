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
const char *p_task_gatekeeper_tx_ok 	= "   ==> Gatekeeper: SPI Tx Complete";
const char *p_task_gatekeeper_tx_error 	= "   ==> Gatekeeper: SPI Tx Error";

static uint32_t g_t_tx_us = 0ul;
static uint32_t g_wcet_tx_us = 0ul;

/********************** external data declaration ****************************/
uint32_t g_task_gatekeeper_cnt;
extern SPI_HandleTypeDef hspi1;
extern QueueHandle_t h_queue_spi;
extern QueueHandle_t h_queue_spi_pool;
extern SemaphoreHandle_t h_sem_spi;

/********************** external functions definition ************************/
/* Task thread */
void task_gatekeeper(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
	g_task_gatekeeper_cnt = G_TASK_GATEKEEPER_CNT_INI;

	s_spi_msg_t *p_msg = NULL;

    /* Print out: Task Initialized */
	LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    for (;;)
    {
    	/* Update Task Counter */
		g_task_gatekeeper_cnt++;

        /* Wait for a message from any task in the transmission queue */
    	if (xQueueReceive(h_queue_spi, &p_msg, portMAX_DELAY) == pdPASS)
		{
    		/* Activate Chip Select (CS on PA4 manually) */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

			/* Reset the clock cycle counter just before starting TX */
			cycle_counter_reset();

			/* Start transmission by interrupts */
			HAL_SPI_Transmit_IT(&hspi1, p_msg->p_data, (uint16_t)p_msg->size);

			/* Sleep the task until the SPI interrupt finishes and releases the semaphore */
			xSemaphoreTake(h_sem_spi, portMAX_DELAY);

			/* Get the elapsed transmission time in (µs) */
			g_t_tx_us = cycle_counter_get_time_us();

			/* Deactivate Chip Select */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

			/* Update the transmission WCET */
			if (g_t_tx_us > g_wcet_tx_us)
			{
				g_wcet_tx_us = g_t_tx_us;
			}

			LOGGER_INFO(p_task_gatekeeper_tx_ok);

			xQueueSend(h_queue_spi_pool, &p_msg, 0ul);
		}
    	else
		{
			LOGGER_INFO(p_task_gatekeeper_tx_error);
		}
}
}

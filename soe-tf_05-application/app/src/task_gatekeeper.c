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
const char *p_task_gatekeeper_transmission_succesful = "   ==> Gatekeeper: SPI Tx Complete";
const char *p_task_gatekeeper_timeout 				 = "   ==> Gatekeeper: SPI Tx Timeout";

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
static uint32_t g_t_tx_us = 0ul;
static uint32_t g_wcet_tx_us = 0ul;

/********************** external data declaration ****************************/
extern SPI_HandleTypeDef hspi1;
extern QueueHandle_t h_queue_spi;

/********************** external functions definition ************************/
/* Task thread */
void task_gatekeeper(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
    s_spi_msg_t s_msg;

    /* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    for (;;)
    {
        /* Wait for transmission request from the queue */
    	if (xQueueReceive(h_queue_spi, &s_msg, portMAX_DELAY) == pdPASS)
		{
			/* Chip Select (CS) to LOW on PA4 to enable the memory */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

			/* Reset the clock cycle counter just before starting TX */
			cycle_counter_reset();

			/* Start non-blocking transmission by DMA */
			HAL_SPI_Transmit_DMA(&hspi1, s_msg.p_data, s_msg.size);

			/* Block the task until the DMA ISR delivers the semaphore */
			xSemaphoreTake(h_sem_spi_tx, portMAX_DELAY);

			/* Calculate elapsed time in microseconds (us) */
			g_t_tx_us = cycle_counter_get_time_us();

			/* Chip Select (CS) to HIGH on PA4 to disable the memory */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

			/* Update the WCET if the current time is greater */
			if (g_t_tx_us > g_wcet_tx_us)
			{
				g_wcet_tx_us = g_t_tx_us;
			}

			LOGGER_INFO(p_task_gatekeeper_transmission_succesful);
		}
    }
}

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
const char *p_task_gatekeeper_transmission_succesful = "   ==> Gatekeeper: SPI transmission successful";

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

    /* Print out: Task Initialized */
	LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    s_spi_msg_t *p_msg_rcv;
    for (;;)
    {
        /* Wait for a message from any task in the transmission queue */
    	if (xQueueReceive(h_queue_spi_tx, &p_msg_rcv, portMAX_DELAY) == pdPASS)
		{
    		/* Activate Chip Select (CS on PA4 manually) */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

			/* Reset the clock cycle counter just before starting TX */
			cycle_counter_reset();

			/* Start transmission by interrupts */
			HAL_SPI_Transmit_IT(&hspi1, p_msg_rcv->p_data, (uint16_t)p_msg_rcv->size);

			/* Sleep the task until the SPI interrupt finishes and releases the semaphore */
			xSemaphoreTake(h_sem_spi_tx_end, portMAX_DELAY);

			/* Get the elapsed transmission time in microseconds */
			g_t_tx_us = cycle_counter_get_time_us();

			/* Deactivate Chip Select */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

			/* Update the transmission WCET */
			if (g_t_tx_us > g_wcet_tx_us)
			{
				g_wcet_tx_us = g_t_tx_us;
			}

			LOGGER_INFO(p_task_gatekeeper_transmission_succesful);

			/* Return the memory block to the Pool */
			xQueueSend(h_queue_spi_pool, &p_msg_rcv, 0ul);
		}
}
}

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
void task_gatekeeper(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
    s_spi_msg_t spi_msg;

    /* Print out: Task Initialized */
	LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    for (;;)
    {
        /* Wait for a message in the queue (Block indefinitely) */
        if (xQueueReceive(h_queue_spi, &spi_msg, portMAX_DELAY) == pdPASS)
        {
            /* Assert CS (Pull PA4 Low) */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

            /* Reset the clock cycle counter just before starting TX */
			cycle_counter_reset();

            /* Start DMA Transmission */
            HAL_SPI_Transmit_DMA(&hspi1, spi_msg.p_data, (uint16_t)spi_msg.size);

            /* Block waiting for DMA Tx Complete Semaphore */
            xSemaphoreTake(h_sem_spi_dma, portMAX_DELAY);

            /* Calculate elapsed time in microseconds (us) */
			g_t_tx_us = cycle_counter_get_time_us();

            /* De-assert CS (Pull PA4 High) */
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

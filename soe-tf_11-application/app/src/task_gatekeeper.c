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

/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
    s_spi_msg_t msg;

    /* Initialize Task Function variables */
    g_task_gatekeeper_cnt = G_TASK_GATEKEEPER_CNT_INI;

    /* Print out: Task Initialized */
    LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    /* Ensure Chip Select (PA4) starts High (Inactive) */
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

    for (;;)
    {
        /* Block until a request arrives in the queue */
        if (pdTRUE == xQueueReceive(h_queue_gatekeeper, &msg, portMAX_DELAY))
        {
            g_task_gatekeeper_cnt++;

            /* Assert CS (PA4 Low) */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

            /* Reset DWT cycle counter before SPI operation */
			cycle_counter_reset();

            /* Start Non-Blocking SPI Reception via Interrupt */
            if (HAL_OK == HAL_SPI_Receive_IT(&hspi1, msg.p_data, msg.size))
            {
                /* Wait for ISR to signal completion */
                if (pdTRUE == xSemaphoreTake(h_sem_spi_cplt, portMAX_DELAY))
                {
                    LOGGER_INFO(p_task_gatekeeper_rx_ok);
                }
            }
            else
            {
                LOGGER_INFO(p_task_gatekeeper_rx_err);
            }

            /* Calculate elapsed time in microseconds */
			g_t_tx_us = cycle_counter_get_time_us();

            /* De-assert CS (PA4 High) */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

            /* Update Worst-Case Execution Time (WCET) */
			if (g_t_tx_us > g_wcet_tx_us)
			{
				g_wcet_tx_us = g_t_tx_us;
			}
        }
    }
}

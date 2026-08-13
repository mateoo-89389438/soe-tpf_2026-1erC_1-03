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

/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
	/*  Declare & Initialize Task Function variables */
	g_task_gatekeeper_cnt = G_TASK_GATEKEEPER_CNT_INI;

    s_spi_msg_t rx_msg;

    /* Print out: Task Initialized */
	LOGGER_INFO(" ");
    LOGGER_INFO("  %s is running", pcTaskGetName(NULL));

    for (;;)
    {
    	/* Update Task Counter */
		g_task_gatekeeper_cnt++;

        /* Wait for a message in the Queue (Blocks indefinitely) */
        if (xQueueReceive(h_queue_spi, &rx_msg, portMAX_DELAY) == pdPASS)
        {
            /* Assert Chip Select (CS Low) on PA4 */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

            /* Reset cycle counter before starting transmission */
            cycle_counter_reset();

            /* Start Non-Blocking SPI Transmission via Interrupts */
            HAL_SPI_Transmit_IT(&hspi1, rx_msg.p_data, rx_msg.size);

            /* Block waiting for the Tx Complete Semaphore from the ISR */
            if (xSemaphoreTake(h_sem_spi, portMAX_DELAY) == pdPASS)
            {
                /* Read execution time in microseconds */
                g_t_tx_us = cycle_counter_get_time_us();

                /* De-assert Chip Select (CS High) on PA4 */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

                /* Update Worst-Case Execution Time (WCET) */
                if (g_t_tx_us > g_wcet_tx_us)
                {
                    g_wcet_tx_us = g_t_tx_us;
                }

                /* Transfer completed successfully */
                LOGGER_INFO(p_task_gatekeeper_tx_ok);
            }
            else
            {
                /* De-assert Chip Select (CS High) on PA4 in case of timeout */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

                LOGGER_INFO(p_task_gatekeeper_tx_error);
            }
        }
    }
}

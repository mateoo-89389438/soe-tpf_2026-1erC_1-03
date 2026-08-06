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
const char *p_task_gatekeeper_timeout = "   ==> Gatekeeper: SPI transmission successful";

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
static uint32_t g_t_tx_us = 0;
static uint32_t g_wcet_tx_us = 0;

/********************** external data declaration ****************************/
extern SPI_HandleTypeDef hspi1;
extern QueueHandle_t h_queue_spi;

/********************** external functions definition ************************/
void task_gatekeeper(void *parameters)
{
    s_spi_msg_t s_msg_received;

    LOGGER_INFO("  %s is running", pcTaskGetName(NULL));

    for (;;)
    {
        /* 1. Wait for a message in the Queue (Blocks indefinitely) */
        if (xQueueReceive(h_queue_spi_tx, &s_msg_received, portMAX_DELAY) == pdPASS)
        {
            /* 2. Assert Chip Select (CS Low) on PA4 */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

            /* Reset cycle counter before starting transmission */
            cycle_counter_reset();

            /* 3. Start Non-Blocking SPI Transmission via Interrupts */
            HAL_SPI_Transmit_IT(&hspi1, s_msg_received.p_data, s_msg_received.size);

            /* 4. Block waiting for the Tx Complete Semaphore from the ISR */
            if (xSemaphoreTake(h_sem_spi_tx, SPI_TIMEOUT_MS) == pdPASS)
            {
                /* Read execution time in microseconds */
                g_t_tx_us = cycle_counter_get_time_us();

                /* 5. De-assert Chip Select (CS High) on PA4 */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

                /* Update Worst-Case Execution Time (WCET) */
                if (g_t_tx_us > g_wcet_tx_us)
                {
                    g_wcet_tx_us = g_t_tx_us;
                }

                /* Transfer completed successfully */
                LOGGER_INFO(p_task_gatekeeper_transmission_succesful);
            }
            else
            {
                /* De-assert Chip Select (CS High) on PA4 in case of timeout */
                HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

                /* Handle Timeout Error */
                LOGGER_INFO(p_task_gatekeeper_timeout);
            }
        }
    }
}

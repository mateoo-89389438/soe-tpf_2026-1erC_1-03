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

/********************** internal data declaration ****************************/
const char *p_task_gatekeeper_transmission_succesful = "   ==> Gatekeeper: SPI transmission successful";

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
    s_spi_msg_t rx_msg;

    LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

    for (;;)
    {
        if (xQueueReceive(h_queue_spi, &rx_msg, portMAX_DELAY) == pdPASS)
        {
            /* Chip Select (CS) to LOW in PA4 */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);
            cycle_counter_reset();

            HAL_SPI_Transmit(&hspi1, rx_msg.p_data, rx_msg.size, HAL_MAX_DELAY);
            g_t_tx_us = cycle_counter_get_time_us();

            /* Chip Select (CS) to HIGH in PA4 */
            HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

            if (g_t_tx_us > g_wcet_tx_us) {
                g_wcet_tx_us = g_t_tx_us;
            }

            LOGGER_INFO(p_task_gatekeeper_transmission_succesful);
        }
    }
}

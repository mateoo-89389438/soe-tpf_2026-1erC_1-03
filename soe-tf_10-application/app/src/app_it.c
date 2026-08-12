/*
 * Copyright (c) 2026 Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its
 *    contributors may be used to endorse or promote products derived from
 *    this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 *
 * @author : Juan Manuel Cruz <jcruz@fi.uba.ar> <jcruz@frba.utn.edu.ar>
 */

/********************** inclusions *******************************************/
/* Project includes */
#include "main.h"
#include "cmsis_os.h"

/* Demo includes */
#include "logger.h"
#include "dwt.h"

/* Application & Tasks includes */
#include "board.h"
#include "app_it.h"
#include "app.h"
#include "task_gatekeeper.h"

/********************** macros and definitions *******************************/
#define G_RX_POOL_SIZE      3ul
#define G_RX_KNOWN_LENGTH   8ul

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
uint8_t g_rx_pool[G_RX_POOL_SIZE][G_RX_KNOWN_LENGTH];
uint32_t g_pool_index = 0ul;

static uint32_t g_t_tx_us = 0ul;
static uint32_t g_wcet_tx_us = 0ul;

/********************** external data declaration ****************************/
extern QueueHandle_t h_queue_spi_rx;

/********************** external functions definition ************************/
void app_it_init(void)
{
	/* Init to be done */

	/* Protect shared resource */
	__asm("CPSID i");	/* disable interrupts */

	__asm("CPSIE i");	/* enable interrupts */
}

/* SPI HAL Rx Complete Callback */
void HAL_SPI_RxCpltCallback(SPI_HandleTypeDef *hspi)
{
    if (hspi->Instance == SPI1)
    {
        BaseType_t x_higher_priority_task_woken = pdFALSE;
        s_spi_msg_t msg;

        /* Measure current execution time in microseconds since reset */
		g_t_tx_us = cycle_counter_get_time_us();

        /* Deassert Chip Select (PA4 -> HIGH) */
        HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

        /* Update Worst-Case Execution Time (WCET) */
		if (g_t_tx_us > g_wcet_tx_us)
		{
			g_wcet_tx_us = g_t_tx_us;
		}

        /* Prepare message pointing to current memory pool buffer */
        msg.p_data = g_rx_pool[g_pool_index];
        msg.size = G_RX_KNOWN_LENGTH;

        /* Update pool index safely for next reception */
        g_pool_index = (g_pool_index + 1ul) % G_RX_POOL_SIZE;

        /* Send message to Gatekeeper Task */
        xQueueSendFromISR(h_queue_spi_rx, &msg, &x_higher_priority_task_woken);

        /* Context switch if higher priority task was woken */
        portYIELD_FROM_ISR(x_higher_priority_task_woken);
    }
}
/********************** end of file ******************************************/

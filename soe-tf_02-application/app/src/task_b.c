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
#include "app.h"

/********************** macros and definitions *******************************/
#define G_TASK_B_CNT_INI	0ul

#define TASK_B_DEL_ZERO		(pdMS_TO_TICKS(0ul))
#define TASK_B_DEL_MAX		(pdMS_TO_TICKS(2500ul))

/********************** internal data declaration ****************************/

/********************** internal functions declaration ***********************/

/********************** internal data definition *****************************/
//const char *p_task_b_wait_250mS			= "   ==> Task    B - Wait:   250mS";

/********************** external data declaration ****************************/
uint32_t g_task_b_cnt;
extern SPI_HandleTypeDef hspi1;

uint32_t g_t_tx_us = 0ul;
uint32_t g_wcet_tx_us = 0ul;

/********************** external functions definition ************************/
/* Task thread */
void task_b(void *parameters)
{
	/* Declare & Initialize Task Function variables */
	g_task_b_cnt = G_TASK_B_CNT_INI;

	/* Print out: Task Initialized */
	LOGGER_INFO(" ");
	LOGGER_INFO("  %s is running - Tick [mS] = %lu", pcTaskGetName(NULL), xTaskGetTickCount());

	/* As per most tasks, this task is implemented in an infinite loop. */
	for (;;)
	{
		uint8_t *p_received_data = NULL;

		/* Task blocks here until a pointer to the queue arrives. */
		if (pdPASS == xQueueReceive(h_queue_spi, &p_received_data, portMAX_DELAY))
		{
			/* Update Task Counter */
			g_task_b_cnt++;

			LOGGER_INFO("  ==> Task B (Gatekeeper): Transmitting packet over SPI...");

			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_RESET);

			/* Reset the DWT cycle counter before transmitting */
			cycle_counter_reset();

			/* SPI transmission by Polling */
			HAL_SPI_Transmit(&hspi1, p_received_data, 4, HAL_MAX_DELAY);

			/* Get the elapsed time in us */
			g_t_tx_us = cycle_counter_get_time_us();

			/* CS to HIGH (Deselect memory) */
			HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);

			/* Update WCET */
			if (g_t_tx_us > g_wcet_tx_us)
			{
				g_wcet_tx_us = g_t_tx_us;
			}

			/* Free the memory requested by the task */
			vPortFree(p_received_data);
		}
	}
}

/********************** end of file ******************************************/

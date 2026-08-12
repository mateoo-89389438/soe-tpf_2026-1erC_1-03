#ifndef TASK_GATEKEEPER_H_
#define TASK_GATEKEEPER_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdint.h>
#include "main.h"
#include "cmsis_os.h"

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/
/* Declare a variable of type QueueHandle_t. This is used to reference queues*/
extern QueueHandle_t h_queue_spi_rx;

/* Declare a variable of type SemaphoreHandle_t (binary or counting) or mutex.
 * This is used to reference the semaphore that is used to synchronize a thread
 * with other thread or to ensure mutual exclusive access to...*/
extern SemaphoreHandle_t h_sem_spi_rx_cplt;

/* Declare a variable of type TaskHandle_t. This is used to reference threads. */
extern TaskHandle_t h_task_gatekeeper;

/********************** external functions declaration ***********************/
void task_gatekeeper(void *parameters);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_GATEKEEPER_H_ */

/********************** end of file ******************************************/



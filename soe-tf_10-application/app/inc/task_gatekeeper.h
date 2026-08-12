#ifndef TASK_GATEKEEPER_H_
#define TASK_GATEKEEPER_H_

/********************** CPP guard ********************************************/
#ifdef __cplusplus
extern "C" {
#endif

/********************** inclusions *******************************************/
#include <stdint.h>

/********************** macros ***********************************************/

/********************** typedef **********************************************/

/********************** external data declaration ****************************/


/* Declare a variable of type QueueHandle_t. This is used to reference queues*/

/* Declare a variable of type SemaphoreHandle_t (binary or counting) or mutex.
 * This is used to reference the semaphore that is used to synchronize a thread
 * with other thread or to ensure mutual exclusive access to...*/

/* Declare a variable of type TaskHandle_t. This is used to reference threads. */

/********************** external functions declaration ***********************/
void task_gatekeeper(void *parameters);

/********************** End of CPP guard *************************************/
#ifdef __cplusplus
}
#endif

#endif /* TASK_GATEKEEPER_H_ */

/********************** end of file ******************************************/



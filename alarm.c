#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "alarm.h"

/* read alarm from queue, if there is none simply return an empty alert with ->open = 0 */
alarm_t get_alarm(alarm_queue_t *queue)
{
    pthread_mutex_lock(&(queue->mutex));
    alarm_t alarm;
    if(queue->size == 0) {
        alarm.open = false;
        alarm.msg[0] = 0x00;
    } else {
        alarm = queue->queue[queue->out];
        queue->size--;
        queue->out++;
        queue->out %= 100;
    }
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->full));
    return alarm;
}

/* add an alarm to the queue */
void add_alarm(alarm_queue_t *queue, char msg[ALARM_MSG_MAXLEN])
{
    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == 100) {
        pthread_cond_wait(&(queue->full), &(queue->mutex));
    }
    alarm_t *alarm = &queue->queue[queue->in];
    alarm->open = true;
	alarm->num = queue->num;
    strncpy(alarm->msg, msg, ALARM_MSG_MAXLEN);
    queue->size++;
    queue->in++;
    queue->num++;
    queue->in %= ALARM_QUEUE_SIZE;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->empty));
}

/* set up thread safe FIFO queue for alarm structs */
alarm_queue_t *init_alarmqueue(void)
{
    alarm_queue_t *rv = calloc(1, sizeof(alarm_queue_t));
    assert(rv);
    return rv;
}

void destroy_alarmqueue(alarm_queue_t *queue)
{
    if(queue) {
        free(queue);
    }
}

#include <stdlib.h>
#include <pthread.h>
#include <string.h>
#include <assert.h>

#include "alarm.h"

/* read alarm from queue, if there is none simply return an empty alert with ->open = 0 */
alarm_t get_alarm(alarm_queue_t *queue)
{
    assert(queue);

    pthread_mutex_lock(&(queue->mutex));
    alarm_t alarm;
    if(queue->size == 0) {
        alarm.open = false;
        alarm.msg[0] = 0x00;
    } else {
        alarm = queue->queue[queue->out];
        queue->size--;
        queue->out++;
        queue->out %= ALARM_QUEUE_SIZE;
    }
    pthread_mutex_unlock(&(queue->mutex));
    return alarm;
}

/* add an alarm to the queue */
bool add_alarm(alarm_queue_t *queue, const char *name, const char *msg)
{
    assert(queue);
    assert(name);
    assert(msg);
    bool rv = false;

    pthread_mutex_lock(&(queue->mutex));
    if (queue->size < ALARM_QUEUE_SIZE) {
        alarm_t *alarm = &queue->queue[queue->in];
        alarm->open = true;
        alarm->num = queue->num;
        strncpy(alarm->name, name, ALARM_NAME_MAXLEN);
        strncpy(alarm->msg, msg, ALARM_MSG_MAXLEN);
        queue->size++;
        queue->in++;
        queue->num++;
        queue->in %= ALARM_QUEUE_SIZE;
        rv = true;
    }
    pthread_mutex_unlock(&(queue->mutex));
    return rv;
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
    free(queue);
}

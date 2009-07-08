#include <stdlib.h>
#include <pthread.h>
#include "alarm.h"
#include <string.h>

/* read alarm from queue, if there is none simply return an empty alert with ->open = 0 */
alarm_t get_alarm(alarm_queue_t *queue)
{
    pthread_mutex_lock(&(queue->mutex));
    alarm_t alarm;
    if(queue->size == 0) {
        alarm.open = 0;
        strncpy(alarm.msg, "", 255);
    } else {
        alarm = *queue->queue[queue->out];
        queue->size--;
        queue->out++;
        queue->out %= 100;
    }
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->full));
    return alarm;
}

/* add an alarm to the queue */
void add_alarm(alarm_queue_t *queue, int runonce, int level,
               int freq, int escfreq, char msg[256])
{
    pthread_mutex_lock(&(queue->mutex));
    while (queue->size == 100)
        pthread_cond_wait(&(queue->full), &(queue->mutex));
    alarm_t *alarm = queue->queue[queue->in];
    alarm->open = 1;
    alarm->runonce = runonce;
    alarm->level = level;
    alarm->levelmax = 5;
	alarm->num = queue->num;
    alarm->freq = freq;
    alarm->escfreq = escfreq;
    strncpy(alarm->msg, msg, 255);
    queue->size++;
    queue->in++;
    queue->num++;
    queue->in %= 100;
    pthread_mutex_unlock(&(queue->mutex));
    pthread_cond_broadcast(&(queue->empty));
}

/* set up thread safe FIFO queue for alarm structs */
void init_alarmqueue(alarm_queue_t *alarmqueue)
{
    alarm_t *alarmbuffer[100];
    /* init all 100 available alarms */
    int i;
    for (i = 0; i < 100; i++)
    {
        alarmbuffer[i] = malloc(sizeof(alarm_t));
    }
    alarmqueue->num = 0;
    alarmqueue->in = 0;
    alarmqueue->out = 0;
    alarmqueue->size = 0;
    /* add the buffer to the alarmqueue */
    memcpy(alarmqueue->queue, alarmbuffer, sizeof(alarmbuffer));
}

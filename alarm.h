#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#ifndef _ALARM_H
#define _ALARM_H

#define ALARM_INITIALIZER() { 1,1,1,1,0,0,"something"}
#define ALARM_QUEUE_INIT(queue) { queue, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, \
            PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER }

#define ALARM_QUEUE_SIZE 100
#define ALARM_MSG_MAXLEN 255

/*! \mainpage conflate-alarm
 * \section intro_sec Introduction
 *
 * conflate-alarm is a FIFO queue for passing alarms between threads. Alarms
 * can be one shot alerts, or require acknowledgement and escalate over time.
 */

/**
 * \defgroup Structs Queue and Alarm Structures
 * \defgroup Functions Queue management functions
 */

/**
 * \addtogroup Structs
 * @{
 */

/**
 * Alarm stucture for storage of a single alarm's parameters
 */
typedef struct alarm_s
{
    /**
     * true for an active alarm, false when inactive
     */
    bool open;
    /**
     * effectively the alarm id and running tally of alarms
     */
    int num;
    /**
     * alarm message
     */
    char msg[ALARM_MSG_MAXLEN + 1];
} alarm_t;

/**
 * FIFO queue for alarms
 */
typedef struct alarm_queue_s
{
    /**
     * array of alarms, prepopulated by mallocing 100 alarm_t structs
     */
    /** \private */
    alarm_t queue[ALARM_QUEUE_SIZE];
    /** \private */
    int in;
    /** \private */
    int out;
    /**
     * number of uncollected alarms
     */
    int size;
    /**
     * running tally of alarms
     */
    int num;
    /**
     * thread locking mutex to use whenever reading/wrintg to this structure
     */
    pthread_mutex_t mutex;
    /**
     * thread condition indicating the queue is full. block on this before adding alert
     */
    pthread_cond_t full;
    /**
     * thread condition indicating the queue is empty. optionally lock on this
	 * before reading from queue
     */
    pthread_cond_t empty;
} alarm_queue_t;

/**
 * @}
 */

/**
 * \addtogroup Functions
 * @{
 */

/**
 * Get alarm from alarm queue. Will return alarm with alarm->open =
 * false if no alarms available.
 *
 * To block until alarm is available to pull, wait on queue->empty
 *
 * @param queue Alarm Queue to pull alert from.
 */
alarm_t get_alarm(alarm_queue_t *queue);

/**
 * Insert new alarm into queue.
 *
 * @param queue The queue to add to.
 * @param msg message of <=255 characters for alarm
 */
void add_alarm(alarm_queue_t *queue, const char *msg);

/**
 * Create and initialize an alarm queue.
 *
 * @return the new alarm queue
 */
alarm_queue_t *init_alarmqueue(void);

/**
 * Destroy an alarm queue.
 *
 * @param queue the alarm queue to tear down
 */
void destroy_alarmqueue(alarm_queue_t *queue);

/**
 * @}
 */
#endif

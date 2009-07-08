#include <pthread.h>
#include <stdio.h>

#ifndef _ALARM_H
#define _ALARM_H

#define ALARM_INITIALIZER() { 1,1,1,1,0,0,"something"}
#define ALARM_QUEUE_INIT(queue) { queue, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, \
            PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER }

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
     * 1 for an active alarm, 0 is closed
     */
    int open;
    /**
     * effectively the alarm id and running tally of alarms
     */
    int num;
    /**
     * 0 for an alarm that can escalate and send multiple notices, 1 otherwise
     */
    int runonce;
    /**
     * initial level
     */
    int level;
    /**
     * maximum level this alarm can escalate to
     */
    int levelmax;
    /**
     * number of seconds between sending alerts given runonce=0
     */
    long freq;
    /**
     * number of alerts sent between escalation
     */
    long escfreq;
    /**
     * alarm message
     */
    char msg[256];
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
    alarm_t *queue[100];
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
 * Get alarm from alarm queue. Will return alarm with alarm->open=0 if no alarms available.
 * To block until alarm is available to pull, wait on queue->empty
 *
 * @param queue Alarm Queue to pull alert from.
 */
alarm_t get_alarm(alarm_queue_t *queue);

/**
 * Insert new alarm into queue.
 *
 * @param queue The queue to add to.
 * @param runonce 0 for alarms that send multiple alerts and potentially escalate
 * @param level initial level of alarm
 * @param freq number of seconds between alerts given runonce=0
 * @param escfreq number of alerts before escalating alarm 1 level
 * @param msg message of <255 characters for alarm
 */
void add_alarm(alarm_queue_t *queue, int runonce, int level, int freq,
               int escfreq, char msg[]);

/**
 * Intializes an alarm_queue_t structure.
 *
 * @param queue Alarm queue to initialize.
 */
void init_alarmqueue(alarm_queue_t *queue);

/**
 * @}
 */
#endif

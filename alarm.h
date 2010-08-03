#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>

#ifndef LIBCONFLATE_ALARM_H
#define LIBCONFLATE_ALARM_H

#ifdef __cplusplus
extern "C" {
#endif

#define ALARM_INITIALIZER() { 1,1,1,1,0,0,"something"}
#define ALARM_QUEUE_INIT(queue) { queue, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, \
            PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER }

#define ALARM_QUEUE_SIZE 100
#define ALARM_MSG_MAXLEN 255
#define ALARM_NAME_MAXLEN 25

/* Deal with an ICC annoyance.  It tries hard to pretend to be GCC<
   but doesn't understand its attributes properly. */
#ifndef __libconflate_gcc_attribute__
#if defined (__ICC) || defined (__SUNPRO_C)
# define __libconflate_gcc_attribute__(x)
#else
# define __libconflate_gcc_attribute__ __attribute__
#endif
#endif

/**
 * \addtogroup Alarm
 * @{
 * conflate-alarm is a FIFO queue for passing alarms between threads.
 */

/**
 * Alarm stucture for storage of a single alarm's parameters
 */
typedef struct {
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
	/**
	 * alarm name
	 */
	char name[ALARM_NAME_MAXLEN + 1];
} alarm_t;

/**
 * FIFO queue for alarms
 */
typedef struct {
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
} alarm_queue_t;

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
 * @return true iff the alarm was enqueued
 */
bool add_alarm(alarm_queue_t *queue, const char *name, const char *msg)
    __libconflate_gcc_attribute__ ((warn_unused_result));

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

#ifdef __cplusplus
}
#endif

/**
 * @}
 */
#endif

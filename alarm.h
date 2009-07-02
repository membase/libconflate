#include <pthread.h>
#include <stdio.h>

#ifndef _ALARM_H
#define _ALARM_H

//#define ALARM_QUEUE_INITIALIZER {  0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER  }
#define ALARM_INITIALIZER() { 1,1,1,1,0,0,"something"}
#define ALARM_QUEUE_INIT(queue) {queue, 0, 0, 0, PTHREAD_MUTEX_INITIALIZER, PTHREAD_COND_INITIALIZER, PTHREAD_COND_INITIALIZER}

typedef struct alarm_s
{
	int open;
	int runonce;
	int level;
	int levelmax;
	long freq;
	long escfreq;
	char msg[256];
} alarm_t;

//typedef struct alarm_s alarm_t;

typedef struct alarm_queue_s
{
	alarm_t *queue[100];
	int in;
	int out;
	int size;
	pthread_mutex_t mutex;
	pthread_cond_t full;
	pthread_cond_t empty;
} alarm_queue_t;

//typedef struct alarm_queue_s alarm_queue_t;
#endif

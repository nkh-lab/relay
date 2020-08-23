#include "scheduler.h"
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

static SCHEDULER_rutine_T m_routine; // for now just one for RELAY client
static pthread_t m_ptid;

void SCHEDULER_add(SCHEDULER_rutine_T routine)
{
    m_routine = routine;
}

void SCHEDULER_run(void)
{
    pthread_create(&m_ptid, NULL, scheduler, NULL);
}

void SCHEDULER_wait(void)
{
    pthread_join(m_ptid, NULL);
}

void* scheduler(void* arg)
{
    (void)arg;

    for (;;)
    {
        if (m_routine() == SCHEDULER_NOTHING_TODO) break;

        fflush(stdout);
        struct timespec ts = {0, 100000000}; // 100 ms
        nanosleep(&ts, &ts);
    }

    return NULL;
}

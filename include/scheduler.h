#pragma once

// Simple Scheduler

typedef enum SCHEDULER_routine_state_ENUM
{
    SCHEDULER_NOTHING_TODO,
    SCHEDULER_ACTIVE
} SCHEDULER_routine_state_E;
typedef SCHEDULER_routine_state_E (*SCHEDULER_rutine_T)(void);

void SCHEDULER_add(SCHEDULER_rutine_T routine);
void SCHEDULER_run(void);
void SCHEDULER_wait(void);
void* scheduler(void* arg);

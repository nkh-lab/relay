#pragma once

// Please configure file to applied system and board

// Current project utility headers
#include "scheduler.h"
#include "types.h"

#include "mdl_clock.h"
#include "mdl_di.h"
#include "mdl_do.h"

#if __linux__
#include <pthread.h>
#include <stdio.h>
#include <time.h>

// Logging
#define LOG(...) printf(__VA_ARGS__), printf("\n")

// Syncronization
#define LOCK_DEFINE static pthread_mutex_t lock // module scope
#define LOCK_INIT pthread_mutex_init(&lock, NULL)
#define LOCK pthread_mutex_lock(&lock)
#define UNLOCK pthread_mutex_unlock(&lock)

#else
#define LOG(...)
#endif

#ifndef MAX_SUPPORTED_RELAYS_NUMBER
#define MAX_SUPPORTED_RELAYS_NUMBER 4u
#endif

#ifndef MAX_STATE_LISTENERS_PER_RELAY
#define MAX_STATE_LISTENERS_PER_RELAY 1u
#endif

#ifndef MAX_ERROR_LISTENERS_PER_RELAY
#define MAX_ERROR_LISTENERS_PER_RELAY 1u
#endif

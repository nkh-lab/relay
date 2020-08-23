#pragma once

#include "mdl_relay_config.h"

typedef enum RELAY_type_ENUM
{
    RELAY_type_NO = 0U, // Normaly Open
    RELAY_type_NC = 1U, // Normaly Closed
} RELAY_type_E;

typedef enum RELAY_state_ENUM
{
    RELAY_state_NOT_INIT = 0U,
    RELAY_state_OPEN,
    RELAY_state_CLOSE,
} RELAY_state_E;

typedef enum RELAY_error_ENUM
{
    RELAY_error_NO = 0U,
    RELAY_error_WELDED,
    RELAY_error_CONSTANTLY_OPEN,
} RELAY_error_E;

typedef struct RELAY_config
{
    RELAY_type_E type;
    DO_index_E control_index;
    DI_index_E feedback_index;
    uint32_t response_ms; // relay responce time
} RELAY_config_T;

enum {RELAY_WO_FEEDBACK = DI_index_NUMBER}; // relay without feedback line

typedef uint32_t RELAY_listener_id_T;
typedef void (*RELAY_state_listener_func_T)(uint32_t relay_id, RELAY_state_E state);
typedef void (*RELAY_error_listener_func_T)(uint32_t relay_id, RELAY_error_E error);

bool RELAY_init(RELAY_config_T* config, uint32_t relays_number);
bool RELAY_is_inited();
void RELAY_deinit();

SCHEDULER_routine_state_E RELAY_routine(void);

bool RELAY_open(uint32_t relay_id);
bool RELAY_close(uint32_t relay_id);

RELAY_state_E RELAY_get_state(uint32_t relay_id);
RELAY_error_E RELAY_get_error(uint32_t relay_id);

bool RELAY_add_state_listener(
    uint32_t relay_id,
    RELAY_state_listener_func_T func,
    RELAY_listener_id_T* listener_id);

bool RELAY_add_error_listener(
    uint32_t relay_id,
    RELAY_error_listener_func_T func,
    RELAY_listener_id_T* listener_id);

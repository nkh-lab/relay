#pragma once

#include "mdl_relay.h"

typedef enum SIMU_mode_ENUM
{
    SIMU_mode_CORRECT,
    SIMU_mode_WRONG
} SIMU_mode_E;

void SIMU_init(SIMU_mode_E mode, RELAY_config_T* config, uint32_t relays_number);

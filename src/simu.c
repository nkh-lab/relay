#include "simu.h"

#include "mdl_clock.h"
#include "mdl_di.h"
#include "mdl_do.h"

static DI_state_E SIMU_inputs[DI_index_NUMBER];
static SIMU_mode_E m_mode;
static RELAY_config_T* m_config;
static uint32_t m_relays_number;

void SIMU_init(SIMU_mode_E mode, RELAY_config_T* config, uint32_t relays_number)
{
    LOG("%s(mode: %s)",
        __PRETTY_FUNCTION__,
        mode == SIMU_mode_CORRECT ? "SIMU_mode_CORRECT" : "SIMU_mode_WRONG");

    m_mode = mode;
    m_config = config;
    m_relays_number = relays_number;

    // Init feedback lines
    for (uint32_t i = 0; i < m_relays_number; ++i)
    {
        DI_state_E state;

        if (config[i].feedback_index == RELAY_WO_FEEDBACK || config[i].feedback_index >= DI_index_NUMBER) break;

        if (mode == SIMU_mode_CORRECT)
        {
            state = config[i].type == RELAY_type_NO ? DI_state_OFF : DI_state_ON;
        }
        else
        {
            state = config[i].type == RELAY_type_NO ? DI_state_ON : DI_state_OFF;
        }

        SIMU_inputs[config[i].feedback_index] = state;
    }
}

DI_state_E DI_getInputState(DI_index_E index)
{
    return SIMU_inputs[index];
}

void DO_setOutputState(DO_index_E index, DO_state_E state)
{
    for (uint32_t i = 0; i < m_relays_number; ++i)
    {
        if (m_config[i].feedback_index == RELAY_WO_FEEDBACK || m_config[i].feedback_index >= DI_index_NUMBER) break;

        if (m_config[i].control_index == index)
        {
            DI_state_E di_state;

            if (m_mode == SIMU_mode_CORRECT)
            {
                if (m_config[i].type == RELAY_type_NO)
                {
                    di_state = state == DO_state_ON ? DI_state_ON : DI_state_OFF;
                }
                else
                    di_state = state == DO_state_ON ? DI_state_OFF : DI_state_ON;
            }
            else
            {
                if (m_config[i].type == RELAY_type_NO)
                {
                    di_state = state == DO_state_ON ? DI_state_OFF : DI_state_ON;
                }
                else
                    di_state = state == DO_state_ON ? DI_state_ON : DI_state_OFF;
            }

            SIMU_inputs[index] = di_state;
        }
    }
}

CLOCK_ticks_T CLOCK_getTicks()
{
    static CLOCK_ticks_T ticks = 0;

    ++ticks;

    return ticks;
}

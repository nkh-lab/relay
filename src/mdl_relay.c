#include "mdl_relay.h"

//
// Module types
//

typedef enum event_ENUM
{
    event_OPEN,
    event_CLOSE,
    event_SELF_CHECK,
    event_DEINIT,
} event_E;

typedef enum sm_state_ENUM
{
    sm_state_NOT_INIT,
    sm_state_OPEN,
    sm_state_OPEN_TO_CLOSE,
    sm_state_CLOSE,
    sm_state_CLOSE_TO_OPEN,
    sm_state_ERROR_CONST_OPEN,
    sm_state_ERROR_WELDED,
    sm_state_DEINIT,
} sm_state_E;

typedef enum sm_state_ret_ENUM
{
    sm_state_ret_NO_TRANSITION,
    sm_state_ret_OK,
    sm_state_ret_NOK,
    sm_state_ret_DEINIT
} sm_state_ret_E;

typedef struct state_listeners
{
    RELAY_state_listener_func_T funcs[MAX_STATE_LISTENERS_PER_RELAY];
    uint32_t number;
} state_listeners_T;

typedef struct error_listeners
{
    RELAY_error_listener_func_T funcs[MAX_ERROR_LISTENERS_PER_RELAY];
    uint32_t number;
} error_listeners_T;

typedef struct relay
{
    sm_state_E sm_state;
    CLOCK_ticks_T start_switch_time;

    bool fire_state;
    bool fire_error;

    state_listeners_T state_listeners;
    error_listeners_T error_listeners;
} relay_T;

typedef sm_state_ret_E (*state_func_T)(uint32_t relay_id, event_E event);

typedef struct transition
{
    sm_state_E src_state;
    sm_state_ret_E ret_code;
    sm_state_E dst_state;
} transition_T;

//
// Module functions prototypes
//

static void log_config(void);
static void notify_error_listeners(uint32_t relay_id, RELAY_error_E error);
static void notify_state_listeners(uint32_t relay_id, RELAY_state_E state);

static bool is_closed(uint32_t relay_id);
static void close(uint32_t relay_id);
static void open(uint32_t relay_id);

static void init_state_machine(uint32_t relay_id);
static void step_state_machine(uint32_t relay_id, event_E event);
static sm_state_E do_transition(sm_state_E cur_state, sm_state_ret_E state_ret);

static sm_state_ret_E not_init_state(uint32_t relay_id, event_E event);
static sm_state_ret_E open_state(uint32_t relay_id, event_E event);
static sm_state_ret_E open_to_close_state(uint32_t relay_id, event_E event);
static sm_state_ret_E close_state(uint32_t relay_id, event_E event);
static sm_state_ret_E close_to_open_state(uint32_t relay_id, event_E event);
static sm_state_ret_E error_const_open_state(uint32_t relay_id, event_E event);
static sm_state_ret_E error_welded_state(uint32_t relay_id, event_E event);
static sm_state_ret_E deinit_state(uint32_t relay_id, event_E event);

//
// Module variables
//

LOCK_DEFINE;
static bool m_inited = false;
static RELAY_config_T* m_config;
static uint32_t m_relays_number;
static relay_T m_relays[MAX_SUPPORTED_RELAYS_NUMBER];

// should mirror sm_state_ENUM
static state_func_T m_state_funcs[] = {
                                       not_init_state,
                                       open_state,
                                       open_to_close_state,
                                       close_state,
                                       close_to_open_state,
                                       error_const_open_state,
                                       error_welded_state,
                                       deinit_state
                                       };

static transition_T m_state_transitions[] = {

    {sm_state_OPEN, sm_state_ret_NO_TRANSITION, sm_state_OPEN},
    {sm_state_OPEN, sm_state_ret_OK, sm_state_OPEN_TO_CLOSE},
    {sm_state_OPEN, sm_state_ret_NOK, sm_state_ERROR_WELDED},
    {sm_state_OPEN, sm_state_ret_DEINIT, sm_state_DEINIT},

    {sm_state_OPEN_TO_CLOSE, sm_state_ret_NO_TRANSITION, sm_state_OPEN_TO_CLOSE},
    {sm_state_OPEN_TO_CLOSE, sm_state_ret_OK, sm_state_CLOSE},
    {sm_state_OPEN_TO_CLOSE, sm_state_ret_NOK, sm_state_ERROR_CONST_OPEN},
    {sm_state_OPEN_TO_CLOSE, sm_state_ret_DEINIT, sm_state_DEINIT},

    {sm_state_CLOSE, sm_state_ret_NO_TRANSITION, sm_state_CLOSE},
    {sm_state_CLOSE, sm_state_ret_OK, sm_state_CLOSE_TO_OPEN},
    {sm_state_CLOSE, sm_state_ret_NOK, sm_state_ERROR_CONST_OPEN},
    {sm_state_CLOSE, sm_state_ret_DEINIT, sm_state_DEINIT},

    {sm_state_CLOSE_TO_OPEN, sm_state_ret_NO_TRANSITION, sm_state_CLOSE_TO_OPEN},
    {sm_state_CLOSE_TO_OPEN, sm_state_ret_OK, sm_state_OPEN},
    {sm_state_CLOSE_TO_OPEN, sm_state_ret_NOK, sm_state_ERROR_WELDED},
    {sm_state_CLOSE_TO_OPEN, sm_state_ret_DEINIT, sm_state_DEINIT},

    {sm_state_ERROR_CONST_OPEN, sm_state_ret_NO_TRANSITION, sm_state_ERROR_CONST_OPEN},
    {sm_state_ERROR_CONST_OPEN, sm_state_ret_DEINIT, sm_state_DEINIT},

    {sm_state_ERROR_WELDED, sm_state_ret_NO_TRANSITION, sm_state_ERROR_WELDED},
    {sm_state_ERROR_WELDED, sm_state_ret_DEINIT, sm_state_DEINIT},

    {sm_state_DEINIT, sm_state_ret_OK, sm_state_NOT_INIT}, //
};

static const size_t m_state_transitions_size = sizeof(m_state_transitions) / sizeof(transition_T);

//
// Functions implementation
//

bool RELAY_init(RELAY_config_T* config, uint32_t relays_number)
{
    LOG("%s()", __PRETTY_FUNCTION__);

    bool ret = false;

    LOCK_INIT;

    LOCK;
    if (!m_inited)
    {
        m_config = config;
        m_relays_number = relays_number;

        log_config();

        for (uint32_t i = 0; i < m_relays_number; ++i)
        {
            init_state_machine(i);
        }

        m_inited = true;
        ret = true;
    }
    UNLOCK;

    return ret;
}

bool RELAY_is_inited()
{
    LOG("%s()", __PRETTY_FUNCTION__);

    LOCK;
    bool inited = m_inited;
    UNLOCK;

    return inited;
}

void RELAY_deinit()
{
    LOG("%s()", __PRETTY_FUNCTION__);

    LOCK;
    if (m_inited)
    {
        for (uint32_t i = 0; i < m_relays_number; ++i)
        {
            for (;;)
            {
                step_state_machine(i, event_DEINIT);

                if (m_relays[i].sm_state == sm_state_NOT_INIT) break;
            }
        }
        m_inited = false;
    }
    UNLOCK;
}

SCHEDULER_routine_state_E RELAY_routine(void)
{
    SCHEDULER_routine_state_E ret = SCHEDULER_NOTHING_TODO;

    LOCK;
    if (m_inited)
    {
        for (uint32_t i = 0; i < m_relays_number; ++i)
        {
            step_state_machine(i, event_SELF_CHECK);
        }

        ret = SCHEDULER_ACTIVE;
    }
    UNLOCK;

    LOG("%s(): %d", __PRETTY_FUNCTION__, ret);

    return ret;
}

bool RELAY_open(uint32_t relay_id)
{
    bool ret = false;

    LOCK;
    if (m_inited)
    {
        step_state_machine(relay_id, event_OPEN);
        ret = true;
    }
    UNLOCK;

    LOG("%s(relay_id: %d): %d", __PRETTY_FUNCTION__, relay_id, ret);

    return ret;
}

bool RELAY_close(uint32_t relay_id)
{
    bool ret = false;

    LOCK;
    if (m_inited)
    {
        step_state_machine(relay_id, event_CLOSE);
        ret = true;
    }
    UNLOCK;

    LOG("%s(relay_id: %d): %d", __PRETTY_FUNCTION__, relay_id, ret);

    return ret;
}

RELAY_state_E RELAY_get_state(uint32_t relay_id)
{
    RELAY_state_E ret = RELAY_state_NOT_INIT;

    LOCK;
    if (m_inited)
    {
        switch (m_relays[relay_id].sm_state)
        {
        case sm_state_NOT_INIT:
        default:
            ret = RELAY_state_NOT_INIT;
            break;

        case sm_state_OPEN:
        case sm_state_OPEN_TO_CLOSE: // fine question, can be clarified
        case sm_state_ERROR_CONST_OPEN:
            ret = RELAY_state_OPEN;
            break;

        case sm_state_CLOSE:
        case sm_state_CLOSE_TO_OPEN: // fine question, can be clarified
        case sm_state_ERROR_WELDED:
            ret = RELAY_state_CLOSE;
            break;
        }
    }
    UNLOCK;

    LOG("%s(relay_id: %d): %d", __PRETTY_FUNCTION__, relay_id, ret);

    return ret;
}

RELAY_error_E RELAY_get_error(uint32_t relay_id)
{
    RELAY_error_E ret = RELAY_error_NO;

    LOCK;
    if (m_inited)
    {
        switch (m_relays[relay_id].sm_state)
        {
        case sm_state_NOT_INIT:
        case sm_state_OPEN:
        case sm_state_OPEN_TO_CLOSE:
        case sm_state_CLOSE:
        case sm_state_CLOSE_TO_OPEN:
        default:
            ret = RELAY_error_NO;
            break;

        case sm_state_ERROR_CONST_OPEN:
            ret = RELAY_error_CONSTANTLY_OPEN;
            break;

        case sm_state_ERROR_WELDED:
            ret = RELAY_error_WELDED;
            break;
        }
    }
    UNLOCK;

    LOG("%s(relay_id: %d): %d", __PRETTY_FUNCTION__, relay_id, ret);

    return ret;
}

bool RELAY_add_state_listener(uint32_t relay_id, RELAY_state_listener_func_T func, RELAY_listener_id_T* listener_id)
{
    bool ret = false;

    LOCK;
    if (m_inited)
    {
        uint32_t* n = &m_relays[relay_id].state_listeners.number;

        if (*n < MAX_STATE_LISTENERS_PER_RELAY)
        {
            m_relays[relay_id].state_listeners.funcs[*n] = func;
            *listener_id = *n;
            ++*n;
            ret = true;
        }
    }
    UNLOCK;

    LOG("%s(relay_id: %d, listener_id: %d): %d", __PRETTY_FUNCTION__, relay_id, *listener_id, ret);

    return ret;
}

bool RELAY_add_error_listener(uint32_t relay_id, RELAY_error_listener_func_T func, RELAY_listener_id_T* listener_id)
{
    bool ret = false;

    LOCK;
    if (m_inited)
    {
        uint32_t* n = &m_relays[relay_id].error_listeners.number;

        if (*n < MAX_ERROR_LISTENERS_PER_RELAY)
        {
            m_relays[relay_id].error_listeners.funcs[*n] = func;
            *listener_id = *n;
            ++*n;
            ret = true;
        }
    }
    UNLOCK;

    LOG("%s(relay_id: %d, listener_id: %d): %d", __PRETTY_FUNCTION__, relay_id, *listener_id, ret);

    return ret;
}

void init_state_machine(uint32_t relay_id)
{
    LOG("%s(relay_id: %d)", __PRETTY_FUNCTION__, relay_id);

    if (m_config[relay_id].type == RELAY_type_NO)
    {
        m_relays[relay_id].sm_state = sm_state_OPEN;
    }
    else
        m_relays[relay_id].sm_state = sm_state_CLOSE;
}

void step_state_machine(uint32_t relay_id, event_E event)
{
    sm_state_E cur_state = m_relays[relay_id].sm_state;

    sm_state_ret_E ret = m_state_funcs[cur_state](relay_id, event);

    m_relays[relay_id].sm_state = do_transition(cur_state, ret);
}

sm_state_ret_E not_init_state(uint32_t relay_id, event_E event)
{
    sm_state_ret_E ret = sm_state_ret_NO_TRANSITION;

    LOG("%s(relay_id: %d, event: %d): %d", __PRETTY_FUNCTION__, relay_id, event, ret);

    return ret;
}

sm_state_ret_E open_state(uint32_t relay_id, event_E event)
{
    sm_state_ret_E ret = sm_state_ret_NO_TRANSITION;

    if (m_relays[relay_id].fire_state)
    {
        m_relays[relay_id].fire_state = false;
        notify_state_listeners(relay_id, RELAY_state_OPEN);
    }

    switch (event)
    {
    case event_OPEN:
    default:
        ret = sm_state_ret_NO_TRANSITION;
        break;

    case event_CLOSE:
        close(relay_id);
        m_relays[relay_id].start_switch_time = CLOCK_getTicks();
        ret = sm_state_ret_OK;
        break;

    case event_SELF_CHECK:
        if (m_config[relay_id].feedback_index != RELAY_WO_FEEDBACK && is_closed(relay_id))
        {
            m_relays[relay_id].fire_error = true;
            ret = sm_state_ret_NOK;
        }
        break;

    case event_DEINIT:
        ret = sm_state_ret_DEINIT;
        break;
    }

    LOG("%s(relay_id: %d, event: %d): %d", __PRETTY_FUNCTION__, relay_id, event, ret);

    return ret;
}

sm_state_ret_E open_to_close_state(uint32_t relay_id, event_E event)
{
    sm_state_ret_E ret = sm_state_ret_NO_TRANSITION;

    if (event == event_DEINIT)
        ret = sm_state_ret_DEINIT;
    else
    {
        if (m_relays[relay_id].start_switch_time + m_config[relay_id].response_ms > CLOCK_getTicks())
        {
            ret = sm_state_ret_NO_TRANSITION; // still wait
        }
        else
        {
            if (m_config[relay_id].feedback_index == RELAY_WO_FEEDBACK)
            {
                m_relays[relay_id].fire_state = true;
                ret = sm_state_ret_OK;
            }
            else
            {
                if (is_closed(relay_id))
                {
                    m_relays[relay_id].fire_state = true;
                    ret = sm_state_ret_OK;
                }
                else
                {
                    m_relays[relay_id].fire_error = true;
                    ret = sm_state_ret_NOK;
                }
            }
        }
    }

    LOG("%s(relay_id: %d, event: %d): %d", __PRETTY_FUNCTION__, relay_id, event, ret);

    return ret;
}

sm_state_ret_E close_state(uint32_t relay_id, event_E event)
{
    sm_state_ret_E ret = sm_state_ret_NO_TRANSITION;

    if (m_relays[relay_id].fire_state)
    {
        m_relays[relay_id].fire_state = false;
        notify_state_listeners(relay_id, RELAY_state_CLOSE);
    }

    switch (event)
    {
    case event_OPEN:
        open(relay_id);
        m_relays[relay_id].start_switch_time = CLOCK_getTicks();
        ret = sm_state_ret_OK;
        break;

    case event_CLOSE:
    default:
        ret = sm_state_ret_NO_TRANSITION;
        break;

    case event_SELF_CHECK:
        if (m_config[relay_id].feedback_index != RELAY_WO_FEEDBACK && !is_closed(relay_id))
        {
            m_relays[relay_id].fire_error = true;
            ret = sm_state_ret_NOK;
        }
        break;

    case event_DEINIT:
        ret = sm_state_ret_DEINIT;
        break;
    }

    LOG("%s(relay_id: %d, event: %d): %d", __PRETTY_FUNCTION__, relay_id, event, ret);

    return ret;
}

sm_state_ret_E close_to_open_state(uint32_t relay_id, event_E event)
{
    sm_state_ret_E ret = sm_state_ret_NO_TRANSITION;

    if (event == event_DEINIT)
        ret = sm_state_ret_DEINIT;
    else
    {
        if (m_relays[relay_id].start_switch_time + m_config[relay_id].response_ms > CLOCK_getTicks())
        {
            ret = sm_state_ret_NO_TRANSITION; // still wait
        }
        else
        {
            if (m_config[relay_id].feedback_index == RELAY_WO_FEEDBACK)
            {
                m_relays[relay_id].fire_state = true;
                ret = sm_state_ret_OK;
            }
            else
            {
                if (!is_closed(relay_id))
                {
                    m_relays[relay_id].fire_state = true;
                    ret = sm_state_ret_OK;
                }
                else
                {
                    m_relays[relay_id].fire_error = true;
                    ret = sm_state_ret_NOK;
                }
            }
        }
    }

    LOG("%s(relay_id: %d, event: %d): %d", __PRETTY_FUNCTION__, relay_id, event, ret);

    return ret;
}

sm_state_ret_E error_const_open_state(uint32_t relay_id, event_E event)
{
    (void)event;

    sm_state_ret_E ret = sm_state_ret_NO_TRANSITION;

    if (m_relays[relay_id].fire_error)
    {
        m_relays[relay_id].fire_error = false;
        notify_error_listeners(relay_id, RELAY_error_CONSTANTLY_OPEN);
    }

    if (event == event_DEINIT) ret = sm_state_ret_DEINIT;

    LOG("%s(relay_id: %d, event: %d): %d", __PRETTY_FUNCTION__, relay_id, event, ret);

    return ret;
}

sm_state_ret_E error_welded_state(uint32_t relay_id, event_E event)
{
    (void)event;

    sm_state_ret_E ret = sm_state_ret_NO_TRANSITION;

    if (m_relays[relay_id].fire_error)
    {
        m_relays[relay_id].fire_error = false;
        notify_error_listeners(relay_id, RELAY_error_WELDED);
    }

    if (event == event_DEINIT) ret = sm_state_ret_DEINIT;

    LOG("%s(relay_id: %d, event: %d): %d", __PRETTY_FUNCTION__, relay_id, event, ret);

    return ret;
}

static sm_state_ret_E deinit_state(uint32_t relay_id, event_E event)
{
    sm_state_ret_E ret = sm_state_ret_OK;

    if(m_config[relay_id].type == RELAY_type_NO)
        open(relay_id);
    else close(relay_id);

    LOG("%s(relay_id: %d, event: %d): %d", __PRETTY_FUNCTION__, relay_id, event, ret);

    return ret;
}

sm_state_E do_transition(sm_state_E cur_state, sm_state_ret_E state_ret)
{
    for (size_t i = 0; i < m_state_transitions_size; ++i)
    {
        if (m_state_transitions[i].src_state == cur_state &&
            m_state_transitions[i].ret_code == state_ret)
        {
            return m_state_transitions[i].dst_state;
        }
    }

    return cur_state;
}

static void close(uint32_t relay_id)
{
    DO_state_E close_state = m_config[relay_id].type == RELAY_type_NO ? DO_state_ON : DO_state_OFF;

    DO_setOutputState(m_config[relay_id].control_index, close_state);
}

static void open(uint32_t relay_id)
{
    DO_state_E open_state = m_config[relay_id].type == RELAY_type_NO ? DO_state_OFF : DO_state_ON;

    DO_setOutputState(m_config[relay_id].control_index, open_state);
}

bool is_closed(uint32_t relay_id)
{
    return DI_getInputState(m_config[relay_id].feedback_index) == DI_state_ON ? true : false;
}

void notify_error_listeners(uint32_t relay_id, RELAY_error_E error)
{
    uint32_t* n = &m_relays[relay_id].error_listeners.number;

    for (uint32_t i = 0; i < *n; ++i)
    {
        m_relays[relay_id].error_listeners.funcs[i](relay_id, error);
    }
}

void notify_state_listeners(uint32_t relay_id, RELAY_state_E state)
{
    uint32_t* n = &m_relays[relay_id].state_listeners.number;

    for (uint32_t i = 0; i < *n; ++i)
    {
        m_relays[relay_id].state_listeners.funcs[i](relay_id, state);
    }
}

static void log_config(void)
{
    LOG("%s()", __PRETTY_FUNCTION__);

    if (m_config == NULL) return;

    for (uint32_t i = 0; i < m_relays_number; ++i)
    {
        RELAY_config_T* c = &m_config[i];

        LOG("Relay[%d] config:", i);
        LOG("  type: %s", c->type == RELAY_type_NO ? "NO" : "NC");
        LOG("  control_index: %d", c->control_index);
        LOG("  feedback_index: %d", c->feedback_index);
        LOG("  response_ms: %d", c->response_ms);
    }
}

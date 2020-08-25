#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include "mdl_relay.h"
#include "scheduler.h"
#include "simu.h"

// clang-format off
enum { RELAYS_NUMBER = 4U };
typedef enum test_return_ENUM { FAILED, PASSED } test_return_E;
enum { RESPONCE_5ms = 5U, RESPONCE_10ms = 10U };
enum { TIME_3s = 3U };
// clang-format on

static test_return_E not_init_test(void);
static test_return_E add_listeners_test(void);
static test_return_E get_state_test(RELAY_config_T* config);
static test_return_E get_error_test(void);
static test_return_E open_test(void);
static test_return_E close_test(void);

int main(int argc, char* argv[])
{
    printf("Hello World from Relay project\n");

    (void)argc;
    (void)argv;

    RELAY_config_T relays_config[RELAYS_NUMBER] = {
        {RELAY_type_NO, DO_index_00, DI_index_00, RESPONCE_10ms},
        {RELAY_type_NC, DO_index_01, DI_index_01, RESPONCE_5ms},
        {RELAY_type_NO, DO_index_02, RELAY_WO_FEEDBACK, RESPONCE_10ms},
        {RELAY_type_NC, DO_index_03, RELAY_WO_FEEDBACK, RESPONCE_5ms}
    };

    // Simulation settings
    //SIMU_init(SIMU_mode_WRONG, relays_config, RELAYS_NUMBER);
    SIMU_init(SIMU_mode_CORRECT, relays_config, RELAYS_NUMBER);

    // Test APIs befor init
    LOG(" ");
    LOG("  %s: not_init_test()", not_init_test() == PASSED ? "PASSED" : "FAILED");
    LOG(" ");

    RELAY_init(relays_config, RELAYS_NUMBER);

    LOG(" ");
    LOG("  %s: add_listeners_test()", add_listeners_test() == PASSED ? "PASSED" : "FAILED");
    LOG(" ");

    SCHEDULER_add(RELAY_routine);
    SCHEDULER_run();

    // After init tests
    LOG(" ");
    LOG("  %s: get_state_test()", get_state_test(relays_config) == PASSED ? "PASSED" : "FAILED");
    LOG(" ");
    LOG("  %s: get_error_test()", get_error_test() == PASSED ? "PASSED" : "FAILED");
    LOG(" ");

    sleep(TIME_3s);

    //
    // Run again init tests after delay, now they should be FAILED in SIMU_mode_WRONG when self
    // check is performed. In SIMU_mode_WRONG we expect on_error() notification.
    //
    LOG(" ");
    LOG("  %s: get_state_test()", get_state_test(relays_config) == PASSED ? "PASSED" : "FAILED");
    LOG(" ");
    LOG("  %s: get_error_test()", get_error_test() == PASSED ? "PASSED" : "FAILED");
    LOG(" ");

    //
    // Open and close tests, expect on_state() notifications
    //
    LOG(" ");
    LOG("  %s: open_test()", open_test() == PASSED ? "PASSED" : "FAILED");
    LOG(" ");
    sleep(TIME_3s); // to pass relay response time
    LOG(" ");
    LOG("  %s: close_test()", close_test() == PASSED ? "PASSED" : "FAILED");
    sleep(TIME_3s); // to pass relay response time
    LOG(" ");

    // All done
    RELAY_deinit();

    // Test APIs after deinit
    LOG(" ");
    LOG("  %s: not_init_test()", not_init_test() == PASSED ? "PASSED" : "FAILED");
    LOG(" ");

    SCHEDULER_wait();

    return 0;
}

void on_state_changed(uint32_t relay_id, RELAY_state_E state)
{
    LOG(" ");
    LOG("%s(relay_id: %d, state: %d)", __PRETTY_FUNCTION__, relay_id, state);
    LOG(" ");
}

void on_error(uint32_t relay_id, RELAY_error_E error)
{
    LOG(" ");
    LOG("%s(relay_id: %d, error: %d)", __PRETTY_FUNCTION__, relay_id, error);
    LOG(" ");
}

test_return_E not_init_test(void)
{
    LOG("%s()", __PRETTY_FUNCTION__);

    RELAY_listener_id_T state_listener_id = 0, error_listener_id = 0;

    if (RELAY_is_inited()) return FAILED;

    for (uint32_t i = 0; i < RELAYS_NUMBER; ++i)
    {
        if (RELAY_open(i)) return FAILED;
        if (RELAY_close(i)) return FAILED;
        if (RELAY_get_state(i) != RELAY_state_NOT_INIT) return FAILED;
        if (RELAY_add_state_listener(i, on_state_changed, &state_listener_id)) return FAILED;
        if (RELAY_add_error_listener(i, on_error, &error_listener_id)) return FAILED;
    }

    return PASSED;
}

test_return_E add_listeners_test(void)
{
    LOG("%s()", __PRETTY_FUNCTION__);

    RELAY_listener_id_T state_listener_id, error_listener_id;
    for (uint32_t i = 0; i < RELAYS_NUMBER; ++i)
    {
        for (uint32_t ii = 0; ii < MAX_STATE_LISTENERS_PER_RELAY; ++ii)
        {
            if (!RELAY_add_state_listener(i, on_state_changed, &state_listener_id)) return FAILED;
        }
        if (RELAY_add_state_listener(i, on_state_changed, &state_listener_id)) return FAILED;

        for (uint32_t ii = 0; ii < MAX_ERROR_LISTENERS_PER_RELAY; ++ii)
        {
            if (!RELAY_add_error_listener(i, on_error, &error_listener_id)) return FAILED;
        }
        if (RELAY_add_error_listener(i, on_error, &error_listener_id)) return FAILED;
    }

    return PASSED;
}

test_return_E get_state_test(RELAY_config_T* config)
{
    LOG("%s()", __PRETTY_FUNCTION__);

    for (uint32_t i = 0; i < RELAYS_NUMBER; ++i)
    {
        if (config[i].type == RELAY_type_NO)
        {
            if (RELAY_get_state(i) != RELAY_state_OPEN) return FAILED;
        }
        else
        {
            if (RELAY_get_state(i) != RELAY_state_CLOSE) return FAILED;
        }
    }

    return PASSED;
}

test_return_E get_error_test(void)
{
    LOG("%s()", __PRETTY_FUNCTION__);
    for (uint32_t i = 0; i < RELAYS_NUMBER; ++i)
    {
        if (RELAY_get_error(i) != RELAY_error_NO) return FAILED;
    }

    return PASSED;
}

test_return_E open_test(void)
{
    LOG("%s()", __PRETTY_FUNCTION__);
    for (uint32_t i = 0; i < RELAYS_NUMBER; ++i)
    {
        if (!RELAY_open(i)) return FAILED;
    }

    return PASSED;
}

test_return_E close_test(void)
{
    LOG("%s()", __PRETTY_FUNCTION__);
    for (uint32_t i = 0; i < RELAYS_NUMBER; ++i)
    {
        if (!RELAY_close(i)) return FAILED;
    }

    return PASSED;
}

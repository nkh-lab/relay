#pragma once

//! Digital Output (DO) indexes
typedef enum DO_index_ENUM
{
    DO_index_00 = 0u, //!< Index of DO pin 0
    DO_index_01 = 1u, //!< Index of DO pin 1
    DO_index_02 = 2u, //!< Index of DO pin 2
    DO_index_03 = 3u, //!< Index of DO pin 3
    DO_index_04 = 4u, //!< Index of DO pin 4
    DO_index_05 = 5u, //!< Index of DO pin 5
    DO_index_06 = 6u, //!< Index of DO pin 6
    DO_index_07 = 7u, //!< Index of DO pin 7
    DO_index_08 = 8u, //!< Index of DO pin 8
    DO_index_09 = 9u, //!< Index of DO pin 9
    DO_index_10 = 10u, //!< Index of DO pin 10
    DO_index_11 = 11u, //!< Index of DO pin 11
    DO_index_12 = 12u, //!< Index of DO pin 12
    DO_index_13 = 13u, //!< Index of DO pin 13
    DO_index_14 = 14u, //!< Index of DO pin 14
    DO_index_15 = 15u, //!< Index of DO pin 15
    DO_index_NUMBER = 16u //!< Number of DO indexes
} DO_index_E;

//! Digital Output (DO) states
typedef enum DO_state_ENUM
{
    DO_state_OFF = 0u, //!< Digital power output state OFF
    DO_state_ON = 1u, //!< Digital power output state ON
    DO_state_NUMBER = 2u //!< Number of digital output states
} DO_state_E;

/********************************************************************************************************
 * @details Function sets state on digital output.
 *********************************************************************************************************
 * @param [in] index - Digital power output pin index see ::DO_index_ENUM.
 * @param [in] state - Digital power output state see ::DO_state_ENUM.
 * @return Nothing.
 ********************************************************************************************************/
void DO_setOutputState(DO_index_E index, DO_state_E state);

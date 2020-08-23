#pragma once

//! Digital Input (DI) indexes
typedef enum DI_index_ENUM
{
    DI_index_00 = 0u, //!< Index of DI pin 0
    DI_index_01 = 1u, //!< Index of DI pin 1
    DI_index_02 = 2u, //!< Index of DI pin 2
    DI_index_03 = 3u, //!< Index of DI pin 3
    DI_index_04 = 4u, //!< Index of DI pin 4
    DI_index_05 = 5u, //!< Index of DI pin 5
    DI_index_06 = 6u, //!< Index of DI pin 6
    DI_index_07 = 7u, //!< Index of DI pin 7
    DI_index_08 = 8u, //!< Index of DI pin 8
    DI_index_09 = 9u, //!< Index of DI pin 9
    DI_index_10 = 10u, //!< Index of DI pin 10
    DI_index_11 = 11u, //!< Index of DI pin 11
    DI_index_12 = 12u, //!< Index of DI pin 12
    DI_index_13 = 13u, //!< Index of DI pin 13
    DI_index_14 = 14u, //!< Index of DI pin 14
    DI_index_15 = 15u, //!< Index of DI pin 15
    DI_index_NUMBER = 16u //!< Number of DI indexes
} DI_index_E;

//! Digital Input (DI) states
typedef enum DI_state_ENUM
{
    DI_state_OFF = 0u, //!< Digital input OFF
    DI_state_ON = 1u, //!< Digital input ON
    DI_state_NUMBER = 2u, //!< Number of digital input states
} DI_state_E;

/********************************************************************************************************
 * @details Function returns digital input state based on index value.
 *********************************************************************************************************
 * @param [in] index - Digital input index see ::DI_index_ENUM.
 * @return Digital input state see ::DI_state_ENUM.
 ********************************************************************************************************/
DI_state_E DI_getInputState(DI_index_E index);

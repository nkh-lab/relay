#include "types.h"

typedef uint32_t CLOCK_ticks_T;

/*********************************************************************************************************
 * @brief Retrieve elapsed miliseconds from power-up. Assume there will be no overflow.
 *********************************************************************************************************
 * @param [in] Nothing.
 * @return Number of ticks elapsed.
 ********************************************************************************************************/
CLOCK_ticks_T CLOCK_getTicks();

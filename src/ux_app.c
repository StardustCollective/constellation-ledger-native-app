/*
 * MIT License, see root folder for full license.
 */

#include "ux_app.h"

/** UI state flag */
#if defined(TARGET_NANOX) || defined(TARGET_NANOS2)
#include "ux.h"
ux_state_t G_ux;
bolos_ux_params_t G_ux_params;
#else // TARGET_NANOX || TARGET_NANOS2
ux_state_t ux;
#endif // TARGET_NANOS


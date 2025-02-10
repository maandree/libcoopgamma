/* See LICENSE file for copyright and license details. */
#include "common.h"


#if defined(__GNUC__)
const char *argv0 __attribute__((weak)) = "libcoopgamma";
#endif


extern inline char *libcoopgamma_next_header__(libcoopgamma_context_t *restrict ctx);
extern inline char *libcoopgamma_next_payload__(libcoopgamma_context_t *restrict ctx, size_t *n);

/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Tell the library that you will not be parsing a receive message
 * 
 * @param  ctx  The state of the library, must be connected
 */
void
libcoopgamma_skip_message(libcoopgamma_context_t *restrict ctx)
{
	size_t _n;
	while (*libcoopgamma_next_header__(ctx));
	(void) libcoopgamma_next_payload__(ctx, &_n);
}

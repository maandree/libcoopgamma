/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Apply, update, or remove a gamma ramp adjustment, receive response part
 * 
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int
libcoopgamma_set_gamma_recv(libcoopgamma_context_t *restrict ctx, libcoopgamma_async_context_t *restrict async)
{
	if (libcoopgamma_check_error__(ctx, async))
		return -(ctx->error.custom || ctx->error.number);

	while (*libcoopgamma_next_header__(ctx));
	(void) libcoopgamma_next_payload__(ctx, &(size_t){0});

	errno = EBADMSG;
	copy_errno(ctx);
	return -1;
}

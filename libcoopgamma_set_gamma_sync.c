/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Apply, update, or remove a gamma ramp adjustment, synchronous version
 * 
 * This is a synchronous request function, as such,
 * you have to ensure that communication is blocking
 * (default), and that there are not asynchronous
 * requests waiting, it also means that EINTR:s are
 * silently ignored and there no wait to cancel the
 * operation without disconnection from the server
 * 
 * @param   filter  The filter to apply, update, or remove, gamma ramp meta-data must match the CRTC's
 * @param   depth   The datatype for the stops in the gamma ramps, must match the CRTC's
 * @param   ctx     The state of the library, must be connected
 * @return          Zero on success, -1 on error, in which case `ctx->error`
 *                  (rather than `errno`) is read for information about the error
 */
int
libcoopgamma_set_gamma_sync(const libcoopgamma_filter_t *restrict filter, libcoopgamma_context_t *restrict ctx)
{
	SYNC_CALL(libcoopgamma_set_gamma_send(filter, ctx, &async),
	          libcoopgamma_set_gamma_recv(ctx, &async), (copy_errno(ctx), -1));
}

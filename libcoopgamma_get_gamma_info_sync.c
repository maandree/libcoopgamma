/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Retrieve information about a CRTC:s gamma ramps, synchronous version
 * 
 * This is a synchronous request function, as such,
 * you have to ensure that communication is blocking
 * (default), and that there are not asynchronous
 * requests waiting, it also means that EINTR:s are
 * silently ignored and there no wait to cancel the
 * operation without disconnection from the server
 * 
 * @param   crtc   The name of the CRTC
 * @param   info   Output parameter for the information, must be initialised
 * @param   ctx    The state of the library, must be connected
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int
libcoopgamma_get_gamma_info_sync(const char *restrict ctrc, libcoopgamma_crtc_info_t *restrict info,
                                 libcoopgamma_context_t *restrict ctx)
{
	SYNC_CALL(libcoopgamma_get_gamma_info_send(ctrc, ctx, &async),
	          libcoopgamma_get_gamma_info_recv(info, ctx, &async), (copy_errno(ctx), -1));
}

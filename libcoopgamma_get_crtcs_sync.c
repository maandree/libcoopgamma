/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * List all available CRTC:s, synchronous version
 * 
 * This is a synchronous request function, as such,
 * you have to ensure that communication is blocking
 * (default), and that there are not asynchronous
 * requests waiting, it also means that EINTR:s are
 * silently ignored and there no wait to cancel the
 * operation without disconnection from the server
 * 
 * @param   ctx  The state of the library, must be connected
 * @return       A `NULL`-terminated list of names. You should only free
 *               the outer pointer, inner pointers are subpointers of the
 *               outer pointer and cannot be freed. `NULL` on error, in
 *               which case `ctx->error` (rather than `errno`) is read
 *               for information about the error.
 */
char **
libcoopgamma_get_crtcs_sync(libcoopgamma_context_t *restrict ctx)
{
	SYNC_CALL(libcoopgamma_get_crtcs_send(ctx, &async),
	          libcoopgamma_get_crtcs_recv(ctx, &async), (copy_errno(ctx), NULL));
}

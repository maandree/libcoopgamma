/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Retrieve the current gamma ramp adjustments, synchronous version
 * 
 * This is a synchronous request function, as such,
 * you have to ensure that communication is blocking
 * (default), and that there are not asynchronous
 * requests waiting, it also means that EINTR:s are
 * silently ignored and there no wait to cancel the
 * operation without disconnection from the server
 * 
 * @param   query  The query to send
 * @param   table  Output for the response, must be initialised
 * @param   ctx    The state of the library, must be connected
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int
libcoopgamma_get_gamma_sync(const libcoopgamma_filter_query_t *restrict query, libcoopgamma_filter_table_t *restrict table,
                            libcoopgamma_context_t *restrict ctx)
{
	SYNC_CALL(libcoopgamma_get_gamma_send(query, ctx, &async),
	          libcoopgamma_get_gamma_recv(table, ctx, &async), (copy_errno(ctx), -1));
}

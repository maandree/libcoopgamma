/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * By default communication is blocking, this function
 * can be used to switch between blocking and nonblocking
 * 
 * After setting the communication to nonblocking,
 * `libcoopgamma_flush`, `libcoopgamma_synchronise` and
 * and request-sending functions can fail with EAGAIN and
 * EWOULDBLOCK. It is safe to continue with `libcoopgamma_flush`
 * (for `libcoopgamma_flush` it selfand equest-sending functions)
 * or `libcoopgamma_synchronise` just like EINTR failure.
 * 
 * @param   ctx          The state of the library, must be connected
 * @param   nonblocking  Nonblocking mode?
 * @return               Zero on success, -1 on error
 */
int
libcoopgamma_set_nonblocking(libcoopgamma_context_t *restrict ctx, int nonblocking)
{
	int flags = fcntl(ctx->fd, F_GETFL);
	if (flags == -1)
		return -1;
	if (nonblocking)
		flags |= O_NONBLOCK;
	else
		flags &= ~O_NONBLOCK;
	if (fcntl(ctx->fd, F_SETFL, flags) == -1)
		return -1;
	ctx->blocking = !nonblocking;
	return 0;
}

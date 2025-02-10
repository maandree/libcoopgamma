/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Send all pending outbound data
 * 
 * If this function or another function that sends a request
 * to the server fails with EINTR, call this function to
 * complete the transfer. The `async` parameter will always
 * be in a properly configured state if a function fails
 * with EINTR.
 * 
 * @param   ctx  The state of the library, must be connected
 * @return       Zero on success, -1 on error
 */
int
libcoopgamma_flush(libcoopgamma_context_t *restrict ctx)
{
	ssize_t sent;
	size_t chunksize = ctx->outbound_head - ctx->outbound_tail;
	size_t sendsize;

	while (ctx->outbound_tail < ctx->outbound_head) {
		sendsize = ctx->outbound_head - ctx->outbound_tail;
		sendsize = sendsize < chunksize ? sendsize : chunksize;
		sent = send(ctx->fd, &ctx->outbound[ctx->outbound_tail], sendsize, MSG_NOSIGNAL);
		if (sent < 0) {
			if (errno == EPIPE)
				errno = ECONNRESET;
			if (errno != EMSGSIZE)
				return -1;
			if (!(chunksize >>= 1))
				return -1;
			continue;
		}

#ifdef DEBUG_MODE
		fprintf(stderr, "\033[31m");
		fwrite(&ctx->outbound[ctx->outbound_tail], (size_t)sent, 1U, stderr);
		fprintf(stderr, "\033[m");
		fflush(stderr);
#endif

		ctx->outbound_tail += (size_t)sent;
	}

	return 0;
}

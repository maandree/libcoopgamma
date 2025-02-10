/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Send a message to the server and wait for response
 * 
 * @param   ctx  The state of the library
 * @param   msg  The message to send
 * @param   n    The length of `msg`
 * @return       Zero on success, -1 on error
 */
int
libcoopgamma_send_message__(libcoopgamma_context_t *restrict ctx, char *msg, size_t n)
{
	void *new;
	if (ctx->outbound_head == ctx->outbound_tail) {
		free(ctx->outbound);
		ctx->outbound = msg;
		ctx->outbound_tail = 0;
		ctx->outbound_head = n;
		ctx->outbound_size = n;
	} else {
		if (ctx->outbound_head + n > ctx->outbound_size) {
			memmove(ctx->outbound, ctx->outbound + ctx->outbound_tail, ctx->outbound_head -= ctx->outbound_tail);
			ctx->outbound_tail = 0;
		}
		if (ctx->outbound_head + n > ctx->outbound_size) {
			new = realloc(ctx->outbound, ctx->outbound_head + n);
			if (!new) {
				free(msg);
				return -1;
			}
			ctx->outbound = new;
			ctx->outbound_size = ctx->outbound_head + n;
		}
		memcpy(ctx->outbound + ctx->outbound_head, msg, n);
		ctx->outbound_head += n;
		free(msg);
	}
	ctx->message_id += 1;
	return libcoopgamma_flush(ctx);
}

/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Wait for the next message to be received
 * 
 * @param   ctx       The state of the library, must be connected
 * @param   pending   Information for each pending request
 * @param   n         The number of elements in `pending`
 * @param   selected  The index of the element in `pending` which corresponds
 *                    to the first inbound message, note that this only means
 *                    that the message is not for any of the other request,
 *                    if the message is corrupt any of the listed requests can
 *                    be selected even if it is not for any of the requests.
 *                    Functions that parse the message will detect such corruption.
 * @return            Zero on success, -1 on error. If the the message is ignored,
 *                    which happens if corresponding `libcoopgamma_async_context_t`
 *                    is not listed, -1 is returned and `errno` is set to 0. If -1
 *                    is returned, `errno` is set to `ENOTRECOVERABLE` you have
 *                    received a corrupt message and the context has been tainted
 *                    beyond recover.
 */
int
libcoopgamma_synchronise(libcoopgamma_context_t *restrict ctx, libcoopgamma_async_context_t *restrict pending,
                         size_t n, size_t *restrict selected)
{
	char temp[3 * sizeof(size_t) + 1];
	ssize_t got;
	size_t i;
	char *p;
	char *line;
	char *value;
	struct pollfd pollfd;
	size_t new_size;
	void *new;

	if (ctx->inbound_head == ctx->inbound_tail) {
		ctx->inbound_head = ctx->inbound_tail = ctx->curline = 0;
	} else if (ctx->inbound_tail > 0) {
		memmove(ctx->inbound, ctx->inbound + ctx->inbound_tail, ctx->inbound_head -= ctx->inbound_tail);
		ctx->curline -= ctx->inbound_tail;
		ctx->inbound_tail = 0;
	}

	pollfd.fd = ctx->fd;
	pollfd.events = POLLIN | POLLRDNORM | POLLRDBAND | POLLPRI;

	if (ctx->inbound_head)
		goto skip_recv;
	for (;;) {
		if (ctx->inbound_head == ctx->inbound_size) {
			new_size = ctx->inbound_size ? (ctx->inbound_size << 1) : 1024U;
			new = realloc(ctx->inbound, new_size);
			if (!new)
				return -1;
			ctx->inbound = new;
			ctx->inbound_size = new_size;
		}

		if (ctx->blocking) {
			pollfd.revents = 0;
			if (poll(&pollfd, (nfds_t)1, -1) < 0)
				return -1;
		}
		got = recv(ctx->fd, &ctx->inbound[ctx->inbound_head], ctx->inbound_size - ctx->inbound_head, 0);
		if (got <= 0) {
			if (got == 0)
				errno = ECONNRESET;
			return -1;
		}

#ifdef DEBUG_MODE
		fprintf(stderr, "\033[32m");
		fwrite(&ctx->inbound[ctx->inbound_head], (size_t)got, 1U, stderr);
		fprintf(stderr, "\033[m");
		fflush(stderr);
#endif

		ctx->inbound_head += (size_t)got;

	skip_recv:
		while (!ctx->have_all_headers) {
			line = &ctx->inbound[ctx->curline];
			p = memchr(line, '\n', ctx->inbound_head - ctx->curline);
			if (!p)
				break;
			if (memchr(line, '\0', (size_t)(p - line)) != NULL)
				ctx->bad_message = 1;
			*p++ = '\0';
			ctx->curline = (size_t)(p - ctx->inbound);
			if (!*line) {
				ctx->have_all_headers = 1;
			} else if (strstr(line, "In response to: ") == line) {
				value = &line[sizeof("In response to: ") - 1U];
				ctx->in_response_to = (uint32_t)atol(value);
			} else if (strstr(line, "Length: ") == line) {
				value = &line[sizeof("Length: ") - 1U];
				ctx->length = (size_t)atol(value);
				sprintf(temp, "%zu", ctx->length);
				if (strcmp(value, temp))
					goto fatal;
			}
		}

		if (ctx->have_all_headers && ctx->inbound_head >= ctx->curline + ctx->length) {
			ctx->curline += ctx->length;
			if (ctx->bad_message) {
				ctx->bad_message = 0;
				ctx->have_all_headers = 0;
				ctx->length = 0;
				ctx->inbound_tail = ctx->curline;
				errno = EBADMSG;
				return -1;
			}
			for (i = 0; i < n; i++) {
				if (pending[i].message_id == ctx->in_response_to) {
					*selected = i;
					return 0;
				}
			}
			*selected = 0;
			ctx->bad_message = 0;
			ctx->have_all_headers = 0;
			ctx->length = 0;
			ctx->inbound_tail = ctx->curline;
			errno = 0;
			return -1;
		}
	}

fatal:
	errno = ENOTRECOVERABLE;
	return -1;
}

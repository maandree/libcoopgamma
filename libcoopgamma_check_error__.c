/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Check whether the server sent an error, if so copy it to `ctx`
 * 
 * This function will also reports EBADMSG if the message ID
 * that the message is a response to does not match the request
 * information, or if it is missing
 * 
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         1 if the server sent an error (even indicating success),
 *                 0 on success, -1 on failure. Information about failure
 *                 is copied `ctx`.
 */
int
libcoopgamma_check_error__(libcoopgamma_context_t *restrict ctx, libcoopgamma_async_context_t *restrict async)
{
	char temp[3 * sizeof(uint64_t) + 1];
	size_t old_tail = ctx->inbound_tail;
	char *line;
	char *value;
	int command_ok = 0;
	int have_in_response_to = 0;
	int have_error = 0;
	int bad = 0;
	char *payload;
	size_t n;

	for (;;) {
		line = libcoopgamma_next_header__(ctx);
		value = &strchr(line, ':')[2U];
		if (!*line) {
			break;
		} else if (!strcmp(line, "Command: error")) {
			command_ok = 1;
		} else if (strstr(line, "In response to: ") == line) {
			uint32_t id = (uint32_t)atol(value);
			have_in_response_to = 1 + !!have_in_response_to;
			if (id != async->message_id) {
				bad = 1;
			} else {
				sprintf(temp, "%" PRIu32, id);
				if (strcmp(value, temp))
					bad = 1;
			}
		} else if (strstr(line, "Error: ") == line) {
			have_error = 1 + !!have_error;
			ctx->error.server_side = 1;
			ctx->error.custom = (strstr(value, "custom") == value);
			if (ctx->error.custom) {
				if (value[6] == '\0') {
					ctx->error.number = 0;
					continue;
				} else if (value[6] != ' ') {
					bad = 1;
					continue;
				}
				value += 7;
			}
			ctx->error.number = (uint64_t)atoll(value);
			sprintf(temp, "%" PRIu64, ctx->error.number);
			if (strcmp(value, temp))
				bad = 1;
		}
	}

	if (!command_ok) {
		ctx->inbound_tail = old_tail;
		return 0;
	}

	payload = libcoopgamma_next_payload__(ctx, &n);
	if (payload) {
		if (memchr(payload, '\0', n) || payload[n - 1U] != '\n')
			goto badmsg;
		ctx->error.description = malloc(n);
		if (ctx->error.description == NULL)
			goto fail;
		memcpy(ctx->error.description, payload, n - 1U);
		ctx->error.description[n - 1U] = '\0';
	}

	if (bad || have_in_response_to != 1 || have_error != 1)
		goto badmsg;

	return 1;

badmsg:
	errno = EBADMSG;
fail:
	copy_errno(ctx);
	return -1;
}

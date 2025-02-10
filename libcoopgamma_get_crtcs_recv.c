/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * List all available CRTC:s, receive response part
 * 
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         A `NULL`-terminated list of names. You should only free
 *                 the outer pointer, inner pointers are subpointers of the
 *                 outer pointer and cannot be freed. `NULL` on error, in
 *                 which case `ctx->error` (rather than `errno`) is read
 *                 for information about the error.
 */
char **
libcoopgamma_get_crtcs_recv(libcoopgamma_context_t *restrict ctx, libcoopgamma_async_context_t *restrict async)
{
	char *line;
	char *payload;
	char *end;
	int command_ok = 0;
	size_t i, n, lines, len, length;
	char **rc;

	if (libcoopgamma_check_error__(ctx, async))
		return NULL;

	for (;;) {
		line = libcoopgamma_next_header__(ctx);
		if (!*line)
			break;
		else if (!strcmp(line, "Command: crtc-enumeration"))
			command_ok = 1;
	}

	payload = libcoopgamma_next_payload__(ctx, &n);

	if (!command_ok || (n > 0 && payload[n - 1U] != '\n')) {
		errno = EBADMSG;
		copy_errno(ctx);
		return NULL;
	}

	line = payload;
	end = &payload[n];
	lines = length = 0;
	while (line != end) {
		lines += 1U;
		length += len = (size_t)(&strchr(line, '\n')[1U] - line);
		line += len;
		line[-1] = '\0';
	}

	rc = malloc((lines + 1U) * sizeof(char *) + length);
	if (!rc) {
		copy_errno(ctx);
		return NULL;
	}

	line = ((char *)rc) + (lines + 1U) * sizeof(char *);
	memcpy(line, payload, length);
	rc[lines] = NULL;
	for (i = 0; i < lines; i++) {
		rc[i] = line;
		line = &strchr(line, '\0')[1U];
	}

	return rc;
}

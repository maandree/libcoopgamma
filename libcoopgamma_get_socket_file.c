/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Get the socket file of the coopgamma server
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @param   method   The adjustment method, `NULL` for automatic
 * @param   site     The site, `NULL` for automatic
 * @return           The pathname of the server's socket, `NULL` on error
 *                   or if there server does have its own socket. The later
 *                   case is detected by checking that `errno` is set to 0,
 *                   and is the case when communicating with a server in a
 *                   multi-server display server like mds.
 */
char *
libcoopgamma_get_socket_file(const char *restrict method, const char *restrict site)
{
	char *raw;
	char *p;

	raw = libcoopgamma_query__(method, site, "-qq");
	if (!raw)
		return NULL;

	p = &strchr(raw, '\0')[-1];
	if (p < raw || *p != '\n') {
		errno = EBADMSG;
		goto fail;
	}
	*p = '\0';
	if (!*raw) {
		errno = EBADMSG;
		goto fail;
	}

	return raw;
fail:
	free(raw);
	return NULL;
}

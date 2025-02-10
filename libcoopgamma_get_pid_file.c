/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Get the PID file of the coopgamma server
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @param   method   The adjustment method, `NULL` for automatic
 * @param   site     The site, `NULL` for automatic
 * @return           The pathname of the server's PID file, `NULL` on error
 *                   or if there server does not use PID files. The later
 *                   case is detected by checking that `errno` is set to 0.
 */
char *
libcoopgamma_get_pid_file(const char *restrict method, const char *restrict site)
{
	char *path;
	size_t n;

	path = libcoopgamma_get_socket_file(method, site);
	if (!path)
		return NULL;

	n = strlen(path);
	if (n < 7U || strcmp(&path[n - 7U], ".socket")) {
		free(path);
		errno = EBADMSG;
		return NULL;
	}

	stpcpy(&path[n - 7U], ".pid");
	return path;
}

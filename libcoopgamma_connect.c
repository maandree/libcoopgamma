/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Connect to a coopgamma server, and start it if necessary
 * 
 * Use `libcoopgamma_context_destroy` to disconnect
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @param   method  The adjustment method, `NULL` for automatic
 * @param   site    The site, `NULL` for automatic
 * @param   ctx     The state of the library, must be initialised
 * @return          Zero on success, -1 on error. On error, `errno` is set
 *                  to 0 if the server could not be initialised.
 */
int
libcoopgamma_connect(const char *restrict method, const char *restrict site, libcoopgamma_context_t *restrict ctx)
{
	const char *(args[6]) = {COOPGAMMAD};
	struct sockaddr_un address;
	char *path;
	int saved_errno;
	int tries = 0, status;
	pid_t pid;
	size_t i = 1;

	ctx->blocking = 1;

	if (method) args[i++] = "-m", args[i++] = method;
	if (site)   args[i++] = "-s", args[i++] = site;
	args[i] = NULL;

	path = libcoopgamma_get_socket_file(method, site);
	if (!path)
		return -1;
	if (strlen(path) >= sizeof(address.sun_path)) {
		free(path);
		errno = ENAMETOOLONG;
		return -1;
	}

	address.sun_family = AF_UNIX;
	stpcpy(address.sun_path, path);
	free(path);
	if ((ctx->fd = socket(PF_UNIX, SOCK_STREAM, 0)) < 0)
		return -1;

retry:
	if (connect(ctx->fd, (const struct sockaddr *)&address, (socklen_t)sizeof(address)) < 0) {
		if ((errno == ECONNREFUSED || errno == ENOENT || errno == ENOTDIR) && !tries++) {
			switch ((pid = fork())) {
			case -1:
				goto fail;

			case 0:
				/* Child */
				close(ctx->fd);
#if defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif
				execvp(COOPGAMMAD, (char *const *)(args));
#if defined(__GNUC__)
# pragma GCC diagnostic pop
#endif
				perror(NAME_OF_THE_PROCESS);
				exit(1);

			default:
				/* Parent */
				if (waitpid(pid, &status, 0) < 0)
					goto fail;
				if (status) {
					errno = 0;
					goto fail;
				}
				break;
			}
			goto retry;
		}
		goto fail;
	}

	return 0;

fail:
	saved_errno = errno;
	close(ctx->fd);
	ctx->fd = -1;
	errno = saved_errno;
	return -1;
}

/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Run coopgammad with -q or -qq and return the response
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @param   method   The adjustment method, `NULL` for automatic
 * @param   site     The site, `NULL` for automatic
 * @param   arg      "-q" or "-qq", which shall be passed to coopgammad?
 * @return           The output of coopgammad, `NULL` on error. This
 *                   will be NUL-terminated and will not contain any
 *                   other `NUL` bytes.
 */
char *
libcoopgamma_query__(const char *restrict method, const char *restrict site, const char *restrict arg)
{
	const char *(args[7]) = {COOPGAMMAD, arg};
	size_t i = 2, n = 0, size = 0;
	int pipe_rw[2] = { -1, -1 };
	pid_t pid;
	int saved_errno, status;
	char *msg = NULL;
	ssize_t got;
	void *new;

	if (method) args[i++] = "-m", args[i++] = method;
	if (site)   args[i++] = "-s", args[i++] = site;
	args[i] = NULL;

	if (pipe(pipe_rw) < 0)
		goto fail;

	switch ((pid = fork())) {
	case -1:
		goto fail;

	case 0:
		/* Child */
		close(pipe_rw[0]);
		if (pipe_rw[1] != STDOUT_FILENO) {
			close(STDOUT_FILENO);
			if (dup2(pipe_rw[1], STDOUT_FILENO) < 0)
				goto fail_child;
			close(pipe_rw[1]);
		}
#if defined(__GNUC__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wcast-qual"
#endif
		execvp(COOPGAMMAD, (char* const*)(args));
#if defined(__GNUC__)
# pragma GCC diagnostic pop
#endif
	fail_child:
		saved_errno = errno;
		perror(NAME_OF_THE_PROCESS);
		if (write(STDOUT_FILENO, &saved_errno, sizeof(int)) != sizeof(int))
			perror(NAME_OF_THE_PROCESS);
		exit(1);

	default:
		/* Parent */
		close(pipe_rw[1]), pipe_rw[1] = -1;
		for (;;) {
			if (n == size) {
				new = realloc(msg, size = (n ? (n << 1) : 256U));
				if (!new)
					goto fail;
				msg = new;
			}
			got = read(pipe_rw[0], &msg[n], size - n);
			if (got < 0) {
				if (errno == EINTR)
					continue;
				goto fail;
			} else if (got == 0) {
				break;
			}
			n += (size_t)got;
		}
		close(pipe_rw[0]), pipe_rw[0] = -1;
		if (waitpid(pid, &status, 0) < 0)
			goto fail;
		if (status) {
			errno = EINVAL;
			if (n == sizeof(int) && *(int *)msg)
				errno = *(int*)msg;
		}
		break;
	}

	if (n == size) {
		new = realloc(msg, n + 1U);
		if (!new)
			goto fail;
		msg = new;
	}
	msg[n] = '\0';

	if (strchr(msg, '\0') != msg + n) {
		errno = EBADMSG;
		goto fail;
	}

	return msg;

fail:
	saved_errno = errno;
	if (pipe_rw[0] >= 0)
		close(pipe_rw[0]);
	if (pipe_rw[1] >= 0)
		close(pipe_rw[1]);
	free(msg);
	errno = saved_errno;
	return NULL;
}

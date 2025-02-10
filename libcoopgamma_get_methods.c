/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * List all recognised adjustment method
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @return  A `NULL`-terminated list of names. You should only free
 *          the outer pointer, inner pointers are subpointers of the
 *          outer pointer and cannot be freed. `NULL` on error.
 */
char **
libcoopgamma_get_methods(void)
{
	char num[5]; /* The size is base on the fact that we have limited `n` in the loop below */
	char **methods = NULL;
	char *method;
	char **rc;
	char *buffer;
	size_t n = 0;
	size_t size = 0;
	void *new;

	methods = malloc(4U * sizeof(*methods));
	if (!methods)
		goto fail;

	for (n = 0; n < 10000 /* just to be safe */; n++) {
		if (n >= 4 && (n & (~n + 1U)) == n) {
			new = realloc(methods, (n << 1) * sizeof(*methods));
			if (!new)
				goto fail;
			methods = new;
		}
		sprintf(num, "%zu", n);
		if (libcoopgamma_get_method_and_site(num, NULL, &method, NULL))
			goto fail;
		if (!strcmp(method, num)) {
			free(method);
			break;
		}
		methods[n] = method;
		size += strlen(method) + 1U;
	}

	rc = malloc((n + 1U) * sizeof(char *) + size);
	if (!rc)
		goto fail;
	buffer = (char *)&rc[n + 1U];
	rc[n] = NULL;
	while (n--) {
		rc[n] = buffer;
		buffer = &stpcpy(buffer, methods[n])[1U];
		free(methods[n]);
	}
	free(methods);

	return rc;

fail:
	while (n--)
		free(methods[n]);
	free(methods);
	return NULL;
}

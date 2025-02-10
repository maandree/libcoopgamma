/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Get the adjustment method and site
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @param   method   The adjustment method, `NULL` for automatic
 * @param   site     The site, `NULL` for automatic
 * @param   methodp  Output pointer for the selected adjustment method,
 *                   which cannot be `NULL`. It is safe to call
 *                   this function with this parameter set to `NULL`.
 * @param   sitep    Output pointer for the selected site, which will
 *                   be `NULL` the method only supports one site or if
 *                   `site == NULL` and no site can be selected
 *                   automatically. It is safe to call this function
 *                   with this parameter set to `NULL`.
 * @return           Zero on success, -1 on error
 */
int
libcoopgamma_get_method_and_site(const char *restrict method, const char *restrict site,
                                 char **restrict methodp, char **restrict sitep)
{
	int saved_errno;
	char *raw;
	char *p;
	char *q;

	raw = libcoopgamma_query__(method, site, "-q");
	if (!raw)
		return -1;

	if (methodp) *methodp = NULL;
	if (sitep)   *sitep   = NULL;

	p = strchr(raw, '\n');
	if (!p) {
		errno = EBADMSG;
		goto fail;
	}
	*p++ = '\0';

	if (methodp) {
		*methodp = malloc(strlen(raw) + 1U);
		if (!*methodp)
			goto fail;
		strcpy(*methodp, raw);
	}

	if (site && *(q = &strchr(p, '\0')[-1])) {
		if (*q != '\n') {
			errno = EBADMSG;
			goto fail;
		}
		*q = '\0';
		*sitep = malloc(strlen(p) + 1U);
		if (!*sitep)
			goto fail;
		strcpy(*sitep, p);
	}

	free(raw);
	return 0;

fail:
	saved_errno = errno;
	if (methodp) {
		free(*methodp);
		*methodp = NULL;
	}
	free(raw);
	errno = saved_errno;
	return -1;
}

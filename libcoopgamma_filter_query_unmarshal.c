/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Unmarshal a `libcoopgamma_filter_query_t` from a buffer
 * 
 * @param   this  The output parameter for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int
libcoopgamma_filter_query_unmarshal(libcoopgamma_filter_query_t *restrict this, const void *restrict vbuf, size_t *restrict np)
{
	UNMARSHAL_PROLOGUE;
	this->crtc = NULL;
	unmarshal_version(LIBCOOPGAMMA_FILTER_QUERY_VERSION);
	unmarshal_string(this->crtc);
	unmarshal_prim(this->coalesce, int);
	unmarshal_prim(this->high_priority, int64_t);
	unmarshal_prim(this->low_priority, int64_t);
	UNMARSHAL_EPILOGUE;
}

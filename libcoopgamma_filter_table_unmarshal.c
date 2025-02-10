/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Unmarshal a `libcoopgamma_filter_table_t` from a buffer
 * 
 * @param   this  The output parameter for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int
libcoopgamma_filter_table_unmarshal(libcoopgamma_filter_table_t *restrict this, const void *restrict vbuf, size_t *restrict np)
{
	size_t i, n, fn;
	int r;
	UNMARSHAL_PROLOGUE;
	this->filter_count = 0;
	this->filters = NULL;
	unmarshal_version(LIBCOOPGAMMA_FILTER_TABLE_VERSION);
	unmarshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
	unmarshal_prim(this->depth);
	unmarshal_prim(this->red_size);
	unmarshal_prim(this->green_size);
	unmarshal_prim(this->blue_size);
	unmarshal_prim(fn);
	this->filters = malloc(fn * sizeof(*this->filters));
	if (!this->filters)
		return LIBCOOPGAMMA_ERRNO_SET;
	for (i = 0; i < fn; i++) {
		r = libcoopgamma_queried_filter_unmarshal(&this->filters[i], NNSUBBUF, &n, this->depth);
		if (r != LIBCOOPGAMMA_SUCCESS)
			return r;
		off += n;
		this->filter_count += 1;
	}
	UNMARSHAL_EPILOGUE;
}

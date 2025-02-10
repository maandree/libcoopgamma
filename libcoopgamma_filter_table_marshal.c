/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Marshal a `libcoopgamma_filter_table_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t
libcoopgamma_filter_table_marshal(const libcoopgamma_filter_table_t *restrict this, void *restrict vbuf)
{
	size_t i;
	MARSHAL_PROLOGUE;
	marshal_version(LIBCOOPGAMMA_FILTER_TABLE_VERSION);
	marshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
	marshal_prim(this->depth, libcoopgamma_depth_t);
	marshal_prim(this->red_size, size_t);
	marshal_prim(this->green_size, size_t);
	marshal_prim(this->blue_size, size_t);
	marshal_prim(this->filter_count, size_t);
	for (i = 0; i < this->filter_count; i++)
		off += libcoopgamma_queried_filter_marshal(&this->filters[i], SUBBUF, this->depth);
	MARSHAL_EPILOGUE;
}

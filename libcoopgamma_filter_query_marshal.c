/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Marshal a `libcoopgamma_filter_query_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t
libcoopgamma_filter_query_marshal(const libcoopgamma_filter_query_t *restrict this, void* restrict vbuf)
{
	MARSHAL_PROLOGUE;
	marshal_version(LIBCOOPGAMMA_FILTER_QUERY_VERSION);
	marshal_string(this->crtc);
	marshal_prim(this->coalesce, int);
	marshal_prim(this->high_priority, int64_t);
	marshal_prim(this->low_priority, int64_t);
	MARSHAL_EPILOGUE;
}

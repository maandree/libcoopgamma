/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Marshal a `libcoopgamma_async_context_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t
libcoopgamma_async_context_marshal(const libcoopgamma_async_context_t *restrict this, void *restrict vbuf)
{
	MARSHAL_PROLOGUE;
	marshal_version(LIBCOOPGAMMA_ASYNC_CONTEXT_VERSION);
	marshal_prim(this->message_id, uint32_t);
	marshal_prim(this->coalesce, int);
	MARSHAL_EPILOGUE;
}

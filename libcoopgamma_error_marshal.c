/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Marshal a `libcoopgamma_error_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t
libcoopgamma_error_marshal(const libcoopgamma_error_t *restrict this, void *restrict vbuf)
{
	MARSHAL_PROLOGUE;
	marshal_version(LIBCOOPGAMMA_ERROR_VERSION);
	marshal_prim(this->number);
	marshal_prim(this->custom);
	marshal_prim(this->server_side);
	marshal_string(this->description);
	MARSHAL_EPILOGUE;
}

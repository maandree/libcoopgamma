/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Unmarshal a `libcoopgamma_async_context_t` from a buffer
 * 
 * @param   this  The output parameter for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int
libcoopgamma_async_context_unmarshal(libcoopgamma_async_context_t *restrict this, const void *restrict vbuf, size_t *restrict np)
{
	UNMARSHAL_PROLOGUE;
	unmarshal_version(LIBCOOPGAMMA_ASYNC_CONTEXT_VERSION);
	unmarshal_prim(this->message_id, uint32_t);
	unmarshal_prim(this->coalesce, int);
	UNMARSHAL_EPILOGUE;
}

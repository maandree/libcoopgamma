/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Unmarshal a `libcoopgamma_error_t` from a buffer
 * 
 * @param   this  The output parameter for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int
libcoopgamma_error_unmarshal(libcoopgamma_error_t *restrict this, const void *restrict vbuf, size_t *restrict np)
{
	UNMARSHAL_PROLOGUE;
	this->description = NULL;
	unmarshal_version(LIBCOOPGAMMA_ERROR_VERSION);
	unmarshal_prim(this->number);
	unmarshal_prim(this->custom);
	unmarshal_prim(this->server_side);
	unmarshal_string(this->description);
	UNMARSHAL_EPILOGUE;
}

/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Unmarshal a `libcoopgamma_filter_t` from a buffer
 * 
 * @param   this  The output parameter for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int
libcoopgamma_filter_unmarshal(libcoopgamma_filter_t *restrict this, const void *restrict vbuf, size_t *restrict np)
{
	int r = LIBCOOPGAMMA_SUCCESS;
	size_t n = 0;
	UNMARSHAL_PROLOGUE;
	memset(this, 0, sizeof(*this));
	unmarshal_version(LIBCOOPGAMMA_FILTER_VERSION);
	unmarshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
	unmarshal_version(LIBCOOPGAMMA_LIFESPAN_VERSION);
	unmarshal_prim(this->depth, libcoopgamma_depth_t);
	unmarshal_prim(this->priority, int64_t);
	unmarshal_string(this->crtc);
	unmarshal_string(this->class);
	unmarshal_prim(this->lifespan, libcoopgamma_lifespan_t);
	switch (this->depth) {
	case LIBCOOPGAMMA_UINT8:  r = libcoopgamma_ramps_unmarshal(&(this->ramps.u8),  NNSUBBUF, &n); break;
	case LIBCOOPGAMMA_UINT16: r = libcoopgamma_ramps_unmarshal(&(this->ramps.u16), NNSUBBUF, &n); break;
	case LIBCOOPGAMMA_UINT32: r = libcoopgamma_ramps_unmarshal(&(this->ramps.u32), NNSUBBUF, &n); break;
	case LIBCOOPGAMMA_UINT64: r = libcoopgamma_ramps_unmarshal(&(this->ramps.u64), NNSUBBUF, &n); break;
	case LIBCOOPGAMMA_FLOAT:  r = libcoopgamma_ramps_unmarshal(&(this->ramps.f),   NNSUBBUF, &n); break;
	case LIBCOOPGAMMA_DOUBLE: r = libcoopgamma_ramps_unmarshal(&(this->ramps.d),   NNSUBBUF, &n); break;
	default:
		break;
	}
	if (r != LIBCOOPGAMMA_SUCCESS)
		return r;
	off += n;
	UNMARSHAL_EPILOGUE;
}

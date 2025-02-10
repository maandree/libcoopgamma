/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Marshal a `libcoopgamma_filter_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t
libcoopgamma_filter_marshal(const libcoopgamma_filter_t *restrict this, void *restrict vbuf)
{
	MARSHAL_PROLOGUE;
	marshal_version(LIBCOOPGAMMA_FILTER_VERSION);
	marshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
	marshal_version(LIBCOOPGAMMA_LIFESPAN_VERSION);
	marshal_prim(this->depth);
	marshal_prim(this->priority);
	marshal_string(this->crtc);
	marshal_string(this->class);
	marshal_prim(this->lifespan);
	switch (this->depth) {
	case LIBCOOPGAMMA_UINT8:  off += libcoopgamma_ramps_marshal(&this->ramps.u8,  SUBBUF); break;
	case LIBCOOPGAMMA_UINT16: off += libcoopgamma_ramps_marshal(&this->ramps.u16, SUBBUF); break;
	case LIBCOOPGAMMA_UINT32: off += libcoopgamma_ramps_marshal(&this->ramps.u32, SUBBUF); break;
	case LIBCOOPGAMMA_UINT64: off += libcoopgamma_ramps_marshal(&this->ramps.u64, SUBBUF); break;
	case LIBCOOPGAMMA_FLOAT:  off += libcoopgamma_ramps_marshal(&this->ramps.f,   SUBBUF); break;
	case LIBCOOPGAMMA_DOUBLE: off += libcoopgamma_ramps_marshal(&this->ramps.d,   SUBBUF); break;
	default:
		break;
	}
	MARSHAL_EPILOGUE;
}

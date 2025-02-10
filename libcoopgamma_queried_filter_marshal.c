/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Marshal a `libcoopgamma_queried_filter_t` into a buffer
 * 
 * @param   this   The record to marshal
 * @param   vbuf   The output buffer, `NULL` to only measure
 *                 how large this buffer has to be
 * @param   depth  The type used of ramp stops
 * @return         The number of marshalled bytes, or if `buf == NULL`,
 *                 how many bytes would be marshalled if `buf != NULL`
 */
size_t
libcoopgamma_queried_filter_marshal(const libcoopgamma_queried_filter_t *restrict this,
                                    void *restrict vbuf, libcoopgamma_depth_t depth)
{
	MARSHAL_PROLOGUE;
	marshal_version(LIBCOOPGAMMA_QUERIED_FILTER_VERSION);
	marshal_prim(this->priority, int64_t);
	marshal_string(this->class);
	switch (depth) {
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

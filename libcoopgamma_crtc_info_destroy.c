/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Release all resources allocated to  a `libcoopgamma_crtc_info_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_crtc_info_initialise`
 * or failed call to `libcoopgamma_crtc_info_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void
libcoopgamma_crtc_info_destroy(libcoopgamma_crtc_info_t *restrict this)
{
	(void) this;
}

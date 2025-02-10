/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Unmarshal a `libcoopgamma_context_t` from a buffer
 * 
 * @param   this  The output parameter for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int
libcoopgamma_context_unmarshal(libcoopgamma_context_t *restrict this, const void *restrict vbuf, size_t *restrict np)
{
	size_t n;
	int r;
	UNMARSHAL_PROLOGUE;
	memset(this, 0, sizeof(*this));
	unmarshal_version(LIBCOOPGAMMA_CONTEXT_VERSION);
	unmarshal_prim(this->fd, int);
	r = libcoopgamma_error_unmarshal(&this->error, NNSUBBUF, &n);
	if (r != LIBCOOPGAMMA_SUCCESS)
		return r;
	off += n;
	unmarshal_prim(this->message_id, uint32_t);
	unmarshal_prim(this->outbound_head, size_t);
	this->outbound_size = this->outbound_head;
	unmarshal_buffer(this->outbound, this->outbound_head);
	unmarshal_prim(this->inbound_head, size_t);
	this->inbound_size = this->inbound_head;
	unmarshal_buffer(this->inbound, this->inbound_head);
	unmarshal_prim(this->length, size_t);
	unmarshal_prim(this->curline, size_t);
	unmarshal_prim(this->in_response_to, uint32_t);
	unmarshal_prim(this->have_all_headers, int);
	unmarshal_prim(this->bad_message, int);
	unmarshal_prim(this->blocking, int);
	UNMARSHAL_EPILOGUE;
}

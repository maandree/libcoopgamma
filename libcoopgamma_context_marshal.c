/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Marshal a `libcoopgamma_context_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t
libcoopgamma_context_marshal(const libcoopgamma_context_t *restrict this, void *restrict vbuf)
{
	MARSHAL_PROLOGUE;
	marshal_version(LIBCOOPGAMMA_CONTEXT_VERSION);
	marshal_prim(this->fd, int);
	off += libcoopgamma_error_marshal(&this->error, SUBBUF);
	marshal_prim(this->message_id, uint32_t);
	marshal_prim(this->outbound_head - this->outbound_tail, size_t);
	marshal_buffer(this->outbound + this->outbound_tail, this->outbound_head - this->outbound_tail);
	marshal_prim(this->inbound_head - this->inbound_tail, size_t);
	marshal_buffer(this->inbound + this->inbound_tail, this->inbound_head - this->inbound_tail);
	marshal_prim(this->length, size_t);
	marshal_prim(this->curline, size_t);
	marshal_prim(this->in_response_to, uint32_t);
	marshal_prim(this->have_all_headers, int);
	marshal_prim(this->bad_message, int);
	marshal_prim(this->blocking, int);
	MARSHAL_EPILOGUE;
}

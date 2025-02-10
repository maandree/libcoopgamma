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
	size_t n;
	MARSHAL_PROLOGUE;
	marshal_version(LIBCOOPGAMMA_CONTEXT_VERSION);
	marshal_prim(this->fd);
	off += libcoopgamma_error_marshal(&this->error, SUBBUF);
	marshal_prim(this->message_id);
	n = this->outbound_head - this->outbound_tail;
	marshal_prim(n);
	marshal_buffer(&this->outbound[this->outbound_tail], n);
	n = this->inbound_head - this->inbound_tail;
	marshal_prim(n);
	marshal_buffer(&this->inbound[this->inbound_tail], n);
	marshal_prim(this->length);
	marshal_prim(this->curline);
	marshal_prim(this->in_response_to);
	marshal_prim(this->have_all_headers);
	marshal_prim(this->bad_message);
	marshal_prim(this->blocking);
	MARSHAL_EPILOGUE;
}

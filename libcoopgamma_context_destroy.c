/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Release all resources allocated to  a `libcoopgamma_context_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_context_initialise`
 * or failed call to `libcoopgamma_context_unmarshal`
 * 
 * @param  this        The record to destroy
 * @param  disconnect  Disconnect from the server?
 */
void
libcoopgamma_context_destroy(libcoopgamma_context_t *restrict this, int disconnect)
{
	if (disconnect && this->fd >= 0) {
		shutdown(this->fd, SHUT_RDWR);
		close(this->fd);
	}
	this->fd = -1;
	libcoopgamma_error_destroy(&this->error);
	free(this->outbound);
	free(this->inbound);
	this->outbound = NULL;
	this->inbound = NULL;
}

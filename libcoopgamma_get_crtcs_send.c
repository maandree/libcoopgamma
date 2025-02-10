/* See LICENSE file for copyright and license details. */
#include "common.h"


/* Silence falls warning for `memcpy(&msg__[n__], (payload), (payload_size))`
 * in `SEND_MESSAGE` where `payload` is assumed to be `NULL` despite being
 * inside `if (payload)` */
#if defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wnonnull"
#endif


/**
 * List all available CRTC:s, send request part
 * 
 * Cannot be used before connecting to the server
 * 
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request, that is needed to
 *                 identify and parse the response, is stored here
 * @return         Zero on success, -1 on error
 */
int
libcoopgamma_get_crtcs_send(libcoopgamma_context_t *restrict ctx, libcoopgamma_async_context_t *restrict async)
{
	async->message_id = ctx->message_id;
	SEND_MESSAGE(ctx, NULL, (size_t)0,
	             "Command: enumerate-crtcs\n"
	             "Message ID: %" PRIu32 "\n"
	             "\n",
	             ctx->message_id);

	return 0;
fail:
	copy_errno(ctx);
	return -1;
}

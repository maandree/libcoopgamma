/* See LICENSE file for copyright and license details. */
#include "common.h"


/* Silence falls warning for `memcpy(&msg__[n__], (payload), (payload_size))`
 * in `SEND_MESSAGE` where `payload` is assumed to be `NULL` despite being
 * inside `if (payload)` */
#if defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wnonnull"
#endif


/**
 * Retrieve information about a CRTC:s gamma ramps, send request part
 * 
 * Cannot be used before connecting to the server
 * 
 * @param   crtc   The name of the CRTC
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request, that is needed to
 *                 identify and parse the response, is stored here
 * @return         Zero on success, -1 on error
 */
int
libcoopgamma_get_gamma_info_send(const char *restrict crtc, libcoopgamma_context_t *restrict ctx,
                                 libcoopgamma_async_context_t *restrict async)
{
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wnonnull-compare"
#endif
	if (crtc == NULL || strchr(crtc, '\n')) {
		errno = EINVAL;
		goto fail;
	}
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic pop
#endif

	async->message_id = ctx->message_id;
	SEND_MESSAGE(ctx, NULL, (size_t)0,
	             "Command: get-gamma-info\n"
	             "Message ID: %" PRIu32 "\n"
	             "CRTC: %s\n"
	             "\n",
	             ctx->message_id, crtc);

	return 0;
fail:
	copy_errno(ctx);
	return 0;
}

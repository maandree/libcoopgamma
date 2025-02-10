/* See LICENSE file for copyright and license details. */
#include "common.h"


/* Silence falls warning for `memcpy(&msg__[n__], (payload), (payload_size))`
 * in `SEND_MESSAGE` where `payload` is assumed to be `NULL` despite being
 * inside `if (payload)` */
#if defined(__GNUC__)
# pragma GCC diagnostic ignored "-Wnonnull"
#endif


/**
 * Retrieve the current gamma ramp adjustments, send request part
 * 
 * Cannot be used before connecting to the server
 * 
 * @param   query  The query to send
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request, that is needed to
 *                 identify and parse the response, is stored here
 * @return         Zero on success, -1 on error
 */
int
libcoopgamma_get_gamma_send(const libcoopgamma_filter_query_t *restrict query, libcoopgamma_context_t *restrict ctx,
                            libcoopgamma_async_context_t *restrict async)
{
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wnonnull-compare"
#endif
	if (!query || !query->crtc || strchr(query->crtc, '\n')) {
		errno = EINVAL;
		goto fail;
	}
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic pop
#endif

	async->message_id = ctx->message_id;
	async->coalesce = query->coalesce;
	SEND_MESSAGE(ctx, NULL, (size_t)0,
	             "Command: get-gamma\n"
	             "Message ID: %" PRIu32 "\n"
	             "CRTC: %s\n"
	             "Coalesce: %s\n"
	             "High priority: %" PRIi64 "\n"
	             "Low priority: %" PRIi64 "\n"
	             "\n",
	             ctx->message_id, query->crtc, query->coalesce ? "yes" : "no",
	             query->high_priority, query->low_priority);

	return 0;
fail:
	copy_errno(ctx);
	return -1;
}

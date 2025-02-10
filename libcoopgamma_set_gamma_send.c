/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Apply, update, or remove a gamma ramp adjustment, send request part
 * 
 * Cannot be used before connecting to the server
 * 
 * @param   filter  The filter to apply, update, or remove, gamma ramp meta-data must match the CRTC's
 * @param   ctx     The state of the library, must be connected
 * @param   async   Information about the request, that is needed to
 *                  identify and parse the response, is stored here
 * @return          Zero on success, -1 on error
 */
int
libcoopgamma_set_gamma_send(const libcoopgamma_filter_t *restrict filter, libcoopgamma_context_t *restrict ctx,
                            libcoopgamma_async_context_t *restrict async)
{
	const void *payload = NULL;
	const char *lifespan;
	char priority[sizeof("Priority: \n") + 3 * sizeof(int64_t)] = {'\0'};
	char length  [sizeof("Length: \n")   + 3 * sizeof(size_t) ] = {'\0'};
	size_t payload_size = 0, stopwidth = 0;

#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic push
# pragma GCC diagnostic ignored "-Wnonnull-compare"
#endif
	if (!filter || !filter->crtc || strchr(filter->crtc, '\n') || !filter->class || strchr(filter->class, '\n')) {
		errno = EINVAL;
		goto fail;
	}
#if defined(__GNUC__) && !defined(__clang__)
# pragma GCC diagnostic pop
#endif

	switch (filter->lifespan) {
	case LIBCOOPGAMMA_REMOVE:        lifespan = "remove";        break;
	case LIBCOOPGAMMA_UNTIL_DEATH:   lifespan = "until-death";   break;
	case LIBCOOPGAMMA_UNTIL_REMOVAL: lifespan = "until-removal"; break;
	default:
		errno = EINVAL;
		goto fail;
	}

	if (filter->lifespan != LIBCOOPGAMMA_REMOVE) {
		switch (filter->depth) {
		case LIBCOOPGAMMA_FLOAT:  stopwidth = sizeof(float);  break;
		case LIBCOOPGAMMA_DOUBLE: stopwidth = sizeof(double); break;
		default: INTEGRAL_DEPTHS
			if (filter->depth <= 0 || (filter->depth & 7)) {
				errno = EINVAL;
				goto fail;
			}
			stopwidth = (size_t)(filter->depth / 8);
			break;
		}

		payload_size  = filter->ramps.u8.red_size;
		payload_size += filter->ramps.u8.green_size;
		payload_size += filter->ramps.u8.blue_size;
		payload_size *= stopwidth;
		payload = filter->ramps.u8.red;
		sprintf(priority, "Priority: %" PRIi64 "\n", filter->priority);
		sprintf(length, "Length: %zu\n", payload_size);
	}

	async->message_id = ctx->message_id;
	SEND_MESSAGE(ctx, payload, payload_size,
	             "Command: set-gamma\n"
	             "Message ID: %" PRIu32 "\n"
	             "CRTC: %s\n"
	             "Class: %s\n"
	             "Lifespan: %s\n"
	             "%s"
	             "%s"
	             "\n",
	             ctx->message_id, filter->crtc, filter->class, lifespan, priority, length);

	return 0;
fail:
	copy_errno(ctx);
	return -1;
}

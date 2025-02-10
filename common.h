/* See LICENSE file for copyright and license details. */
#include "libcoopgamma.h"

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#if !defined(COOPGAMMAD)
# define COOPGAMMAD "coopgammad"
#endif


#if defined(__clang__)
# pragma clang diagnostic ignored "-Wdocumentation"
# pragma clang diagnostic ignored "-Wcovered-switch-default"
# pragma clang diagnostic ignored "-Wcast-align"
#endif


#if defined(__GNUC__)
# define NAME_OF_THE_PROCESS  (argv0)
extern const char *argv0;
#else
# define NAME_OF_THE_PROCESS  ("libcoopgamma")
#endif


#if defined(__GNUC__)
# define HIDDEN  __attribute__((__visibility__("hidden")))
#else
# define HIDDEN
#endif



#define SUBBUF\
	(buf ? &buf[off] : NULL)

#define NNSUBBUF\
	(&buf[off])

#define MARSHAL_PROLOGUE\
	char *restrict buf = vbuf;\
	size_t off = 0;

#define UNMARSHAL_PROLOGUE\
	const char *restrict buf = vbuf;\
	size_t off = 0;

#define MARSHAL_EPILOGUE\
	return off

#define UNMARSHAL_EPILOGUE\
	return *np = off, LIBCOOPGAMMA_SUCCESS

#define marshal_prim(datum, type)\
	((buf != NULL ? *(type *)&buf[off] = (datum) : 0), off += sizeof(type))

#define unmarshal_prim(datum, type)\
	((datum) = *(const type *)&buf[off], off += sizeof(type))

#define marshal_version(version)\
	marshal_prim(version, int)

#define unmarshal_version(version)\
	do {\
		int version__;\
		unmarshal_prim(version__, int);\
		if (version__ < (version))\
			return LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE;\
		if (version__ > (version))\
			return LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE;\
	} while (0)

#define marshal_buffer(data, n)\
	((buf ? (memcpy(&buf[off], (data), (n)), 0) : 0), off += (n))

#define unmarshal_buffer(data, n)\
	do {\
		(data) = malloc((n));\
		if (!(data))\
			return LIBCOOPGAMMA_ERRNO_SET;\
		memcpy((data), &buf[off], (n));\
		off += (n);\
	} while (0)

#define marshal_string(datum)\
	(!(datum) ? marshal_prim(0, char) :\
	 (marshal_prim(1, char), marshal_buffer((datum), strlen(datum) + 1U)))

#define unmarshal_string(datum)\
	do {\
		char nonnull__;\
		unmarshal_prim(nonnull__, char);\
		if (nonnull__)\
			unmarshal_buffer((datum), strlen(&buf[off]) + 1U);\
		else\
			(datum) = NULL;\
	} while (0)



#define copy_errno(ctx)\
	(!errno ? NULL :\
	 ((ctx)->error.number = (uint64_t)errno,\
	  (ctx)->error.custom = 0,\
	  (ctx)->error.server_side = 0,\
	  free((ctx)->error.description),\
	  (ctx)->error.description = NULL))


#define SYNC_CALL(send_call, recv_call, fail_return)\
	libcoopgamma_async_context_t async;\
	if (send_call < 0) {\
	reflush:\
		if (errno != EINTR)\
			return fail_return;\
		if (libcoopgamma_flush(ctx) < 0)\
			goto reflush;\
	}\
	resync:\
	if (libcoopgamma_synchronise(ctx, &async, (size_t)1, &(size_t){0}) < 0) {\
		if (errno != EINTR && errno)\
			return fail_return;\
		goto resync;\
	}\
	return recv_call


#define INTEGRAL_DEPTHS\
	case LIBCOOPGAMMA_UINT8:\
	case LIBCOOPGAMMA_UINT16:\
	case LIBCOOPGAMMA_UINT32:\
	case LIBCOOPGAMMA_UINT64:


/**
 * Send a message to the server and wait for response
 * 
 * @param  resp:char**                  Output parameter for the response,
 *                                      will be NUL-terminated
 * @param  ctx:libcoopgamma_context_t*  The state of the library
 * @param  payload:void*                Data to append to the end of the message
 * @param  payload_size:size_t          Byte-size of `payload`
 * @param  format:string-literal        Message formatting string
 * @param  ...                          Message formatting arguments
 * 
 * On error, the macro goes to `fail`.
 */
#define SEND_MESSAGE(ctx, payload, payload_size, format, ...)\
	do {\
		ssize_t n__;\
		char *msg__;\
		snprintf(NULL, (size_t)0, format "%zn", __VA_ARGS__, &n__);\
		msg__ = malloc((size_t)n__ + (payload_size) + (size_t)1);\
		if (!msg__)\
			goto fail;\
		sprintf(msg__, format, __VA_ARGS__);\
		if (payload)\
			memcpy(&msg__[n__], (payload), (payload_size));\
		if (libcoopgamma_send_message__((ctx), msg__, (size_t)n__ + (payload_size)) < 0)\
			goto fail;\
	} while (0)


/**
 * Run coopgammad with -q or -qq and return the response
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @param   method   The adjustment method, `NULL` for automatic
 * @param   site     The site, `NULL` for automatic
 * @param   arg      "-q" or "-qq", which shall be passed to coopgammad?
 * @return           The output of coopgammad, `NULL` on error. This
 *                   will be NUL-terminated and will not contain any
 *                   other `NUL` bytes.
 */
HIDDEN char *libcoopgamma_query__(const char *restrict method, const char *restrict site, const char *restrict arg);

/**
 * Send a message to the server and wait for response
 * 
 * @param   ctx  The state of the library
 * @param   msg  The message to send
 * @param   n    The length of `msg`
 * @return       Zero on success, -1 on error
 */
HIDDEN int libcoopgamma_send_message__(libcoopgamma_context_t *restrict ctx, char *msg, size_t n);

/**
 * Check whether the server sent an error, if so copy it to `ctx`
 * 
 * This function will also reports EBADMSG if the message ID
 * that the message is a response to does not match the request
 * information, or if it is missing
 * 
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         1 if the server sent an error (even indicating success),
 *                 0 on success, -1 on failure. Information about failure
 *                 is copied `ctx`.
 */
HIDDEN int libcoopgamma_check_error__(libcoopgamma_context_t *restrict ctx, libcoopgamma_async_context_t *restrict async);


/**
 * Get the next header of the inbound message
 * 
 * All headers must be read before the payload is read
 * 
 * @param   ctx  The state of the library, must be connected
 * @return       The next header line, can never be `NULL`,
 *               the empty string marks the end of the headers.
 *               This memory segment must not be freed.
 */
HIDDEN inline char *
libcoopgamma_next_header__(libcoopgamma_context_t *restrict ctx)
{
	char *rc = ctx->inbound + ctx->inbound_tail;
	ctx->inbound_tail += strlen(rc) + 1U;
	return rc;
}


/**
 * Get the payload of the inbound message
 * 
 * Calling this function marks that the inbound message
 * has been fully ready. You must call this function
 * even if you do not expect a payload
 * 
 * @param   ctx  The state of the library, must be connected
 * @param   n    Output parameter for the size of the payload
 * @return       The payload (not NUL-terminated), `NULL` if
 *               there is no payload. Failure is impossible.
 *               This memory segment must not be freed.
 */
HIDDEN inline char *
libcoopgamma_next_payload__(libcoopgamma_context_t *restrict ctx, size_t *n)
{
	char *rc;
	ctx->have_all_headers = 0;
	if ((*n = ctx->length)) {
		rc = ctx->inbound + ctx->inbound_tail;
		ctx->inbound_tail += *n;
		ctx->length = 0;
		return rc;
	}
	return NULL;
}

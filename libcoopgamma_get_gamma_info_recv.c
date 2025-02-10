/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Retrieve information about a CRTC:s gamma ramps, receive response part
 * 
 * @param   info   Output parameter for the information, must be initialised
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int
libcoopgamma_get_gamma_info_recv(libcoopgamma_crtc_info_t *restrict info, libcoopgamma_context_t *restrict ctx,
                                 libcoopgamma_async_context_t *restrict async)
{
	char temp[3 * sizeof(size_t) + 1];
	char *line;
	char *value;
	size_t _n;
	int have_cooperative = 0, have_gamma_support = 0, have_colourspace = 0;
	int have_depth       = 0, have_white_x = 0, have_white_y = 0;
	int have_red_size    = 0, have_red_x   = 0, have_red_y   = 0;
	int have_green_size  = 0, have_green_x = 0, have_green_y = 0;
	int have_blue_size   = 0, have_blue_x  = 0, have_blue_y  = 0;
	int bad = 0, r = 0, g = 0, b = 0, x = 0;
	size_t *outz;
	unsigned *outu;
	int *have;

	if (libcoopgamma_check_error__(ctx, async))
		return -1;

	info->cooperative = 0; /* Should be in the response, but ... */
	info->colourspace = LIBCOOPGAMMA_UNKNOWN;

	for (;;) {
		line = libcoopgamma_next_header__(ctx);
		value = &strchr(line, ':')[2U];
		if (!*line) {
			break;
		} else if (strstr(line, "Cooperative: ") == line) {
			have_cooperative = 1 + !!have_cooperative;
			if      (!strcmp(value, "yes")) info->cooperative = 1;
			else if (!strcmp(value, "no"))  info->cooperative = 0;
			else
				bad = 1;
		} else if (strstr(line, "Depth: ") == line) {
			have_depth = 1 + !!have_depth;
			if      (!strcmp(value, "8"))  info->depth = LIBCOOPGAMMA_UINT8;
			else if (!strcmp(value, "16")) info->depth = LIBCOOPGAMMA_UINT16;
			else if (!strcmp(value, "32")) info->depth = LIBCOOPGAMMA_UINT32;
			else if (!strcmp(value, "64")) info->depth = LIBCOOPGAMMA_UINT64;
			else if (!strcmp(value, "f"))  info->depth = LIBCOOPGAMMA_FLOAT;
			else if (!strcmp(value, "d"))  info->depth = LIBCOOPGAMMA_DOUBLE;
			else
				bad = 1;
		} else if (strstr(line, "Gamma support: ") == line) {
			have_gamma_support = 1 + !!have_gamma_support;
			if      (!strcmp(value, "yes"))   info->supported = LIBCOOPGAMMA_YES;
			else if (!strcmp(value, "no"))    info->supported = LIBCOOPGAMMA_NO;
			else if (!strcmp(value, "maybe")) info->supported = LIBCOOPGAMMA_MAYBE;
			else
				bad = 1;
		} else if ((r = (strstr(line, "Red size: ")   == line)) ||
			   (g = (strstr(line, "Green size: ") == line)) ||
			         strstr(line, "Blue size: ")  == line) {
			if (r)      have_red_size   = 1 + !!have_red_size,   outz = &info->red_size;
			else if (g) have_green_size = 1 + !!have_green_size, outz = &info->green_size;
			else        have_blue_size  = 1 + !!have_blue_size,  outz = &info->blue_size;
			*outz = (size_t)atol(value);
			sprintf(temp, "%zu", *outz);
			if (strcmp(value, temp))
				bad = 1;
		} else if ((x = r = (strstr(line, "Red x: ")   == line)) ||
			       (r = (strstr(line, "Red y: ")   == line)) ||
			   (x = g = (strstr(line, "Green x: ") == line)) ||
			       (g = (strstr(line, "Green y: ") == line)) ||
			   (x = b = (strstr(line, "Blue x: ")  == line)) ||
			       (b = (strstr(line, "Blue y: ")  == line)) ||
			   (x =     (strstr(line, "White x: ") == line)) ||
			             strstr(line, "White y: ") == line) {
			if      (r && x) have = &have_red_x,   outu = &info->red_x;
			else if (r)      have = &have_red_y,   outu = &info->red_y;
			else if (g && x) have = &have_green_x, outu = &info->green_x;
			else if (g)      have = &have_green_y, outu = &info->green_y;
			else if (b && x) have = &have_blue_x,  outu = &info->blue_x;
			else if (b)      have = &have_blue_y,  outu = &info->blue_y;
			else if (x)      have = &have_white_x, outu = &info->white_x;
			else             have = &have_white_y, outu = &info->white_y;
			*have = 1 + !!*have;
			*outu = (unsigned)atoi(value);
			sprintf(temp, "%u", *outu);
			if (strcmp(value, temp))
				bad = 1;
		} else if (strstr(line, "Colour space: ") == line) {
			have_colourspace = 1 + !!have_colourspace;
			if      (!strcmp(value, "sRGB"))    info->colourspace = LIBCOOPGAMMA_SRGB;
			else if (!strcmp(value, "RGB"))     info->colourspace = LIBCOOPGAMMA_RGB;
			else if (!strcmp(value, "non-RGB")) info->colourspace = LIBCOOPGAMMA_NON_RGB;
			else if (!strcmp(value, "grey"))    info->colourspace = LIBCOOPGAMMA_GREY;
			else
				info->colourspace = LIBCOOPGAMMA_UNKNOWN;
		}
	}

	(void) libcoopgamma_next_payload__(ctx, &_n);

	info->have_gamut = (have_red_x && have_green_x && have_blue_x && have_white_x &&
			    have_red_y && have_green_y && have_blue_y && have_white_y);

	if (bad || have_gamma_support != 1 || have_red_x > 1 || have_red_y > 1 ||
	    have_green_x > 1 || have_green_y > 1 || have_blue_x > 1 || have_blue_y > 1 ||
	    have_white_x > 1 || have_white_y > 1 || have_colourspace > 1) {
		errno = EBADMSG;
		copy_errno(ctx);
		return -1;
	}
	if (info->supported != LIBCOOPGAMMA_NO) {
		if (have_cooperative > 1 || have_depth != 1 || have_gamma_support != 1 ||
		    have_red_size != 1 || have_green_size != 1 || have_blue_size != 1) {
			errno = EBADMSG;
			copy_errno(ctx);
			return -1;
		}
	}

	return 0;
}

/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Retrieve the current gamma ramp adjustments, receive response part
 * 
 * @param   table  Output for the response, must be initialised
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int
libcoopgamma_get_gamma_recv(libcoopgamma_filter_table_t *restrict table, libcoopgamma_context_t *restrict ctx,
                            libcoopgamma_async_context_t *restrict async)
{
	char temp[3 * sizeof(size_t) + 1];
	char *line;
	char *value;
	char *payload;
	size_t i, n, width, clutsize;
	int have_depth = 0;
	int have_red_size = 0;
	int have_green_size = 0;
	int have_blue_size = 0;
	int have_tables = 0;
	int bad = 0, r = 0, g = 0;
	size_t *out;
	size_t off, len;

	if (libcoopgamma_check_error__(ctx, async))
		return -1;

	libcoopgamma_filter_table_destroy(table);

	for (;;) {
		line = libcoopgamma_next_header__(ctx);
		value = &strchr(line, ':')[2U];
		if (!*line) {
			break;
		} else if (strstr(line, "Depth: ") == line) {
			have_depth = 1 + !!have_depth;
			if      (!strcmp(value, "8"))  table->depth = LIBCOOPGAMMA_UINT8;
			else if (!strcmp(value, "16")) table->depth = LIBCOOPGAMMA_UINT16;
			else if (!strcmp(value, "32")) table->depth = LIBCOOPGAMMA_UINT32;
			else if (!strcmp(value, "64")) table->depth = LIBCOOPGAMMA_UINT64;
			else if (!strcmp(value, "f"))  table->depth = LIBCOOPGAMMA_FLOAT;
			else if (!strcmp(value, "d"))  table->depth = LIBCOOPGAMMA_DOUBLE;
			else
				bad = 1;
		} else if ((r = (strstr(line, "Red size: ")   == line)) ||
		           (g = (strstr(line, "Green size: ") == line)) ||
		                 strstr(line, "Blue size: ")  == line) {
			if (r)      have_red_size   = 1 + !!have_red_size,   out = &table->red_size;
			else if (g) have_green_size = 1 + !!have_green_size, out = &table->green_size;
			else        have_blue_size  = 1 + !!have_blue_size,  out = &table->blue_size;
			*out = (size_t)atol(value);
			sprintf(temp, "%zu", *out);
			if (strcmp(value, temp))
				bad = 1;
		} else if (strstr(line, "Tables: ") == line) {
			have_tables = 1 + have_tables;
			table->filter_count = (size_t)atol(value);
			sprintf(temp, "%zu", table->filter_count);
			if (strcmp(value, temp))
				bad = 1;
		}
	}

	payload = libcoopgamma_next_payload__(ctx, &n);

	if (bad || have_depth != 1 || have_red_size != 1 || have_green_size != 1 ||
	    have_blue_size != 1 || (async->coalesce ? have_tables > 1 : !have_tables) ||
	    ((!payload || !n) && (async->coalesce || table->filter_count > 0)) ||
	    (n > 0 && have_tables && !table->filter_count) ||
	    (async->coalesce && have_tables && table->filter_count != 1))
		goto bad;

	switch (table->depth) {
	case LIBCOOPGAMMA_FLOAT:  width = sizeof(float);  break;
	case LIBCOOPGAMMA_DOUBLE: width = sizeof(double); break;
	default: INTEGRAL_DEPTHS
		if (table->depth <= 0 || (table->depth & 7))
			goto bad;
		width = (size_t)(table->depth / 8);
		break;
	}

	clutsize = table->red_size + table->green_size + table->blue_size;
	clutsize *= width;

	if (async->coalesce) {
		if (n != clutsize)
			goto bad;
		table->filters = malloc(sizeof(*(table->filters)));
		if (!table->filters)
			goto fail;
		table->filters->priority = 0;
		table->filters->class = NULL;
		table->filters->ramps.u8.red_size   = table->red_size;
		table->filters->ramps.u8.green_size = table->green_size;
		table->filters->ramps.u8.blue_size  = table->blue_size;
		if (libcoopgamma_ramps_initialise_(&table->filters->ramps, width) < 0)
			goto fail;
		memcpy(table->filters->ramps.u8.red, payload, clutsize);
		table->filter_count = 1;
	} else if (!table->filter_count) {
		table->filters = NULL;
	} else {
		off = 0;
		table->filters = calloc(table->filter_count, sizeof(*table->filters));
		if (!table->filters)
			goto fail;
		for (i = 0; i < table->filter_count; i++) {
			if (off + sizeof(int64_t) > n)
				goto bad;
			table->filters[i].priority = *(int64_t *)&payload[off];
			off += sizeof(int64_t);
			if (!memchr(&payload[off], '\0', n - off))
				goto bad;
			len = strlen(&payload[off]) + 1U;
			table->filters[i].class = malloc(len);
			if (!table->filters[i].class)
				goto fail;
			memcpy(table->filters[i].class, &payload[off], len);
			off += len;
			if (off + clutsize > n)
				goto bad;
			table->filters[i].ramps.u8.red_size   = table->red_size;
			table->filters[i].ramps.u8.green_size = table->green_size;
			table->filters[i].ramps.u8.blue_size  = table->blue_size;
			if (libcoopgamma_ramps_initialise_(&table->filters[i].ramps, width) < 0)
				goto fail;
			memcpy(table->filters[i].ramps.u8.red, &payload[off], clutsize);
			off += clutsize;
		}
		if (off != n)
			goto bad;
	}

	return 0;
bad:
	errno = EBADMSG;
fail:
	copy_errno(ctx);
	return -1;
}

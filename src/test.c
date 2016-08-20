/**
 * libcoopgamma -- Library for interfacing with cooperative gamma servers
 * Copyright (C) 2016  Mattias Andrée (maandree@kth.se)
 * 
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "libcoopgamma.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


static int streq(const char* a, const char* b)
{
  if ((a == NULL) != (b == NULL))
    return 0;
  return (a == NULL) || !strcmp(a, b);
}


static int rampseq(const libcoopgamma_ramps_t* a, const libcoopgamma_ramps_t* b, libcoopgamma_depth_t depth)
{
  size_t nr, ng, nb, width;
  
  if ((a->u8.red_size   != b->u8.red_size)   ||
      (a->u8.green_size != b->u8.green_size) ||
      (a->u8.blue_size  != b->u8.blue_size))
    return 0;
  
  if (depth == -2)
    width = sizeof(double);
  else if (depth == -1)
    width = sizeof(float);
  else
    width = ((size_t)depth) / 8;
  
  nr = a->u8.red_size   * width;
  ng = a->u8.green_size * width;
  nb = a->u8.blue_size  * width;
  
  if (memcmp(a->u8.red, b->u8.red, nr + ng + nb))
    return 0;
  if (memcmp(a->u8.green, b->u8.green, ng + nb))
    return 0;
  if (memcmp(a->u8.blue, b->u8.blue, nb))
    return 0;
  
  return 1;
}


int main(void)
{
  libcoopgamma_filter_t filter1, filter2;
  libcoopgamma_crtc_info_t crtc1, crtc2;
  libcoopgamma_filter_query_t query1, query2;
  libcoopgamma_filter_table_t table1, table2;
  libcoopgamma_context_t ctx1, ctx2;
  libcoopgamma_async_context_t async1, async2;
  size_t n, m, i;
  char* buf;
  
  filter1.priority = INT64_MIN;
  filter1.crtc = "CRTC";
  filter1.class = "A::B::C::D";
  filter1.lifespan = LIBCOOPGAMMA_UNTIL_REMOVAL;
  filter1.depth = LIBCOOPGAMMA_DOUBLE;
  filter1.ramps.d.red_size = 4;
  filter1.ramps.d.green_size = 5;
  filter1.ramps.d.blue_size = 6;
  filter1.ramps.d.red = (double[]){0.0, 0.1, 0.2, 0.5,
				   0.3, 0.11, 0.22, 0.45, 0.9,
				   -1, -0.5, 0, 0.5, 1, 1.5};
  filter1.ramps.d.green = filter1.ramps.d.red + filter1.ramps.d.red_size;
  filter1.ramps.d.blue = filter1.ramps.d.green + filter1.ramps.d.green_size;
  
  crtc1.cooperative = 0;
  crtc1.depth = LIBCOOPGAMMA_DOUBLE;
  crtc1.supported = LIBCOOPGAMMA_YES;
  crtc1.red_size = 4;
  crtc1.green_size = 5;
  crtc1.blue_size = 6;
  crtc1.colourspace = LIBCOOPGAMMA_SRGB;
  crtc1.have_gamut = 1;
  crtc1.red_x = 100;
  crtc1.red_y = 50;
  crtc1.green_x = 300;
  crtc1.green_y = 350;
  crtc1.blue_x = 200;
  crtc1.blue_y = 260;
  crtc1.white_x = 500;
  crtc1.white_y = 999;
  
  query1.high_priority = INT64_MIN;
  query1.low_priority = INT64_MIN;
  query1.crtc = "crtc";
  query1.coalesce = 1;
  
  table1.red_size = 4;
  table1.green_size = 5;
  table1.blue_size = 6;
  table1.filter_count = 2;
  table1.filters = (libcoopgamma_queried_filter_t[]){
    {
      .priority = UINT64_MAX,
      .class = "a::b::c",
      .ramps.d = {
	.red_size = 4,
	.green_size = 5,
	.blue_size = 6,
	.red = (double[]){0.0, 0.1, 0.2, 0.5,
			  0.3, 0.11, 0.22, 0.45, 0.9,
			  -1, -0.5, 0, 0.5, 1, 1.5}
      }
    }, {
      .priority = UINT64_MAX - 1,
      .class = NULL,
      .ramps.d = {
	.red_size = 4,
	.green_size = 5,
	.blue_size = 6,
	.red = (double[]){0.02, 0.12, 0.22, 0.52,
			  0.32, 0.112, 0.222, 0.452, 0.92,
			  -12, -0.52, 0.2, 0.52, 12, 1.52}
      }
    }
  };
  table1.depth = LIBCOOPGAMMA_DOUBLE;
  table1.filters[0].ramps.d.green = table1.filters[0].ramps.d.red + table1.filters[0].ramps.d.red_size;
  table1.filters[1].ramps.d.green = table1.filters[1].ramps.d.red + table1.filters[1].ramps.d.red_size;
  table1.filters[0].ramps.d.blue = table1.filters[0].ramps.d.green + table1.filters[0].ramps.d.green_size;
  table1.filters[1].ramps.d.blue = table1.filters[1].ramps.d.green + table1.filters[1].ramps.d.green_size;
  
  ctx1.error.number = UINT64_MAX;
  ctx1.error.custom = 1;
  ctx1.error.server_side = 0;
  ctx1.error.description = "description";
  ctx1.fd = 3;
  ctx1.have_all_headers = 1;
  ctx1.bad_message = 0;
  ctx1.blocking = 2;
  ctx1.message_id = UINT32_MAX;
  ctx1.in_response_to = UINT32_MAX - 1;
  ctx1.outbound = "0123456789";
  ctx1.outbound_head = 7;
  ctx1.outbound_tail = 2;
  ctx1.outbound_size = 10;
  ctx1.inbound = "abcdefghi";
  ctx1.inbound_head = 6;
  ctx1.inbound_tail = 3;
  ctx1.inbound_size = 9;
  ctx1.length = 100;
  ctx1.curline = 5;
  
  async1.message_id = UINT32_MAX;
  async1.coalesce = 1;
  
  n  = libcoopgamma_filter_marshal(&filter1, NULL);
  n += libcoopgamma_crtc_info_marshal(&crtc1, NULL);
  n += libcoopgamma_filter_query_marshal(&query1, NULL);
  n += libcoopgamma_filter_table_marshal(&table1, NULL);
  n += libcoopgamma_context_marshal(&ctx1, NULL);
  n += libcoopgamma_async_context_marshal(&async1, NULL);
  
  buf = malloc(n);
  n  = libcoopgamma_filter_marshal(&filter1, buf);
  n += libcoopgamma_crtc_info_marshal(&crtc1, buf + n);
  n += libcoopgamma_filter_query_marshal(&query1, buf + n);
  n += libcoopgamma_filter_table_marshal(&table1, buf + n);
  n += libcoopgamma_context_marshal(&ctx1, buf + n);
  n += libcoopgamma_async_context_marshal(&async1, buf + n);
  
  if (libcoopgamma_filter_unmarshal(&filter2, buf, &m))           return 1; else n = m;
  if (libcoopgamma_crtc_info_unmarshal(&crtc2, buf + n, &m))      return 2; else n += m;
  if (libcoopgamma_filter_query_unmarshal(&query2, buf + n, &m))  return 3; else n += m;
  if (libcoopgamma_filter_table_unmarshal(&table2, buf + n, &m))  return 4; else n += m;
  if (libcoopgamma_context_unmarshal(&ctx2, buf + n, &m))         return 5; else n += m;
  if (libcoopgamma_async_context_unmarshal(&async2, buf + n, &m)) return 6;
  free(buf);
  
  if ((filter1.priority != filter2.priority) ||
      (!streq(filter1.crtc, filter2.crtc)) ||
      (!streq(filter1.class, filter2.class)) ||
      (filter1.lifespan != filter2.lifespan) ||
      (filter1.depth != filter2.depth) ||
      (!rampseq(&(filter1.ramps), &(filter2.ramps), filter1.depth)))
    return 7;
  
  if ((crtc1.cooperative != crtc2.cooperative) ||
      (crtc1.depth != crtc2.depth) ||
      (crtc1.supported != crtc2.supported) ||
      (crtc1.red_size != crtc2.red_size) ||
      (crtc1.green_size != crtc2.green_size) ||
      (crtc1.blue_size != crtc2.blue_size) ||
      (crtc1.colourspace != crtc2.colourspace) ||
      (crtc1.have_gamut != crtc2.have_gamut) ||
      (crtc1.red_x != crtc2.red_x) ||
      (crtc1.red_y != crtc2.red_y) ||
      (crtc1.green_x != crtc2.green_x) ||
      (crtc1.green_y != crtc2.green_y) ||
      (crtc1.blue_x != crtc2.blue_x) ||
      (crtc1.blue_y != crtc2.blue_y) ||
      (crtc1.white_x != crtc2.white_x) ||
      (crtc1.white_y != crtc2.white_y))
    return 8;
  
  if ((query1.high_priority != query2.high_priority) ||
      (query1.low_priority != query2.low_priority) ||
      !streq(query1.crtc, query2.crtc) ||
      (query1.coalesce != query2.coalesce))
    return 9;
  
  if ((table1.red_size != table2.red_size) ||
      (table1.green_size != table2.green_size) ||
      (table1.blue_size != table2.blue_size) ||
      (table1.filter_count != table2.filter_count) ||
      (table1.depth != table2.depth))
    return 10;
  for (i = 0; i < table1.filter_count; i++)
    if ((table1.filters[i].priority != table2.filters[i].priority) ||
	!streq(table1.filters[i].class, table2.filters[i].class) ||
	!rampseq(&(table1.filters[i].ramps), &(table2.filters[i].ramps), table1.depth))
      return 11;
  
  if ((ctx1.error.number != ctx2.error.number) ||
      (ctx1.error.custom != ctx2.error.custom) ||
      (ctx1.error.server_side != ctx2.error.server_side) ||
      !streq(ctx1.error.description, ctx2.error.description))
    return 12;
  
  if ((ctx1.fd != ctx2.fd) ||
      (ctx1.have_all_headers != ctx2.have_all_headers) ||
      (ctx1.bad_message != ctx2.bad_message) ||
      (ctx1.blocking != ctx2.blocking) ||
      (ctx1.message_id != ctx2.message_id) ||
      (ctx1.in_response_to != ctx2.in_response_to) ||
      (ctx1.length != ctx2.length) ||
      (ctx1.curline != ctx2.curline))
    return 13;
  
  if ((ctx2.outbound_head > ctx2.outbound_size) ||
      (ctx2.outbound_tail > ctx2.outbound_size) ||
      (ctx2.inbound_head > ctx2.inbound_size) ||
      (ctx2.inbound_tail > ctx2.inbound_size))
    return 14;
  
  if (((n = ctx1.outbound_head - ctx1.outbound_tail) != ctx2.outbound_head - ctx2.outbound_tail) ||
      ((m = ctx1.inbound_head - ctx1.inbound_tail) != ctx2.inbound_head - ctx2.inbound_tail))
    return 15;
  
  if (memcmp(ctx1.outbound + ctx1.outbound_tail, ctx2.outbound + ctx2.outbound_tail, n) ||
      memcmp(ctx1.inbound + ctx1.inbound_tail, ctx2.inbound + ctx2.inbound_tail, m))
    return 16;
  
  if ((async1.message_id != async2.message_id) ||
      (async1.coalesce != async2.coalesce))
    return 17;
  
  return 0;
}


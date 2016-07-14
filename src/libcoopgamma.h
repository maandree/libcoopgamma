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
#ifndef LIBCOOPGAMMA_H
#define LIBCOOPGAMMA_H



#include <stddef.h>
#include <stdint.h>



typedef enum libcoopgamma_support
{
  LIBCOOPGAMMA_NO = 0,
  LIBCOOPGAMMA_MAYBE = 1,
  LIBCOOPGAMMA_YES = 2
} libcoopgamma_support_t;


typedef enum libcoopgamma_depth
{
  LIBCOOPGAMMA_UINT8 = 8,
  LIBCOOPGAMMA_UINT16 = 16,
  LIBCOOPGAMMA_UINT32 = 32,
  LIBCOOPGAMMA_UINT64 = 64,
  LIBCOOPGAMMA_FLOAT = -1,
  LIBCOOPGAMMA_DOUBLE = -2
} libcoopgamma_depth_t;


typedef enum libcoopgamma_lifespan
{
  LIBCOOPGAMMA_REMOVE = 0,
  LIBCOOPGAMMA_UNTIL_DEATH = 1,
  LIBCOOPGAMMA_UNTIL_REMOVAL = 2
} libcoopgamma_lifespan_t;


typedef struct libcoopgamma_context
{
  int fd;
} libcoopgamma_context_t;


#define LIBCOOPGAMMA_RAMPS__(suffix, type)	\
typedef struct libcoopgamma_ramps##suffix	\
{						\
  size_t red_size;				\
  size_t green_size;				\
  size_t blue_size;				\
  type* red;					\
  type* green;					\
  type* blue;					\
} libcoopgamma_ramps##suffix##_t;
LIBCOOPGAMMA_RAMPS__(8,  uint8_t);
LIBCOOPGAMMA_RAMPS__(16, uint16_t);
LIBCOOPGAMMA_RAMPS__(32, uint32_t);
LIBCOOPGAMMA_RAMPS__(64, uint64_t);
LIBCOOPGAMMA_RAMPS__(f,  float);
LIBCOOPGAMMA_RAMPS__(d,  double);


typedef struct libcoopgamma_filter
{
  libcoopgamma_depth_t depth;
  int64_t priority;
  char* crtc;
  char* class;
  libcoopgamma_lifespan_t lifespan;
  union
  {
    libcoopgamma_ramps8_t u8;
    libcoopgamma_ramps16_t u16;
    libcoopgamma_ramps32_t u32;
    libcoopgamma_ramps64_t u64;
    libcoopgamma_rampsf_t f;
    libcoopgamma_rampsd_t d;
  } ramps;
} libcoopgamma_filter_t;


typedef struct libcoopgamma_crtc_info
{
  int cooperative;
  libcoopgamma_depth_t depth;
  size_t red_size;
  size_t green_size;
  size_t blue_size;
  libcoopgamma_support_t supported;
} libcoopgamma_crtc_info_t;


typedef struct libcoopgamma_filter_query
{
  char* crtc;
  int coalesce;
  int64_t high_priority;
  int64_t low_priority;
} libcoopgamma_filter_query_t;


typedef struct libcoopgamma_queried_filter
{
  int64_t priority;
  char* class;
  union
  {
    libcoopgamma_ramps8_t u8;
    libcoopgamma_ramps16_t u16;
    libcoopgamma_ramps32_t u32;
    libcoopgamma_ramps64_t u64;
    libcoopgamma_rampsf_t f;
    libcoopgamma_rampsd_t d;
  } ramps;
} libcoopgamma_queried_filter_t;


typedef struct libcoopgamma_filter_table
{
  libcoopgamma_depth_t depth;
  size_t red_size;
  size_t green_size;
  size_t blue_size;
  size_t filter_count;
  libcoopgamma_queried_filter_t* filters;
} libcoopgamma_filter_table_t;


typedef struct libcoopgamma_error
{
  uint64_t number;
  int custom;
  char* description;
} libcoopgamma_error_t;



#endif


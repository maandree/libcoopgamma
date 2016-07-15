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

#include <sys/socket.h>
#include <sys/wait.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>




#if defined(__GNUC__)
# define NAME_OF_THE_PROCESS  (argv0)
char* argv0 __attribute__((weak)) = "libcoopgamma";
#else
# define NAME_OF_THE_PROCESS  ("libcoopgamma")
#endif



#define SUBBUF  \
  (buf ? buf + off : NULL)

#define MARSHAL_PROLOGUE	\
  char* restrict buf = vbuf;	\
  size_t off = 0;

#define UNMARSHAL_PROLOGUE		\
  const char* restrict buf = vbuf;	\
  size_t off = 0;

#define MARSHAL_EPILOGUE  \
  return off

#define UNMARSHAL_EPILOGUE  \
  return *np = off, LIBCOOPGAMMA_SUCCESS

#define marshal_prim(datum, type)  \
  ((buf != NULL ? *(type*)(buf + off) = (datum) : 0), off += sizeof(type))

#define unmarshal_prim(datum, type)  \
  ((datum) = *(type*)(buf + off), off += sizeof(type))

#define marshal_version(version)  \
  marshal_prim(version, int)

#define unmarshal_version(version)			\
  do							\
    {							\
      int version__;					\
      unmarshal_prim(version__, int);			\
      if (version__ < version)				\
	return LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE;	\
      if (version__ > version)				\
	return LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE;	\
    }							\
  while (0)

#define marshal_buffer(data, n)  \
  ((buf != NULL ? (memcpy(buf + off, (data), (n)), 0) : 0), off += (n))

#define unmarshal_buffer(data, n)	\
  do					\
    {					\
      (data) = malloc(n);		\
      if ((data) == NULL)		\
	return LIBCOOPGAMMA_ERRNO_SET;	\
      memcpy((data), buf + off, (n));	\
      off += (n);			\
    }					\
  while (0)

#define marshal_string(datum)						\
  ((datum) == NULL ? marshal_prim(1, char) :				\
   (marshal_prim(0, char), marshal_buffer(datum, strlen(datum) + 1)))

#define unmarshal_string(datum)					\
  do								\
    {								\
      char nonnull__;						\
      unmarshal_prim(nonnull__, char);				\
      if (nonnull__)						\
	unmarshal_buffer((datum), strlen(buf + off) + 1);	\
      else							\
	(datum) = NULL;						\
    }								\
  while (0)



#define copy_errno(ctx)				\
  ((errno == 0) ? NULL :			\
   ((ctx)->error.number = (uint64_t)errno,	\
    (ctx)->error.custom = 0,			\
    (ctx)->error.server_side = 0,		\
    free((ctx)->error.description),		\
    (ctx)->error.description = NULL))



/**
 * Initialise a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t`
 * 
 * `this->red_size`, `this->green_size`, and `this->blue_size` must already be set
 * 
 * @param   this   The record to initialise
 * @para    width  The `sizeof(*(this->red))`
 * @return         Zero on success, -1 on error
 */
int (libcoopgamma_ramps_initialise)(void* restrict this, size_t width)
{
  libcoopgamma_ramps8_t* restrict this8 = (libcoopgamma_ramps8_t* restrict)this;
  this8->red = this8->green = this8->blue = NULL;
  this8->red = malloc((this8->red_size + this8->green_size + this8->blue_size) * width);
  if (this8->red == NULL)
    return -1;
  this8->green = this8->red   + this8->red_size   * width;
  this8->blue  = this8->green + this8->green_size * width;
  return 0;
}


/**
 * Release all resources allocated to  a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`,
 * `libcoopgamma_ramps32_t`, `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or
 * `libcoopgamma_rampsd_t`, the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_ramps_initialise`
 * or failed call to `libcoopgamma_ramps_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_ramps_destroy(void* restrict this)
{
  libcoopgamma_ramps8_t* restrict this8 = (libcoopgamma_ramps8_t* restrict)this;
  free(this8->red);
  this8->red = this8->green = this8->blue = NULL;
}


/**
 * Marshal a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t` into a buffer
 * 
 * @param   this   The record to marshal
 * @param   vbuf    The output buffer, `NULL` to only measure
 *                 how large this buffer has to be
 * @param   width  The `sizeof(*(this->red))`
 * @return         The number of marshalled bytes, or if `buf == NULL`,
 *                 how many bytes would be marshalled if `buf != NULL`
 */
size_t (libcoopgamma_ramps_marshal)(const void* restrict this, void* restrict vbuf, size_t width)
{
  libcoopgamma_ramps8_t* restrict this8 = (libcoopgamma_ramps8_t* restrict)this;
  MARSHAL_PROLOGUE;
  marshal_version(LIBCOOPGAMMA_RAMPS_VERSION);
  marshal_prim(this8->red_size, size_t);
  marshal_prim(this8->green_size, size_t);
  marshal_prim(this8->blue_size, size_t);
  marshal_buffer(this8->red, (this8->red_size + this8->green_size + this8->blue_size) * width);
  MARSHAL_EPILOGUE;
}


/**
 * Unmarshal a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t` from a buffer
 * 
 * @param   this   The output paramater for unmarshalled record
 * @param   vbuf   The buffer with the marshalled record
 * @param   np     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @param   width  The `sizeof(*(this->red))`
 * @return         `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                 `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int (libcoopgamma_ramps_unmarshal)(void* restrict this, const void* restrict vbuf,
				   size_t* restrict np, size_t width)
{
  libcoopgamma_ramps8_t* restrict this8 = (libcoopgamma_ramps8_t* restrict)this;
  UNMARSHAL_PROLOGUE;
  unmarshal_version(LIBCOOPGAMMA_RAMPS_VERSION);
  unmarshal_prim(this8->red_size, size_t);
  unmarshal_prim(this8->green_size, size_t);
  unmarshal_prim(this8->blue_size, size_t);
  unmarshal_buffer(this8->red, (this8->red_size + this8->green_size + this8->blue_size) * width);
  UNMARSHAL_EPILOGUE;
}



/**
 * Initialise a `libcoopgamma_filter_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_filter_initialise(libcoopgamma_filter_t* restrict this)
{
  memset(this, 0, sizeof(*this));
  return 0;
}


/**
 * Release all resources allocated to  a `libcoopgamma_filter_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_filter_initialise`
 * or failed call to `libcoopgamma_filter_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_filter_destroy(libcoopgamma_filter_t* restrict this)
{
  free(this->crtc);
  free(this->class);
  free(this->ramps.u8.red);
  memset(this, 0, sizeof(*this));
}


/**
 * Marshal a `libcoopgamma_filter_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_filter_marshal(const libcoopgamma_filter_t* restrict this, void* restrict vbuf)
{
  MARSHAL_PROLOGUE;
  marshal_version(LIBCOOPGAMMA_FILTER_VERSION);
  marshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
  marshal_version(LIBCOOPGAMMA_LIFESPAN_VERSION);
  marshal_prim(this->depth, libcoopgamma_depth_t);
  marshal_prim(this->priority, int64_t);
  marshal_string(this->crtc);
  marshal_string(this->class);
  marshal_prim(this->lifespan, libcoopgamma_lifespan_t);
  switch (this->depth)
    {
    case LIBCOOPGAMMA_UINT8:   off += libcoopgamma_ramps_marshal(&(this->ramps.u8),  SUBBUF);  break;
    case LIBCOOPGAMMA_UINT16:  off += libcoopgamma_ramps_marshal(&(this->ramps.u16), SUBBUF);  break;
    case LIBCOOPGAMMA_UINT32:  off += libcoopgamma_ramps_marshal(&(this->ramps.u32), SUBBUF);  break;
    case LIBCOOPGAMMA_UINT64:  off += libcoopgamma_ramps_marshal(&(this->ramps.u64), SUBBUF);  break;
    case LIBCOOPGAMMA_FLOAT:   off += libcoopgamma_ramps_marshal(&(this->ramps.f),   SUBBUF);  break;
    case LIBCOOPGAMMA_DOUBLE:  off += libcoopgamma_ramps_marshal(&(this->ramps.d),   SUBBUF);  break;
    default:
      break;
    }
  MARSHAL_EPILOGUE;
}


/**
 * Unmarshal a `libcoopgamma_filter_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_filter_unmarshal(libcoopgamma_filter_t* restrict this,
				  const void* restrict vbuf, size_t* restrict np)
{
  int r = LIBCOOPGAMMA_SUCCESS;
  size_t n = 0;
  UNMARSHAL_PROLOGUE;
  memset(this, 0, sizeof(*this));
  unmarshal_version(LIBCOOPGAMMA_FILTER_VERSION);
  unmarshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
  unmarshal_version(LIBCOOPGAMMA_LIFESPAN_VERSION);
  unmarshal_prim(this->depth, libcoopgamma_depth_t);
  unmarshal_prim(this->priority, int64_t);
  unmarshal_string(this->crtc);
  unmarshal_string(this->class);
  unmarshal_prim(this->lifespan, libcoopgamma_lifespan_t);
  switch (this->depth)
    {
    case LIBCOOPGAMMA_UINT8:   r = libcoopgamma_ramps_unmarshal(&(this->ramps.u8),  SUBBUF, &n);  break;
    case LIBCOOPGAMMA_UINT16:  r = libcoopgamma_ramps_unmarshal(&(this->ramps.u16), SUBBUF, &n);  break;
    case LIBCOOPGAMMA_UINT32:  r = libcoopgamma_ramps_unmarshal(&(this->ramps.u32), SUBBUF, &n);  break;
    case LIBCOOPGAMMA_UINT64:  r = libcoopgamma_ramps_unmarshal(&(this->ramps.u64), SUBBUF, &n);  break;
    case LIBCOOPGAMMA_FLOAT:   r = libcoopgamma_ramps_unmarshal(&(this->ramps.f),   SUBBUF, &n);  break;
    case LIBCOOPGAMMA_DOUBLE:  r = libcoopgamma_ramps_unmarshal(&(this->ramps.d),   SUBBUF, &n);  break;
    default:
      break;
    }
  if (r != LIBCOOPGAMMA_SUCCESS)
    return r;
  off += n;
  UNMARSHAL_EPILOGUE;
}



/**
 * Initialise a `libcoopgamma_crtc_info_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_crtc_info_initialise(libcoopgamma_crtc_info_t* restrict this)
{
  memset(this, 0, sizeof(*this));
  return 0;
}


/**
 * Release all resources allocated to  a `libcoopgamma_crtc_info_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_crtc_info_initialise`
 * or failed call to `libcoopgamma_crtc_info_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_crtc_info_destroy(libcoopgamma_crtc_info_t* restrict this)
{
  (void) this;
}


/**
 * Marshal a `libcoopgamma_crtc_info_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_crtc_info_marshal(const libcoopgamma_crtc_info_t* restrict this, void* restrict vbuf)
{
  MARSHAL_PROLOGUE;
  marshal_version(LIBCOOPGAMMA_CRTC_INFO_VERSION);
  marshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
  marshal_version(LIBCOOPGAMMA_SUPPORT_VERSION);
  marshal_prim(this->cooperative, int);
  marshal_prim(this->depth, libcoopgamma_depth_t);
  marshal_prim(this->red_size, size_t);
  marshal_prim(this->green_size, size_t);
  marshal_prim(this->blue_size, size_t);
  marshal_prim(this->supported, libcoopgamma_support_t);
  MARSHAL_EPILOGUE;
}


/**
 * Unmarshal a `libcoopgamma_crtc_info_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_crtc_info_unmarshal(libcoopgamma_crtc_info_t* restrict this,
				     const void* restrict vbuf, size_t* restrict np)
{
  UNMARSHAL_PROLOGUE;
  unmarshal_version(LIBCOOPGAMMA_CRTC_INFO_VERSION);
  unmarshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
  unmarshal_version(LIBCOOPGAMMA_SUPPORT_VERSION);
  unmarshal_prim(this->cooperative, int);
  unmarshal_prim(this->depth, libcoopgamma_depth_t);
  unmarshal_prim(this->red_size, size_t);
  unmarshal_prim(this->green_size, size_t);
  unmarshal_prim(this->blue_size, size_t);
  unmarshal_prim(this->supported, libcoopgamma_support_t);
  UNMARSHAL_EPILOGUE;
}



/**
 * Initialise a `libcoopgamma_filter_query_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_filter_query_initialise(libcoopgamma_filter_query_t* restrict this)
{
  this->crtc = NULL;
  this->coalesce = 0;
  this->high_priority = INT64_MAX;
  this->low_priority = INT64_MIN;
  return 0;
}


/**
 * Release all resources allocated to  a `libcoopgamma_filter_query_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_filter_query_initialise`
 * or failed call to `libcoopgamma_filter_query_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_filter_query_destroy(libcoopgamma_filter_query_t* restrict this)
{
  free(this->crtc), this->crtc = NULL;
}


/**
 * Marshal a `libcoopgamma_filter_query_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_filter_query_marshal(const libcoopgamma_filter_query_t* restrict this, void* restrict vbuf)
{
  MARSHAL_PROLOGUE;
  marshal_version(LIBCOOPGAMMA_FILTER_QUERY_VERSION);
  marshal_string(this->crtc);
  marshal_prim(this->coalesce, int);
  marshal_prim(this->high_priority, int64_t);
  marshal_prim(this->low_priority, int64_t);
  MARSHAL_EPILOGUE;
}


/**
 * Unmarshal a `libcoopgamma_filter_query_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_filter_query_unmarshal(libcoopgamma_filter_query_t* restrict this,
					const void* restrict vbuf, size_t* restrict np)
{
  UNMARSHAL_PROLOGUE;
  this->crtc = NULL;
  unmarshal_version(LIBCOOPGAMMA_FILTER_QUERY_VERSION);
  unmarshal_string(this->crtc);
  unmarshal_prim(this->coalesce, int);
  unmarshal_prim(this->high_priority, int64_t);
  unmarshal_prim(this->low_priority, int64_t);
  UNMARSHAL_EPILOGUE;
}



/**
 * Initialise a `libcoopgamma_queried_filter_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_queried_filter_initialise(libcoopgamma_queried_filter_t* restrict this)
{
  memset(this, 0, sizeof(*this));
  return 0;
}


/**
 * Release all resources allocated to  a `libcoopgamma_queried_filter_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_queried_filter_initialise`
 * or failed call to `libcoopgamma_queried_filter_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_queried_filter_destroy(libcoopgamma_queried_filter_t* restrict this)
{
  free(this->class), this->class = NULL;
  libcoopgamma_ramps_destroy(&(this->ramps.u8));
}


/**
 * Marshal a `libcoopgamma_queried_filter_t` into a buffer
 * 
 * @param   this   The record to marshal
 * @param   vbuf   The output buffer, `NULL` to only measure
 *                 how large this buffer has to be
 * @param   depth  The type used of ramp stops
 * @return         The number of marshalled bytes, or if `buf == NULL`,
 *                 how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_queried_filter_marshal(const libcoopgamma_queried_filter_t* restrict this,
					   void* restrict vbuf, libcoopgamma_depth_t depth)
{
  MARSHAL_PROLOGUE;
  marshal_version(LIBCOOPGAMMA_QUERIED_FILTER_VERSION);
  marshal_prim(this->priority, int64_t);
  marshal_string(this->class);
  switch (depth)
    {
    case LIBCOOPGAMMA_UINT8:   off += libcoopgamma_ramps_marshal(&(this->ramps.u8),  SUBBUF);  break;
    case LIBCOOPGAMMA_UINT16:  off += libcoopgamma_ramps_marshal(&(this->ramps.u16), SUBBUF);  break;
    case LIBCOOPGAMMA_UINT32:  off += libcoopgamma_ramps_marshal(&(this->ramps.u32), SUBBUF);  break;
    case LIBCOOPGAMMA_UINT64:  off += libcoopgamma_ramps_marshal(&(this->ramps.u64), SUBBUF);  break;
    case LIBCOOPGAMMA_FLOAT:   off += libcoopgamma_ramps_marshal(&(this->ramps.f),   SUBBUF);  break;
    case LIBCOOPGAMMA_DOUBLE:  off += libcoopgamma_ramps_marshal(&(this->ramps.d),   SUBBUF);  break;
    default:
      break;
    }
  MARSHAL_EPILOGUE;
}


/**
 * Unmarshal a `libcoopgamma_queried_filter_t` from a buffer
 * 
 * @param   this   The output paramater for unmarshalled record
 * @param   vbuf   The buffer with the marshalled record
 * @param   np     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @param   depth  The type used of ramp stops
 * @return         `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                 `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_queried_filter_unmarshal(libcoopgamma_queried_filter_t* restrict this,
					  const void* restrict vbuf, size_t* restrict np,
					  libcoopgamma_depth_t depth)
{
  int r = LIBCOOPGAMMA_SUCCESS;
  size_t n = 0;
  UNMARSHAL_PROLOGUE;
  memset(this, 0, sizeof(*this));
  unmarshal_version(LIBCOOPGAMMA_QUERIED_FILTER_VERSION);
  unmarshal_prim(this->priority, int64_t);
  unmarshal_string(this->class);
  switch (depth)
    {
    case LIBCOOPGAMMA_UINT8:   r = libcoopgamma_ramps_unmarshal(&(this->ramps.u8),  SUBBUF, &n);  break;
    case LIBCOOPGAMMA_UINT16:  r = libcoopgamma_ramps_unmarshal(&(this->ramps.u16), SUBBUF, &n);  break;
    case LIBCOOPGAMMA_UINT32:  r = libcoopgamma_ramps_unmarshal(&(this->ramps.u32), SUBBUF, &n);  break;
    case LIBCOOPGAMMA_UINT64:  r = libcoopgamma_ramps_unmarshal(&(this->ramps.u64), SUBBUF, &n);  break;
    case LIBCOOPGAMMA_FLOAT:   r = libcoopgamma_ramps_unmarshal(&(this->ramps.f),   SUBBUF, &n);  break;
    case LIBCOOPGAMMA_DOUBLE:  r = libcoopgamma_ramps_unmarshal(&(this->ramps.d),   SUBBUF, &n);  break;
    default:
      break;
    }
  if (r != LIBCOOPGAMMA_SUCCESS)
    return r;
  off += n;
  UNMARSHAL_EPILOGUE;
}



/**
 * Initialise a `libcoopgamma_filter_table_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_filter_table_initialise(libcoopgamma_filter_table_t* restrict this)
{
  memset(this, 0, sizeof(*this));
  return 0;
}


/**
 * Release all resources allocated to  a `libcoopgamma_filter_table_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_filter_table_initialise`
 * or failed call to `libcoopgamma_filter_table_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_filter_table_destroy(libcoopgamma_filter_table_t* restrict this)
{
  while (this->filter_count)
    libcoopgamma_queried_filter_destroy(this->filters + --(this->filter_count));
  free(this->filters), this->filters = NULL;
}


/**
 * Marshal a `libcoopgamma_filter_table_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_filter_table_marshal(const libcoopgamma_filter_table_t* restrict this, void* restrict vbuf)
{
  size_t i;
  MARSHAL_PROLOGUE;
  marshal_version(LIBCOOPGAMMA_FILTER_TABLE_VERSION);
  marshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
  marshal_prim(this->depth, libcoopgamma_depth_t);
  marshal_prim(this->red_size, size_t);
  marshal_prim(this->green_size, size_t);
  marshal_prim(this->blue_size, size_t);
  marshal_prim(this->filter_count, size_t);
  for (i = 0; i < this->filter_count; i++)
    off += libcoopgamma_queried_filter_marshal(this->filters + i, SUBBUF, this->depth);
  MARSHAL_EPILOGUE;
}


/**
 * Unmarshal a `libcoopgamma_filter_table_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_filter_table_unmarshal(libcoopgamma_filter_table_t* restrict this,
					const void* restrict vbuf, size_t* restrict np)
{
  size_t i, n, fn;
  int r;
  UNMARSHAL_PROLOGUE;
  this->filter_count = 0;
  this->filters = NULL;
  unmarshal_version(LIBCOOPGAMMA_FILTER_TABLE_VERSION);
  unmarshal_version(LIBCOOPGAMMA_DEPTH_VERSION);
  unmarshal_prim(this->depth, libcoopgamma_depth_t);
  unmarshal_prim(this->red_size, size_t);
  unmarshal_prim(this->green_size, size_t);
  unmarshal_prim(this->blue_size, size_t);
  unmarshal_prim(fn, size_t);
  this->filters = malloc(fn * sizeof(*(this->filters)));
  if (this->filters == NULL)
    return LIBCOOPGAMMA_ERRNO_SET;
  for (i = 0; i < fn; i++)
    {
      r = libcoopgamma_queried_filter_unmarshal(this->filters + i, SUBBUF, &n, this->depth);
      if (r != LIBCOOPGAMMA_SUCCESS)
	return r;
      off += n;
      this->filter_count += 1;
    }
  UNMARSHAL_EPILOGUE;
}



/**
 * Initialise a `libcoopgamma_error_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_error_initialise(libcoopgamma_error_t* restrict this)
{
  this->number = 0;
  this->custom = 0;
  this->description = NULL;
  return 0;
}


/**
 * Release all resources allocated to  a `libcoopgamma_error_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_error_initialise`
 * or failed call to `libcoopgamma_error_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_error_destroy(libcoopgamma_error_t* restrict this)
{
  free(this->description), this->description = NULL;
}


/**
 * Marshal a `libcoopgamma_error_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_error_marshal(const libcoopgamma_error_t* restrict this, void* restrict vbuf)
{
  MARSHAL_PROLOGUE;
  marshal_version(LIBCOOPGAMMA_ERROR_VERSION);
  marshal_prim(this->number, uint64_t);
  marshal_prim(this->custom, int);
  marshal_prim(this->server_side, int);
  marshal_string(this->description);
  MARSHAL_EPILOGUE;
}


/**
 * Unmarshal a `libcoopgamma_error_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_error_unmarshal(libcoopgamma_error_t* restrict this,
				 const void* restrict vbuf, size_t* restrict np)
{
  UNMARSHAL_PROLOGUE;
  this->description = NULL;
  unmarshal_version(LIBCOOPGAMMA_ERROR_VERSION);
  unmarshal_prim(this->number, uint64_t);
  unmarshal_prim(this->custom, int);
  unmarshal_string(this->description);
  UNMARSHAL_EPILOGUE;
}



/**
 * Initialise a `libcoopgamma_context_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_context_initialise(libcoopgamma_context_t* restrict this)
{
  memset(this, 0, sizeof(*this));
  this->fd = -1;
  return 0;
}


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
void libcoopgamma_context_destroy(libcoopgamma_context_t* restrict this, int disconnect)
{
  if (disconnect && (this->fd >= 0))
    {
      shutdown(this->fd, SHUT_RDWR);
      close(this->fd);
    }
  this->fd = -1;
  libcoopgamma_error_destroy(&(this->error));
  free(this->outbound), this->outbound = NULL;
  free(this->inbound), this->inbound = NULL;
}


/**
 * Marshal a `libcoopgamma_context_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_context_marshal(const libcoopgamma_context_t* restrict this, void* restrict vbuf)
{
  MARSHAL_PROLOGUE;
  marshal_version(LIBCOOPGAMMA_CONTEXT_VERSION);
  marshal_prim(this->fd, int);
  off += libcoopgamma_error_marshal(&(this->error), SUBBUF);
  marshal_prim(this->message_id, uint32_t);
  marshal_prim(this->outbound_head - this->outbound_tail, size_t);
  marshal_buffer(this->outbound + this->outbound_head, this->outbound_head - this->outbound_tail);
  marshal_prim(this->inbound_head - this->inbound_tail, size_t);
  marshal_buffer(this->inbound + this->inbound_head, this->inbound_head - this->inbound_tail);
  marshal_prim(this->length, size_t);
  marshal_prim(this->curline, size_t);
  marshal_prim(this->in_response_to, uint32_t);
  marshal_prim(this->have_all_headers, int);
  marshal_prim(this->bad_message, int);
  MARSHAL_EPILOGUE;
}


/**
 * Unmarshal a `libcoopgamma_context_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_context_unmarshal(libcoopgamma_context_t* restrict this,
				   const void* restrict vbuf, size_t* restrict np)
{
  size_t n;
  int r;
  UNMARSHAL_PROLOGUE;
  memset(this, 0, sizeof(*this));
  unmarshal_version(LIBCOOPGAMMA_CONTEXT_VERSION);
  unmarshal_prim(this->fd, int);
  r = libcoopgamma_error_unmarshal(&(this->error), SUBBUF, &n);
  if (r != LIBCOOPGAMMA_SUCCESS)
    return r;
  off += n;
  unmarshal_prim(this->message_id, uint32_t);
  unmarshal_prim(this->inbound_head, size_t);
  this->outbound_size = this->outbound_head;
  unmarshal_buffer(this->outbound, this->outbound_head);
  this->inbound_size = this->inbound_head;
  unmarshal_buffer(this->inbound, this->inbound_head);
  unmarshal_prim(this->length, size_t);
  unmarshal_prim(this->curline, size_t);
  unmarshal_prim(this->in_response_to, uint32_t);
  unmarshal_prim(this->have_all_headers, int);
  unmarshal_prim(this->bad_message, int);
  UNMARSHAL_EPILOGUE;
}



/**
 * Initialise a `libcoopgamma_async_context_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_async_context_initialise(libcoopgamma_async_context_t* restrict this)
{
  this->message_id = 0;
  this->coalesce = 0;
  return 0;
}


/**
 * Release all resources allocated to  a `libcoopgamma_async_context_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_async_context_initialise`
 * or failed call to `libcoopgamma_async_context_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_async_context_destroy(libcoopgamma_async_context_t* restrict this)
{
  (void) this;
}


/**
 * Marshal a `libcoopgamma_async_context_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   vbuf  The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_async_context_marshal(const libcoopgamma_async_context_t* restrict this,
					  void* restrict vbuf)
{
  MARSHAL_PROLOGUE;
  marshal_version(LIBCOOPGAMMA_ASYNC_CONTEXT_VERSION);
  marshal_prim(this->message_id, uint32_t);
  marshal_prim(this->coalesce, int);
  MARSHAL_EPILOGUE;
}


/**
 * Unmarshal a `libcoopgamma_async_context_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   vbuf  The buffer with the marshalled record
 * @param   np    Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_async_context_unmarshal(libcoopgamma_async_context_t* restrict this,
					 const void* restrict vbuf, size_t* restrict np)
{
  UNMARSHAL_PROLOGUE;
  unmarshal_version(LIBCOOPGAMMA_ASYNC_CONTEXT_VERSION);
  unmarshal_prim(this->message_id, uint32_t);
  unmarshal_prim(this->coalesce, int);
  UNMARSHAL_EPILOGUE;
}



/**
 * List all recognised adjustment method
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @return  A `NULL`-terminated list of names. You should only free
 *          the outer pointer, inner pointers are subpointers of the
 *          outer pointer and cannot be freed. `NULL` on error.
 */
char** libcoopgamma_get_methods(void)
{
  char num[5]; /* The size is base on the fact that we have limited `n` in the loop below */
  char** methods = NULL;
  char** rc;
  char* buffer;
  int n = 0, saved_errno;
  size_t size;
  
  methods = malloc(4 * sizeof(*methods));
  if (methods == NULL)
    goto fail;
  
  for (n = 0; n < 10000 /* just to be safe */; n++)
    {
      char* method;
      if ((n >= 4) && ((n & -n) == n))
	{
	  void* new = realloc(methods, (n << 1) * sizeof(*methods));
	  if (new == NULL)
	    goto fail;
	  methods = new;
	}
      sprintf(num, "%i", n);
      if (libcoopgamma_get_method_and_site(num, NULL, &method, NULL))
	goto fail;
      if (!strcmp(method, num))
	{
	  free(method);
	  break;
	}
      methods[n] = method;
      size += strlen(method) + 1;
    }
  
  rc = malloc((n + 1) * sizeof(char*) + size);
  if (rc == NULL)
    goto fail;
  buffer = ((char*)rc) + (n + 1) * sizeof(char*);
  rc[n] = NULL;
  while (n--)
    {
      buffer = stpcpy(buffer, methods[n]) + 1;
      free(methods[n]);
    }
  free(methods);
  
  return rc;
  
 fail:
  saved_errno = errno;
  while (n--)
    free(methods[n]);
  free(methods);
  errno = saved_errno;
  return NULL;
}


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
static char* libcoopgamma_query(const char* restrict method, const char* restrict site, const char* restrict arg)
{
  const char* (args[7]) = {"coopgammad", arg};
  size_t i = 2, n = 0, size = 0;
  int pipe_rw[2] = { -1, -1 };
  pid_t pid;
  int saved_errno, status;
  char* msg = NULL;
  ssize_t got;
  
  if (method != NULL)  args[i++] = "-m", args[i++] = method;
  if (site   != NULL)  args[i++] = "-s", args[i++] = site;
  
  args[i] = NULL;
  
  if (pipe(pipe_rw) < 0)
    goto fail;
  
  switch ((pid = fork()))
    {
    case -1:
      goto fail;
    case 0:
      /* Child */
      close(pipe_rw[0]);
      if (pipe_rw[1] != STDOUT_FILENO)
	{
	  close(STDOUT_FILENO);
	  if (dup2(pipe_rw[1], STDOUT_FILENO) < 0)
	    goto fail_child;
	  close(pipe_rw[1]);
	}
      execvp("coopgammad", (char* const*)(args));
    fail_child:
      saved_errno = errno;
      perror(NAME_OF_THE_PROCESS);
      if (write(STDOUT_FILENO, &saved_errno, sizeof(int)) != sizeof(int))
	perror(NAME_OF_THE_PROCESS);
      exit(1);
    default:
      /* Parent */
      close(pipe_rw[1]), pipe_rw[1] = -1;
      for (;;)
	{
	  if (n == size)
	    {
	      void* new = realloc(msg, size = (n ? (n << 1) : 256));
	      if (new == NULL)
		goto fail;
	      msg = new;
	    }
	  got = read(pipe_rw[0], msg + n, size - n);
	  if (got < 0)
	    {
	      if (errno == EINTR)
		continue;
	      goto fail;
	    }
	  else if (got == 0)
	    break;
	  n += (size_t)got;
	}
      close(pipe_rw[0]), pipe_rw[0] = -1;
      if (waitpid(pid, &status, 0) < 0)
	goto fail;
      if (status)
	{
	  errno = EPIPE;
	  if ((n == sizeof(int)) && (*(int*)msg != 0))
	    errno = *(int*)msg;
	}
      break;
    }
  
  if (n == size)
    {
      void* new = realloc(msg, n + 1);
      if (new == NULL)
	goto fail;
      msg = new;
    }
  msg[n] = '\0';
  
  if (strchr(msg, '\0') != msg + n)
    {
      errno = EBADMSG;
      goto fail;
    }
  
  return msg;
 fail:
  saved_errno = errno;
  if (pipe_rw[0] >= 0)  close(pipe_rw[0]);
  if (pipe_rw[1] >= 0)  close(pipe_rw[1]);
  free(msg);
  errno = saved_errno;
  return NULL;
}


/**
 * Get the adjustment method and site
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @param   method   The adjustment method, `NULL` for automatic
 * @param   site     The site, `NULL` for automatic
 * @param   methodp  Output pointer for the selected adjustment method,
 *                   which cannot be `NULL`. It is safe to call
 *                   this function with this parameter set to `NULL`.
 * @param   sitep    Output pointer for the selected site, which will
 *                   be `NULL` the method only supports one site or if
 *                   `site == NULL` and no site can be selected
 *                   automatically. It is safe to call this function
 *                   with this parameter set to `NULL`.
 * @return           Zero on success, -1 on error
 */
int libcoopgamma_get_method_and_site(const char* restrict method, const char* restrict site,
				     char** restrict methodp, char** restrict sitep)
{
  int saved_errno;
  char* raw;
  char* p;
  char* q;
  
  raw = libcoopgamma_query(method, site, "-q");
  if (raw == NULL)
    return -1;
  
  if (methodp != NULL)  *methodp = NULL;
  if (sitep   != NULL)  *sitep   = NULL;
  
  p = strchr(raw, '\n');
  if (p == NULL)
    {
      errno = EBADMSG;
      goto fail;
    }
  *p++ = '\0';
  
  if (methodp != NULL)
    {
      *methodp = malloc(strlen(raw) + 1);
      if (*methodp == NULL)
	goto fail;
      strcpy(*methodp, raw);
    }
  
  if ((site != NULL) && *(q = strchr(p, '\0') - 1))
    {
      if (*q != '\n')
	{
	   errno = EBADMSG;
	   goto fail;
	}
      *q = '\0';
      *sitep = malloc(strlen(p) + 1);
      if (*sitep == NULL)
	goto fail;
      strcpy(*sitep, p);
    }
  
  free(raw);
  return 0;
 fail:
  saved_errno = errno;
  if (methodp != NULL)
    free(*methodp), *methodp = NULL;
  free(raw);
  errno = saved_errno;
  return -1;
}


/**
 * Get the PID file of the coopgamma server
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @param   method   The adjustment method, `NULL` for automatic
 * @param   site     The site, `NULL` for automatic
 * @return           The pathname of the server's PID file, `NULL` on error
 *                   or if there server does not use PID files. The later
 *                   case is detected by checking that `errno` is set to 0.
 */
char* libcoopgamma_get_pid_file(const char* restrict method, const char* restrict site)
{
  char* path;
  size_t n;
  
  path = libcoopgamma_get_socket_file(method, site);
  if (path == NULL)
    return NULL;
  
  n = strlen(path);
  if (n < 7 || strcmp(path + n - 7, ".socket"))
    {
      free(path);
      errno = EBADMSG;
      return NULL;
    }
  
  strcpy(path + n - 7, ".pid");
  return path;
}


/**
 * Get the socket file of the coopgamma server
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @param   method   The adjustment method, `NULL` for automatic
 * @param   site     The site, `NULL` for automatic
 * @return           The pathname of the server's socket, `NULL` on error
 *                   or if there server does have its own socket. The later
 *                   case is detected by checking that `errno` is set to 0,
 *                   and is the case when communicating with a server in a
 *                   multi-server display server like mds.
 */
char* libcoopgamma_get_socket_file(const char* restrict method, const char* restrict site)
{
  int saved_errno;
  char* raw;
  char* p;
  
  raw = libcoopgamma_query(method, site, "-qq");
  if (raw == NULL)
    return NULL;
  
  p = strchr(raw, '\0') - 1;
  if (*p != '\n')
    {
      errno = EBADMSG;
      goto fail;
    }
  *p = '\0';
  if (*raw == '\0')
    {
      errno = EBADMSG;
      goto fail;
    }
  
  return raw;
 fail:
  saved_errno = errno;
  free(raw);
  errno = saved_errno;
  return NULL;
}



/**
 * Connect to a coopgamma server, and start it if necessary
 * 
 * Use `libcoopgamma_context_destroy` to disconnect
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @param   method  The adjustment method, `NULL` for automatic
 * @param   site    The site, `NULL` for automatic
 * @param   ctx     The state of the library, must be initialised
 * @return          Zero on success, -1 on error. On error, `errno` is set
 *                  to 0 if the server could not be initialised.
 */
int libcoopgamma_connect(const char* restrict method, const char* restrict site,
			 libcoopgamma_context_t* restrict ctx)
{
  /* TODO */
}


/**
 * By default communication is blocking, this function
 * can be used to switch between blocking and nonblocking
 * 
 * After setting the communication to nonblocking,
 * `libcoopgamma_flush`, `libcoopgamma_synchronise` and
 * and request-sending functions can fail with EAGAIN and
 * EWOULDBLOCK. It is safe to continue with `libcoopgamma_flush`
 * (for `libcoopgamma_flush` it selfand equest-sending functions)
 * or `libcoopgamma_synchronise` just like EINTR failure.
 * 
 * @param   ctx          The state of the library, must be connected
 * @param   nonblocking  Nonblocking mode?
 * @return               Zero on success, -1 on error
 */
int libcoopgamma_set_nonblocking(libcoopgamma_context_t* restrict ctx, int nonblocking)
{
  int flags = fcntl(ctx->fd, F_GETFL);
  if (nonblocking)
    flags |= O_NONBLOCK;
  else
    flags &= ~O_NONBLOCK;
  if (fcntl(ctx->fd, F_SETFL, flags) == -1)
    return -1;
  return 0;
}


/**
 * Send all pending outbound data
 * 
 * If this function or another function that sends a request
 * to the server fails with EINTR, call this function to
 * complete the transfer. The `async` parameter will always
 * be in a properly configured state if a function fails
 * with EINTR.
 * 
 * @param   ctx  The state of the library, must be connected
 * @return       Zero on success, -1 on error
 */
int libcoopgamma_flush(libcoopgamma_context_t* restrict ctx)
{
  ssize_t sent;
  size_t chunksize = ctx->outbound_head - ctx->outbound_tail;
  size_t sendsize;
  
  while (ctx->outbound_tail < ctx->outbound_head)
    {
      sendsize = ctx->outbound_head - ctx->outbound_tail;
      sendsize = sendsize < chunksize ? sendsize : chunksize;
      sent = send(ctx->fd, ctx->outbound + ctx->outbound_tail, sendsize, 0);
      if (sent < 0)
	{
	  if (errno != EMSGSIZE)
	    return -1;
	  if ((chunksize >>= 1) == 0)
	    return -1;
	  continue;
	}
      ctx->outbound_tail += (size_t)sent;
    }
  
  return 0;
}


/**
 * Wait for the next message to be received
 * 
 * @param   ctx       The state of the library, must be connected
 * @param   pending   Information for each pending request
 * @param   n         The number of elements in `pending`
 * @param   selected  The index of the element in `pending` which corresponds
 *                    to the first inbound message, note that this only means
 *                    that the message is not for any of the other request,
 *                    if the message is corrupt any of the listed requests can
 *                    be selected even if it is not for any of the requests.
 *                    Functions that parse the message will detect such corruption.
 * @return            Zero on success, -1 on error, -2 if the message is ignored
 *                    which happens if corresponding `libcoopgamma_async_context_t`
 *                    is not listed. If `-1` is returned, `errno` will be set,
 *                    if it is set to `ENOTRECOVERABLE` you have receive a corrupt
 *                    message and the context has been tainted beyond recover.
 */
int libcoopgamma_synchronise(libcoopgamma_context_t* restrict ctx,
			     libcoopgamma_async_context_t* restrict pending,
			     size_t n, size_t* restrict selected)
{
  char temp[3 * sizeof(size_t) + 1];
  ssize_t got;
  size_t i;
  char* p;
  char* line;
  char* value;
  
  if (ctx->inbound_head == ctx->inbound_tail)
    ctx->inbound_head = ctx->inbound_tail = 0;
  
  if (ctx->inbound_tail > 0)
    {
      memmove(ctx->inbound, ctx->inbound + ctx->inbound_tail, ctx->inbound_head -= ctx->inbound_tail);
      ctx->inbound_tail = 0;
    }
  
  for (;;)
    {
      if (ctx->inbound_head == ctx->inbound_size)
	{
	  size_t new_size = ctx->inbound_size ? (ctx->inbound_size << 1) : 1024;
	  void* new = realloc(ctx->inbound, new_size);
	  if (new == NULL)
	    return -1;
	  ctx->inbound = new;
	  ctx->inbound_size = new_size;
	}
      
      got = recv(ctx->fd, ctx->inbound + ctx->inbound_head, ctx->inbound_size - ctx->inbound_head, 0);
      if (got <= 0)
	{
	  if (got == 0)
	    errno = ECONNRESET;
	  return -1;
	}
      ctx->inbound_head += (size_t)got;
      
      while (ctx->have_all_headers == 0)
	{
	  line = ctx->inbound + ctx->curline;
	  p = memchr(line, '\n', ctx->inbound_head - ctx->curline);
	  if (p == NULL)
	    break;
	  if (memchr(line, '\0', ctx->inbound_head - ctx->curline) != NULL)
	    ctx->bad_message = 1;
	  *p++ = '\0';
	  ctx->curline += (size_t)(p - ctx->inbound);
	  if (!*line)
	    {
	      ctx->have_all_headers = 1;
	    }
	  else if (strstr(line, "In response to: ") == line)
	    {
	      value = line + (sizeof("In response to: ") - 1);
	      ctx->in_response_to = (uint32_t)atol(value);
	    }
	  else if (strstr(line, "Length: ") == line)
	    {
	      value = line + (sizeof("Length: ") - 1);
	      ctx->length = (size_t)atol(value);
	      sprintf(temp, "%zu", ctx->length);
	      if (strcmp(value, temp))
		goto fatal;
	    }
	}
      
      if (ctx->have_all_headers && (ctx->inbound_head >= ctx->curline + ctx->length))
	{
	  ctx->curline += ctx->length;
	  if (ctx->bad_message)
	    {
	      ctx->bad_message = 0;
	      ctx->have_all_headers = 0;
	      ctx->length = 0;
	      errno = EBADMSG;
	      return -1;
	    }
	  for (i = 0; i < n; i++)
	    if (pending[i].message_id == ctx->in_response_to)
	      {
		*selected = i;
		return 0;
	      }
	  *selected = 0;
	  return -2;
	}
    }
  
 fatal:
  errno = ENOTRECOVERABLE;
  return -1;
}


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
#define SEND_MESSAGE(ctx, payload, payload_size, format, ...)		\
  do									\
    {									\
      ssize_t n__;							\
      char* msg__;							\
      snprintf(NULL, 0, format "%zn", __VA_ARGS__, &n__);		\
      msg__ = malloc((size_t)n__ + (payload_size));			\
      if (msg__ == NULL)						\
	goto fail;							\
      sprintf(msg__, format, __VA_ARGS__);				\
      if ((payload) != NULL)						\
	memcpy(msg__ + n__, (payload), (payload_size));			\
      if (send_message((ctx), msg__, (size_t)n__ + (payload_size)) < 0)	\
	goto fail;							\
      free(msg__);							\
    }									\
  while (0)


/**
 * Send a message to the server and wait for response
 * 
 * @param   ctx  The state of the library
 * @param   msg  The message to send
 * @param   n    The length of `msg`
 * @return       Zero on success, -1 on error
 */
static int send_message(libcoopgamma_context_t* restrict ctx, char* msg, size_t n)
{
  if (ctx->outbound_head == ctx->outbound_tail)
    {
      free(ctx->outbound);
      ctx->outbound = msg;
      ctx->outbound_tail = 0;
      ctx->outbound_head = n;
      ctx->outbound_size = n;
    }
  else
    {
      if (ctx->outbound_head + n > ctx->outbound_size)
	{
	  memmove(ctx->outbound, ctx->outbound + ctx->outbound_tail, ctx->outbound_head -= ctx->outbound_tail);
	  ctx->outbound_tail = 0;
	}
      if (ctx->outbound_head + n > ctx->outbound_size)
	{
	  void* new = realloc(ctx->outbound, ctx->outbound_head + n);
	  if (new == NULL)
	    {
	      int saved_errno = errno;
	      free(msg);
	      errno = saved_errno;
	      return -1;
	    }
	  ctx->outbound = new;
	  ctx->outbound_size = ctx->outbound_head + n;
	}
      memcpy(ctx->outbound + ctx->outbound_head, msg, n);
      ctx->outbound_head += n;
      ctx->message_id += 1;
      free(msg);
    }
  return libcoopgamma_flush(ctx);
}


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
static char* next_header(libcoopgamma_context_t* restrict ctx)
{
  char* rc = ctx->inbound + ctx->inbound_tail;
  ctx->inbound_tail += strlen(ctx->inbound) + 1;
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
static char* next_payload(libcoopgamma_context_t* restrict ctx, size_t* n)
{
  ctx->have_all_headers = 0;
  if ((*n = ctx->length))
    {
      char* rc = ctx->inbound + ctx->inbound_tail;
      ctx->inbound_tail += *n;
      ctx->length = 0;
      return rc;
    }
  else
    return NULL;
}


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
static int check_error(libcoopgamma_context_t* restrict ctx, libcoopgamma_async_context_t* restrict async)
{
  char temp[3 * sizeof(uint64_t) + 1];
  size_t old_tail = ctx->inbound_tail;
  char* line;
  char* value;
  int command_ok = 0;
  int have_in_response_to = 0;
  int have_error = 0;
  int bad = 0;
  
  for (;;)
    {
      line = next_header(ctx);
      value = strchr(line, ':') + 2;
      if (!*line)
	break;
      else if (!strcmp(line, "Command: error"))
	command_ok = 1;
      else if (strstr(line, "In response to: ") == line)
	{
	  uint32_t id = (uint32_t)atol(value);
	  have_in_response_to = 1 + !!have_in_response_to;
	  if (id != async->message_id)
	    bad = 1;
	  else
	    {
	      sprintf(temp, "%" PRIu32, id);
	      if (strcmp(value, temp))
		bad = 1;
	    }
	}
      else if (strstr(line, "Error: ") == line)
	{
	  have_error = 1 + !!have_error;
	  ctx->error.server_side = 1;
	  ctx->error.custom = (strstr(value, "custom") == value);
	  if (ctx->error.custom)
	    {
	      if (value[6] == '\0')
		{
		  ctx->error.number = 0;
		  continue;
		}
	      else if (value[6] != ' ')
		{
		  bad = 1;
		  continue;
		}
	    }
	  ctx->error.number = (uint64_t)atoll(value);
	  sprintf(temp, "%" PRIu64, ctx->error.number);
	  if (strcmp(value, temp))
	    bad = 1;
	}
    }
  
  if (command_ok == 0)
    {
      ctx->inbound_tail = old_tail;
      return 0;
    }
  
  if (bad || (have_in_response_to != 1) || (have_error != 1))
    {
      errno = EBADMSG;
      copy_errno(ctx);
      return -1;
    }
  
  return 1;
}



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
int libcoopgamma_get_crtcs_send(libcoopgamma_context_t* restrict ctx,
				libcoopgamma_async_context_t* restrict async)
{
  async->message_id = ctx->message_id;
  SEND_MESSAGE(ctx, NULL, 0,
	       "Command: enumerate-crtcs\n"
	       "Message ID: %" PRIu32 "\n"
	       "\n",
	       ctx->message_id);
  
  return 0;
 fail:
  copy_errno(ctx);
  return -1;
}


/**
 * List all available CRTC:s, receive response part
 * 
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         A `NULL`-terminated list of names. You should only free
 *                 the outer pointer, inner pointers are subpointers of the
 *                 outer pointer and cannot be freed. `NULL` on error, in
 *                 which case `ctx->error` (rather than `errno`) is read
 *                 for information about the error.
 */
char** libcoopgamma_get_crtcs_recv(libcoopgamma_context_t* restrict ctx,
				   libcoopgamma_async_context_t* restrict async)
{
  char* line;
  char* payload;
  char* end;
  int command_ok = 0;
  size_t i, n, lines, len, length;
  char** rc;
  
  if (check_error(ctx, async))
    return NULL;
  
  for (;;)
    {
      line = next_header(ctx);
      if (!*line)
	break;
      else if (!strcmp(line, "Command: crtc-enumeration"))
	command_ok = 1;
    }
  
  payload = next_payload(ctx, &n);
  
  if (!command_ok || ((n > 0) && (payload[n - 1] != '\n')))
    {
      errno = EBADMSG;
      copy_errno(ctx);
      return NULL;
    }
  
  line = payload;
  end = payload + n;
  lines = length = 0;
  while (line != end)
    {
      lines += 1;
      length += len = (size_t)(strchr(line, '\n') + 1 - line);
      line += len;
      line[-1] = '\0';
    }
  
  rc = malloc((lines + 1) * sizeof(char*) + length);
  if (rc == NULL)
    {
      copy_errno(ctx);
      return NULL;
    }
  
  line = ((char*)rc) + (lines + 1) * sizeof(char*);
  memcpy(line, payload, length);
  rc[lines] = NULL;
  for (i = 0; i < lines; i++)
    {
      rc[i] = line;
      line = strchr(line, '\0') + 1;
    }
  
  return rc;
}


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
int libcoopgamma_get_gamma_info_send(const char* crtc, libcoopgamma_context_t* restrict ctx,
				     libcoopgamma_async_context_t* restrict async)
{
  if ((crtc == NULL) || strchr(crtc, '\n'))
    {
      errno = EINVAL;
      goto fail;
    }
  
  async->message_id = ctx->message_id;
  SEND_MESSAGE(ctx, NULL, 0,
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


/**
 * Retrieve information about a CRTC:s gamma ramps, receive response part
 * 
 * @param   info   Output parameter for the information, must be initialised
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int libcoopgamma_get_gamma_info_recv(libcoopgamma_crtc_info_t* restrict info,
				     libcoopgamma_context_t* restrict ctx,
				     libcoopgamma_async_context_t* restrict async)
{
  char temp[3 * sizeof(size_t) + 1];
  char* line;
  char* value;
  size_t _n;
  int have_cooperative = 0;
  int have_depth = 0;
  int have_red_size = 0;
  int have_green_size = 0;
  int have_blue_size = 0;
  int have_gamma_support = 0;
  int bad = 0, r = 0, g = 0;
  
  if (check_error(ctx, async))
    return -1;
  
  info->cooperative = 0; /* Should be in the response, but ... */
  
  for (;;)
    {
      line = next_header(ctx);
      value = strchr(line, ':') + 2;
      if (!*line)
	break;
      else if (strstr(line, "Cooperative: ") == line)
	{
	  have_cooperative = 1 + !!have_cooperative;
	  if      (!strcmp(value, "yes"))  info->cooperative = 1;
	  else if (!strcmp(value, "no"))   info->cooperative = 0;
	  else
	    bad = 1;
	}
      else if (strstr(line, "Depth: ") == line)
	{
	  have_depth = 1 + !!have_depth;
	  if      (!strcmp(value, "8"))   info->depth = LIBCOOPGAMMA_UINT8;
	  else if (!strcmp(value, "16"))  info->depth = LIBCOOPGAMMA_UINT16;
	  else if (!strcmp(value, "32"))  info->depth = LIBCOOPGAMMA_UINT32;
	  else if (!strcmp(value, "64"))  info->depth = LIBCOOPGAMMA_UINT64;
	  else if (!strcmp(value, "f"))   info->depth = LIBCOOPGAMMA_FLOAT;
	  else if (!strcmp(value, "d"))   info->depth = LIBCOOPGAMMA_DOUBLE;
	  else
	    bad = 1;
	}
      else if (strstr(line, "Gamma support: ") == line)
	{
	  have_gamma_support = 1 + !!have_gamma_support;
	  if      (!strcmp(value, "yes"))    info->supported = LIBCOOPGAMMA_YES;
	  else if (!strcmp(value, "no"))     info->supported = LIBCOOPGAMMA_NO;
	  else if (!strcmp(value, "maybe"))  info->supported = LIBCOOPGAMMA_MAYBE;
	  else
	    bad = 1;
	}
      else if (((r = (strstr(line, "Red size: ")   == line))) ||
	       ((g = (strstr(line, "Green size: ") == line))) ||
	             (strstr(line, "Blue size: ")  == line))
	{
	  size_t* out;
	  if (r)       have_red_size   = 1 + !!have_red_size,   out = &(info->red_size);
	  else if (g)  have_green_size = 1 + !!have_green_size, out = &(info->green_size);
	  else         have_blue_size  = 1 + !!have_blue_size,  out = &(info->blue_size);
	  *out = (size_t)atol(value);
	  sprintf(temp, "%zu", *out);
	  if (strcmp(value, temp))
	    bad = 1;
	}
    }
  
  (void) next_payload(ctx, &_n);
  
  if (bad || (have_gamma_support != 1))
    {
      errno = EBADMSG;
      copy_errno(ctx);
      return -1;
    }
  if (info->supported != LIBCOOPGAMMA_NO)
    if ((have_cooperative > 1) || (have_depth != 1) || (have_gamma_support != 1) ||
	(have_red_size != 1) || (have_green_size != 1) || (have_blue_size != 1))
      {
	errno = EBADMSG;
	copy_errno(ctx);
	return -1;
      }
  
  return 0;
}


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
int libcoopgamma_get_gamma_send(libcoopgamma_filter_query_t* restrict query,
				libcoopgamma_context_t* restrict ctx,
				libcoopgamma_async_context_t* restrict async)
{
  if ((query == NULL) || (query->crtc == NULL) || strchr(query->crtc, '\n'))
    {
      errno = EINVAL;
      goto fail;
    }
  
  async->message_id = ctx->message_id;
  async->coalesce = query->coalesce;
  SEND_MESSAGE(ctx, NULL, 0,
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


/**
 * Retrieve the current gamma ramp adjustments, receive response part
 * 
 * @param   table  Output for the response, must be initialised
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int libcoopgamma_get_gamma_recv(libcoopgamma_filter_table_t* restrict table,
				libcoopgamma_context_t* restrict ctx,
				libcoopgamma_async_context_t* restrict async)
{
  char temp[3 * sizeof(size_t) + 1];
  char* line;
  char* value;
  char* payload;
  size_t i, n, width, clutsize;
  int have_depth = 0;
  int have_red_size = 0;
  int have_green_size = 0;
  int have_blue_size = 0;
  int have_tables = 0;
  int bad = 0, r = 0, g = 0;
  
  if (check_error(ctx, async))
    return -1;
  
  libcoopgamma_filter_table_destroy(table);
  
  for (;;)
    {
      line = next_header(ctx);
      value = strchr(line, ':') + 2;
      if (!*line)
	break;
      else if (strstr(line, "Depth: ") == line)
	{
	  have_depth = 1 + !!have_depth;
	  if      (!strcmp(value, "8"))   table->depth = LIBCOOPGAMMA_UINT8;
	  else if (!strcmp(value, "16"))  table->depth = LIBCOOPGAMMA_UINT16;
	  else if (!strcmp(value, "32"))  table->depth = LIBCOOPGAMMA_UINT32;
	  else if (!strcmp(value, "64"))  table->depth = LIBCOOPGAMMA_UINT64;
	  else if (!strcmp(value, "f"))   table->depth = LIBCOOPGAMMA_FLOAT;
	  else if (!strcmp(value, "d"))   table->depth = LIBCOOPGAMMA_DOUBLE;
	  else
	    bad = 1;
	}
      else if (((r = (strstr(line, "Red size: ")   == line))) ||
	       ((g = (strstr(line, "Green size: ") == line))) ||
	             (strstr(line, "Blue size: ")  == line))
	{
	  size_t* out;
	  if (r)       have_red_size   = 1 + !!have_red_size,   out = &(table->red_size);
	  else if (g)  have_green_size = 1 + !!have_green_size, out = &(table->green_size);
	  else         have_blue_size  = 1 + !!have_blue_size,  out = &(table->blue_size);
	  *out = (size_t)atol(value);
	  sprintf(temp, "%zu", *out);
	  if (strcmp(value, temp))
	    bad = 1;
	}
      else if (strstr(line, "Tables: ") == line)
	{
	  have_tables = 1 + have_tables;
	  table->filter_count = (size_t)atol(value);
	  sprintf(temp, "%zu", table->filter_count);
	  if (strcmp(value, temp))
	    bad = 1;
	}
    }
  
  payload = next_payload(ctx, &n);
  
  if (bad || (have_depth != 1) || (have_red_size != 1) || (have_green_size != 1) ||
      (have_blue_size != 1) || (async->coalesce ? (have_tables > 1) : (have_tables == 0)) ||
      (payload == NULL))
    goto bad;
  
  switch (table->depth)
    {
    case LIBCOOPGAMMA_FLOAT:   width = sizeof(float);   break;
    case LIBCOOPGAMMA_DOUBLE:  width = sizeof(double);  break;
    default:
      width = table->depth / 8;
      break;
    }
  
  clutsize = table->red_size + table->green_size + table->blue_size;
  clutsize *= width;
  
  if (async->coalesce)
    {
      if (n != clutsize)
	goto bad;
      table->filters = malloc(sizeof(*(table->filters)));
      if (table->filters == NULL)
	goto fail;
      table->filters->priority = 0;
      table->filters->class = NULL;
      if ((libcoopgamma_ramps_initialise)(&(table->filters->ramps), width) < 0)
	goto fail;
      memcpy(table->filters->ramps.u8.red, payload, clutsize);
    }
  else
    {
      size_t off = 0, len;
      table->filters = calloc(table->filter_count, sizeof(*(table->filters)));
      if (table->filters == NULL)
	goto fail;
      for (i = 0; i < table->filter_count; i++)
	{
	  if (off + sizeof(int64_t) > n)
	    goto bad;
	  table->filters[i].priority = *(int64_t*)payload;
	  off += sizeof(int64_t);
	  if (memchr(payload + off, '\0', n - off) == NULL)
	    goto bad;
	  len = strlen(payload + off) + 1;
	  table->filters[i].class = malloc(len);
	  if (table->filters[i].class == NULL)
	    goto fail;
	  memcpy(table->filters[i].class, payload + off, len);
	  off += len;
	  if (off + clutsize > n)
	    goto bad;
	  if ((libcoopgamma_ramps_initialise)(&(table->filters[i].ramps), width) < 0)
	    goto fail;
	  memcpy(table->filters->ramps.u8.red, payload + off, clutsize);
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


/**
 * Apply, update, or remove a gamma ramp adjustment, send request part
 * 
 * Cannot be used before connecting to the server
 * 
 * @param   filter  The filter to apply, update, or remove, gamma ramp meta-data must match the CRTC's
 * @param   depth   The datatype for the stops in the gamma ramps, must match the CRTC's
 * @param   ctx     The state of the library, must be connected
 * @param   async   Information about the request, that is needed to
 *                  identify and parse the response, is stored here
 * @return          Zero on success, -1 on error
 */
int libcoopgamma_set_gamma_send(libcoopgamma_filter_t* restrict filter, libcoopgamma_depth_t depth,
				libcoopgamma_context_t* restrict ctx,
				libcoopgamma_async_context_t* restrict async)
{
  const void* payload = NULL;
  const char* lifespan;
  char priority[sizeof("Priority: \n") + 3 * sizeof(int64_t)] = {'\0'};
  char length  [sizeof("Length: \n")   + 3 * sizeof(size_t) ] = {'\0'};
  size_t payload_size = 0, stopwidth = 0;
  
  if ((filter == NULL) || (filter->crtc == NULL) || strchr(filter->crtc, '\n') ||
      (filter->class == NULL) || strchr(filter->class, '\n'))
    {
      errno = EINVAL;
      goto fail;
    }
  
  switch (filter->lifespan)
    {
    case LIBCOOPGAMMA_REMOVE:         lifespan = "remove";         break;
    case LIBCOOPGAMMA_UNTIL_DEATH:    lifespan = "until-death";    break;
    case LIBCOOPGAMMA_UNTIL_REMOVAL:  lifespan = "until-removal";  break;
    default:
      errno = EINVAL;
      goto fail;
    }
  
  switch (depth)
    {
    case LIBCOOPGAMMA_FLOAT:   stopwidth = sizeof(float);   break;
    case LIBCOOPGAMMA_DOUBLE:  stopwidth = sizeof(double);  break;
    default:
      if ((depth <= 0) || ((depth & 7) != 0))
	{
	  errno = EINVAL;
	  goto fail;
	}
      stopwidth = depth / 8;
      break;
    }
  
  if (filter->lifespan != LIBCOOPGAMMA_REMOVE)
    {
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
	       ctx->message_id, filter->crtc, filter->class, lifespan,
	       priority, length);
  
  return 0;
 fail:
  copy_errno(ctx);
  return -1;
}


/**
 * Apply, update, or remove a gamma ramp adjustment, receive response part
 * 
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int libcoopgamma_set_gamma_recv(libcoopgamma_context_t* restrict ctx,
				libcoopgamma_async_context_t* restrict async)
{
  size_t _n = 0;
  
  if (check_error(ctx, async))
    return -(ctx->error.custom || ctx->error.number);
  
  while (*next_header(ctx));
  (void) next_payload(ctx, &_n);
  
  errno = EBADMSG;
  copy_errno(ctx);
  return -1;
}


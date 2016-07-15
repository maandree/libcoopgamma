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
  ((ctx)->error.number = (uint64_t)errno,	\
   (ctx)->error.custom = 0,			\
   (ctx)->error.server_side = 0,		\
   free((ctx)->error.description),		\
   (ctx)->error.description = NULL)



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
  this->fd = -1;
  libcoopgamma_error_initialise(&(this->error));
  this->message_id = 0;
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
  UNMARSHAL_EPILOGUE;
}



/**
 * List all recognised adjustment method
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
 * @param   method  The adjustment method, `NULL` for automatic
 * @param   site    The site, `NULL` for automatic
 * @param   ctx     The state of the library, must be initialised
 * @return          Zero on success, -1 on error. On error, `errno` is set
 *                  to 0 if the server could not be initialised.
 */
int libcoopgamma_connect(const char* restrict method, const char* restrict site,
			 libcoopgamma_context_t* restrict ctx)
{
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
 * On and only on error, `*(resp)` is set to `NULL`
 */
#define communicate(respp, ctx, payload, payload_size, format, ...)		\
  do										\
    {										\
      ssize_t n__;								\
      char* msg__;								\
      snprintf(NULL, 0, format "%zn", __VA_ARGS__, &n__);			\
      msg__ = malloc((size_t)n__ + (payload_size));				\
      if (msg__ == NULL)							\
	{									\
	  copy_errno(ctx);							\
	  *(respp) = NULL;							\
	  break;								\
	}									\
      sprintf(msg__, format, __VA_ARGS__);					\
      if ((payload) != NULL)							\
	memcpy(msg__ + n__, (payload), (payload_size));				\
      *(respp) = (communicate)((ctx), msg__, (size_t)n__ + (payload_size));	\
    }										\
  while (0)


/**
 * Send a message to the server and wait for response
 * 
 * @param   ctx  The state of the library
 * @param   msg  The message to send
 * @param   n    The length of `msg`
 * @return       NUL-terminated response, `NULL` on error
 */
static char* (communicate)(libcoopgamma_context_t* restrict ctx, char* msg, size_t n)
{
}


/**
 * List all available CRTC:s
 * 
 * Cannot be used before connecting to the server
 * 
 * @param   ctx  The state of the library, must be connected
 * @return       A `NULL`-terminated list of names. You should only free
 *               the outer pointer, inner pointers are subpointers of the
 *               outer pointer and cannot be freed. `NULL` on error, in
 *               which case `ctx->error` (rather than `errno`) is read
 *               for information about the error.
 */
char** libcoopgamma_get_crtcs(libcoopgamma_context_t* restrict ctx)
{
  char* resp;
  
  communicate(&resp, ctx, NULL, 0,
	      "Command: enumerate-crtcs\n"
	      "Message ID: %" PRIu32 "\n"
	      "\n",
	      ctx->message_id);
}


/**
 * Retrieve information about a CRTC:s gamma ramps
 * 
 * Cannot be used before connecting to the serve
 * 
 * @param   crtc  The name of the CRTC
 * @param   info  Output parameter for the information, must be initialised
 * @param   ctx   The state of the library, must be connected
 * @return        Zero on success, -1 on error, in which case `ctx->error`
 *                (rather than `errno`) is read for information about the error
 */
int libcoopgamma_get_gamma_info(const char* crtc, libcoopgamma_crtc_info_t* restrict info,
				libcoopgamma_context_t* restrict ctx)
{
  char* resp;
  
  if ((crtc == NULL) || strchr(crtc, '\n'))
    {
      errno = EINVAL;
      goto fail;
    }
  
  communicate(&resp, ctx, NULL, 0,
	      "Command: get-gamma-info\n"
	      "Message ID: %" PRIu32 "\n"
	      "CRTC: %s\n"
	      "\n",
	      ctx->message_id, crtc);
  if (resp == NULL)
    return -1;
  
 fail:
  copy_errno(ctx);
  return -1;
}


/**
 * Retrieve the current gamma ramp adjustments
 * 
 * Cannot be used before connecting to the serve
 * 
 * @param   query  The query to send
 * @param   table  Output for the response, must be initialised
 * @param   ctx    The state of the library, must be connected
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int libcoopgamma_get_gamma(libcoopgamma_filter_query_t* restrict query,
			   libcoopgamma_filter_table_t* restrict table, libcoopgamma_context_t* restrict ctx)
{
  char* resp;
  
  if ((query == NULL) || (query->crtc == NULL) || strchr(query->crtc, '\n'))
    {
      errno = EINVAL;
      goto fail;
    }
  
  communicate(&resp, ctx, NULL, 0,
	      "Command: get-gamma\n"
	      "Message ID: %" PRIu32 "\n"
	      "CRTC: %s\n"
	      "Coalesce: %s\n"
	      "High priority: %" PRIi64 "\n"
	      "Low priority: %" PRIi64 "\n"
	      "\n",
	      ctx->message_id, query->crtc, query->coalesce ? "yes" : "no",
	      query->high_priority, query->low_priority);
  if (resp == NULL)
    return -1;
  
 fail:
  copy_errno(ctx);
  return -1;
}


/**
 * Apply, update, or remove a gamma ramp adjustment
 * 
 * @param   filter  The filter to apply, update, or remove, gamma ramp meta-data must match the CRTC's
 * @param   depth   The datatype for the stops in the gamma ramps, must match the CRTC's
 * @param   ctx     The state of the library, must be connected
 * @return          Zero on success, -1 on error, in which case `ctx->error`
 *                  (rather than `errno`) is read for information about the error
 */
int libcoopgamma_set_gamma(libcoopgamma_filter_t* restrict filter, libcoopgamma_depth_t depth,
			   libcoopgamma_context_t* restrict ctx)
{
  const void* payload = NULL;
  const char* lifespan;
  char priority[sizeof("Priority: \n") + 3 * sizeof(int64_t)] = {'\0'};
  char length  [sizeof("Length: \n")   + 3 * sizeof(size_t) ] = {'\0'};
  size_t payload_size = 0, stopwidth = 0;
  char* resp;
  
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
  
  communicate(&resp, ctx, payload, payload_size,
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
  if (resp == NULL)
    return -1;
  
 fail:
  copy_errno(ctx);
  return -1;
}


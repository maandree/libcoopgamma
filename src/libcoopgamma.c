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
#include <stdlib.h>
#include <string.h>
#include <unistd.h>



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


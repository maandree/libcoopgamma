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



/**
 * Unmarshal was successful
 */
#define LIBCOOPGAMMA_SUCCESS  0

/**
 * Unmarshal failed: the marshalled data was created
 * with a older version of libcoopgamma that does not
 * marshall the data in a compatible way
 */
#define LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE  1

/**
 * Unmarshal failed: the marshalled data was created with
 * a newer version libcoopgamma that does not marshall
 * the data in a compatible way
 */
#define LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE  2

/**
 * Unmarshal failed because of an error, `errno` has been set
 */
#define LIBCOOPGAMMA_ERRNO_SET  -1



/**
 * Number used to identify implementation
 * version of `libcoopgamma_support_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_SUPPORT_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_depth_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_DEPTH_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_lifespan_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_LIFESPAN_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_ramps*_t`, if they
 * are ever modified, this number is increased
 */
#define LIBCOOPGAMMA_RAMPS_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_filter_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_FILTER_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_ctrc_info_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_CRTC_INFO_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_filter_query_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_FILTER_QUERY_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_queried_filter_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_QUERIED_FILTER_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_filter_table_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_FILTER_TABLE_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_error_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_ERROR_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_context_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_CONTEXT_VERSION  0

/**
 * Number used to identify implementation
 * version of `libcoopgamma_async_context_t`, if it
 * is ever modified, this number is increased
 */
#define LIBCOOPGAMMA_ASYNC_CONTEXT_VERSION  0



/**
 * Values used to indicate the support
 * for gamma adjustments
 */
typedef enum libcoopgamma_support
{
  /**
   * Gamma adjustments are not supported
   */
  LIBCOOPGAMMA_NO = 0,
  
  /**
   * Don't know whether gamma
   ' adjustments are supported
   */
  LIBCOOPGAMMA_MAYBE = 1,
  
  /**
   * Gamma adjustments are supported
   */
  LIBCOOPGAMMA_YES = 2
  
} libcoopgamma_support_t;


/**
 * Values used to tell which datatype
 * is used for the gamma ramp stops
 */
typedef enum libcoopgamma_depth
{
  /**
   * `uint8_t`
   */
  LIBCOOPGAMMA_UINT8 = 8,
  
  /**
   * `uint16_t`
   */
  LIBCOOPGAMMA_UINT16 = 16,
  
  /**
   * `uint32_t`
   */
  LIBCOOPGAMMA_UINT32 = 32,
  
  /**
   * `uint64_t`
   */
  LIBCOOPGAMMA_UINT64 = 64,
  
  /**
   * `float`
   */
  LIBCOOPGAMMA_FLOAT = -1,
  
  /**
   * `double`
   */
  LIBCOOPGAMMA_DOUBLE = -2
  
} libcoopgamma_depth_t;


/**
 * Values used to tell when a filter
 * should be removed
 */
typedef enum libcoopgamma_lifespan
{
  /**
   * Remove the filter now
   */
  LIBCOOPGAMMA_REMOVE = 0,
  
  /**
   * Remove the filter when disconnecting
   * from the coopgamma server
   */
  LIBCOOPGAMMA_UNTIL_DEATH = 1,
  
  /**
   * Only remove the filter when it
   * is explicitly requested
   */
  LIBCOOPGAMMA_UNTIL_REMOVAL = 2
  
} libcoopgamma_lifespan_t;


/**
 * Define a gamma ramp structure
 * 
 * @param  suffix:identifier  The end of the name of the `struct`
 * @param  type:scalar-type   The datatype of the stops
 */
#define LIBCOOPGAMMA_RAMPS__(suffix, type)	\
typedef struct libcoopgamma_ramps##suffix	\
{						\
  /**
   * The number of stops in the red ramp
   */						\
  size_t red_size;				\
						\
  /**
   * The number of stops in the green ramp
   */						\
  size_t green_size;				\
						\
  /**
   * The number of stops in the blue ramp
   */						\
  size_t blue_size;				\
						\
  /**
   * The red ramp
   */						\
  type* red;					\
						\
  /**
   * The green ramp
   */						\
  type* green;					\
						\
  /**
   * The blue ramp
   */						\
  type* blue;					\
						\
} libcoopgamma_ramps##suffix##_t;

/**
 * `typedef struct libcoopgamma_ramps8 libcoopgamma_ramps8_t`
 * 
 * Gamma ramp structure with `uint8_t` stops
 */
LIBCOOPGAMMA_RAMPS__(8, uint8_t);

/**
 * `typedef struct libcoopgamma_ramps16 libcoopgamma_ramps16_t`
 * 
 * Gamma ramp structure with `uint16_t` stops
 */
LIBCOOPGAMMA_RAMPS__(16, uint16_t);

/**
 * `typedef struct libcoopgamma_ramps32 libcoopgamma_ramps32_t`
 * 
 * Gamma ramp structure with `uint32_t` stops
 */
LIBCOOPGAMMA_RAMPS__(32, uint32_t);

/**
 * `typedef struct libcoopgamma_ramps64 libcoopgamma_ramps64_t`
 * 
 * Gamma ramp structure with `uint64_t` stops
 */
LIBCOOPGAMMA_RAMPS__(64, uint64_t);

/**
 * `typedef struct libcoopgamma_rampsf libcoopgamma_rampsf_t`
 * 
 * Gamma ramp structure with `float` stops
 */
LIBCOOPGAMMA_RAMPS__(f, float);

/**
 * `typedef struct libcoopgamma_rampsd libcoopgamma_rampsd_t`
 * 
 * Gamma ramp structure with `double` stops
 */
LIBCOOPGAMMA_RAMPS__(d, double);


/**
 * Data set to the coopgamma server to apply,
 * update, or remove a filter.
 */
typedef struct libcoopgamma_filter
{
  /**
   * The data type and bit-depth of the ramp stops
   */
  libcoopgamma_depth_t depth;
  
  /**
   * The priority of the filter, higher priority
   * is applied first. The gamma correction should
   * have priority 0.
   */
  int64_t priority;
  
  /**
   * The CRTC for which this filter shall be applied
   */
  char* crtc;
  
  /**
   * Identifier for the filter
   * 
   * The syntax must be "${PACKAGE_NAME}::${COMMAND_NAME}::${RULE}"
   */
  char* class;
  
  /**
   * When shall the filter be removed?
   * 
   * If this member's value is `LIBCOOPGAMMA_REMOVE`,
   * only `.crtc` and `.class` need also be defined
   */
  libcoopgamma_lifespan_t lifespan;
  
  /**
   * The gamma ramp adjustments of the filter
   */
  union
  {
    /**
     * 8-bit version
     */
    libcoopgamma_ramps8_t u8;
    
    /**
     * 16-bit version
     */
    libcoopgamma_ramps16_t u16;
    
    /**
     * 32-bit version
     */
    libcoopgamma_ramps32_t u32;
    
    /**
     * 64-bit version
     */
    libcoopgamma_ramps64_t u64;
    
    /**
     * Single precision floating-point version
     */
    libcoopgamma_rampsf_t f;
    
    /**
     * Double precision floating-point version
     */
    libcoopgamma_rampsd_t d;
    
  } ramps;
  
} libcoopgamma_filter_t;


/**
 * Gamma ramp meta information for a CRTC
 */
typedef struct libcoopgamma_crtc_info
{
  /**
   * Cooperative gamma server is running
   */
  int cooperative;
  
  /**
   * The data type and bit-depth of the ramp stops
   */
  libcoopgamma_depth_t depth;
  
  /**
   * The number of stops in the red ramp
   */
  size_t red_size;
  
  /**
   * The number of stops in the green ramp
   */
  size_t green_size;
  
  /**
   * The number of stops in the blue ramp
   */
  size_t blue_size;
  
  /**
   * Is gamma adjustments supported on the CRTC?
   * If not, `.depth`, `.red_size`, `.green_size`,
   * and `.blue_size` are undefined
   */
  libcoopgamma_support_t supported;
  
} libcoopgamma_crtc_info_t;


/**
 * Data sent to the coopgamma server
 * when requestng the current filter
 * table
 */
typedef struct libcoopgamma_filter_query
{
  /**
   * The CRTC for which the the current
   * filters shall returned
   */
  char* crtc;
  
  /**
   * Whether to coalesce all filters
   * into one gamma ramp triplet
   */
  int coalesce;
  
  /**
   * Do no return filters with higher
   * priority than this value
   */
  int64_t high_priority;
  
  /**
   * Do no return filters with lower
   * priority than this value
   */
  int64_t low_priority;
  
} libcoopgamma_filter_query_t;


/**
 * Stripped down version of `libcoopgamma_filter`
 * which only contains the information returned
 * in response to "Command: get-gamma"
 */
typedef struct libcoopgamma_queried_filter
{
  /**
   * The filter's priority
   */
  int64_t priority;
  
  /**
   * The filter's class
   */
  char* class;
  
  /**
   * The gamma ramp adjustments of the filter
   */
  union
  {
    /**
     * 8-bit version
     */
    libcoopgamma_ramps8_t u8;
    
    /**
     * 16-bit version
     */
    libcoopgamma_ramps16_t u16;
    
    /**
     * 32-bit version
     */
    libcoopgamma_ramps32_t u32;
    
    /**
     * 64-bit version
     */
    libcoopgamma_ramps64_t u64;
    
    /**
     * Single precision floating-point version
     */
    libcoopgamma_rampsf_t f;
    
    /**
     * Double precision floating-point version
     */
    libcoopgamma_rampsd_t d;
    
  } ramps;
  
} libcoopgamma_queried_filter_t;


/**
 * Response type for "Command: get-gamma": a
 * list of applied filters and meta-information
 * that was necessary for decoding the response
 */
typedef struct libcoopgamma_filter_table
{
  /**
   * The data type and bit-depth of the ramp stops
   */
  libcoopgamma_depth_t depth;
  
  /**
   * The number of stops in the red ramp
   */
  size_t red_size;
  
  /**
   * The number of stops in the green ramp
   */
  size_t green_size;
  
  /**
   * The number of stops in the blue ramp
   */
  size_t blue_size;
  
  /**
   * The number of filters
   */
  size_t filter_count;
  
  /**
   * The filters, should be ordered by priority
   * in descending order, lest there is something
   * wrong with the coopgamma server
   * 
   * If filter coalition was requested, there will
   * be exactly one filter (`.filter_count == 1`)
   * and `.filters->class == NULL` and
   * `.filters->priority` is undefined.
   */
  libcoopgamma_queried_filter_t* filters;
  
} libcoopgamma_filter_table_t;


/**
 * Error message from coopgamma server
 */
typedef struct libcoopgamma_error
{
  /**
   * Error code number
   * 
   * If `.custom` is false, 0 indicates
   * success, otherwise, 0 indicates that
   * no error number has been assigned
   */
  uint64_t number;
  
  /**
   * Is this a custom error?
   */
  int custom;
  
  /**
   * Did the error occur on the server-side?
   */
  int server_side;
  
  /**
   * Error message, can be `NULL` if
   * `.custom` is false
   */
  char* description;
  
} libcoopgamma_error_t;


/**
 * Library state
 * 
 * Use of this structure is not thread-safe,
 * create one instance per thread that uses
 * this structure
 */
typedef struct libcoopgamma_context
{
  /**
   * File descriptor for the socket
   */
  int fd;
  
  /**
   * The error of the last failed function call
   * 
   * This member is undefined after successful function call
   */
  libcoopgamma_error_t error;
  
  /**
   * Message ID of the next message
   */
  uint32_t message_id;
  
  /**
   * Buffer with the outbound message
   */
  char* outbound;
  
  /**
   * The write head for `outbound`
   */
  size_t outbound_head;
  
  /**
   * The read head for `outbound`
   */
  size_t outbound_tail;
  
  /**
   * The allocation size of `outbound`
   */
  size_t outbound_size;
  
  /**
   * Buffer with the inbound message
   */
  char* inbound;
  
  /**
   * The write head for `inbound`
   */
  size_t inbound_head;
  
  /**
   * The read head for `inbound`
   */
  size_t inbound_tail;
  
  /**
   * The allocation size of `inbound`
   */
  size_t inbound_size;
  
  /**
   * The value of 'Length' header in
   * the inbound message
   */
  size_t length;
  
} libcoopgamma_context_t;


/**
 * Information necessary to identify and
 * parse a response from the server
 */
typedef struct libcoopgamma_async_context
{
  /**
   * The value of the 'In response to' header
   * in the waited message
   */
  uint32_t message_id;
  
  /**
   * Whether to coalesce all filters
   * into one gamma ramp triplet
   */
  int coalesce;
  
} libcoopgamma_async_context_t;



/**
 * Initialise a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t`
 * 
 * `this->red_size`, `this->green_size`, and `this->blue_size` must already be set
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
#define libcoopgamma_ramps_initialise(this)  \
  ((libcoopgamma_ramps_initialise)((this), sizeof(*((this)->red))))

/**
 * Marshal a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   buf   The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
#define libcoopgamma_ramps_marshal(this, buf)  \
  ((libcoopgamma_ramps_marshal)((this), (buf), sizeof(*((this)->red))))

/**
 * Unmarshal a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   buf   The buffer with the marshalled record
 * @param   n     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
#define libcoopgamma_ramps_unmarshal(this, buf, n)  \
  ((libcoopgamma_ramps_unmarshal)((this), (buf), (n), sizeof(*((this)->red))))


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
int (libcoopgamma_ramps_initialise)(void* restrict, size_t);

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
void libcoopgamma_ramps_destroy(void* restrict);

/**
 * Marshal a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t` into a buffer
 * 
 * @param   this   The record to marshal
 * @param   buf    The output buffer, `NULL` to only measure
 *                 how large this buffer has to be
 * @param   width  The `sizeof(*(this->red))`
 * @return         The number of marshalled bytes, or if `buf == NULL`,
 *                 how many bytes would be marshalled if `buf != NULL`
 */
size_t (libcoopgamma_ramps_marshal)(const void* restrict, void* restrict, size_t);

/**
 * Unmarshal a `libcoopgamma_ramps8_t`, `libcoopgamma_ramps16_t`, `libcoopgamma_ramps32_t`,
 * `libcoopgamma_ramps64_t`, `libcoopgamma_rampsf_t`, or `libcoopgamma_rampsd_t` from a buffer
 * 
 * @param   this   The output paramater for unmarshalled record
 * @param   buf    The buffer with the marshalled record
 * @param   n      Output parameter for the number of unmarshalled bytes, undefined on failure
 * @param   width  The `sizeof(*(this->red))`
 * @return         `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                 `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int (libcoopgamma_ramps_unmarshal)(void* restrict, const void* restrict, size_t* restrict, size_t);


/**
 * Initialise a `libcoopgamma_filter_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_filter_initialise(libcoopgamma_filter_t* restrict);

/**
 * Release all resources allocated to  a `libcoopgamma_filter_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_filter_initialise`
 * or failed call to `libcoopgamma_filter_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_filter_destroy(libcoopgamma_filter_t* restrict);

/**
 * Marshal a `libcoopgamma_filter_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   buf   The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_filter_marshal(const libcoopgamma_filter_t* restrict, void* restrict);

/**
 * Unmarshal a `libcoopgamma_filter_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   buf   The buffer with the marshalled record
 * @param   n     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_filter_unmarshal(libcoopgamma_filter_t* restrict, const void* restrict, size_t* restrict);


/**
 * Initialise a `libcoopgamma_crtc_info_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_crtc_info_initialise(libcoopgamma_crtc_info_t* restrict);

/**
 * Release all resources allocated to  a `libcoopgamma_crtc_info_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_crtc_info_initialise`
 * or failed call to `libcoopgamma_crtc_info_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_crtc_info_destroy(libcoopgamma_crtc_info_t* restrict);

/**
 * Marshal a `libcoopgamma_crtc_info_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   buf   The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_crtc_info_marshal(const libcoopgamma_crtc_info_t* restrict, void* restrict);

/**
 * Unmarshal a `libcoopgamma_crtc_info_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   buf   The buffer with the marshalled record
 * @param   n     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_crtc_info_unmarshal(libcoopgamma_crtc_info_t* restrict,
				     const void* restrict, size_t* restrict);


/**
 * Initialise a `libcoopgamma_filter_query_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_filter_query_initialise(libcoopgamma_filter_query_t* restrict);

/**
 * Release all resources allocated to  a `libcoopgamma_filter_query_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_filter_query_initialise`
 * or failed call to `libcoopgamma_filter_query_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_filter_query_destroy(libcoopgamma_filter_query_t* restrict);

/**
 * Marshal a `libcoopgamma_filter_query_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   buf   The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_filter_query_marshal(const libcoopgamma_filter_query_t* restrict, void* restrict);

/**
 * Unmarshal a `libcoopgamma_filter_query_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   buf   The buffer with the marshalled record
 * @param   n     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_filter_query_unmarshal(libcoopgamma_filter_query_t* restrict,
					const void* restrict, size_t* restrict);


/**
 * Initialise a `libcoopgamma_queried_filter_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_queried_filter_initialise(libcoopgamma_queried_filter_t* restrict);

/**
 * Release all resources allocated to  a `libcoopgamma_queried_filter_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_queried_filter_initialise`
 * or failed call to `libcoopgamma_queried_filter_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_queried_filter_destroy(libcoopgamma_queried_filter_t* restrict);

/**
 * Marshal a `libcoopgamma_queried_filter_t` into a buffer
 * 
 * @param   this   The record to marshal
 * @param   buf    The output buffer, `NULL` to only measure
 *                 how large this buffer has to be
 * @param   depth  The type used of ramp stops
 * @return         The number of marshalled bytes, or if `buf == NULL`,
 *                 how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_queried_filter_marshal(const libcoopgamma_queried_filter_t* restrict, void* restrict,
					   libcoopgamma_depth_t);

/**
 * Unmarshal a `libcoopgamma_queried_filter_t` from a buffer
 * 
 * @param   this   The output paramater for unmarshalled record
 * @param   buf    The buffer with the marshalled record
 * @param   n      Output parameter for the number of unmarshalled bytes, undefined on failure
 * @param   depth  The type used of ramp stops
 * @return         `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                 `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_queried_filter_unmarshal(libcoopgamma_queried_filter_t* restrict,
					  const void* restrict, size_t* restrict, libcoopgamma_depth_t);


/**
 * Initialise a `libcoopgamma_filter_table_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_filter_table_initialise(libcoopgamma_filter_table_t* restrict);

/**
 * Release all resources allocated to  a `libcoopgamma_filter_table_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_filter_table_initialise`
 * or failed call to `libcoopgamma_filter_table_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_filter_table_destroy(libcoopgamma_filter_table_t* restrict);

/**
 * Marshal a `libcoopgamma_filter_table_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   buf   The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_filter_table_marshal(const libcoopgamma_filter_table_t* restrict, void* restrict);

/**
 * Unmarshal a `libcoopgamma_filter_table_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   buf   The buffer with the marshalled record
 * @param   n     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_filter_table_unmarshal(libcoopgamma_filter_table_t* restrict,
					const void* restrict, size_t* restrict);


/**
 * Initialise a `libcoopgamma_error_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_error_initialise(libcoopgamma_error_t* restrict);

/**
 * Release all resources allocated to  a `libcoopgamma_error_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_error_initialise`
 * or failed call to `libcoopgamma_error_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_error_destroy(libcoopgamma_error_t* restrict);

/**
 * Marshal a `libcoopgamma_error_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   buf   The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_error_marshal(const libcoopgamma_error_t* restrict, void* restrict);

/**
 * Unmarshal a `libcoopgamma_error_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   buf   The buffer with the marshalled record
 * @param   n     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_error_unmarshal(libcoopgamma_error_t* restrict, const void* restrict, size_t* restrict);


/**
 * Initialise a `libcoopgamma_context_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_context_initialise(libcoopgamma_context_t* restrict);

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
void libcoopgamma_context_destroy(libcoopgamma_context_t* restrict, int);

/**
 * Marshal a `libcoopgamma_context_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   buf   The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_context_marshal(const libcoopgamma_context_t* restrict, void* restrict);

/**
 * Unmarshal a `libcoopgamma_context_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   buf   The buffer with the marshalled record
 * @param   n     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_context_unmarshal(libcoopgamma_context_t* restrict, const void* restrict, size_t* restrict);


/**
 * Initialise a `libcoopgamma_async_context_t`
 * 
 * @param   this  The record to initialise
 * @return        Zero on success, -1 on error
 */
int libcoopgamma_async_context_initialise(libcoopgamma_async_context_t* restrict);

/**
 * Release all resources allocated to  a `libcoopgamma_async_context_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_async_context_initialise`
 * or failed call to `libcoopgamma_async_context_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void libcoopgamma_async_context_destroy(libcoopgamma_async_context_t* restrict);

/**
 * Marshal a `libcoopgamma_async_context_t` into a buffer
 * 
 * @param   this  The record to marshal
 * @param   buf   The output buffer, `NULL` to only measure
 *                how large this buffer has to be
 * @return        The number of marshalled bytes, or if `buf == NULL`,
 *                how many bytes would be marshalled if `buf != NULL`
 */
size_t libcoopgamma_async_context_marshal(const libcoopgamma_async_context_t* restrict, void* restrict);

/**
 * Unmarshal a `libcoopgamma_async_context_t` from a buffer
 * 
 * @param   this  The output paramater for unmarshalled record
 * @param   buf   The buffer with the marshalled record
 * @param   n     Output parameter for the number of unmarshalled bytes, undefined on failure
 * @return        `LIBCOOPGAMMA_SUCCESS` (0), `LIBCOOPGAMMA_INCOMPATIBLE_DOWNGRADE`,
 *                `LIBCOOPGAMMA_INCOMPATIBLE_UPGRADE`, or `LIBCOOPGAMMA_ERRNO_SET`
 */
int libcoopgamma_async_context_unmarshal(libcoopgamma_async_context_t* restrict, const void* restrict,
					 size_t* restrict);


/**
 * List all recognised adjustment method
 * 
 * SIGCHLD must not be ignored or blocked
 * 
 * @return  A `NULL`-terminated list of names. You should only free
 *          the outer pointer, inner pointers are subpointers of the
 *          outer pointer and cannot be freed. `NULL` on error.
 */
char** libcoopgamma_get_methods(void);

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
int libcoopgamma_get_method_and_site(const char* restrict, const char* restrict,
				     char** restrict, char** restrict);

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
char* libcoopgamma_get_pid_file(const char* restrict, const char* restrict);

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
char* libcoopgamma_get_socket_file(const char* restrict, const char* restrict);


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
int libcoopgamma_connect(const char* restrict, const char* restrict, libcoopgamma_context_t* restrict);

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
int libcoopgamma_set_nonblocking(libcoopgamma_context_t* restrict, int);

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
int libcoopgamma_flush(libcoopgamma_context_t* restrict);

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
 *                    is not listed
 */
int libcoopgamma_synchronise(libcoopgamma_context_t* restrict, libcoopgamma_async_context_t* restrict,
			     size_t, size_t* restrict);


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
int libcoopgamma_get_crtcs_send(libcoopgamma_context_t* restrict, libcoopgamma_async_context_t* restrict);

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
char** libcoopgamma_get_crtcs_recv(libcoopgamma_context_t* restrict, libcoopgamma_async_context_t* restrict);

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
int libcoopgamma_get_gamma_info_send(const char*, libcoopgamma_context_t* restrict,
				     libcoopgamma_async_context_t* restrict);

/**
 * Retrieve information about a CRTC:s gamma ramps, receive response part
 * 
 * @param   info   Output parameter for the information, must be initialised
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int libcoopgamma_get_gamma_info_recv(libcoopgamma_crtc_info_t* restrict, libcoopgamma_context_t* restrict,
				     libcoopgamma_async_context_t* restrict);

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
int libcoopgamma_get_gamma_send(libcoopgamma_filter_query_t* restrict, libcoopgamma_context_t* restrict,
				libcoopgamma_async_context_t* restrict);

/**
 * Retrieve the current gamma ramp adjustments, receive response part
 * 
 * @param   table  Output for the response, must be initialised
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int libcoopgamma_get_gamma_recv(libcoopgamma_filter_table_t* restrict, libcoopgamma_context_t* restrict,
				libcoopgamma_async_context_t* restrict);

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
int libcoopgamma_set_gamma_send(libcoopgamma_filter_t* restrict, libcoopgamma_depth_t,
				libcoopgamma_context_t* restrict, libcoopgamma_async_context_t* restrict);


/**
 * Apply, update, or remove a gamma ramp adjustment, receive response part
 * 
 * @param   ctx    The state of the library, must be connected
 * @param   async  Information about the request
 * @return         Zero on success, -1 on error, in which case `ctx->error`
 *                 (rather than `errno`) is read for information about the error
 */
int libcoopgamma_set_gamma_recv(libcoopgamma_context_t* restrict, libcoopgamma_async_context_t* restrict);



#endif


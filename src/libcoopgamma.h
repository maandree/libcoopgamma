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
LIBCOOPGAMMA_RAMPS__(8,  uint8_t);

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
LIBCOOPGAMMA_RAMPS__(f,  float);

/**
 * `typedef struct libcoopgamma_rampsd libcoopgamma_rampsd_t`
 * 
 * Gamma ramp structure with `double` stops
 */
LIBCOOPGAMMA_RAMPS__(d,  double);


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
   * Error message, can be `NULL` if
   * `.custom` is false
   */
  char* description;
  
} libcoopgamma_error_t;


/**
 * Library state
 */
typedef struct libcoopgamma_context
{
  /**
   * File descriptor for the socket
   */
  int fd;
  
} libcoopgamma_context_t;



#endif


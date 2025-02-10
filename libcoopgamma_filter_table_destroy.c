/* See LICENSE file for copyright and license details. */
#include "common.h"


/**
 * Release all resources allocated to  a `libcoopgamma_filter_table_t`,
 * the allocation of the record itself is not freed
 * 
 * Always call this function after failed call to `libcoopgamma_filter_table_initialise`
 * or failed call to `libcoopgamma_filter_table_unmarshal`
 * 
 * @param  this  The record to destroy
 */
void
libcoopgamma_filter_table_destroy(libcoopgamma_filter_table_t *restrict this)
{
	while (this->filter_count)
		libcoopgamma_queried_filter_destroy(this->filters + --this->filter_count);
	free(this->filters);
	this->filters = NULL;
}

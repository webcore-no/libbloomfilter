#ifndef BLOOMFILTER_H_
#define BLOOMFILTER_H_

#include <aio.h>

typedef struct bloomfilter_s bloomfilter_t;

typedef void *(*bloomfilter_allocator)(size_t);
typedef void (*bloomfilter_deallocator)(void *, size_t);

// Swap Filter

// Contruction
bloomfilter_t *bloomfilter_new(bloomfilter_allocator allocators);
void bloomfilter_destroy(bloomfilter_t **filter,
			 bloomfilter_deallocator deallocators);
//
void bloomfilter_clear(bloomfilter_t *filter);
void bloomfilter_add(bloomfilter_t *filter, const void *key, size_t keylen);
int bloomfilter_test(bloomfilter_t *filter, const void *key, size_t keylen);

// ------ Allocators --------
// Normal
void *bloomfilter_alloc(size_t);
void bloomfilter_free(void *filter, size_t);
// SHM
void *bloomfilter_shm_alloc(size_t);
void bloomfilter_shm_free(void *filter, size_t);

// ------- Swaping bloomfilter -----------
typedef struct bloomfilter_swap_s bloomfilter_swap_t;

bloomfilter_swap_t *bloomfilterswap_new(bloomfilter_allocator allocator);
void bloomfilterswap_destroy(bloomfilter_swap_t **swap,
			     bloomfilter_deallocator deallocator);

// swap active and passive filter
void bloomfilterswap_swap(bloomfilter_swap_t *filter);
// clear passive filter
void bloomfilterswap_clear(bloomfilter_swap_t *filter);

void bloomfilterswap_add(bloomfilter_swap_t *filter, const void *key,
			 size_t keylen);
int bloomfilterswap_test(bloomfilter_swap_t *filter, const void *key,
			 size_t keylen);

#endif
